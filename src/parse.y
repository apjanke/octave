/*

Copyright (C) 1996, 1997 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// Parser for Octave.

// C decarations.

%{
#define YYDEBUG 1

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cassert>
#include <cstdio>

#ifdef YYBYACC
#include <cstdlib>
#endif

#include <strstream.h>

#include "Matrix.h"
#include "cmd-edit.h"
#include "cmd-hist.h"
#include "file-ops.h"
#include "file-stat.h"

#include "defun.h"
#include "dynamic-ld.h"
#include "error.h"
#include "input.h"
#include "lex.h"
#include "oct-hist.h"
#include "ov-usr-fcn.h"
#include "toplev.h"
#include "pager.h"
#include "parse.h"
#include "pt-all.h"
#include "symtab.h"
#include "token.h"
#include "unwind-prot.h"
#include "utils.h"
#include "variables.h"

// TRUE means we print 
static bool Vdefault_eval_print_flag = true;

// If TRUE, generate a warning for the assignment in things like
//
//   octave> if (a = 2 < n)
//
// but not
//
//   octave> if ((a = 2) < n)
//
static bool Vwarn_assign_as_truth_value;

// If TRUE, generate a warning for variable swich labels.
static bool Vwarn_variable_switch_label;

// If TRUE, generate warning if declared function name disagrees with
// the name of the file in which it is defined.
static bool Vwarn_function_name_clash;

// TRUE means warn about function files that have time stamps in the future.
bool Vwarn_future_time_stamp;

// If TRUE, generate warning if a statement in a function is not
// terminated with a semicolon.  Useful for checking functions that
// should only produce output using explicit printing statements.
static bool Vwarn_missing_semicolon;

// Temporary symbol table pointer used to cope with bogus function syntax.
symbol_table *tmp_local_sym_tab = 0;

// The current input line number.
int input_line_number = 0;

// The column of the current token.
int current_input_column = 1;

// Buffer for help text snagged from function files.
string help_buf;

// TRUE means we are using readline.
// (--no-line-editing)
bool line_editing = true;

// TRUE means we printed messages about reading startup files.
bool reading_startup_message_printed = false;

// TRUE means input is coming from startup file.
bool input_from_startup_file = false;

// TRUE means that input is coming from a file that was named on
// the command line.
bool input_from_command_line_file = true;

// Forward declarations for some functions defined at the bottom of
// the file.

// Generic error messages.
static void
yyerror (const char *s);

// Error mesages for mismatched end tokens.
static void
end_error (const char *type, token::end_tok_type ettype, int l, int c);

// Check to see that end tokens are properly matched.
static bool
end_token_ok (token *tok, token::end_tok_type expected);

// Maybe print a warning if an assignment expression is used as the
// test in a logical expression.
static void
maybe_warn_assign_as_truth_value (tree_expression *expr);

// Maybe print a warning about switch labels that aren't constants.
static void
maybe_warn_variable_switch_label (tree_expression *expr);

// Create a plot command.
static tree_plot_command *
make_plot_command (token *tok, plot_limits *range, subplot_list *list);

// Finish building a range.
static tree_expression *
finish_colon_expression (tree_colon_expression *e);

// Build a constant.
static tree_constant *
make_constant (int op, token *tok_val);

// Build a binary expression.
static tree_expression *
make_binary_op (int op, tree_expression *op1, token *tok_val,
		tree_expression *op2);

// Build a boolean expression.
static tree_expression *
make_boolean_op (int op, tree_expression *op1, token *tok_val,
		 tree_expression *op2);

// Build a prefix expression.
static tree_expression *
make_prefix_op (int op, tree_expression *op1, token *tok_val);

// Build a postfix expression.
static tree_expression *
make_postfix_op (int op, tree_expression *op1, token *tok_val);

// Build an unwind-protect command.
static tree_command *
make_unwind_command (token *unwind_tok, tree_statement_list *body,
		     tree_statement_list *cleanup, token *end_tok);

// Build a try-catch command.
static tree_command *
make_try_command (token *try_tok, tree_statement_list *body,
		  tree_statement_list *cleanup, token *end_tok);

// Build a while command.
static tree_command *
make_while_command (token *while_tok, tree_expression *expr,
		    tree_statement_list *body, token *end_tok);

// Build a for command.
static tree_command *
make_for_command (token *for_tok, tree_argument_list *lhs,
		  tree_expression *expr, tree_statement_list *body,
		  token *end_tok);

// Build a break command.
static tree_command *
make_break_command (token *break_tok);

// Build a continue command.
static tree_command *
make_continue_command (token *continue_tok);

// Build a return command.
static tree_command *
make_return_command (token *return_tok);

// Start an if command.
static tree_if_command_list *
start_if_command (tree_expression *expr, tree_statement_list *list);

// Finish an if command.
static tree_if_command *
finish_if_command (token *if_tok, tree_if_command_list *list, token *end_tok);

// Build an elseif clause.
static tree_if_clause *
make_elseif_clause (tree_expression *expr, tree_statement_list *list);

// Finish a switch command.
static tree_switch_command *
finish_switch_command (token *switch_tok, tree_expression *expr,
		       tree_switch_case_list *list, token *end_tok);

// Build a switch case.
static tree_switch_case *
make_switch_case (tree_expression *expr, tree_statement_list *list);

// Build an assignment to a variable.
static tree_expression *
make_assign_op (int op, tree_argument_list *lhs, token *eq_tok,
		tree_expression *rhs);

// Begin defining a function.
static octave_user_function *
start_function (tree_parameter_list *param_list, tree_statement_list *body);

// Do most of the work for defining a function.
static octave_user_function *
frob_function (tree_identifier *id, octave_user_function *fcn);

// Finish defining a function.
static octave_user_function *
finish_function (tree_identifier *id, octave_user_function *fcn);

// Finish defining a function a different way.
static octave_user_function *
finish_function (tree_parameter_list *ret_list, octave_user_function *fcn);

// Reset state after parsing function.
static void
recover_from_parsing_function (void);

// Make an index expression.
static tree_index_expression *
make_index_expression (tree_expression *expr, tree_argument_list *args);

// Make an indirect reference expression.
static tree_indirect_ref *
make_indirect_ref (tree_expression *expr, const string&);

// Make a declaration command.
static tree_decl_command *
make_decl_command (int tok, token *tok_val, tree_decl_init_list *lst);

// Finish building a matrix list.
static tree_expression *
finish_matrix (tree_matrix *m);

// Maybe print a warning.  Duh.
static void
maybe_warn_missing_semi (tree_statement_list *);

// Set the print flag for a statement based on the separator type.
static void
set_stmt_print_flag (tree_statement_list *, char, bool);

#define ABORT_PARSE \
  do \
    { \
      global_command = 0; \
      yyerrok; \
      if (interactive || forced_interactive) \
	YYACCEPT; \
      else \
	YYABORT; \
    } \
  while (0)

%}

// Bison declarations.

%union
{
  // The type of the basic tokens returned by the lexer.
  token *tok_val;

  // Types for the nonterminals we generate.
  char sep_type;
  tree *tree_type;
  tree_matrix *tree_matrix_type;
  tree_expression *tree_expression_type;
  tree_constant *tree_constant_type;
  tree_identifier *tree_identifier_type;
  tree_index_expression *tree_index_expression_type;
  tree_colon_expression *tree_colon_expression_type;
  tree_argument_list *tree_argument_list_type;
  tree_parameter_list *tree_parameter_list_type;
  tree_command *tree_command_type;
  tree_if_command *tree_if_command_type;
  tree_if_clause *tree_if_clause_type;
  tree_if_command_list *tree_if_command_list_type;
  tree_switch_command *tree_switch_command_type;
  tree_switch_case *tree_switch_case_type;
  tree_switch_case_list *tree_switch_case_list_type;
  tree_decl_elt *tree_decl_elt_type;
  tree_decl_init_list *tree_decl_init_list_type;
  tree_decl_command *tree_decl_command_type;
  tree_statement *tree_statement_type;
  tree_statement_list *tree_statement_list_type;
  tree_plot_command *tree_plot_command_type;
  subplot *subplot_type;
  subplot_list *subplot_list_type;
  plot_limits *plot_limits_type;
  plot_range *plot_range_type;
  subplot_using *subplot_using_type;
  subplot_style *subplot_style_type;
  subplot_axes *subplot_axes_type;
  octave_user_function *octave_user_function_type;
}

// Tokens with line and column information.
%token <tok_val> '=' ':' '-' '+' '*' '/'
%token <tok_val> ADD_EQ SUB_EQ MUL_EQ DIV_EQ LEFTDIV_EQ 
%token <tok_val> EMUL_EQ EDIV_EQ ELEFTDIV_EQ AND_EQ OR_EQ
%token <tok_val> LSHIFT_EQ RSHIFT_EQ LSHIFT RSHIFT
%token <tok_val> EXPR_AND_AND EXPR_OR_OR
%token <tok_val> EXPR_AND EXPR_OR EXPR_NOT
%token <tok_val> EXPR_LT EXPR_LE EXPR_EQ EXPR_NE EXPR_GE EXPR_GT
%token <tok_val> LEFTDIV EMUL EDIV ELEFTDIV EPLUS EMINUS
%token <tok_val> QUOTE TRANSPOSE
%token <tok_val> PLUS_PLUS MINUS_MINUS POW EPOW
%token <tok_val> NUM IMAG_NUM
%token <tok_val> STRUCT_ELT
%token <tok_val> NAME
%token <tok_val> END
%token <tok_val> PLOT
%token <tok_val> TEXT STYLE AXES_TAG
%token <tok_val> FOR WHILE
%token <tok_val> IF ELSEIF ELSE
%token <tok_val> SWITCH CASE OTHERWISE
%token <tok_val> BREAK CONTINUE FUNC_RET
%token <tok_val> UNWIND CLEANUP
%token <tok_val> TRY CATCH
%token <tok_val> GLOBAL STATIC

// Other tokens.
%token END_OF_INPUT LEXICAL_ERROR
%token FCN ELLIPSIS ALL_VA_ARGS
%token USING TITLE WITH AXES COLON OPEN_BRACE CLOSE_BRACE CLEAR

// Nonterminals we construct.
%type <sep_type> sep_no_nl opt_sep_no_nl sep opt_sep
%type <tree_type> input
%type <tree_constant_type> constant magic_colon
%type <tree_matrix_type> rows rows1
%type <tree_expression_type> title matrix
%type <tree_expression_type> primary_expr postfix_expr prefix_expr binary_expr
%type <tree_expression_type> simple_expr colon_expr assign_expr expression
%type <tree_identifier_type> identifier
%type <octave_user_function_type> function1 function2 function3
%type <tree_index_expression_type> word_list_cmd
%type <tree_colon_expression_type> colon_expr1
%type <tree_argument_list_type> arg_list word_list assign_lhs matrix_row
%type <tree_parameter_list_type> param_list param_list1
%type <tree_parameter_list_type> return_list return_list1
%type <tree_command_type> command select_command loop_command
%type <tree_command_type> jump_command except_command function
%type <tree_if_command_type> if_command
%type <tree_if_clause_type> elseif_clause else_clause
%type <tree_if_command_list_type> if_cmd_list1 if_cmd_list
%type <tree_switch_command_type> switch_command
%type <tree_switch_case_type> switch_case default_case
%type <tree_switch_case_list_type> case_list1 case_list
%type <tree_decl_elt_type> decl2
%type <tree_decl_init_list_type> decl1
%type <tree_decl_command_type> declaration
%type <tree_statement_type> statement
%type <tree_statement_list_type> simple_list simple_list1 list list1
%type <tree_statement_list_type> opt_list input1 function4
%type <tree_plot_command_type> plot_command 
%type <subplot_type> plot_command2 plot_options
%type <subplot_list_type> plot_command1
%type <plot_limits_type> ranges
%type <plot_range_type> ranges1 
%type <subplot_using_type> using using1 
%type <subplot_style_type> style
%type <subplot_axes_type> axes

// Precedence and associativity.
%left ';' ',' '\n'
%right '=' ADD_EQ SUB_EQ MUL_EQ DIV_EQ LEFTDIV_EQ EMUL_EQ EDIV_EQ ELEFTDIV_EQ OR_EQ AND_EQ LSHIFT_EQ RSHIFT_EQ
%left EXPR_AND_AND EXPR_OR_OR
%left EXPR_AND EXPR_OR
%left EXPR_LT EXPR_LE EXPR_EQ EXPR_NE EXPR_GE EXPR_GT
%left LSHIFT RSHIFT
%left ':'
%left '-' '+' EPLUS EMINUS
%left '*' '/' LEFTDIV EMUL EDIV ELEFTDIV
%left QUOTE TRANSPOSE
%left UNARY PLUS_PLUS MINUS_MINUS EXPR_NOT
%right POW EPOW
%left '(' '.'

// Where to start.
%start input

%%

// ==============================
// Statements and statement lists
// ==============================

input		: input1
		  {
		    global_command = $1;
		    promptflag = 1;
		    YYACCEPT;
		  }
		| END_OF_INPUT
		  {
		    global_command = 0;
		    promptflag = 1;
		    YYABORT;
		  }
		| simple_list parse_error
		  { ABORT_PARSE; }
		| parse_error
		  { ABORT_PARSE; }
		;

input1		: '\n'
		  { $$ = 0; }
		| simple_list
		  { $$ = $1; }
		| simple_list '\n'
		  { $$ = $1; }
		| simple_list END_OF_INPUT
		  { $$ = $1; }
		;

simple_list	: simple_list1 opt_sep_no_nl
		  {
		    set_stmt_print_flag ($1, $2, false);
		    $$ = $1;
		  }
		;

simple_list1	: statement
		  { $$ = new tree_statement_list ($1); }
		| simple_list1 sep_no_nl statement
		  {
		    set_stmt_print_flag ($1, $2, false);
		    $1->append ($3);
		    $$ = $1;
		  }
		;

opt_list	: // empty
		  { $$ = new tree_statement_list (); }
		| list
		  { $$ = $1; }
		;

list		: list1 opt_sep
		  {
		    set_stmt_print_flag ($1, $2, true);
		    $$ = $1;
		  }
		;

list1		: statement
		  {
		    lexer_flags.beginning_of_function = false;
		    $$ = new tree_statement_list ($1);
		  }
		| list1 sep statement
		  {
		    set_stmt_print_flag ($1, $2, true);
		    $1->append ($3);
		    $$ = $1;
		  }
		;

statement	: expression
		  { $$ = new tree_statement ($1); }
		| command
		  { $$ = new tree_statement ($1); }
		| PLOT CLEAR
		  {
		    symbol_record *sr = lookup_by_name ("clearplot", 0);
		    tree_identifier *id = new tree_identifier (sr);
		    $$ = new tree_statement (id);
		  }
		;

// ===========
// Expressions
// ===========

identifier	: NAME
		  {
		    $$ = new tree_identifier
		      ($1->sym_rec (), $1->line (), $1->column ());
		  }
		;

constant	: NUM
		  { $$ = make_constant (NUM, $1); }
		| IMAG_NUM
		  { $$ = make_constant (IMAG_NUM, $1); }
		| TEXT
		  { $$ = make_constant (TEXT, $1); }
		;

in_matrix_or_assign_lhs
		: // empty
		  { lexer_flags.looking_at_matrix_or_assign_lhs = true; }
		;

matrix		: '[' ']'
		  { $$ = new tree_constant (octave_value (Matrix ())); }
		| '[' ';' ']'
		  { $$ = new tree_constant (octave_value (Matrix ())); }
		| '[' in_matrix_or_assign_lhs rows ']'
		  {
		    $$ = finish_matrix ($3);
		    lexer_flags.looking_at_matrix_or_assign_lhs = false;
		  }
		;

rows		: rows1
		  { $$ = $1; }
		| rows1 ';'	// Ignore trailing semicolon.
		  { $$ = $1; }
		;

rows1		: matrix_row
		  { $$ = new tree_matrix ($1); }
		| rows1 ';' matrix_row
		  {
		    $1->append ($3);
		    $$ = $1;
		  }
		;

matrix_row	: arg_list
		  { $$ = $1; }
		| arg_list ','	// Ignore trailing comma.
		  { $$ = $1; }
		;

primary_expr	: identifier
		  { $$ = $1; }
		| constant
		  { $$ = $1; }
		| matrix
		  { $$ = $1; }
		| '(' expression ')'
		  { $$ = $2->mark_in_parens (); }
		;

magic_colon	: ':'
		  {
		    octave_value tmp (octave_value::magic_colon_t);
		    $$ = new tree_constant (tmp);
		  }
		;

arg_list	: expression
		  { $$ = new tree_argument_list ($1); }
		| magic_colon
		  { $$ = new tree_argument_list ($1); }
		| ALL_VA_ARGS
		  {
		    octave_value tmp (octave_value::all_va_args_t);
		    tree_constant *all_va_args = new tree_constant (tmp);
		    $$ = new tree_argument_list (all_va_args);
		  }
		| arg_list ',' magic_colon
		  {
		    $1->append ($3);
		    $$ = $1;
		  }
		| arg_list ',' expression
		  {
		    $1->append ($3);
		    $$ = $1;
		  }
		| arg_list ',' ALL_VA_ARGS
		  {
		    octave_value tmp (octave_value::all_va_args_t);
		    tree_constant *all_va_args = new tree_constant (tmp);
		    $1->append (all_va_args);
		    $$ = $1;
		  }
		;

parsing_indir	: // empty
		  { lexer_flags.looking_at_indirect_ref = true; }
		;

postfix_expr	: primary_expr
		  { $$ = $1; }
		| postfix_expr '(' ')'
		  { $$ = make_index_expression ($1, 0); }
		| postfix_expr '(' arg_list ')'
		  { $$ = make_index_expression ($1, $3); }
		| postfix_expr PLUS_PLUS
		  { $$ = make_postfix_op (PLUS_PLUS, $1, $2); }
		| postfix_expr MINUS_MINUS
		  { $$ = make_postfix_op (MINUS_MINUS, $1, $2); }
		| postfix_expr QUOTE
		  { $$ = make_postfix_op (QUOTE, $1, $2); }
		| postfix_expr TRANSPOSE
		  { $$ = make_postfix_op (TRANSPOSE, $1, $2); }
		| postfix_expr '.' parsing_indir STRUCT_ELT
		  { $$ = make_indirect_ref ($1, $4->text ()); }
		;

prefix_expr	: postfix_expr
		  { $$ = $1; }
		| binary_expr
		  { $$ = $1; }
		| PLUS_PLUS prefix_expr %prec UNARY
		  { $$ = make_prefix_op (PLUS_PLUS, $2, $1); }
		| MINUS_MINUS prefix_expr %prec UNARY
		  { $$ = make_prefix_op (MINUS_MINUS, $2, $1); }
		| EXPR_NOT prefix_expr %prec UNARY
		  { $$ = make_prefix_op (EXPR_NOT, $2, $1); }
		| '+' prefix_expr %prec UNARY
		  { $$ = $2; }
		| '-' prefix_expr %prec UNARY
		  { $$ = make_prefix_op ('-', $2, $1); }
		;

binary_expr	: prefix_expr POW prefix_expr
		  { $$ = make_binary_op (POW, $1, $2, $3); }
		| prefix_expr EPOW prefix_expr
		  { $$ = make_binary_op (EPOW, $1, $2, $3); }
		| prefix_expr '+' prefix_expr
		  { $$ = make_binary_op ('+', $1, $2, $3); }
		| prefix_expr '-' prefix_expr
		  { $$ = make_binary_op ('-', $1, $2, $3); }
		| prefix_expr '*' prefix_expr
		  { $$ = make_binary_op ('*', $1, $2, $3); }
		| prefix_expr '/' prefix_expr
		  { $$ = make_binary_op ('/', $1, $2, $3); }
		| prefix_expr EPLUS prefix_expr
		  { $$ = make_binary_op ('+', $1, $2, $3); }
		| prefix_expr EMINUS prefix_expr
		  { $$ = make_binary_op ('-', $1, $2, $3); }
		| prefix_expr EMUL prefix_expr
		  { $$ = make_binary_op (EMUL, $1, $2, $3); }
		| prefix_expr EDIV prefix_expr
		  { $$ = make_binary_op (EDIV, $1, $2, $3); }
		| prefix_expr LEFTDIV prefix_expr
		  { $$ = make_binary_op (LEFTDIV, $1, $2, $3); }
		| prefix_expr ELEFTDIV prefix_expr
		  { $$ = make_binary_op (ELEFTDIV, $1, $2, $3); }
		;

colon_expr	: colon_expr1
		  { $$ = finish_colon_expression ($1); }
		;

colon_expr1	: prefix_expr
		  { $$ = new tree_colon_expression ($1); }
		| colon_expr1 ':' prefix_expr
		  {
		    if (! ($$ = $1->append ($3)))
		      ABORT_PARSE;
		  }
		;

simple_expr	: colon_expr
		  { $$ = $1; }
		| simple_expr LSHIFT simple_expr
		  { $$ = make_binary_op (LSHIFT, $1, $2, $3); }
		| simple_expr RSHIFT simple_expr
		  { $$ = make_binary_op (RSHIFT, $1, $2, $3); }
		| simple_expr EXPR_LT simple_expr
		  { $$ = make_binary_op (EXPR_LT, $1, $2, $3); }
		| simple_expr EXPR_LE simple_expr
		  { $$ = make_binary_op (EXPR_LE, $1, $2, $3); }
		| simple_expr EXPR_EQ simple_expr
		  { $$ = make_binary_op (EXPR_EQ, $1, $2, $3); }
		| simple_expr EXPR_GE simple_expr
		  { $$ = make_binary_op (EXPR_GE, $1, $2, $3); }
		| simple_expr EXPR_GT simple_expr
		  { $$ = make_binary_op (EXPR_GT, $1, $2, $3); }
		| simple_expr EXPR_NE simple_expr
		  { $$ = make_binary_op (EXPR_NE, $1, $2, $3); }
		| simple_expr EXPR_AND simple_expr
		  { $$ = make_binary_op (EXPR_AND, $1, $2, $3); }
		| simple_expr EXPR_OR simple_expr
		  { $$ = make_binary_op (EXPR_OR, $1, $2, $3); }
		| simple_expr EXPR_AND_AND simple_expr
		  { $$ = make_boolean_op (EXPR_AND_AND, $1, $2, $3); }
		| simple_expr EXPR_OR_OR simple_expr
		  { $$ = make_boolean_op (EXPR_OR_OR, $1, $2, $3); }
		;

// Arrange for the lexer to return CLOSE_BRACE for `]' by looking ahead
// one token for an assignment op.

assign_lhs	: simple_expr
		  { $$ = new tree_argument_list ($1); }
		| '[' in_matrix_or_assign_lhs arg_list CLOSE_BRACE
		  {
		    $$ = $3;
		    lexer_flags.looking_at_matrix_or_assign_lhs = false;
		  }
		;

assign_expr	: assign_lhs '=' expression
		  { $$ = make_assign_op ('=', $1, $2, $3); }
		| assign_lhs ADD_EQ expression
		  { $$ = make_assign_op (ADD_EQ, $1, $2, $3); }
		| assign_lhs SUB_EQ expression
		  { $$ = make_assign_op (SUB_EQ, $1, $2, $3); }
		| assign_lhs MUL_EQ expression
		  { $$ = make_assign_op (MUL_EQ, $1, $2, $3); }
		| assign_lhs DIV_EQ expression
		  { $$ = make_assign_op (DIV_EQ, $1, $2, $3); }
		| assign_lhs LEFTDIV_EQ expression
		  { $$ = make_assign_op (LEFTDIV_EQ, $1, $2, $3); }
		| assign_lhs LSHIFT_EQ expression
		  { $$ = make_assign_op (LSHIFT_EQ, $1, $2, $3); }
		| assign_lhs RSHIFT_EQ expression
		  { $$ = make_assign_op (RSHIFT_EQ, $1, $2, $3); }
		| assign_lhs EMUL_EQ expression
		  { $$ = make_assign_op (EMUL_EQ, $1, $2, $3); }
		| assign_lhs EDIV_EQ expression
		  { $$ = make_assign_op (EDIV_EQ, $1, $2, $3); }
		| assign_lhs ELEFTDIV_EQ expression
		  { $$ = make_assign_op (ELEFTDIV_EQ, $1, $2, $3); }
		| assign_lhs AND_EQ expression
		  { $$ = make_assign_op (AND_EQ, $1, $2, $3); }
		| assign_lhs OR_EQ expression
		  { $$ = make_assign_op (OR_EQ, $1, $2, $3); }
		;

word_list_cmd	: identifier word_list
		  { $$ = make_index_expression ($1, $2); }
		;

word_list	: TEXT
		  {
		    tree_constant *tmp = make_constant (TEXT, $1);
		    $$ = new tree_argument_list (tmp);
		  }
		| word_list TEXT
		  {
		    tree_constant *tmp = make_constant (TEXT, $2);
		    $1->append (tmp);
		    $$ = $1;
		  }
		;

expression	: simple_expr
		  { $$ = $1; }
		| word_list_cmd
		  { $$ = $1; }
		| assign_expr
		  { $$ = $1; }
		;

// ================================================
// Commands, declarations, and function definitions
// ================================================

command		: declaration
		  { $$ = $1; }
		| select_command
		  { $$ = $1; }
		| loop_command
		  { $$ = $1; }
		| jump_command
		  { $$ = $1; }
		| except_command
		  { $$ = $1; }
		| function
		  { $$ = $1; }
		| plot_command
		  { $$ = $1; }
		;

// =====================
// Declaration statemnts
// =====================

declaration	: GLOBAL decl1
		  { $$ = make_decl_command (GLOBAL, $1, $2); }
		| STATIC decl1
		  { $$ = make_decl_command (STATIC, $1, $2); }
		;

decl1		: decl2
		  { $$ = new tree_decl_init_list ($1); }
		| decl1 decl2
		  {
		    $1->append ($2);
		    $$ = $1;
		  }
		;

decl2		: identifier
		  { $$ = new tree_decl_elt ($1); }
		| identifier '=' expression
		  { $$ = new tree_decl_elt ($1, $3); }
		;

// ====================
// Selection statements
// ====================

select_command	: if_command
		  { $$ = $1; }
		| switch_command
		  { $$ = $1; }
		;

// ============
// If statement
// ============

if_command	: IF if_cmd_list END
		  {
		    if (! ($$ = finish_if_command ($1, $2, $3)))
		      ABORT_PARSE;
		  }
		;

if_cmd_list	: if_cmd_list1
		  { $$ = $1; }
		| if_cmd_list1 else_clause
		  {
		    $1->append ($2);
		    $$ = $1;
		  }
		;

if_cmd_list1	: expression opt_sep opt_list
		  { $$ = start_if_command ($1, $3); }
		| if_cmd_list1 elseif_clause
		  {
		    $1->append ($2);
		    $$ = $1;
		  }
		;

elseif_clause	: ELSEIF opt_sep expression opt_sep opt_list
		  { $$ = make_elseif_clause ($3, $5); }
		;

else_clause	: ELSE opt_sep opt_list
		  { $$ = new tree_if_clause ($3); }
		;

// ================
// Switch statement
// ================

switch_command	: SWITCH expression opt_sep case_list END
		  {
		    if (! ($$ = finish_switch_command ($1, $2, $4, $5)))
		      ABORT_PARSE;
		  }
		;

case_list	: case_list1
		  { $$ = $1; }
		| case_list1 default_case
		  {
		    $1->append ($2);
		    $$ = $1;
		  }		
		;

case_list1	: switch_case
		  { $$ = new tree_switch_case_list ($1); }
		| case_list1 switch_case
		  {
		    $1->append ($2);
		    $$ = $1;
		  }
		;

switch_case	: CASE opt_sep expression opt_sep list
		  { $$ = make_switch_case ($3, $5); }
		;

default_case	: OTHERWISE opt_sep opt_list
		  { $$ = new tree_switch_case ($3); }
		;

// =======
// Looping
// =======

loop_command	: WHILE expression opt_sep opt_list END
		  {
		    if (! ($$ = make_while_command ($1, $2, $4, $5)))
		      ABORT_PARSE;
		  }
		| FOR assign_lhs '=' expression opt_sep opt_list END
		  {
		    if (! ($$ = make_for_command ($1, $2, $4, $6, $7)))
		      ABORT_PARSE;
		  }
		;

// =======
// Jumping
// =======

jump_command	: BREAK
		  {
		    if (! ($$ = make_break_command ($1)))
		      ABORT_PARSE;
		  }
		| CONTINUE
		  {
		    if (! ($$ = make_continue_command ($1)))
		      ABORT_PARSE;
		  }
		| FUNC_RET
		  {
		    if (! ($$ = make_return_command ($1)))
		      ABORT_PARSE;
		  }
		;

// ==========
// Exceptions
// ==========

except_command	: UNWIND opt_sep opt_list CLEANUP opt_sep opt_list END
		  {
		    if (! ($$ = make_unwind_command ($1, $3, $6, $7)))
		      ABORT_PARSE;
		  }
		| TRY opt_sep opt_list CATCH opt_sep opt_list END
		  {
		    if (! ($$ = make_try_command ($1, $3, $6, $7)))
		      ABORT_PARSE;
		  }
		;

// ===========================================
// Some `subroutines' for function definitions
// ===========================================

global_symtab	: // empty
		  { curr_sym_tab = global_sym_tab; }
		;

local_symtab	: // empty
		  { curr_sym_tab = tmp_local_sym_tab; }
		;

in_return_list	: // empty
		  { lexer_flags.looking_at_return_list = true; }
		;

parsed_fcn_name	: // empty
		  { lexer_flags.parsed_function_name = true; }
		;

// ===========================
// List of function parameters
// ===========================

param_list_beg	: '('
		  { lexer_flags.looking_at_parameter_list = true; }
		;

param_list_end	: ')'
		  { lexer_flags.looking_at_parameter_list = false; }
		;

param_list	: param_list_beg param_list_end
		  {
		    lexer_flags.quote_is_transpose = false;
		    $$ = 0;
		  }
		| param_list_beg ELLIPSIS param_list_end
		  {
		    lexer_flags.quote_is_transpose = false;
		    tree_parameter_list *tmp = new tree_parameter_list ();
		    tmp->mark_varargs_only ();
		    $$ = tmp;
		  }
		| param_list1 param_list_end
		  {
		    lexer_flags.quote_is_transpose = false;
		    $1->mark_as_formal_parameters ();
		    $$ = $1;
		  }
		| param_list1 ',' ELLIPSIS param_list_end
		  {
		    lexer_flags.quote_is_transpose = false;
		    $1->mark_as_formal_parameters ();
		    $1->mark_varargs ();
		    $$ = $1;
		  }
		;

param_list1	: param_list_beg identifier
		  { $$ = new tree_parameter_list ($2); }
		| param_list1 ',' identifier
		  {
		    $1->append ($3);
		    $$ = $1;
		  }
		| param_list_beg error
		  {
		    yyerror ("invalid parameter list");
		    $$ = 0;
		    ABORT_PARSE;
		  }
		| param_list1 ',' error
		  {
		    yyerror ("invalid parameter list");
		    $$ = 0;
		    ABORT_PARSE;
		  }
		;

// ===================================
// List of function return value names
// ===================================

return_list_beg	: '[' in_return_list local_symtab
		;

return_list	: return_list_beg return_list_end
		  {
		    lexer_flags.looking_at_return_list = false;
		    $$ = new tree_parameter_list ();
		  }
		| return_list_beg ELLIPSIS return_list_end
		  {
		    lexer_flags.looking_at_return_list = false;
		    tree_parameter_list *tmp = new tree_parameter_list ();
		    tmp->mark_varargs_only ();
		    $$ = tmp;
		  }
		| return_list_beg return_list1 return_list_end
		  {
		    lexer_flags.looking_at_return_list = false;
		    $$ = $2;
		  }
		| return_list_beg return_list1 ',' ELLIPSIS return_list_end
		  {
		    lexer_flags.looking_at_return_list = false;
		    $2->mark_varargs ();
		    $$ = $2;
		  }
		;

return_list1	: identifier
		  { $$ = new tree_parameter_list ($1); }
		| return_list1 ',' identifier
		  {
		    $1->append ($3);
		    $$ = $1;
		  }
		;

return_list_end	: global_symtab ']'
		;

// ===================
// Function definition
// ===================

function_beg	: FCN global_symtab
		;

function	: function_beg function2
		  {
		    recover_from_parsing_function ();
		    $$ = 0;
		  }
		| function_beg identifier function1
		  {
		    finish_function ($2, $3);
		    recover_from_parsing_function ();
		    $$ = 0;
		  }
		| function_beg return_list function1
		  {
		    finish_function ($2, $3);
		    recover_from_parsing_function ();
		    $$ = 0;
		  }
		;

function1	: global_symtab '=' function2
		  { $$ = $3; }
		;

function2	: identifier local_symtab parsed_fcn_name function3
		  {
		    if (! ($$ = frob_function ($1, $4)))
		      ABORT_PARSE;
		  }
		;

function3	: param_list function4
		  { $$ = start_function ($1, $2); }
		| function4
		  { $$ = start_function (0, $1); }
		;

function4	: opt_sep opt_list function_end
		  { $$ = $2; }
		;

function_end	: END
		  {
		    if (end_token_ok ($1, token::function_end))
		      {
			if (reading_fcn_file)
			  check_for_garbage_after_fcn_def ();
		      }
		    else
		      ABORT_PARSE;
		  }
		| END_OF_INPUT
		  {
		    if (! (reading_fcn_file || reading_script_file))
		      YYABORT;
		  }
		;

// ========
// Plotting
// ========

plot_command	: PLOT
		  {
		    if (! ($$ = make_plot_command ($1, 0, 0)))
		      ABORT_PARSE;
		  }
		| PLOT ranges
		  {
		    if (! ($$ = make_plot_command ($1, $2, 0)))
		      ABORT_PARSE;
		  }
		| PLOT plot_command1
		  {
		    if (! ($$ = make_plot_command ($1, 0, $2)))
		      ABORT_PARSE;
		  }
		| PLOT ranges plot_command1
		  {
		    if (! ($$ = make_plot_command ($1, $2, $3)))
		      ABORT_PARSE;
		  }
		;

ranges		: ranges1
		  { $$ = new plot_limits ($1); }
		| ranges1 ranges1
		  { $$ = new plot_limits ($1, $2); }
		| ranges1 ranges1 ranges1
		  { $$ = new plot_limits ($1, $2, $3); }
		;

ranges1		: OPEN_BRACE expression COLON expression CLOSE_BRACE
		  { $$ = new plot_range ($2, $4); }
		| OPEN_BRACE COLON expression CLOSE_BRACE
		  { $$ = new plot_range (0, $3); }
		| OPEN_BRACE expression COLON CLOSE_BRACE
		  { $$ = new plot_range ($2, 0); }
		| OPEN_BRACE COLON CLOSE_BRACE
		  { $$ = new plot_range (); }
		| OPEN_BRACE CLOSE_BRACE
		  { $$ = new plot_range (); }
		;

plot_command1	: plot_command2
		  { $$ = new subplot_list ($1); }
		| plot_command1 ',' plot_command2
		  {
		    $1->append ($3);
		    $$ = $1;
		  }
		;

plot_command2	: expression
		  { $$ = new subplot ($1); }
		| expression plot_options
		  { $$ = $2->add_data ($1); }
		;

plot_options	: using
		  {
		    subplot *tmp = new subplot ();
		    $$ = tmp->add_clause ($1);
		  }
		| title
		  {
		    subplot *tmp = new subplot ();
		    $$ = tmp->add_clause ($1);
		  }
		| style
		  {
		    subplot *tmp = new subplot ();
		    $$ = tmp->add_clause ($1);
		  }
		| axes
		  {
		    subplot *tmp = new subplot ();
		    $$ = tmp->add_clause ($1);
		  }
		| plot_options using
		  {
		    if (! ($$ = $1->add_clause ($2)))
		      {
			yyerror ("only one using option may be specified");
			ABORT_PARSE;
		      }
		  }
		| plot_options title
		  {
		    if (! ($$ = $1->add_clause ($2)))
		      {
			yyerror ("only one title option my be specified");
			ABORT_PARSE;
		      }
		  }
		| plot_options style
		  {
		    if (! ($$ = $1->add_clause ($2)))
		      {
			yyerror ("only one style option my be specified");
			ABORT_PARSE;
		      }
		  }
		| plot_options axes
		  {
		    if (! ($$ = $1->add_clause ($2)))
		      {
			yyerror ("only one axes option may be specified");
			ABORT_PARSE;
		      }
		  }
		;

axes		: AXES AXES_TAG
		  {
		    lexer_flags.in_plot_axes = false;
		    $$ = new subplot_axes ($2->text ());
		  }
		;

using		: using1
		  {
		    lexer_flags.in_plot_using = false;
		    $$ = $1;
		  }
		| using1 expression
		  {
		    lexer_flags.in_plot_using = false;
		    $$ = $1->set_format ($2);
		  }
		;

using1		: USING expression
		  {
		    subplot_using *tmp = new subplot_using ();
		    $$ = tmp->add_qualifier ($2);
		  }
		| using1 COLON expression
		  { $$ = $1->add_qualifier ($3); }
		;

title		: TITLE expression
		  { $$ = $2; }
		;

style		: WITH STYLE
		  { $$ = new subplot_style ($2->text ()); }
		| WITH STYLE expression
		  { $$ = new subplot_style ($2->text (), $3); }
		| WITH STYLE expression expression
		  { $$ = new subplot_style ($2->text (), $3, $4); }
		;

// =============
// Miscellaneous
// =============

parse_error	: LEXICAL_ERROR
		  { yyerror ("parse error"); }
		| error
		;

sep_no_nl	: ','
		  { $$ = ','; }
		| ';'
		  { $$ = ';'; }
		| sep_no_nl ','
		  { $$ = $1; }
		| sep_no_nl ';'
		  { $$ = $1; }
		;

opt_sep_no_nl	: // empty
		  { $$ = 0; }
		| sep_no_nl
		  { $$ = $1; }
		;

sep		: ','
		  { $$ = ','; }
		| ';'
		  { $$ = ';'; }
		| '\n'
		  { $$ = '\n'; }
		| sep ','
		  { $$ = $1; }
		| sep ';'
		  { $$ = $1; }
		| sep '\n'
		  { $$ = $1; }
		;

opt_sep		: // empty
		  { $$ = 0; }
		| sep
		  { $$ = $1; }
		;

%%

// Generic error messages.

static void
yyerror (const char *s)
{
  int err_col = current_input_column - 1;

  ostrstream output_buf;

  if (reading_fcn_file || reading_script_file)
    output_buf << "parse error near line " << input_line_number
	       << " of file " << curr_fcn_file_full_name;
  else
    output_buf << "parse error:";

  if (s && strcmp (s, "parse error") != 0)
    output_buf << "\n\n  " << s;

  output_buf << "\n\n";

  if (! current_input_line.empty ())
    {
      size_t len = current_input_line.length ();

      if (current_input_line[len-1] == '\n')
        current_input_line.resize (len-1);

// Print the line, maybe with a pointer near the error token.

      output_buf << ">>> " << current_input_line << "\n";

      if (err_col == 0)
	err_col = len;

      for (int i = 0; i < err_col + 3; i++)
	output_buf << " ";

      output_buf << "^";
    }

  output_buf << "\n" << ends;

  char *msg = output_buf.str ();

  parse_error ("%s", msg);

  delete [] msg;
}

// Error mesages for mismatched end tokens.

static void
end_error (const char *type, token::end_tok_type ettype, int l, int c)
{
  static const char *fmt
    = "`%s' command matched by `%s' near line %d column %d";

  switch (ettype)
    {
    case token::simple_end:
      error (fmt, type, "end", l, c);
      break;

    case token::for_end:
      error (fmt, type, "endfor", l, c);
      break;

    case token::function_end:
      error (fmt, type, "endfunction", l, c);
      break;

    case token::if_end:
      error (fmt, type, "endif", l, c);
      break;

    case token::switch_end:
      error (fmt, type, "endswitch", l, c); 
      break;

    case token::while_end:
      error (fmt, type, "endwhile", l, c); 
      break;

    case token::unwind_protect_end:
      error (fmt, type, "end_unwind_protect", l, c); 
      break;

    default:
      panic_impossible ();
      break;
    }
}

// Check to see that end tokens are properly matched.

static bool
end_token_ok (token *tok, token::end_tok_type expected)
{
  bool retval = true;

  token::end_tok_type ettype = tok->ettype ();

  if (ettype != expected && ettype != token::simple_end)
    {
      retval = false;

      yyerror ("parse error");

      int l = tok->line ();
      int c = tok->column ();

      switch (expected)
	{
	case token::for_end:
	  end_error ("for", ettype, l, c);
	  break;

	case token::function_end:
	  end_error ("function", ettype, l, c);
	  break;

	case token::if_end:
	  end_error ("if", ettype, l, c);
	  break;

	case token::try_catch_end:
	  end_error ("try", ettype, l, c);
	  break;

	case token::switch_end:
	  end_error ("switch", ettype, l, c);
	  break;

	case token::unwind_protect_end:
	  end_error ("unwind_protect", ettype, l, c);
	  break;

	case token::while_end:
	  end_error ("while", ettype, l, c);
	  break;

	default:
	  panic_impossible ();
	  break;
	}
    }

  return retval;
}

// Maybe print a warning if an assignment expression is used as the
// test in a logical expression.

static void
maybe_warn_assign_as_truth_value (tree_expression *expr)
{
  if (Vwarn_assign_as_truth_value
      && expr->is_assignment_expression ()
      && expr->paren_count () < 2)
    {
      warning ("suggest parenthesis around assignment used as truth value");
    }
}

// Maybe print a warning about switch labels that aren't constants.

static void
maybe_warn_variable_switch_label (tree_expression *expr)
{
  if (Vwarn_variable_switch_label && ! expr->is_constant ())
    {
      warning ("variable switch label");
    }
}

// Create a plot command.

static tree_plot_command *
make_plot_command (token *tok, plot_limits *range, subplot_list *list)
{
  if (range)
    {
      if (tok->pttype () == token::replot)
	{
	  yyerror ("cannot specify new ranges with replot");
	  return 0;
	}
    }
  else if (! list && tok->pttype () != token::replot)
    {
      yyerror ("must have something to plot");
      return 0;
    }

  lexer_flags.plotting = false;
  lexer_flags.past_plot_range = false;
  lexer_flags.in_plot_range = false;
  lexer_flags.in_plot_using = false;
  lexer_flags.in_plot_style = false;
  
  return new tree_plot_command (list, range, tok->pttype ());
}

static tree_expression *
fold (tree_binary_expression *e)
{
  tree_expression *retval = e;

  unwind_protect::begin_frame ("fold");

  unwind_protect_int (error_state);

  unwind_protect_bool (buffer_error_messages);
  buffer_error_messages = true;

  unwind_protect::add (clear_global_error_variable, 0);

  tree_expression *op1 = e->lhs ();
  tree_expression *op2 = e->rhs ();

  if (op1->is_constant () && op2->is_constant ())
    {
      octave_value tmp = e->rvalue ();

      if (! error_state)
	{
	  tree_constant *tc_retval = new tree_constant (tmp);

	  ostrstream buf;

	  tree_print_code tpc (buf);

	  e->accept (tpc);

	  buf << ends;

	  char *s = buf.str ();

	  tc_retval->stash_original_text (s);

	  delete [] s;

	  delete e;

	  retval = tc_retval;
	}
    }

  unwind_protect::run_frame ("fold");

  return retval;
}

// Finish building a range.

static tree_expression *
finish_colon_expression (tree_colon_expression *e)
{
  tree_expression *retval = e;

  unwind_protect::begin_frame ("finish_colon_expression");

  unwind_protect_int (error_state);

  unwind_protect_bool (buffer_error_messages);
  buffer_error_messages = true;

  unwind_protect::add (clear_global_error_variable, 0);

  tree_expression *base = e->base ();
  tree_expression *limit = e->limit ();
  tree_expression *incr = e->increment ();

  if (base)
    {
      if (limit)
	{
	  if (base->is_constant () && limit->is_constant ()
	      && (! incr || (incr && incr->is_constant ())))
	    {
	      octave_value tmp = e->rvalue ();

	      if (! error_state)
		{
		  tree_constant *tc_retval = new tree_constant (tmp);

		  ostrstream buf;

		  tree_print_code tpc (buf);

		  e->accept (tpc);

		  buf << ends;

		  char *s = buf.str ();

		  tc_retval->stash_original_text (s);

		  delete [] s;

		  delete e;

		  retval = tc_retval;
		}
	    }
	}
      else
	{
	  e->preserve_base ();
	  delete e;

	  // XXX FIXME XXX -- need to attempt constant folding here
	  // too (we need a generic way to do that).
	  retval = base;
	}
    }

  unwind_protect::run_frame ("finish_colon_expression");

  return retval;
}

// Make a constant.

static tree_constant *
make_constant (int op, token *tok_val)
{
  int l = tok_val->line ();
  int c = tok_val->column ();

  tree_constant *retval = 0;

  switch (op)
    {
    case NUM:
      {
	octave_value tmp (tok_val->number ());
	retval = new tree_constant (tmp, l, c);
	retval->stash_original_text (tok_val->text_rep ());
      }
      break;

    case IMAG_NUM:
      {
	octave_value tmp (Complex (0.0, tok_val->number ()));
	retval = new tree_constant (tmp, l, c);
	retval->stash_original_text (tok_val->text_rep ());
      }
      break;

    case TEXT:
      {
	octave_value tmp (tok_val->text ());
	retval = new tree_constant (tmp, l, c);
      }
      break;

    default:
      panic_impossible ();
      break;
    }

  return retval;
}

// Build a binary expression.

static tree_expression *
make_binary_op (int op, tree_expression *op1, token *tok_val,
		tree_expression *op2)
{
  octave_value::binary_op t = octave_value::unknown_binary_op;

  switch (op)
    {
    case POW:
      t = octave_value::pow;
      break;

    case EPOW:
      t = octave_value::el_pow;
      break;

    case '+':
      t = octave_value::add;
      break;

    case '-':
      t = octave_value::sub;
      break;

    case '*':
      t = octave_value::mul;
      break;

    case '/':
      t = octave_value::div;
      break;

    case EMUL:
      t = octave_value::el_mul;
      break;

    case EDIV:
      t = octave_value::el_div;
      break;

    case LEFTDIV:
      t = octave_value::ldiv;
      break;

    case ELEFTDIV:
      t = octave_value::el_ldiv;
      break;

    case LSHIFT:
      t = octave_value::lshift;
      break;

    case RSHIFT:
      t = octave_value::rshift;
      break;

    case EXPR_LT:
      t = octave_value::lt;
      break;

    case EXPR_LE:
      t = octave_value::le;
      break;

    case EXPR_EQ:
      t = octave_value::eq;
      break;

    case EXPR_GE:
      t = octave_value::ge;
      break;

    case EXPR_GT:
      t = octave_value::gt;
      break;

    case EXPR_NE:
      t = octave_value::ne;
      break;

    case EXPR_AND:
      t = octave_value::el_and;
      break;

    case EXPR_OR:
      t = octave_value::el_or;
      break;

    default:
      panic_impossible ();
      break;
    }

  int l = tok_val->line ();
  int c = tok_val->column ();

  tree_binary_expression *e
    = new tree_binary_expression (op1, op2, l, c, t);

  return fold (e);
}

// Build a boolean expression.

static tree_expression *
make_boolean_op (int op, tree_expression *op1, token *tok_val,
		 tree_expression *op2)
{
  tree_boolean_expression::type t;

  switch (op)
    {
    case EXPR_AND_AND:
      t = tree_boolean_expression::bool_and;
      break;

    case EXPR_OR_OR:
      t = tree_boolean_expression::bool_or;
      break;

    default:
      panic_impossible ();
      break;
    }

  int l = tok_val->line ();
  int c = tok_val->column ();

  tree_boolean_expression *e
    = new tree_boolean_expression (op1, op2, l, c, t);

  return fold (e);
}

// Build a prefix expression.

static tree_expression *
make_prefix_op (int op, tree_expression *op1, token *tok_val)
{
  octave_value::unary_op t = octave_value::unknown_unary_op;

  switch (op)
    {
    case EXPR_NOT:
      t = octave_value::not;
      break;

    case '-':
      t = octave_value::uminus;
      break;

    case PLUS_PLUS:
      t = octave_value::incr;
      break;

    case MINUS_MINUS:
      t = octave_value::decr;
      break;

    default:
      panic_impossible ();
      break;
    }

  int l = tok_val->line ();
  int c = tok_val->column ();

  // XXX FIXME XXX -- what about constant folding here?

  return new tree_prefix_expression (op1, l, c, t);
}

// Build a postfix expression.

static tree_expression *
make_postfix_op (int op, tree_expression *op1, token *tok_val)
{
  octave_value::unary_op t = octave_value::unknown_unary_op;

  switch (op)
    {
    case QUOTE:
      t = octave_value::hermitian;
      break;

    case TRANSPOSE:
      t = octave_value::transpose;
      break;

    case PLUS_PLUS:
      t = octave_value::incr;
      break;

    case MINUS_MINUS:
      t = octave_value::decr;
      break;

    default:
      panic_impossible ();
      break;
    }

  int l = tok_val->line ();
  int c = tok_val->column ();

  // XXX FIXME XXX -- what about constant folding here?

  return new tree_postfix_expression (op1, l, c, t);
}

// Build an unwind-protect command.

static tree_command *
make_unwind_command (token *unwind_tok, tree_statement_list *body,
		     tree_statement_list *cleanup, token *end_tok)
{
  tree_command *retval = 0;

  if (end_token_ok (end_tok, token::unwind_protect_end))
    {
      int l = unwind_tok->line ();
      int c = unwind_tok->column ();

      retval = new tree_unwind_protect_command (body, cleanup, l, c);
    }

  return retval;
}

// Build a try-catch command.

static tree_command *
make_try_command (token *try_tok, tree_statement_list *body,
		  tree_statement_list *cleanup, token *end_tok)
{
  tree_command *retval = 0;

  if (end_token_ok (end_tok, token::try_catch_end))
    {
      int l = try_tok->line ();
      int c = try_tok->column ();

      retval = new tree_try_catch_command (body, cleanup, l, c);
    }

  return retval;
}

// Build a while command.

static tree_command *
make_while_command (token *while_tok, tree_expression *expr,
		    tree_statement_list *body, token *end_tok)
{
  tree_command *retval = 0;

  maybe_warn_assign_as_truth_value (expr);

  if (end_token_ok (end_tok, token::while_end))
    {
      lexer_flags.looping--;

      int l = while_tok->line ();
      int c = while_tok->column ();

      retval = new tree_while_command (expr, body, l, c);
    }

  return retval;
}

// Build a for command.

static tree_command *
make_for_command (token *for_tok, tree_argument_list *lhs,
		  tree_expression *expr, tree_statement_list *body,
		  token *end_tok)
{
  tree_command *retval = 0;

  if (end_token_ok (end_tok, token::for_end))
    {
      lexer_flags.looping--;

      int l = for_tok->line ();
      int c = for_tok->column ();

      if (lhs->length () == 1)
	{
	  tree_expression *tmp = lhs->remove_front ();

	  retval = new tree_simple_for_command (tmp, expr, body, l, c);

	  delete lhs;
	}
      else
	retval = new tree_complex_for_command (lhs, expr, body, l, c);
    }

  return retval;
}

// Build a break command.

static tree_command *
make_break_command (token *break_tok)
{
  tree_command *retval = 0;

  int l = break_tok->line ();
  int c = break_tok->column ();

  if (lexer_flags.looping || lexer_flags.defining_func || reading_script_file)
    retval = new tree_break_command (l, c);
  else
    retval = new tree_no_op_command ("break", l, c);

  return retval;
}

// Build a continue command.

static tree_command *
make_continue_command (token *continue_tok)
{
  tree_command *retval = 0;

  int l = continue_tok->line ();
  int c = continue_tok->column ();

  if (lexer_flags.looping)
    retval = new tree_continue_command (l, c);
  else
    retval = new tree_no_op_command ("continue", l, c);

  return retval;
}

// Build a return command.

static tree_command *
make_return_command (token *return_tok)
{
  tree_command *retval = 0;

  int l = return_tok->line ();
  int c = return_tok->column ();

  if (lexer_flags.defining_func || reading_script_file)
    retval = new tree_return_command (l, c);
  else
    retval = new tree_no_op_command ("return", l, c);

  return retval;
}

// Start an if command.

static tree_if_command_list *
start_if_command (tree_expression *expr, tree_statement_list *list)
{
  maybe_warn_assign_as_truth_value (expr);

  tree_if_clause *t = new tree_if_clause (expr, list);

  return new tree_if_command_list (t);
}

// Finish an if command.

static tree_if_command *
finish_if_command (token *if_tok, tree_if_command_list *list,
		   token *end_tok)
{
  tree_if_command *retval = 0;

  if (end_token_ok (end_tok, token::if_end))
    {
      int l = if_tok->line ();
      int c = if_tok->column ();

      retval = new tree_if_command (list, l, c);
    }

  return retval;
}

// Build an elseif clause.

static tree_if_clause *
make_elseif_clause (tree_expression *expr, tree_statement_list *list)
{
  maybe_warn_assign_as_truth_value (expr);

  return new tree_if_clause (expr, list);
}

// Finish a switch command.

static tree_switch_command *
finish_switch_command (token *switch_tok, tree_expression *expr,
		       tree_switch_case_list *list, token *end_tok)
{
  tree_switch_command *retval = 0;

  if (end_token_ok (end_tok, token::switch_end))
    {
      int l = switch_tok->line ();
      int c = switch_tok->column ();

      retval = new tree_switch_command (expr, list, l, c);
    }

  return retval;
}

// Build a switch case.

static tree_switch_case *
make_switch_case (tree_expression *expr, tree_statement_list *list)
{
  maybe_warn_variable_switch_label (expr);

  return new tree_switch_case (expr, list);
}

// Build an assignment to a variable.

static tree_expression *
make_assign_op (int op, tree_argument_list *lhs, token *eq_tok,
		tree_expression *rhs)
{
  tree_expression *retval = 0;

  octave_value::assign_op t = octave_value::unknown_assign_op;

  switch (op)
    {
    case '=':
      t = octave_value::asn_eq;
      break;

    case ADD_EQ:
      t = octave_value::add_eq;
      break;

    case SUB_EQ:
      t = octave_value::sub_eq;
      break;

    case MUL_EQ:
      t = octave_value::mul_eq;
      break;

    case DIV_EQ:
      t = octave_value::div_eq;
      break;

    case LEFTDIV_EQ:
      t = octave_value::ldiv_eq;
      break;

    case LSHIFT_EQ:
      t = octave_value::lshift_eq;
      break;

    case RSHIFT_EQ:
      t = octave_value::rshift_eq;
      break;

    case EMUL_EQ:
      t = octave_value::el_mul_eq;
      break;

    case EDIV_EQ:
      t = octave_value::el_div_eq;
      break;

    case ELEFTDIV_EQ:
      t = octave_value::el_ldiv_eq;
      break;

    case AND_EQ:
      t = octave_value::el_and_eq;
      break;

    case OR_EQ:
      t = octave_value::el_or_eq;
      break;

    default:
      panic_impossible ();
      break;
    }

  int l = eq_tok->line ();
  int c = eq_tok->column ();

  if (lhs->length () == 1)
    {
      tree_expression *tmp = lhs->remove_front ();

      retval = new tree_simple_assignment (tmp, rhs, false, l, c, t);

      delete lhs;
    }
  else
    return new tree_multi_assignment (lhs, rhs, false, l, c, t);

  return retval;
}

// Begin defining a function.

static octave_user_function *
start_function (tree_parameter_list *param_list, tree_statement_list *body)
{
  body->mark_as_function_body ();

  // We'll fill in the return list later.

  octave_user_function *fcn
    = new octave_user_function (param_list, 0, body, curr_sym_tab);

  return fcn;
}

// Do most of the work for defining a function.

static octave_user_function *
frob_function (tree_identifier *id, octave_user_function *fcn)
{
  string id_name = id->name ();

  // If input is coming from a file, issue a warning if the name of
  // the file does not match the name of the function stated in the
  // file.  Matlab doesn't provide a diagnostic (it ignores the stated
  // name).

  fcn->stash_function_name (id_name);

  if (reading_fcn_file)
    {
      if (curr_fcn_file_name != id_name)
	{
	  if (Vwarn_function_name_clash)
	    warning ("function name `%s' does not agree with function\
 file name `%s'", id_name.c_str (), curr_fcn_file_full_name.c_str ());

	  global_sym_tab->rename (id_name, curr_fcn_file_name);

	  if (error_state)
	    return 0;

	  id_name = id->name ();
	}

      time_t now = time (0);

      fcn->stash_function_name (id_name);
      fcn->stash_fcn_file_name ();
      fcn->stash_fcn_file_time (now);
      fcn->mark_as_system_fcn_file ();

      if (Vwarn_future_time_stamp)
	{
	  string nm = fcn->fcn_file_name ();

	  file_stat fs (nm);

	  if (fs && fs.is_newer (now))
	    warning ("time stamp for `%s' is in the future", nm.c_str ());
	}
    }
  else if (! (input_from_tmp_history_file || input_from_startup_file)
	   && reading_script_file
	   && curr_fcn_file_name == id_name)
    {
      warning ("function `%s' defined within script file `%s'",
	       id_name.c_str (), curr_fcn_file_full_name.c_str ());
    }

  top_level_sym_tab->clear (id_name);

  symbol_record *sr = global_sym_tab->lookup (id_name);

  if (sr)
    fcn->stash_symtab_ptr (sr);
  else
    panic_impossible ();

  id->define (fcn, symbol_record::USER_FUNCTION);

  id->document (help_buf);

  return fcn;
}

// Finish defining a function.

static octave_user_function *
finish_function (tree_identifier *id, octave_user_function *fcn)
{
  tree_parameter_list *tpl = new tree_parameter_list (id);

  tpl->mark_as_formal_parameters ();

  return fcn->define_ret_list (tpl);
}

// Finish defining a function a different way.

static octave_user_function *
finish_function (tree_parameter_list *ret_list, octave_user_function *fcn)
{
  ret_list->mark_as_formal_parameters ();

  return fcn->define_ret_list (ret_list);
}

static void
recover_from_parsing_function (void)
{
  curr_sym_tab = top_level_sym_tab;

  lexer_flags.defining_func = false;
  lexer_flags.beginning_of_function = false;
  lexer_flags.parsed_function_name = false;
  lexer_flags.looking_at_return_list = false;
  lexer_flags.looking_at_parameter_list = false;
}

// Make an index expression.

static tree_index_expression *
make_index_expression (tree_expression *expr, tree_argument_list *args)
{
  tree_index_expression *retval = 0;

  int l = expr->line ();
  int c = expr->column ();

  expr->mark_postfix_indexed ();

  retval =  new tree_index_expression (expr, args, l, c);

  return retval;
}

// Make an indirect reference expression.

static tree_indirect_ref *
make_indirect_ref (tree_expression *expr, const string& elt)
{
  tree_indirect_ref *retval = 0;

  int l = expr->line ();
  int c = expr->column ();

  retval = new tree_indirect_ref (expr, elt, l, c);

  lexer_flags.looking_at_indirect_ref = false;

  return retval;
}

// Make a declaration command.

static tree_decl_command *
make_decl_command (int tok, token *tok_val, tree_decl_init_list *lst)
{
  tree_decl_command *retval = 0;

  int l = tok_val->line ();
  int c = tok_val->column ();

  switch (tok)
    {
    case GLOBAL:
      retval = new tree_global_command (lst, l, c);
      break;

    case STATIC:
      if (lexer_flags.defining_func)
	retval = new tree_static_command (lst, l, c);
      else
	{
	  if (reading_script_file)
	    warning ("ignoring static declaration near line %d of file `%s'",
		     l, curr_fcn_file_full_name.c_str ());
	  else
	    warning ("ignoring static declaration near line %d", l);
	}
      break;

    default:
      panic_impossible ();
      break;
    }

  return retval;
}

// Finish building a matrix list.

static tree_expression *
finish_matrix (tree_matrix *m)
{
  tree_expression *retval = m;

  unwind_protect::begin_frame ("finish_matrix");

  unwind_protect_int (error_state);

  unwind_protect_bool (buffer_error_messages);
  buffer_error_messages = true;

  unwind_protect::add (clear_global_error_variable, 0);

  if (m->all_elements_are_constant ())
    {
      octave_value tmp = m->rvalue ();

      if (! error_state)
	{
	  tree_constant *tc_retval = new tree_constant (tmp);

	  ostrstream buf;

	  tree_print_code tpc (buf);

	  m->accept (tpc);

	  buf << ends;

	  char *s = buf.str ();

	  tc_retval->stash_original_text (s);

	  delete [] s;

	  delete m;

	  retval = tc_retval;
	}
    }

  unwind_protect::run_frame ("finish_matrix");

  return retval;
}

static void
maybe_warn_missing_semi (tree_statement_list *t)
{
  if (lexer_flags.defining_func && Vwarn_missing_semicolon)
    {
      tree_statement *tmp = t->rear();

      if (tmp->is_expression ())
	warning ("missing semicolon near line %d, column %d in file `%s'",
		 tmp->line (), tmp->column (),
		 curr_fcn_file_full_name.c_str ());
    }
}

static void
set_stmt_print_flag (tree_statement_list *list, char sep,
		     bool warn_missing_semi)
{
  switch (sep)
    {
    case ';':
      {
	tree_statement *tmp = list->rear ();
	tmp->set_print_flag (0);
      }
      break;

    case 0:
    case ',':
    case '\n':
      if (warn_missing_semi)
	maybe_warn_missing_semi (list);
      break;

    default:
      warning ("unrecognized separator type!");
      break;
    }
}

void
parse_and_execute (FILE *f)
{
  unwind_protect::begin_frame ("parse_and_execute");
  
  YY_BUFFER_STATE old_buf = current_buffer ();
  YY_BUFFER_STATE new_buf = create_buffer (f);

  unwind_protect::add (restore_input_buffer, old_buf);
  unwind_protect::add (delete_input_buffer, new_buf);

  switch_to_buffer (new_buf);

  unwind_protect_bool (line_editing);
  unwind_protect_bool (input_from_command_line_file);

  line_editing = false;
  input_from_command_line_file = false;

  unwind_protect_ptr (curr_sym_tab);

  int retval;
  do
    {
      reset_parser ();

      retval = yyparse ();

      if (retval == 0 && global_command)
	{
	  global_command->eval ();

	  delete global_command;

	  global_command = 0;

	  bool quit = (tree_return_command::returning
		       || tree_break_command::breaking);

	  if (tree_return_command::returning)
	    tree_return_command::returning = 0;

	  if (tree_break_command::breaking)
	    tree_break_command::breaking--;

	  if (error_state)
	    {
	      error ("near line %d of file `%s'", input_line_number,
		     curr_fcn_file_full_name.c_str ());

	      break;
	    }

	  if (quit)
	    break;
	}
    }
  while (retval == 0);

  unwind_protect::run_frame ("parse_and_execute");
}

static void
safe_fclose (void *f)
{
  if (f)
    fclose (static_cast<FILE *> (f));
}

void
parse_and_execute (const string& s, bool verbose, const char *warn_for)
{
  unwind_protect::begin_frame ("parse_and_execute_2");

  unwind_protect_bool (reading_script_file);
  unwind_protect_str (curr_fcn_file_full_name);

  reading_script_file = true;
  curr_fcn_file_full_name = s;

  FILE *f = get_input_from_file (s, 0);

  if (f)
    {
      unwind_protect::add (safe_fclose, f);

      unwind_protect_int (input_line_number);
      unwind_protect_int (current_input_column);

      input_line_number = 0;
      current_input_column = 1;

      if (verbose)
	{
	  cout << "reading commands from " << s << " ... ";
	  reading_startup_message_printed = true;
	  cout.flush ();
	}

      parse_and_execute (f);

      if (verbose)
	cout << "done." << endl;
    }
  else if (warn_for)
    error ("%s: unable to open file `%s'", warn_for, s.c_str ());

  unwind_protect::run_frame ("parse_and_execute_2");
}

static bool
looks_like_octave_copyright (const string& s)
{
  bool retval = false;

  string t = s.substr (0, 15);

  if (t == " Copyright (C) ")
    {
      size_t pos = s.find ('\n');

      if (pos != NPOS)
	{
	  pos = s.find ('\n', pos + 1);

	  if (pos != NPOS)
	    {
	      pos++;

	      t = s.substr (pos, 29);

	      if (t == " This file is part of Octave."
		  || t == " This program is free softwar")
		retval = true;
	    }
	}
    }

  return retval;
}

// Eat whitespace and comments from FFILE, returning the text of the
// comments read if it doesn't look like a copyright notice.  If
// IN_PARTS, consider each block of comments separately; otherwise,
// grab them all at once.  If UPDATE_POS is TRUE, line and column
// number information is updated.

// XXX FIXME XXX -- grab_help_text() in lex.l duplicates some of this
// code!

static string
gobble_leading_white_space (FILE *ffile, bool in_parts, bool update_pos)
{
  string help_txt;

  bool first_comments_seen = false;
  bool begin_comment = false;
  bool have_help_text = false;
  bool in_comment = false;
  int c;

  while ((c = getc (ffile)) != EOF)
    {
      if (update_pos)
	current_input_column++;

      if (begin_comment)
	{
	  if (c == '%' || c == '#')
	    continue;
	  else
	    begin_comment = false;
	}

      if (in_comment)
	{
	  if (! have_help_text)
	    {
	      first_comments_seen = true;
	      help_txt += (char) c;
	    }

	  if (c == '\n')
	    {
	      if (update_pos)
		{
		  input_line_number++;
		  current_input_column = 0;
		}
	      in_comment = false;

	      if (in_parts)
		{
		  if ((c = getc (ffile)) != EOF)
		    {
		      if (update_pos)
			current_input_column--;
		      ungetc (c, ffile);
		      if (c == '\n')
			break;
		    }
		  else
		    break;
		}
	    }
	}
      else
	{
	  switch (c)
	    {
	    case ' ':
	    case '\t':
	      if (first_comments_seen)
		have_help_text = true;
	      break;

	    case '\n':
	      if (first_comments_seen)
		have_help_text = true;
	      if (update_pos)
		{
		  input_line_number++;
		  current_input_column = 0;
		}
	      continue;

	    case '%':
	    case '#':
	      begin_comment = true;
	      in_comment = true;
	      break;

	    default:
	      if (update_pos)
		current_input_column--;
	      ungetc (c, ffile);
	      goto done;
	    }
	}
    }

 done:

  if (! help_txt.empty ())
    {
      if (looks_like_octave_copyright (help_txt)) 
	help_txt.resize (0);

      if (in_parts && help_txt.empty ())
	help_txt = gobble_leading_white_space (ffile, in_parts, update_pos);
    }

  return help_txt;
}

string
get_help_from_file (const string& path)
{
  string retval;

  if (! path.empty ())
    {
      FILE *fptr = fopen (path.c_str (), "r");

      if (fptr)
	{
	  unwind_protect::add (safe_fclose, (void *) fptr);

	  retval = gobble_leading_white_space (fptr, true, true);

	  unwind_protect::run ();
	}
    }

  return retval;
}

static int
is_function_file (FILE *ffile)
{
  int status = 0;

  long pos = ftell (ffile);

  gobble_leading_white_space (ffile, false, false);

  char buf [10];
  fgets (buf, 10, ffile);
  int len = strlen (buf);
  if (len > 8 && strncmp (buf, "function", 8) == 0
      && ! (isalnum (buf[8]) || buf[8] == '_'))
    status = 1;

  fseek (ffile, pos, SEEK_SET);

  return status;
}

static void
restore_command_history (void *)
{
  command_history::ignore_entries (! Vsaving_history);
}

static void
restore_input_stream (void *f)
{
  command_editor::set_input_stream (static_cast<FILE *> (f));
}

static void
clear_current_script_file_name (void *)
{
  bind_builtin_variable ("current_script_file_name", octave_value ());
}

static bool
parse_fcn_file (const string& ff, bool exec_script, bool force_script = false)
{
  unwind_protect::begin_frame ("parse_fcn_file");

  int script_file_executed = false;

  // Open function file and parse.

  bool old_reading_fcn_file_state = reading_fcn_file;

  FILE *in_stream = command_editor::get_input_stream ();

  unwind_protect::add (restore_input_stream, in_stream);

  unwind_protect_ptr (ff_instream);

  unwind_protect_int (input_line_number);
  unwind_protect_int (current_input_column);
  unwind_protect_bool (reading_fcn_file);
  unwind_protect_bool (line_editing);

  input_line_number = 0;
  current_input_column = 1;
  reading_fcn_file = true;
  line_editing = false;

  FILE *ffile = get_input_from_file (ff, 0);

  unwind_protect::add (safe_fclose, ffile);

  if (ffile)
    {
      // Check to see if this file defines a function or is just a
      // list of commands.

      if (! force_script && is_function_file (ffile))
	{
	  // XXX FIXME XXX -- we shouldn't need both the
	  // command_history object and the
	  // Vsaving_history variable...
	  command_history::ignore_entries ();

	  unwind_protect::add (restore_command_history, 0);

	  unwind_protect_int (Vecho_executing_commands);
	  unwind_protect_bool (Vsaving_history);
	  unwind_protect_bool (reading_fcn_file);
	  unwind_protect_bool (input_from_command_line_file);

	  Vecho_executing_commands = ECHO_OFF;
	  Vsaving_history = false;
	  reading_fcn_file = true;
	  input_from_command_line_file = false;

	  YY_BUFFER_STATE old_buf = current_buffer ();
	  YY_BUFFER_STATE new_buf = create_buffer (ffile);

	  unwind_protect::add (restore_input_buffer, (void *) old_buf);
	  unwind_protect::add (delete_input_buffer, (void *) new_buf);

	  switch_to_buffer (new_buf);

	  unwind_protect_ptr (curr_sym_tab);

	  reset_parser ();

	  help_buf = gobble_leading_white_space (ffile, true, true);

	  // XXX FIXME XXX -- this should not be necessary.
	  gobble_leading_white_space (ffile, false, true);

	  int status = yyparse ();

	  if (status != 0)
	    {
	      error ("parse error while reading function file %s",
		     ff.c_str ());
	      global_sym_tab->clear (curr_fcn_file_name);
	    }
	}
      else if (exec_script)
	{
	  // The value of `reading_fcn_file' will be restored to the
	  // proper value when we unwind from this frame.
	  reading_fcn_file = old_reading_fcn_file_state;

	  // XXX FIXME XXX -- we shouldn't need both the
	  // command_history object and the
	  // Vsaving_history variable...
	  command_history::ignore_entries ();

	  unwind_protect::add (restore_command_history, 0);

	  unwind_protect_bool (Vsaving_history);
	  unwind_protect_bool (reading_script_file);

	  Vsaving_history = false;
	  reading_script_file = true;

	  unwind_protect::add (clear_current_script_file_name, 0);

	  bind_builtin_variable ("current_script_file_name", ff);

	  parse_and_execute (ffile);

	  script_file_executed = true;
	}
    }

  unwind_protect::run_frame ("parse_fcn_file");

  return script_file_executed;
}

bool
load_fcn_from_file (symbol_record *sym_rec, bool exec_script)
{
  bool script_file_executed = false;

  string nm = sym_rec->name ();

  if (octave_dynamic_loader::load_fcn_from_dot_oct_file (nm))
    {
      force_link_to_function (nm);
    }
  else
    {
      string ff = fcn_file_in_path (nm);

      // These are needed by yyparse.

      unwind_protect::begin_frame ("load_fcn_from_file");

      unwind_protect_str (curr_fcn_file_name);
      unwind_protect_str (curr_fcn_file_full_name);

      curr_fcn_file_name = nm;
      curr_fcn_file_full_name = ff;

      if (ff.length () > 0)
	script_file_executed = parse_fcn_file (ff, exec_script);

      if (! (error_state || script_file_executed))
	force_link_to_function (nm);

      unwind_protect::run_frame ("load_fcn_from_file");
    }

  return script_file_executed;
}

DEFUN (source, args, ,
  "source (FILE)\n\
\n\
Parse and execute the contents of FILE.  Like executing commands in a\n\
script file but without requiring the file to be named `FILE.m'.")
{
  octave_value_list retval;

  int nargin = args.length ();

  if (nargin == 1)
    {
      string file = args(0).string_value ();

      if (! error_state)
	{
	  file = file_ops::tilde_expand (file);

	  parse_fcn_file (file, true, true);

	  if (error_state)
	    error ("source: error sourcing file `%s'", file.c_str ());
	}
      else
	error ("source: expecting file name as argument");
    }
  else
    print_usage ("source");

  return retval;
}

octave_value_list
feval (const string& name, const octave_value_list& args, int nargout)
{
  octave_value_list retval;

  octave_function *fcn = is_valid_function (name, "feval", 1);

  if (fcn)
    retval = fcn->do_index_op (nargout, args);

  return retval;
}

octave_value_list
feval (const octave_value_list& args, int nargout)
{
  octave_value_list retval;

  if (args.length () > 0)
    {
      string name = args(0).string_value ();

      if (! error_state)
	{
	  int tmp_nargin = args.length () - 1;

	  octave_value_list tmp_args (tmp_nargin, octave_value ());

	  for (int i = 0; i < tmp_nargin; i++)
	    tmp_args(i) = args(i+1);

	  string_vector arg_names = args.name_tags ();

	  if (! arg_names.empty ())
	    {
	      assert (arg_names.length () == tmp_nargin + 1);

	      string_vector tmp_arg_names (tmp_nargin);

	      for (int i = 0; i < tmp_nargin; i++)
		tmp_arg_names(i) = arg_names(i+1);

	      tmp_args.stash_name_tags (tmp_arg_names);
	    }

	  retval = feval (name, tmp_args, nargout);
	}
    }

  return retval;
}

DEFUN (feval, args, nargout,
  "feval (NAME, ARGS, ...)\n\
\n\
evaluate NAME as a function, passing ARGS as its arguments")
{
  octave_value_list retval;

  int nargin = args.length ();

  if (nargin > 0)
    retval = feval (args, nargout);
  else
    print_usage ("feval");

  return retval;
}

octave_value_list
eval_string (const string& s, bool silent, int& parse_status, int nargout)
{
  unwind_protect::begin_frame ("eval_string");

  unwind_protect_bool (get_input_from_eval_string);
  unwind_protect_bool (input_from_command_line_file);
  unwind_protect_ptr (global_command);
  unwind_protect_str (current_eval_string);

  get_input_from_eval_string = true;
  input_from_command_line_file = false;
  current_eval_string = s;

  YY_BUFFER_STATE old_buf = current_buffer ();
  YY_BUFFER_STATE new_buf = create_buffer (0);

  unwind_protect::add (restore_input_buffer, old_buf);
  unwind_protect::add (delete_input_buffer, new_buf);

  switch_to_buffer (new_buf);

  unwind_protect_ptr (curr_sym_tab);

  reset_parser ();

  parse_status = yyparse ();

  // Important to reset the idea of where input is coming from before
  // trying to eval the command we just parsed -- it might contain the
  // name of an function file that still needs to be parsed!

  tree_statement_list *command = global_command;

  unwind_protect::run_frame ("eval_string");

  octave_value_list retval;

  if (parse_status == 0 && command)
    {
      retval = command->eval (silent, nargout);
      delete command;
    }

  return retval;
}

octave_value
eval_string (const string& s, bool silent, int& parse_status)
{
  octave_value retval;

  octave_value_list tmp = eval_string (s, silent, parse_status, 1);

  if (! tmp.empty ())
    retval = tmp(0);

  return retval;
}

static octave_value_list
eval_string (const octave_value& arg, bool silent, int& parse_status,
	     int nargout)
{
  string s = arg.string_value ();

  if (error_state)
    {
      error ("eval: expecting string argument");
      return -1.0;
    }

  return eval_string (s, silent, parse_status, nargout);
}

DEFUN (eval, args, nargout,
  "eval (TRY, CATCH)\n\
\n\
Evaluate the string TRY as octave code.  If that fails, evaluate the\n\
string CATCH.")
{
  octave_value_list retval;

  int nargin = args.length ();

  if (nargin > 0)
    {
      unwind_protect::begin_frame ("Feval");

      if (nargin > 1)
	{
	  unwind_protect_bool (buffer_error_messages);
	  buffer_error_messages = true;
	}

      int parse_status = 0;

      retval = eval_string (args(0), ! Vdefault_eval_print_flag,
			    parse_status, nargout);

      if (nargin > 1 && (parse_status != 0 || error_state))
	{
	  error_state = 0;

	  // Set up for letting the user print any messages from
	  // errors that occurred in the first part of this eval().

	  buffer_error_messages = false;
	  bind_global_error_variable ();
	  unwind_protect::add (clear_global_error_variable, 0);

	  eval_string (args(1), 0, parse_status, nargout);

	  retval = octave_value_list ();
	}

      unwind_protect::run_frame ("Feval");
    }
  else
    print_usage ("eval");

  return retval;
}

static int
default_eval_print_flag (void)
{
  Vdefault_eval_print_flag = check_preference ("default_eval_print_flag");

  return 0;
}

static int
warn_assign_as_truth_value (void)
{
  Vwarn_assign_as_truth_value
    = check_preference ("warn_assign_as_truth_value");

  return 0;
}

static int
warn_function_name_clash (void)
{
  Vwarn_function_name_clash = check_preference ("warn_function_name_clash");

  return 0;
}

static int
warn_future_time_stamp (void)
{
  Vwarn_future_time_stamp = check_preference ("warn_future_time_stamp");

  return 0;
}

static int
warn_missing_semicolon (void)
{
  Vwarn_missing_semicolon = check_preference ("warn_missing_semicolon");

  return 0;
}

static int
warn_variable_switch_label (void)
{
  Vwarn_variable_switch_label
    = check_preference ("warn_variable_switch_label");

  return 0;
}

void
symbols_of_parse (void)
{
  DEFVAR (default_eval_print_flag, 1.0, default_eval_print_flag,
    "If the value of this variable is nonzero, Octave will print the\n\
results of commands executed by eval() that do not end with semicolons.");

  DEFVAR (warn_assign_as_truth_value, 1.0, warn_assign_as_truth_value,
    "produce warning for assignments used as truth values");

  DEFVAR (warn_function_name_clash, 1.0, warn_function_name_clash,
    "produce warning if function name conflicts with file name");

  DEFVAR (warn_future_time_stamp, 1.0, warn_future_time_stamp,
    "warn if a function file has a time stamp that is in the future");

  DEFVAR (warn_missing_semicolon, 0.0, warn_missing_semicolon,
    "produce a warning if a statement in a function file is not\n\
terminated with a semicolon");

  DEFVAR (warn_variable_switch_label, 0.0, warn_variable_switch_label,
    "produce warning for variables used as switch labels");
}

/*
;;; Local Variables: ***
;;; mode: text ***
;;; End: ***
*/
