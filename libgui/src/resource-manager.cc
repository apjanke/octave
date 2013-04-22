/*

Copyright (C) 2011-2012 Jacob Dawid

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>

#include <QFile>
#include <QDir>
#include <QNetworkProxy>
 #include <QLibraryInfo>

#include "error.h"
#include "file-ops.h"
#include "oct-env.h"
#include "singleton-cleanup.h"

#include "defaults.h"

#include "resource-manager.h"

resource_manager *resource_manager::instance = 0;

resource_manager::resource_manager (void)
  : settings (0), home_path (), first_run (false)
{
  do_reload_settings ();
}

resource_manager::~resource_manager (void)
{
  delete settings;
}


QString
resource_manager::get_gui_translation_dir (void)
{
  // get environment variable for the locale dir (e.g. from run-octave)
  std::string dldir = octave_env::getenv ("OCTAVE_LOCALE_DIR");
  if (dldir.empty ())
    dldir = Voct_locale_dir; // env-var empty, load the default location
  return QString::fromStdString (dldir);
}

void
resource_manager::config_translators (QTranslator *qt_tr,QTranslator *gui_tr)
{
  QSettings *settings = resource_manager::get_settings ();
  // FIXME -- what should happen if settings is 0?
  // get the locale from the settings
  QString language = settings->value ("language","SYSTEM").toString ();
  if (language == "SYSTEM")
      language = QLocale::system().name();    // get system wide locale
  // load the translator file for qt strings
  qt_tr->load("qt_" + language,
              QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  // load the translator file for gui strings
  gui_tr->load (language, get_gui_translation_dir ());
}

bool
resource_manager::instance_ok (void)
{
  bool retval = true;

  if (! instance)
    {
      instance = new resource_manager ();

      if (instance)
        singleton_cleanup_list::add (cleanup_instance);
    }

  if (! instance)
    {
      ::error ("unable to create resource_manager object!");

      retval = false;
    }

  return retval;
}

QSettings *
resource_manager::do_get_settings (void)
{
  return settings;
}

QString
resource_manager::do_get_home_path (void)
{
  return home_path;
}

static std::string
default_qt_settings_file (void)
{
  std::string dsf = octave_env::getenv ("OCTAVE_DEFAULT_QT_SETTINGS");

  if (dsf.empty ())
    dsf = Voct_etc_dir + file_ops::dir_sep_str () + "default-qt-settings";

  return dsf;
}

void
resource_manager::do_reload_settings (void)
{
  QDesktopServices desktopServices;
  home_path = desktopServices.storageLocation (QDesktopServices::HomeLocation);
  QString settings_path = home_path + "/.config/octave/";
  QString settings_file = settings_path + "qt-settings";

  if (!QFile::exists (settings_file))
    {
      QDir("/").mkpath (settings_path);
      QFile::copy (QString::fromStdString (default_qt_settings_file ()),
                   settings_file);
      first_run = true;
    }
  else
    first_run = false;

  do_set_settings (settings_file);
}

void
resource_manager::do_set_settings (const QString& file)
{
  delete settings;
  settings = new QSettings (file, QSettings::IniFormat);
}

bool
resource_manager::do_is_first_run (void)
{
  return first_run;
}

void
resource_manager::do_update_network_settings (void)
{
  QNetworkProxy::ProxyType proxyType = QNetworkProxy::NoProxy;

  if (settings->value ("useProxyServer",false).toBool ())
    {
      QString proxyTypeString = settings->value ("proxyType").toString ();

      if (proxyTypeString == "Socks5Proxy")
        proxyType = QNetworkProxy::Socks5Proxy;
      else if (proxyTypeString == "HttpProxy")
        proxyType = QNetworkProxy::HttpProxy;
    }

  QNetworkProxy proxy;

  proxy.setType (proxyType);
  proxy.setHostName (settings->value ("proxyHostName").toString ());
  proxy.setPort (settings->value ("proxyPort",80).toInt ());
  proxy.setUser (settings->value ("proxyUserName").toString ());
  proxy.setPassword (settings->value ("proxyPassword").toString ());

  QNetworkProxy::setApplicationProxy (proxy);
}

const char*
resource_manager::octave_keywords (void)
{
  return
      ".nargin. "
      "EDITOR "
      "EXEC_PATH "
      "F_DUPFD "
      "F_GETFD "
      "F_GETFL "
      "F_SETFD "
      "F_SETFL "
      "I "
      "IMAGE_PATH "
      "Inf "
      "J "
      "NA "
      "NaN "
      "OCTAVE_HOME "
      "OCTAVE_VERSION "
      "O_APPEND "
      "O_ASYNC "
      "O_CREAT "
      "O_EXCL "
      "O_NONBLOCK "
      "O_RDONLY "
      "O_RDWR "
      "O_SYNC "
      "O_TRUNC "
      "O_WRONLY "
      "PAGER "
      "PAGER_FLAGS "
      "PS1 "
      "PS2 "
      "PS4 "
      "P_tmpdir "
      "SEEK_CUR "
      "SEEK_END "
      "SEEK_SET "
      "SIG "
      "S_ISBLK "
      "S_ISCHR "
      "S_ISDIR "
      "S_ISFIFO "
      "S_ISLNK "
      "S_ISREG "
      "S_ISSOCK "
      "WCONTINUE "
      "WCOREDUMP "
      "WEXITSTATUS "
      "WIFCONTINUED "
      "WIFEXITED "
      "WIFSIGNALED "
      "WIFSTOPPED "
      "WNOHANG "
      "WSTOPSIG "
      "WTERMSIG "
      "WUNTRACED "
      "__accumarray_max__ "
      "__accumarray_min__ "
      "__accumarray_sum__ "
      "__accumdim_sum__ "
      "__all_opts__ "
      "__builtins__ "
      "__calc_dimensions__ "
      "__contourc__ "
      "__current_scope__ "
      "__delaunayn__ "
      "__dispatch__ "
      "__display_tokens__ "
      "__dsearchn__ "
      "__dump_symtab_info__ "
      "__end__ "
      "__error_text__ "
      "__finish__ "
      "__fltk_ginput__ "
      "__fltk_print__ "
      "__fltk_uigetfile__ "
      "__ftp__ "
      "__ftp_ascii__ "
      "__ftp_binary__ "
      "__ftp_close__ "
      "__ftp_cwd__ "
      "__ftp_delete__ "
      "__ftp_dir__ "
      "__ftp_mget__ "
      "__ftp_mkdir__ "
      "__ftp_mode__ "
      "__ftp_mput__ "
      "__ftp_pwd__ "
      "__ftp_rename__ "
      "__ftp_rmdir__ "
      "__get__ "
      "__glpk__ "
      "__gnuplot_drawnow__ "
      "__gnuplot_get_var__ "
      "__gnuplot_ginput__ "
      "__gnuplot_has_feature__ "
      "__gnuplot_open_stream__ "
      "__gnuplot_print__ "
      "__gnuplot_version__ "
      "__go_axes__ "
      "__go_axes_init__ "
      "__go_close_all__ "
      "__go_delete__ "
      "__go_draw_axes__ "
      "__go_draw_figure__ "
      "__go_execute_callback__ "
      "__go_figure__ "
      "__go_figure_handles__ "
      "__go_handles__ "
      "__go_hggroup__ "
      "__go_image__ "
      "__go_line__ "
      "__go_patch__ "
      "__go_surface__ "
      "__go_text__ "
      "__go_uimenu__ "
      "__gud_mode__ "
      "__image_pixel_size__ "
      "__init_fltk__ "
      "__isa_parent__ "
      "__keywords__ "
      "__lexer_debug_flag__ "
      "__lin_interpn__ "
      "__list_functions__ "
      "__magick_finfo__ "
      "__magick_format_list__ "
      "__magick_read__ "
      "__magick_write__ "
      "__makeinfo__ "
      "__marching_cube__ "
      "__next_line_color__ "
      "__next_line_style__ "
      "__operators__ "
      "__parent_classes__ "
      "__parser_debug_flag__ "
      "__pathorig__ "
      "__pchip_deriv__ "
      "__plt_get_axis_arg__ "
      "__print_parse_opts__ "
      "__qp__ "
      "__request_drawnow__ "
      "__sort_rows_idx__ "
      "__strip_html_tags__ "
      "__token_count__ "
      "__unimplemented__ "
      "__varval__ "
      "__version_info__ "
      "__voronoi__ "
      "__which__ "
      "abs "
      "accumarray "
      "accumdim "
      "acos "
      "acosd "
      "acosh "
      "acot "
      "acotd "
      "acoth "
      "acsc "
      "acscd "
      "acsch "
      "add_input_event_hook "
      "addlistener "
      "addpath "
      "addproperty "
      "addtodate "
      "airy "
      "all "
      "allchild "
      "allow_noninteger_range_as_index "
      "amd "
      "ancestor "
      "and "
      "angle "
      "anova "
      "ans "
      "any "
      "arch_fit "
      "arch_rnd "
      "arch_test "
      "area "
      "arg "
      "argnames "
      "argv "
      "arma_rnd "
      "arrayfun "
      "asctime "
      "asec "
      "asecd "
      "asech "
      "asin "
      "asind "
      "asinh "
      "assert "
      "assignin "
      "atan "
      "atan2 "
      "atand "
      "atanh "
      "atexit "
      "autocor "
      "autocov "
      "autoload "
      "autoreg_matrix "
      "autumn "
      "available_graphics_toolkits "
      "axes "
      "axis "
      "balance "
      "bar "
      "barh "
      "bartlett "
      "bartlett_test "
      "base2dec "
      "beep "
      "beep_on_error "
      "bessel "
      "besselh "
      "besseli "
      "besselj "
      "besselk "
      "bessely "
      "beta "
      "betacdf "
      "betai "
      "betainc "
      "betainv "
      "betaln "
      "betapdf "
      "betarnd "
      "bicgstab "
      "bicubic "
      "bin2dec "
      "bincoeff "
      "binocdf "
      "binoinv "
      "binopdf "
      "binornd "
      "bitand "
      "bitcmp "
      "bitget "
      "bitmax "
      "bitor "
      "bitpack "
      "bitset "
      "bitshift "
      "bitunpack "
      "bitxor "
      "blackman "
      "blanks "
      "blkdiag "
      "blkmm "
      "bone "
      "box "
      "break "
      "brighten "
      "bsxfun "
      "bug_report "
      "builtin "
      "bunzip2 "
      "bzip2 "
      "calendar "
      "canonicalize_file_name "
      "cart2pol "
      "cart2sph "
      "case "
      "cast "
      "cat "
      "catch "
      "cauchy_cdf "
      "cauchy_inv "
      "cauchy_pdf "
      "cauchy_rnd "
      "caxis "
      "cbrt "
      "ccolamd "
      "cd "
      "ceil "
      "cell "
      "cell2mat "
      "cell2struct "
      "celldisp "
      "cellfun "
      "cellidx "
      "cellindexmat "
      "cellslices "
      "cellstr "
      "center "
      "cgs "
      "char "
      "chdir "
      "chi2cdf "
      "chi2inv "
      "chi2pdf "
      "chi2rnd "
      "chisquare_test_homogeneity "
      "chisquare_test_independence "
      "chol "
      "chol2inv "
      "choldelete "
      "cholinsert "
      "cholinv "
      "cholshift "
      "cholupdate "
      "chop "
      "circshift "
      "cla "
      "clabel "
      "class "
      "clc "
      "clear "
      "clf "
      "clg "
      "clock "
      "cloglog "
      "close "
      "closereq "
      "colamd "
      "colloc "
      "colon "
      "colorbar "
      "colormap "
      "colperm "
      "colstyle "
      "columns "
      "comet "
      "comet3 "
      "comma "
      "command_line_path "
      "common_size "
      "commutation_matrix "
      "compan "
      "compare_versions "
      "compass "
      "complement "
      "completion_append_char "
      "completion_matches "
      "complex "
      "computer "
      "cond "
      "condest "
      "confirm_recursive_rmdir "
      "conj "
      "continue "
      "contour "
      "contour3 "
      "contourc "
      "contourf "
      "contrast "
      "conv "
      "conv2 "
      "convhull "
      "convhulln "
      "convn "
      "cool "
      "copper "
      "copyfile "
      "cor "
      "cor_test "
      "corrcoef "
      "cos "
      "cosd "
      "cosh "
      "cot "
      "cotd "
      "coth "
      "cov "
      "cplxpair "
      "cputime "
      "cquad "
      "crash_dumps_octave_core "
      "create_set "
      "cross "
      "csc "
      "cscd "
      "csch "
      "cstrcat "
      "csvread "
      "csvwrite "
      "csymamd "
      "ctime "
      "ctranspose "
      "cummax "
      "cummin "
      "cumprod "
      "cumsum "
      "cumtrapz "
      "curl "
      "cut "
      "cylinder "
      "daspect "
      "daspk "
      "daspk_options "
      "dasrt "
      "dasrt_options "
      "dassl "
      "dassl_options "
      "date "
      "datenum "
      "datestr "
      "datetick "
      "datevec "
      "dbclear "
      "dbcont "
      "dbdown "
      "dblquad "
      "dbnext "
      "dbquit "
      "dbstack "
      "dbstatus "
      "dbstep "
      "dbstop "
      "dbtype "
      "dbup "
      "dbwhere "
      "deal "
      "deblank "
      "debug "
      "debug_on_error "
      "debug_on_interrupt "
      "debug_on_warning "
      "dec2base "
      "dec2bin "
      "dec2hex "
      "deconv "
      "default_save_options "
      "del2 "
      "delaunay "
      "delaunay3 "
      "delaunayn "
      "delete "
      "dellistener "
      "demo "
      "det "
      "detrend "
      "diag "
      "diary "
      "diff "
      "diffpara "
      "diffuse "
      "dir "
      "discrete_cdf "
      "discrete_inv "
      "discrete_pdf "
      "discrete_rnd "
      "disp "
      "dispatch "
      "display "
      "divergence "
      "dlmread "
      "dlmwrite "
      "dmperm "
      "dmult "
      "do "
      "do_braindead_shortcircuit_evaluation "
      "do_string_escapes "
      "doc "
      "doc_cache_file "
      "dos "
      "dot "
      "double "
      "drawnow "
      "dsearch "
      "dsearchn "
      "dump_prefs "
      "dup2 "
      "duplication_matrix "
      "durbinlevinson "
      "e "
      "echo "
      "echo_executing_commands "
      "edit "
      "edit_history "
      "eig "
      "eigs "
      "ellipsoid "
      "else "
      "elseif "
      "empirical_cdf "
      "empirical_inv "
      "empirical_pdf "
      "empirical_rnd "
      "end "
      "end_try_catch "
      "end_unwind_protect "
      "endfor "
      "endfunction "
      "endgrent "
      "endif "
      "endpwent "
      "endswitch "
      "endwhile "
      "eomday "
      "eps "
      "eq "
      "erf "
      "erfc "
      "erfcx "
      "erfinv "
      "errno "
      "errno_list "
      "error "
      "error_text "
      "errorbar "
      "etime "
      "etree "
      "etreeplot "
      "eval "
      "evalin "
      "example "
      "exec "
      "exist "
      "exit "
      "exp "
      "expcdf "
      "expinv "
      "expm "
      "expm1 "
      "exppdf "
      "exprnd "
      "eye "
      "ezcontour "
      "ezcontourf "
      "ezmesh "
      "ezmeshc "
      "ezplot "
      "ezplot3 "
      "ezpolar "
      "ezsurf "
      "ezsurfc "
      "f_test_regression "
      "factor "
      "factorial "
      "fail "
      "false "
      "fcdf "
      "fclear "
      "fclose "
      "fcntl "
      "fdisp "
      "feather "
      "feof "
      "ferror "
      "feval "
      "fflush "
      "fft "
      "fft2 "
      "fftconv "
      "fftfilt "
      "fftn "
      "fftshift "
      "fftw "
      "fgetl "
      "fgets "
      "fieldnames "
      "figure "
      "file_in_loadpath "
      "file_in_path "
      "fileattrib "
      "filemarker "
      "fileparts "
      "fileread "
      "filesep "
      "fill "
      "filter "
      "filter2 "
      "find "
      "find_dir_in_path "
      "findall "
      "findobj "
      "findstr "
      "finite "
      "finv "
      "fix "
      "fixed_point_format "
      "flag "
      "flipdim "
      "fliplr "
      "flipud "
      "floor "
      "fminbnd "
      "fminunc "
      "fmod "
      "fnmatch "
      "fopen "
      "for "
      "fork "
      "format "
      "formula "
      "fpdf "
      "fplot "
      "fprintf "
      "fputs "
      "fractdiff "
      "fread "
      "freport "
      "freqz "
      "freqz_plot "
      "frewind "
      "frnd "
      "fscanf "
      "fseek "
      "fskipl "
      "fsolve "
      "fstat "
      "ftell "
      "full "
      "fullfile "
      "func2str "
      "function "
      "functions "
      "fwrite "
      "fzero "
      "gamcdf "
      "gaminv "
      "gamma "
      "gammai "
      "gammainc "
      "gammaln "
      "gampdf "
      "gamrnd "
      "gca "
      "gcbf "
      "gcbo "
      "gcd "
      "gcf "
      "ge "
      "gen_doc_cache "
      "genpath "
      "genvarname "
      "geocdf "
      "geoinv "
      "geopdf "
      "geornd "
      "get "
      "get_first_help_sentence "
      "get_help_text "
      "get_help_text_from_file "
      "getappdata "
      "getegid "
      "getenv "
      "geteuid "
      "getfield "
      "getgid "
      "getgrent "
      "getgrgid "
      "getgrnam "
      "gethostname "
      "getpgrp "
      "getpid "
      "getppid "
      "getpwent "
      "getpwnam "
      "getpwuid "
      "getrusage "
      "getuid "
      "ginput "
      "givens "
      "glob "
      "global "
      "glpk "
      "glpkmex "
      "gls "
      "gmap40 "
      "gmres "
      "gmtime "
      "gnuplot_binary "
      "gplot "
      "gradient "
      "graphics_toolkit "
      "gray "
      "gray2ind "
      "grid "
      "griddata "
      "griddata3 "
      "griddatan "
      "gt "
      "gtext "
      "gunzip "
      "gzip "
      "hadamard "
      "hamming "
      "hankel "
      "hanning "
      "help "
      "hess "
      "hex2dec "
      "hex2num "
      "hggroup "
      "hidden "
      "hilb "
      "hist "
      "histc "
      "history "
      "history_control "
      "history_file "
      "history_size "
      "history_timestamp_format_string "
      "hold "
      "home "
      "horzcat "
      "hot "
      "hotelling_test "
      "hotelling_test_2 "
      "housh "
      "hsv "
      "hsv2rgb "
      "hurst "
      "hygecdf "
      "hygeinv "
      "hygepdf "
      "hygernd "
      "hypot "
      "i "
      "idivide "
      "if "
      "ifelse "
      "ifft "
      "ifft2 "
      "ifftn "
      "ifftshift "
      "ignore_function_time_stamp "
      "imag "
      "image "
      "imagesc "
      "imfinfo "
      "imread "
      "imshow "
      "imwrite "
      "ind2gray "
      "ind2rgb "
      "ind2sub "
      "index "
      "inf "
      "inferiorto "
      "info "
      "info_file "
      "info_program "
      "inline "
      "inpolygon "
      "input "
      "inputname "
      "int16 "
      "int2str "
      "int32 "
      "int64 "
      "int8 "
      "interp1 "
      "interp1q "
      "interp2 "
      "interp3 "
      "interpft "
      "interpn "
      "intersect "
      "intmax "
      "intmin "
      "intwarning "
      "inv "
      "inverse "
      "invhilb "
      "ipermute "
      "iqr "
      "is_absolute_filename "
      "is_duplicate_entry "
      "is_global "
      "is_leap_year "
      "is_rooted_relative_filename "
      "is_valid_file_id "
      "isa "
      "isalnum "
      "isalpha "
      "isappdata "
      "isargout "
      "isascii "
      "isbool "
      "iscell "
      "iscellstr "
      "ischar "
      "iscntrl "
      "iscolumn "
      "iscommand "
      "iscomplex "
      "isdebugmode "
      "isdefinite "
      "isdeployed "
      "isdigit "
      "isdir "
      "isempty "
      "isequal "
      "isequalwithequalnans "
      "isfield "
      "isfigure "
      "isfinite "
      "isfloat "
      "isglobal "
      "isgraph "
      "ishandle "
      "ishermitian "
      "ishghandle "
      "ishold "
      "isieee "
      "isindex "
      "isinf "
      "isinteger "
      "iskeyword "
      "isletter "
      "islogical "
      "islower "
      "ismac "
      "ismatrix "
      "ismember "
      "ismethod "
      "isna "
      "isnan "
      "isnull "
      "isnumeric "
      "isobject "
      "isocolors "
      "isonormals "
      "isosurface "
      "ispc "
      "isprime "
      "isprint "
      "isprop "
      "ispunct "
      "israwcommand "
      "isreal "
      "isrow "
      "isscalar "
      "issorted "
      "isspace "
      "issparse "
      "issquare "
      "isstr "
      "isstrprop "
      "isstruct "
      "issymmetric "
      "isunix "
      "isupper "
      "isvarname "
      "isvector "
      "isxdigit "
      "j "
      "jet "
      "kbhit "
      "kendall "
      "keyboard "
      "kill "
      "kolmogorov_smirnov_cdf "
      "kolmogorov_smirnov_test "
      "kolmogorov_smirnov_test_2 "
      "kron "
      "kruskal_wallis_test "
      "krylov "
      "krylovb "
      "kurtosis "
      "laplace_cdf "
      "laplace_inv "
      "laplace_pdf "
      "laplace_rnd "
      "lasterr "
      "lasterror "
      "lastwarn "
      "lchol "
      "lcm "
      "ldivide "
      "le "
      "legend "
      "legendre "
      "length "
      "lgamma "
      "license "
      "lin2mu "
      "line "
      "link "
      "linkprop "
      "linspace "
      "list "
      "list_in_columns "
      "list_primes "
      "load "
      "loadaudio "
      "loadimage "
      "loadobj "
      "localtime "
      "log "
      "log10 "
      "log1p "
      "log2 "
      "logical "
      "logistic_cdf "
      "logistic_inv "
      "logistic_pdf "
      "logistic_regression "
      "logistic_rnd "
      "logit "
      "loglog "
      "loglogerr "
      "logm "
      "logncdf "
      "logninv "
      "lognpdf "
      "lognrnd "
      "logspace "
      "lookfor "
      "lookup "
      "lower "
      "ls "
      "ls_command "
      "lsode "
      "lsode_options "
      "lsqnonneg "
      "lstat "
      "lt "
      "lu "
      "luinc "
      "luupdate "
      "magic "
      "mahalanobis "
      "make_absolute_filename "
      "makeinfo_program "
      "manova "
      "mark_as_command "
      "mark_as_rawcommand "
      "mat2cell "
      "mat2str "
      "matlabroot "
      "matrix_type "
      "max "
      "max_recursion_depth "
      "mcnemar_test "
      "md5sum "
      "mean "
      "meansq "
      "median "
      "menu "
      "merge "
      "mesh "
      "meshc "
      "meshgrid "
      "meshz "
      "methods "
      "mex "
      "mexext "
      "mfilename "
      "mgorth "
      "min "
      "minus "
      "mislocked "
      "missing_function_hook "
      "mist "
      "mkdir "
      "mkfifo "
      "mkoctfile "
      "mkpp "
      "mkstemp "
      "mktime "
      "mldivide "
      "mlock "
      "mod "
      "mode "
      "moment "
      "more "
      "most "
      "movefile "
      "mpoles "
      "mpower "
      "mrdivide "
      "mtimes "
      "mu2lin "
      "munlock "
      "namelengthmax "
      "nan "
      "nargchk "
      "nargin "
      "nargout "
      "nargoutchk "
      "native_float_format "
      "nbincdf "
      "nbininv "
      "nbinpdf "
      "nbinrnd "
      "nchoosek "
      "ndgrid "
      "ndims "
      "ne "
      "newplot "
      "news "
      "nextpow2 "
      "nfields "
      "nnz "
      "nonzeros "
      "norm "
      "normcdf "
      "normest "
      "norminv "
      "normpdf "
      "normrnd "
      "not "
      "now "
      "nproc "
      "nth_element "
      "nthroot "
      "ntsc2rgb "
      "null "
      "num2cell "
      "num2hex "
      "num2str "
      "numel "
      "nzmax "
      "ocean "
      "octave_config_info "
      "octave_core_file_limit "
      "octave_core_file_name "
      "octave_core_file_options "
      "octave_tmp_file_name "
      "ols "
      "onCleanup "
      "onenormest "
      "ones "
      "optimget "
      "optimize_subsasgn_calls "
      "optimset "
      "or "
      "orderfields "
      "orient "
      "orth "
      "otherwise "
      "output_max_field_width "
      "output_precision "
      "pack "
      "page_output_immediately "
      "page_screen_output "
      "paren "
      "pareto "
      "parseparams "
      "pascal "
      "patch "
      "path "
      "pathdef "
      "pathsep "
      "pause "
      "pbaspect "
      "pcg "
      "pchip "
      "pclose "
      "pcolor "
      "pcr "
      "peaks "
      "periodogram "
      "perl "
      "perms "
      "permute "
      "perror "
      "persistent "
      "pi "
      "pie "
      "pie3 "
      "pink "
      "pinv "
      "pipe "
      "pkg "
      "planerot "
      "playaudio "
      "plot "
      "plot3 "
      "plotmatrix "
      "plotyy "
      "plus "
      "poisscdf "
      "poissinv "
      "poisspdf "
      "poissrnd "
      "pol2cart "
      "polar "
      "poly "
      "polyaffine "
      "polyarea "
      "polyder "
      "polyderiv "
      "polyfit "
      "polygcd "
      "polyint "
      "polyout "
      "polyreduce "
      "polyval "
      "polyvalm "
      "popen "
      "popen2 "
      "postpad "
      "pow2 "
      "power "
      "powerset "
      "ppder "
      "ppint "
      "ppjumps "
      "ppplot "
      "ppval "
      "pqpnonneg "
      "prctile "
      "prepad "
      "primes "
      "print "
      "print_empty_dimensions "
      "print_struct_array_contents "
      "print_usage "
      "printf "
      "prism "
      "probit "
      "prod "
      "program_invocation_name "
      "program_name "
      "prop_test_2 "
      "putenv "
      "puts "
      "pwd "
      "qp "
      "qqplot "
      "qr "
      "qrdelete "
      "qrinsert "
      "qrshift "
      "qrupdate "
      "quad "
      "quad_options "
      "quadcc "
      "quadgk "
      "quadl "
      "quadv "
      "quantile "
      "quit "
      "quiver "
      "quiver3 "
      "qz "
      "qzhess "
      "rainbow "
      "rand "
      "rande "
      "randg "
      "randi "
      "randn "
      "randp "
      "randperm "
      "range "
      "rank "
      "ranks "
      "rat "
      "rats "
      "rcond "
      "rdivide "
      "re_read_readline_init_file "
      "read_readline_init_file "
      "readdir "
      "readlink "
      "real "
      "reallog "
      "realmax "
      "realmin "
      "realpow "
      "realsqrt "
      "record "
      "rectangle "
      "rectint "
      "refresh "
      "refreshdata "
      "regexp "
      "regexpi "
      "regexprep "
      "regexptranslate "
      "rehash "
      "rem "
      "remove_input_event_hook "
      "rename "
      "repelems "
      "replot "
      "repmat "
      "reset "
      "reshape "
      "residue "
      "resize "
      "restoredefaultpath "
      "rethrow "
      "return "
      "rgb2hsv "
      "rgb2ind "
      "rgb2ntsc "
      "ribbon "
      "rindex "
      "rmappdata "
      "rmdir "
      "rmfield "
      "rmpath "
      "roots "
      "rose "
      "rosser "
      "rot90 "
      "rotdim "
      "round "
      "roundb "
      "rows "
      "rref "
      "rsf2csf "
      "run "
      "run_count "
      "run_history "
      "run_test "
      "rundemos "
      "runlength "
      "runtests "
      "save "
      "save_header_format_string "
      "save_precision "
      "saveas "
      "saveaudio "
      "saveimage "
      "saveobj "
      "savepath "
      "saving_history "
      "scanf "
      "scatter "
      "scatter3 "
      "schur "
      "sec "
      "secd "
      "sech "
      "semicolon "
      "semilogx "
      "semilogxerr "
      "semilogy "
      "semilogyerr "
      "set "
      "setappdata "
      "setaudio "
      "setdiff "
      "setenv "
      "setfield "
      "setgrent "
      "setpwent "
      "setstr "
      "setxor "
      "shading "
      "shell_cmd "
      "shg "
      "shift "
      "shiftdim "
      "sighup_dumps_octave_core "
      "sign "
      "sign_test "
      "sigterm_dumps_octave_core "
      "silent_functions "
      "sin "
      "sinc "
      "sind "
      "sinetone "
      "sinewave "
      "single "
      "sinh "
      "size "
      "size_equal "
      "sizemax "
      "sizeof "
      "skewness "
      "sleep "
      "slice "
      "sombrero "
      "sort "
      "sortrows "
      "source "
      "spalloc "
      "sparse "
      "sparse_auto_mutate "
      "spatan2 "
      "spaugment "
      "spchol "
      "spchol2inv "
      "spcholinv "
      "spconvert "
      "spcumprod "
      "spcumsum "
      "spdet "
      "spdiag "
      "spdiags "
      "spearman "
      "spectral_adf "
      "spectral_xdf "
      "specular "
      "speed "
      "spencer "
      "speye "
      "spfind "
      "spfun "
      "sph2cart "
      "sphcat "
      "sphere "
      "spinmap "
      "spinv "
      "spkron "
      "splchol "
      "spline "
      "split "
      "split_long_rows "
      "splu "
      "spmax "
      "spmin "
      "spones "
      "spparms "
      "spprod "
      "spqr "
      "sprand "
      "sprandn "
      "sprandsym "
      "sprank "
      "spring "
      "sprintf "
      "spstats "
      "spsum "
      "spsumsq "
      "spvcat "
      "spy "
      "sqp "
      "sqrt "
      "sqrtm "
      "squeeze "
      "sscanf "
      "stairs "
      "stat "
      "static "
      "statistics "
      "std "
      "stderr "
      "stdin "
      "stdnormal_cdf "
      "stdnormal_inv "
      "stdnormal_pdf "
      "stdnormal_rnd "
      "stdout "
      "stem "
      "stem3 "
      "stft "
      "str2double "
      "str2func "
      "str2mat "
      "str2num "
      "strcat "
      "strchr "
      "strcmp "
      "strcmpi "
      "strerror "
      "strfind "
      "strftime "
      "string_fill_char "
      "strjust "
      "strmatch "
      "strncmp "
      "strncmpi "
      "strptime "
      "strread "
      "strrep "
      "strsplit "
      "strtok "
      "strtrim "
      "strtrunc "
      "struct "
      "struct2cell "
      "struct_levels_to_print "
      "structfun "
      "strvcat "
      "studentize "
      "sub2ind "
      "subplot "
      "subsasgn "
      "subsindex "
      "subspace "
      "subsref "
      "substr "
      "substruct "
      "sum "
      "summer "
      "sumsq "
      "superiorto "
      "suppress_verbose_help_message "
      "surf "
      "surface "
      "surfc "
      "surfl "
      "surfnorm "
      "svd "
      "svd_driver "
      "svds "
      "swapbytes "
      "switch "
      "syl "
      "sylvester_matrix "
      "symamd "
      "symbfact "
      "symlink "
      "symrcm "
      "symvar "
      "synthesis "
      "system "
      "t_test "
      "t_test_2 "
      "t_test_regression "
      "table "
      "tan "
      "tand "
      "tanh "
      "tar "
      "tcdf "
      "tempdir "
      "tempname "
      "terminal_size "
      "test "
      "test2 "
      "test3 "
      "text "
      "textread "
      "textscan "
      "tic "
      "tilde_expand "
      "time "
      "times "
      "tinv "
      "title "
      "tmpfile "
      "tmpnam "
      "toascii "
      "toc "
      "toeplitz "
      "tolower "
      "toupper "
      "tpdf "
      "trace "
      "transpose "
      "trapz "
      "treelayout "
      "treeplot "
      "tril "
      "trimesh "
      "triplequad "
      "triplot "
      "trisurf "
      "triu "
      "trnd "
      "true "
      "try "
      "tsearch "
      "tsearchn "
      "type "
      "typecast "
      "typeinfo "
      "u_test "
      "uigetdir "
      "uigetfile "
      "uimenu "
      "uint16 "
      "uint32 "
      "uint64 "
      "uint8 "
      "uiputfile "
      "umask "
      "uminus "
      "uname "
      "undo_string_escapes "
      "unidcdf "
      "unidinv "
      "unidpdf "
      "unidrnd "
      "unifcdf "
      "unifinv "
      "unifpdf "
      "unifrnd "
      "union "
      "unique "
      "unix "
      "unlink "
      "unmark_command "
      "unmark_rawcommand "
      "unmkpp "
      "unpack "
      "untabify "
      "untar "
      "until "
      "unwind_protect "
      "unwind_protect_cleanup "
      "unwrap "
      "unzip "
      "uplus "
      "upper "
      "urlread "
      "urlwrite "
      "usage "
      "usleep "
      "validatestring "
      "values "
      "vander "
      "var "
      "var_test "
      "varargin "
      "varargout "
      "vec "
      "vech "
      "vectorize "
      "ver "
      "version "
      "vertcat "
      "view "
      "voronoi "
      "voronoin "
      "waitforbuttonpress "
      "waitpid "
      "warning "
      "warning_ids "
      "warranty "
      "wavread "
      "wavwrite "
      "wblcdf "
      "wblinv "
      "wblpdf "
      "wblrnd "
      "weekday "
      "weibcdf "
      "weibinv "
      "weibpdf "
      "weibrnd "
      "welch_test "
      "what "
      "which "
      "while "
      "white "
      "whitebg "
      "who "
      "whos "
      "whos_line_format "
      "wienrnd "
      "wilcoxon_test "
      "wilkinson "
      "winter "
      "xlabel "
      "xlim "
      "xor "
      "yes_or_no "
      "ylabel "
      "ylim "
      "yulewalker "
      "z_test "
      "z_test_2 "
      "zeros "
      "zip "
      "zlabel "
      "zlim ";
  /*            "break case catch continue do else elseif end end_unwind_protect "
              "endfor endfunction endif endswitch endwhile for function "
              "global if otherwise persistent return switch try until "
              "unwind_protect unwind_protect_cleanup while";
  */
}
