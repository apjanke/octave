/* tex-make.c: Run external programs to make TeX-related files.

Copyright (C) 1993, 94, 95, 96, 97 Karl Berry.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include <kpathsea/config.h>

#include <kpathsea/c-fopen.h>
#include <kpathsea/c-pathch.h>
#include <kpathsea/concatn.h>
#include <kpathsea/db.h>
#include <kpathsea/fn.h>
#include <kpathsea/magstep.h>
#include <kpathsea/readable.h>
#include <kpathsea/tex-make.h>
#include <kpathsea/variable.h>


/* We never throw away stdout, since that is supposed to be the filename
   found, if all is successful.  This variable controls whether stderr
   is thrown away.  */
boolean kpse_make_tex_discard_errors = false;

/* We set the envvar MAKETEX_MAG, which is part of the default spec for
   MakeTeXPK above, based on KPATHSEA_DPI and MAKETEX_BASE_DPI.  */

static void
set_maketex_mag P1H(void)
{
  char q[MAX_INT_LENGTH * 3 + 3];
  int m;
  string dpi_str = getenv ("KPATHSEA_DPI");
  string bdpi_str = getenv ("MAKETEX_BASE_DPI");
  unsigned dpi = dpi_str ? atoi (dpi_str) : 0;
  unsigned bdpi = bdpi_str ? atoi (bdpi_str) : 0;

  /* If the environment variables aren't set, it's a bug.  */
  assert (dpi != 0 && bdpi != 0);
  
  /* Fix up for roundoff error.  Hopefully the driver has already fixed
     up DPI, but may as well be safe, and also get the magstep number.  */
  (void) kpse_magstep_fix (dpi, bdpi, &m);
  
  /* Have to do something different for DOS?  */
  if (m == 0)
    sprintf (q, "%d+%d/%d", dpi / bdpi, dpi % bdpi, bdpi);
  else
    { /* m is encoded with LSB being a ``half'' bit (see magstep.h).  Are
         we making an assumption here about two's complement?  Probably.
         In any case, if m is negative, we have to put in the sign
         explicitly, since m/2==0 if m==-1.  */
      const_string sign = "";
      if (m < 0)
        {
          m *= -1;
          sign = "-";
        }
      sprintf (q, "magstep\\(%s%d.%d\\)", sign, m / 2, (m & 1) * 5);
    }  
  xputenv ("MAKETEX_MAG", q);
}

/* This MakeTeX... program was disabled, or the script failed.  If this
   was a font creation (according to FORMAT), append CMD
   to a file missfont.log in the current directory.  */

static void
misstex P2C(kpse_file_format_type, format,  const_string, cmd)
{
  static FILE *missfont = NULL;

  /* If we weren't trying to make a font, do nothing.  Maybe should
     allow people to specify what they want recorded?  */
  if (format > kpse_any_glyph_format && format != kpse_tfm_format
      && format != kpse_vf_format)
    return;

  /* If this is the first time, have to open the log file.  But don't
     bother logging anything if they were discarding errors.  */
  if (!missfont && !kpse_make_tex_discard_errors) {
    const_string missfont_name = kpse_var_value ("MISSFONT_LOG");
    if (!missfont_name || *missfont_name == '1') {
      missfont_name = "missfont.log"; /* take default name */
    } else if (missfont_name
               && (*missfont_name == 0 || *missfont_name == '0')) {
      missfont_name = NULL; /* user requested no missfont.log */
    } /* else use user's name */

    missfont = missfont_name ? fopen (missfont_name, FOPEN_A_MODE) : NULL;
    if (!missfont && kpse_var_value ("TEXMFOUTPUT")) {
      missfont_name = concat3 (kpse_var_value ("TEXMFOUTPUT"), DIR_SEP_STRING,
                               missfont_name);
      missfont = fopen (missfont_name, FOPEN_A_MODE);
    }

    if (missfont)
      fprintf (stderr, "kpathsea: Appending font creation commands to %s.\n",
               missfont_name);
  }
  
  /* Write the command if we have a log file.  */
  if (missfont) {
    fputs (cmd, missfont);
    putc ('\n', missfont);
  }
}  


/* Assume the script outputs the filename it creates (and nothing
   else) on standard output; hence, we run the script with `popen'.  */

static string
maketex P2C(kpse_file_format_type, format,  const_string, passed_cmd)
{
  string ret;
  unsigned i;
  FILE *f;
  string cmd = xstrdup (passed_cmd);
  
  /* If the user snuck `backquotes` or $(command) substitutions into the
     name, foil them.  */
  for (i = 0; i < strlen (cmd); i++) {
    if (cmd[i] == '`' || (cmd[i] == '$' && cmd[i+1] == '(')) {
      cmd[i] = '#';
    }
  }

  /* Tell the user we are running the script, so they have a clue as to
     what's going on if something messes up.  But if they asked to
     discard output, they probably don't want to see this, either.  */
  if (!kpse_make_tex_discard_errors) {
    fprintf (stderr, "kpathsea: Running %s\n", cmd);
  }
  
  /* Run the script.  The Amiga has a different interface.  */
#ifdef AMIGA
  ret = system (cmd) == 0 ? getenv ("LAST_FONT_CREATED") : NULL;
#else /* not AMIGA */
  f = popen (cmd, FOPEN_R_MODE);

  if (f) {
    int c;
    string fn;             /* The final filename.  */
    unsigned len;          /* And its length.  */
    fn_type output;
    output = fn_init ();   /* Collect the script output.  */

    /* Read all the output and terminate with a null.  */
    while ((c = getc (f)) != EOF)
      fn_1grow (&output, c);
    fn_1grow (&output, 0);

    /* Maybe should check for `EXIT_SUCCESS' status before even
       looking at the output?  In some versions of Linux, pclose fails
       with ECHILD (No child processes), maybe only if we're being run
       by lpd.  So don't make this a fatal error.  */
    if (pclose (f) == -1) {
      perror ("pclose(MakeTeXPK)");
      WARNING ("kpathsea: This is probably the Linux pclose bug; continuing");
    }

    len = FN_LENGTH (output);
    fn = FN_STRING (output);

    /* Remove trailing newlines and returns.  */
    while (len > 1 && (fn[len - 2] == '\n' || fn[len - 2] == '\r')) {
      fn[len - 2] = 0;
      len--;
    }

    /* If no output from script, return NULL.  Otherwise check
       what it output.  */
    ret = len == 1 ? NULL : kpse_readable_file (fn);
    if (!ret && len > 1) {
      WARNING1 ("kpathsea: MakeTeXPK output `%s' instead of a filename", fn);
    }

    /* Free the name if we're not returning it.  */
    if (fn != ret)
      free (fn);
  } else {
    /* popen failed.  */
    perror ("kpathsea");
    ret = NULL;
  }
#endif /* not AMIGA */

  if (ret == NULL)
    misstex (format, cmd);
  else
    kpse_db_insert (ret);
    
  return ret;
}


/* Create BASE in FORMAT and return the generated filename, or
   return NULL.  */

string
kpse_make_tex P2C(kpse_file_format_type, format,  const_string, base)
{
  kpse_format_info_type spec; /* some compilers lack struct initialization */
  string ret = NULL;
  
  spec = kpse_format_info[format];
  if (!spec.type) { /* Not initialized yet? */
    kpse_init_format (format);
    spec = kpse_format_info[format];
  }
  
  if (spec.program && spec.program_enabled_p) {
    /* See the documentation for the envvars we're dealing with here.  */
    string args, cmd;
    const_string prog = spec.program;
    const_string arg_spec = spec.program_args;
    string mode = getenv ("MAKETEX_MODE");
    boolean unset_mode = false;

    if (format <= kpse_any_glyph_format)
      set_maketex_mag ();

    /* Here's an awful kludge: if the mode is `/', unset it for the call
       and then reset it.  We could ignore a mode of / in MakeTeXPK, but
       then everyone's MakeTeXPK would have to handle that special case,
       which seems too onerous.  `kpse_prog_init' sets it to this in the
       first place when no mode is otherwise specified; this is so when
       the user defines a resolution, they don't also have to specify a
       mode; instead, MakeTeXPK's guesses will take over.  They use /
       for the value because then when it is expanded as part of the
       PKFONTS et al. path values, we'll wind up searching all the pk
       directories.  We put $MAKETEX_MODE in the path values in the
       first place so that sites with two different devices with the
       same resolution can find the right fonts; but such sites are
       uncommon, so they shouldn't make things harder for everyone else.  */
    if (mode && STREQ (mode, DIR_SEP_STRING)) {
      xputenv ("MAKETEX_MODE", "");
      unset_mode = true;
    }
    args = arg_spec ? kpse_var_expand (arg_spec) : (string) "";
    if (unset_mode)
      xputenv ("MAKETEX_MODE", DIR_SEP_STRING);

    /* The command is the program name plus the arguments.  */
    cmd = concatn (prog, " ", base, " ", args, NULL);

    /* Only way to discard errors is redirect stderr inside another
       shell; otherwise, if the MakeTeX... script doesn't exist, we
       will see the `sh: MakeTeX...: not found' error.  No point in
       doing this if we're not actually going to run anything.  */
    if (kpse_make_tex_discard_errors) {
      string old_cmd = cmd;
#ifdef OS2
      cmd = concat3 ("cmd /c \"", cmd, "\" 2>/dev/nul");
#else
      cmd = concat3 ("sh -c \"", cmd, "\" 2>/dev/null");
#endif
      free (old_cmd);
    }

    ret = maketex (format, cmd);

    free (cmd);
    if (*args)
      free (args);
  }  

  return ret;
}

#ifdef TEST

void
test_make_tex (kpse_file_format_type fmt, const_string base)
{
  string answer;
  
  printf ("\nAttempting %s in format %d:\n", base, fmt);

  answer = kpse_make_tex (fmt, base);
  puts (answer ? answer : "(nil)");
}


int
main ()
{
  xputenv ("KPATHSEA_DPI", "781"); /* call MakeTeXPK */
  xputenv ("MAKETEX_BASE_DPI", "300"); /* call MakeTeXPK */
  KPSE_MAKE_SPEC_ENABLED (kpse_make_specs[kpse_pk_format]) = true;
  test_make_tex (kpse_pk_format, "cmr10");

  /* Fail with MakeTeXTFM.  */
  KPSE_MAKE_SPEC_ENABLED (kpse_make_specs[kpse_tfm_format]) = true;
  test_make_tex (kpse_tfm_format, "foozler99");
  
  /* Call something disabled.  */
  test_make_tex (kpse_bst_format, "no-way");
  
  return 0;
}

#endif /* TEST */


/*
Local variables:
test-compile-command: "gcc -g -I. -I.. -DTEST tex-make.c kpathsea.a"
End:
*/
