/* pathsearch.c: look up a filename in a path.

Copyright (C) 1993, 94, 95, 96, 97, 98 Karl Berry.
Copyright (C) 1993, 94, 95, 96, 97 Karl Berry & O. Weber.
Copyright (C) 1992, 93, 94, 95, 96 Free Software Foundation, Inc.

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

#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif

#include <map>
#include <string>

#include "kpse-config.h"
#include "kpse-xfns.h"
#include "kpse.h"

#include "lo-sstream.h"
#include "oct-env.h"
#include "oct-passwd.h"

/* c-std.h: the first header files.  */

/* Header files that essentially all of our sources need, and
   that all implementations have.  We include these first, to help with
   NULL being defined multiple times.  */
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <cerrno>
#include <cassert>

#ifdef HAVE_UNISTD_H
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <unistd.h>
#endif

#include "sysdir.h"
#include "statdefs.h"

/* define NAME_MAX, the maximum length of a single
   component in a filename.  No such limit may exist, or may vary
   depending on the filesystem.  */

/* Most likely the system will truncate filenames if it is not POSIX,
   and so we can use the BSD value here.  */
#ifndef _POSIX_NAME_MAX
#define _POSIX_NAME_MAX 255
#endif

#ifndef NAME_MAX
#define NAME_MAX _POSIX_NAME_MAX
#endif

/* c-ctype.h: ASCII-safe versions of the <ctype.h> macros.  */

#include <cctype>

/* What separates elements in environment variable path lists?  */
#ifndef ENV_SEP
#ifdef DOSISH
#define ENV_SEP ';'
#define ENV_SEP_STRING ";"
#else
#define ENV_SEP ':'
#define ENV_SEP_STRING ":"
#endif /* not DOS */
#endif /* not ENV_SEP */

#ifndef IS_ENV_SEP
#define IS_ENV_SEP(ch) ((ch) == ENV_SEP)
#endif

/* c-pathmx.h: define PATH_MAX, the maximum length of a filename.
   Since no such limit may exist, it's preferable to dynamically grow
   filenames as needed.  */

/* Cheat and define this as a manifest constant no matter what, instead
   of using pathconf.  I forget why we want to do this.  */

#ifndef _POSIX_PATH_MAX
#define _POSIX_PATH_MAX 255
#endif

#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX _POSIX_PATH_MAX
#endif
#endif /* not PATH_MAX */

/* debug.h: Runtime tracing.  */

/* If NO_DEBUG is defined (not recommended), skip all this.  */
#ifndef NO_DEBUG

/* OK, we'll have tracing support.  */
#define KPSE_DEBUG

/* Test if a bit is on.  */
#define KPSE_DEBUG_P(bit) (kpathsea_debug & (1 << (bit)))

#define KPSE_DEBUG_STAT 0		/* stat calls */
#define KPSE_DEBUG_HASH 1		/* hash lookups */
#define KPSE_DEBUG_FOPEN 2		/* fopen/fclose calls */
#define KPSE_DEBUG_PATHS 3		/* search path initializations */
#define KPSE_DEBUG_EXPAND 4		/* path element expansion */
#define KPSE_DEBUG_SEARCH 5		/* searches */
#define KPSE_DEBUG_VARS 6		/* variable values */
#define KPSE_LAST_DEBUG KPSE_DEBUG_VARS

/* A printf for the debugging.  */
#define DEBUGF_START() do { fputs ("kdebug:", stderr)
#define DEBUGF_END()        fflush (stderr); } while (0)

#define DEBUGF(str)							\
  DEBUGF_START (); fputs (str, stderr); DEBUGF_END ()
#define DEBUGF1(str, e1)						\
  DEBUGF_START (); fprintf (stderr, str, e1); DEBUGF_END ()
#define DEBUGF2(str, e1, e2)						\
  DEBUGF_START (); fprintf (stderr, str, e1, e2); DEBUGF_END ()
#define DEBUGF3(str, e1, e2, e3)					\
  DEBUGF_START (); fprintf (stderr, str, e1, e2, e3); DEBUGF_END ()
#define DEBUGF4(str, e1, e2, e3, e4)					\
  DEBUGF_START (); fprintf (stderr, str, e1, e2, e3, e4); DEBUGF_END ()

#undef fopen
#define fopen kpse_fopen_trace
extern FILE *fopen (const char *filename, const char *mode);
#undef fclose
#define fclose kpse_fclose_trace
extern int fclose (FILE *);

#endif /* not NO_DEBUG */

#if defined (WIN32) && !defined (__MINGW32__)

/* System description file for Windows NT.  */

/*
 *      Define symbols to identify the version of Unix this is.
 *      Define all the symbols that apply correctly.
 */

#ifndef DOSISH
#define DOSISH
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN      _MAX_PATH
#endif

/* These have to be defined because our compilers treat __STDC__ as being
   defined (most of them anyway). */

#define access  _access
#define stat    _stat
#define strcasecmp _stricmp
#define strdup  _strdup
#define strncasecmp _strnicmp

#define S_IFMT   _S_IFMT
#define S_IFDIR  _S_IFDIR

/* Define this so that winsock.h definitions don't get included when
   windows.h is...  For this to have proper effect, config.h must
   always be included before windows.h.  */
#define _WINSOCKAPI_    1

#include <windows.h>

/* For proper declaration of environ.  */
#include <io.h>
#include <fcntl.h>
#include <process.h>

/* ============================================================ */

#endif /* WIN32 */

/* lib.h: other stuff.  */

/* Define common sorts of messages.  */

/* This should be called only after a system call fails.  Don't exit
   with status `errno', because that might be 256, which would mean
   success (exit statuses are truncated to eight bits).  */
#define FATAL_PERROR(str) do { \
  fputs ("pathsearch: ", stderr); \
  perror (str); exit (EXIT_FAILURE); } while (0)

#define START_FATAL() do { \
  fputs ("pathsearch: fatal: ", stderr);
#define END_FATAL() fputs (".\n", stderr); exit (1); } while (0)

#define FATAL(str)							\
  START_FATAL (); fputs (str, stderr); END_FATAL ()
#define FATAL1(str, e1)							\
  START_FATAL (); fprintf (stderr, str, e1); END_FATAL ()
#define FATAL2(str, e1, e2)						\
  START_FATAL (); fprintf (stderr, str, e1, e2); END_FATAL ()
#define FATAL3(str, e1, e2, e3)						\
  START_FATAL (); fprintf (stderr, str, e1, e2, e3); END_FATAL ()
#define FATAL4(str, e1, e2, e3, e4)					\
  START_FATAL (); fprintf (stderr, str, e1, e2, e3, e4); END_FATAL ()
#define FATAL5(str, e1, e2, e3, e4, e5)					\
  START_FATAL (); fprintf (stderr, str, e1, e2, e3, e4, e5); END_FATAL ()
#define FATAL6(str, e1, e2, e3, e4, e5, e6)				\
  START_FATAL (); fprintf (stderr, str, e1, e2, e3, e4, e5, e6); END_FATAL ()

#define START_WARNING() do { fputs ("warning: ", stderr)
#define END_WARNING() fputs (".\n", stderr); fflush (stderr); } while (0)

#define WARNING(str)							\
  START_WARNING (); fputs (str, stderr); END_WARNING ()
#define WARNING1(str, e1)						\
  START_WARNING (); fprintf (stderr, str, e1); END_WARNING ()
#define WARNING2(str, e1, e2)						\
  START_WARNING (); fprintf (stderr, str, e1, e2); END_WARNING ()
#define WARNING3(str, e1, e2, e3)					\
  START_WARNING (); fprintf (stderr, str, e1, e2, e3); END_WARNING ()
#define WARNING4(str, e1, e2, e3, e4)					\
  START_WARNING (); fprintf (stderr, str, e1, e2, e3, e4); END_WARNING ()

/* I find this easier to read.  */
#define STREQ(s1, s2) (strcmp (s1, s2) == 0)
#define STRNEQ(s1, s2, n) (strncmp (s1, s2, n) == 0)

/* Support for FAT/ISO-9660 filesystems.  Theoretically this should be
   done at runtime, per filesystem, but that's painful to program.  */
#ifdef MONOCASE_FILENAMES
#define FILESTRCASEEQ(s1, s2) (strcasecmp (s1, s2) == 0)
#define FILESTRNCASEEQ(s1, s2, l) (strncasecmp (s1, s2, l) == 0)
#define FILECHARCASEEQ(c1, c2) (toupper (c1) == toupper (c2))
#else
#define FILESTRCASEEQ STREQ
#define FILESTRNCASEEQ STRNEQ
#define FILECHARCASEEQ(c1, c2) ((c1) == (c2))
#endif

/* (Re)Allocate N items of type T using xmalloc/xrealloc.  */
#define XTALLOC(n, t) ((t *) xmalloc ((n) * sizeof (t)))
#define XTALLOC1(t) XTALLOC (1, t)
#define XRETALLOC(addr, n, t) ((addr) = (t *) xrealloc (addr, (n) * sizeof(t)))

extern "C" char *xbasename (const char *name);

#ifndef WIN32
static void xclosedir (DIR *d);
#endif

#ifndef WIN32
int dir_links (const char *fn);
#endif

static void str_llist_add (str_llist_type *l, const std::string& str);

static void str_llist_float (str_llist_type *l, str_llist_elt_type *mover);

static std::string kpse_var_expand (const std::string& src);

#include <ctime> /* for `time' */

/* xmalloc.c: malloc with error checking.  */

static void *
xmalloc (unsigned size)
{
  void *new_mem = (void *) malloc (size);

  if (new_mem == NULL)
    {
      fprintf (stderr, "fatal: memory exhausted (xmalloc of %u bytes).\n",
               size);
      /* 1 means success on VMS, so pick a random number (ASCII `K').  */
      exit (75);
    }

  return new_mem;
}

/* xrealloc.c: realloc with error checking.  */

static void *
xrealloc (void *old_ptr, unsigned size)
{
  void *new_mem;

  if (old_ptr == NULL)
    new_mem = xmalloc (size);
  else
    {
      new_mem = (void *) realloc (old_ptr, size);
      if (new_mem == NULL)
        {
          /* We used to print OLD_PTR here using %x, and casting its
             value to unsigned, but that lost on the Alpha, where
             pointers and unsigned had different sizes.  Since the info
             is of little or no value anyway, just don't print it.  */
          fprintf (stderr, "fatal: memory exhausted (realloc of %u bytes).\n",
                   size);
          /* 1 means success on VMS, so pick a random number (ASCII `B').  */
          exit (66);
        }
    }

  return new_mem;
}

/* Return a copy of S in new storage.  */

static char *
xstrdup (const char *s)
{
  char *new_string = (char *) xmalloc (strlen (s) + 1);
  return strcpy (new_string, s);
}

/* These routines just check the return status from standard library
   routines and abort if an error happens.  */

static FILE *
xfopen (const std::string& filename, const char *mode)
{
  FILE *f;

  assert (! filename.empty () && mode);

  f = fopen (filename.c_str (), mode);

  if (! f)
    FATAL_PERROR (filename.c_str ());

  return f;
}

static void
xfclose (FILE *f, const std::string& filename)
{
  assert (f);

  if (! fclose (f))
    FATAL_PERROR (filename.c_str ());
}

/* Return the concatenation of S1 and S2.  See `concatn.c' for a
   `concatn', which takes a variable number of arguments.  */

static char *
concat (const char *s1, const char *s2)
{
  char *answer = (char *) xmalloc (strlen (s1) + strlen (s2) + 1);
  strcpy (answer, s1);
  strcat (answer, s2);

  return answer;
}

/* concat3.c: concatenate three strings.  */

static char *
concat3 (const char *s1, const char *s2, const char *s3)
{
  char *answer
    = (char *) xmalloc (strlen (s1) + strlen (s2) + strlen (s3) + 1);
  strcpy (answer, s1);
  strcat (answer, s2);
  strcat (answer, s3);

  return answer;
}

/* A single (key,value) pair.  */

struct hash_element_type
{
  std::string key;
  std::string value;
  struct hash_element_type *next;
};

/* The usual arrangement of buckets initialized to null.  */

struct hash_table_type
{
  hash_element_type **buckets;
  unsigned size;
};

/* The hash function.  We go for simplicity here.  */

/* All our hash tables are related to filenames.  */
#ifdef MONOCASE_FILENAMES
#define TRANSFORM(x) toupper (x)
#else
#define TRANSFORM(x) (x)
#endif

static unsigned
hash (hash_table_type table, const std::string& key)
{
  unsigned n = 0;

  /* Our keys aren't often anagrams of each other, so no point in
     weighting the characters.  */
  size_t len = key.length ();
  for (size_t i = 0; i < len; i++)
    n = (n + n + TRANSFORM (key[i])) % table.size;

  return n;
}

static hash_table_type
hash_create (unsigned size)
{
  /* hash_table_type ret; changed into "static ..." to work around gcc
     optimizer bug for Alpha.  */
  static hash_table_type ret;
  unsigned b;
  ret.buckets = new hash_element_type * [size];
  ret.size = size;

  /* calloc's zeroes aren't necessarily NULL, so be safe.  */
  for (b = 0; b <ret.size; b++)
    ret.buckets[b] = NULL;

  return ret;
}

/* Whether or not KEY is already in MAP, insert it and VALUE.  Do not
   duplicate the strings, in case they're being purposefully shared.  */

static void
hash_insert (hash_table_type *table, const std::string& key,
	     const std::string& value)
{
  unsigned n = hash (*table, key);
  hash_element_type *new_elt = new hash_element_type;

  new_elt->key = key;
  new_elt->value = value;
  new_elt->next = NULL;

  /* Insert the new element at the end of the list.  */
  if (! table->buckets[n])
    /* first element in bucket is a special case.  */
    table->buckets[n] = new_elt;
  else
    {
      hash_element_type *loc = table->buckets[n];
      while (loc->next)		/* Find the last element.  */
        loc = loc->next;
      loc->next = new_elt;	/* Insert the new one after.  */
    }
}

/* Look up STR in MAP.  Return a (dynamically-allocated) list of the
   corresponding strings or NULL if no match.  */

static string_vector
hash_lookup (hash_table_type table, const std::string& key)
{
  hash_element_type *p;
  string_vector ret;
  unsigned n = hash (table, key);

  /* Look at everything in this bucket.  */
  for (p = table.buckets[n]; p != NULL; p = p->next)
    if (FILESTRCASEEQ (key.c_str (), p->key.c_str ()))
      ret.append (p->value);

#ifdef KPSE_DEBUG
  if (KPSE_DEBUG_P (KPSE_DEBUG_HASH))
    {
      DEBUGF1 ("hash_lookup (%s) =>", key.c_str ());
      if (ret.empty ())
        fputs (" (nil)\n", stderr);
      else
        {
	  int len = ret.length ();
	  for (int i = 0; i < len; i++)
            {
              putc (' ', stderr);
	      fputs (ret[i].c_str (), stderr);
            }
          putc ('\n', stderr);
        }
      fflush (stderr);
    }
#endif

  return ret;
}

/* We only print nonempty buckets, to decrease output volume.  */

static void
hash_print (hash_table_type table, int summary_only)
{
  unsigned b;
  unsigned total_elements = 0, total_buckets = 0;

  for (b = 0; b < table.size; b++)
    {
      hash_element_type *bucket = table.buckets[b];

      if (bucket)
	{
	  unsigned len = 1;
	  hash_element_type *tb;

	  total_buckets++;
	  if (! summary_only)
	    fprintf (stderr, "%4d ", b);

	  for (tb = bucket->next; tb != NULL; tb = tb->next)
	    len++;

	  if (! summary_only)
	    fprintf (stderr, ":%-5d", len);

	  total_elements += len;

	  if (! summary_only)
	    {
	      for (tb = bucket; tb != NULL; tb = tb->next)
		fprintf (stderr, " %s=>%s", tb->key.c_str (),
			 tb->value.c_str ());

	      putc ('\n', stderr);
	    }
	}
    }

  fprintf (stderr,
	   "%u buckets, %u nonempty (%u%%); %u entries, average chain %.1f.\n",
	   table.size,
	   total_buckets,
	   100 * total_buckets / table.size,
	   total_elements,
	   total_buckets ? total_elements / (double) total_buckets : 0.0);
}

/* Here's the simple one, when a program just wants a value.  */

static std::string
kpse_var_value (const std::string& var)
{
  std::string ret;

  std::string tmp = octave_env::getenv (var);

  if (! tmp.empty ())
    ret = kpse_var_expand (tmp);

#ifdef KPSE_DEBUG
  if (KPSE_DEBUG_P (KPSE_DEBUG_VARS))
    DEBUGF2 ("variable: %s = %s\n", var.c_str (),
	     tmp.empty () ? "(nil)" :  tmp.c_str ());
#endif

  return ret;
}

/* Truncate any too-long components in NAME, returning the result.  It's
   too bad this is necessary.  See comments in readable.c for why.  */

static std::string
kpse_truncate_filename (const std::string& name)
{
  unsigned c_len = 0;        /* Length of current component.  */
  unsigned ret_len = 0;      /* Length of constructed result.  */

  std::string ret = name;

  size_t len = name.length ();

  for (size_t i = 0; i < len; i++)
    {
      if (IS_DIR_SEP (name[i]) || IS_DEVICE_SEP (name[i]))
        {
	  /* At a directory delimiter, reset component length.  */
          c_len = 0;
        }
      else if (c_len > NAME_MAX)
        {
	  /* If past the max for a component, ignore this character.  */
          continue;
        }

      /* Copy this character.  */
      ret[ret_len++] = name[i];
      c_len++;
    }

  ret.resize (ret_len);

  return ret;
}

/* If access can read FN, run stat (assigning to stat buffer ST) and
   check that fn is not a directory.  Don't check for just being a
   regular file, as it is potentially useful to read fifo's or some
   kinds of devices.  */

#ifdef WIN32
static inline bool
READABLE (const std::string& fn, struct stat&)
{
  const char *t = fn.c_str ();
  return (GetFileAttributes (t) != 0xFFFFFFFF
	  && ! (GetFileAttributes (t) & FILE_ATTRIBUTE_DIRECTORY));
}
#else
static inline bool
READABLE (const std::string& fn, struct stat& st)
{
  const char *t = fn.c_str ();
  return (access (t, R_OK) == 0
	  && stat (t, &(st)) == 0 && ! S_ISDIR (st.st_mode));
}
#endif

/* POSIX invented the brain-damage of not necessarily truncating
   filename components; the system's behavior is defined by the value of
   the symbol _POSIX_NO_TRUNC, but you can't change it dynamically!

   Generic const return warning.  See extend-fname.c.  */

static std::string
kpse_readable_file (const std::string& name)
{
  struct stat st;
  std::string ret;

  if (READABLE (name, st))
    {
      ret = name;

#ifdef ENAMETOOLONG
    }
  else if (errno == ENAMETOOLONG)
    {
      ret = kpse_truncate_filename (name);

      /* Perhaps some other error will occur with the truncated name,
	 so let's call access again.  */

      if (! READABLE (ret, st))
	{
	  /* Failed.  */
	  ret = std::string ();
	}
#endif /* ENAMETOOLONG */

    }
  else
    {
      /* Some other error.  */
      if (errno == EACCES)
	{
	  /* Maybe warn them if permissions are bad.  */
	  perror (name.c_str ());
	}

      ret = std::string ();
    }

  return ret;
}

/* Sorry this is such a system-dependent mess, but I can't see any way
   to usefully generalize.  */

static bool
kpse_absolute_p (const std::string& filename, int relative_ok)
{
  size_t len = filename.length ();

  int absolute = IS_DIR_SEP (len > 0 && filename[0])
#ifdef DOSISH
                     /* Novell allows non-alphanumeric drive letters. */
                     || (len > 0 && IS_DEVICE_SEP (filename[1]))
#endif /* DOSISH */
#ifdef WIN32
                     /* UNC names */
                     || (len > 1 && filename[0] == '\\' && filename[1] == '\\')
#endif
		      ;
  int explicit_relative
    = relative_ok
      && (len > 1
	  && filename[0] == '.'
	  && (IS_DIR_SEP (filename[1])
	      || (len > 2 && filename[1] == '.' && IS_DIR_SEP (filename[2]))));

  return absolute || explicit_relative;
}

/* The very first search is for texmf.cnf, called when someone tries to
   initialize the TFM path or whatever.  init_path calls kpse_cnf_get
   which calls kpse_all_path_search to find all the texmf.cnf's.  We
   need to do various special things in this case, since we obviously
   don't yet have the configuration files when we're searching for the
   configuration files.  */
static bool first_search = true;

/* This function is called after every search (except the first, since
   we definitely want to allow enabling the logging in texmf.cnf) to
   record the filename(s) found in $TEXMFLOG.  */

static void
log_search (const string_vector& filenames)
{
  static FILE *log_file = NULL;
  static bool first_time = true; /* Need to open the log file?  */

  if (first_time)
    {
      first_time = false;

      /* Get name from either envvar or config file.  */
      std::string log_name = kpse_var_value ("TEXMFLOG");

      if (! log_name.empty ())
	{
	  log_file = xfopen (log_name.c_str (), "a");

	  if (! log_file)
	    perror (log_name.c_str ());
	}
    }

  if (KPSE_DEBUG_P (KPSE_DEBUG_SEARCH) || log_file)
    {
      /* FILENAMES should never be null, but safety doesn't hurt.  */
      for (int e = 0; e < filenames.length () && ! filenames[e].empty (); e++)
	{
	  std::string filename = filenames[e];

	  /* Only record absolute filenames, for privacy.  */
	  if (log_file && kpse_absolute_p (filename.c_str (), false))
	    fprintf (log_file, "%lu %s\n", (long unsigned) time (NULL),
		     filename.c_str ());

	  /* And show them online, if debugging.  We've already started
	     the debugging line in `search', where this is called, so
	     just print the filename here, don't use DEBUGF.  */
	  if (KPSE_DEBUG_P (KPSE_DEBUG_SEARCH))
	    fputs (filename.c_str (), stderr);
	}
    }
}

/* Concatenate each element in DIRS with NAME (assume each ends with a
   /, to save time).  If SEARCH_ALL is false, return the first readable
   regular file.  Else continue to search for more.  In any case, if
   none, return a list containing just NULL.

   We keep a single buffer for the potential filenames and reallocate
   only when necessary.  I'm not sure it's noticeably faster, but it
   does seem cleaner.  (We do waste a bit of space in the return
   value, though, since we don't shrink it to the final size returned.)  */

static string_vector
dir_list_search (str_llist_type *dirs, const std::string& name,
		 bool search_all)
{
  str_llist_elt_type *elt;
  string_vector ret;

  for (elt = *dirs; elt; elt = STR_LLIST_NEXT (*elt))
    {
      const std::string dir = STR_LLIST (*elt);

      std::string potential = dir + name;

      std::string tmp = kpse_readable_file (potential);

      if (! tmp.empty ())
        {
          ret.append (potential);

          /* Move this element towards the top of the list.  */
          str_llist_float (dirs, elt);

          if (! search_all)
            return ret;
        }
    }

  return ret;
}

/* This is called when NAME is absolute or explicitly relative; if it's
   readable, return (a list containing) it; otherwise, return NULL.  */

static string_vector
absolute_search (const std::string& name)
{
  string_vector ret_list;
  std::string found = kpse_readable_file (name);

  /* Add `found' to the return list even if it's null; that tells
     the caller we didn't find anything.  */
  ret_list.append (std::string (found));

  return ret_list;
}

/* This is the hard case -- look for NAME in PATH.  If ALL is false,
   return the first file found.  Otherwise, search all elements of PATH.  */

static string_vector
path_search (const std::string& path_arg, const std::string& name,
	     bool must_exist, bool all)
{
  char *elt;
  string_vector ret_list;
  bool done = false;

  const char *path = path_arg.c_str ();

  for (elt = kpse_path_element (path); !done && elt;
       elt = kpse_path_element (NULL))
    {
      string_vector found;
      bool allow_disk_search = true;

      if (*elt == '!' && *(elt + 1) == '!')
	{
	  /* Those magic leading chars in a path element means don't
	     search the disk for this elt.  And move past the magic to
	     get to the name.  */
	  allow_disk_search = false;
	  elt += 2;
	}

      /* Do not touch the device if present */
      if (NAME_BEGINS_WITH_DEVICE (elt))
	{
	  while (IS_DIR_SEP (*(elt + 2)) && IS_DIR_SEP (*(elt + 3)))
	    {
	      *(elt + 2) = *(elt + 1);
	      *(elt + 1) = *elt;
	      elt++;
	    }
	}
      else
	{
	  /* We never want to search the whole disk.  */
	  while (IS_DIR_SEP (*elt) && IS_DIR_SEP (*(elt + 1)))
	    elt++;
	}

      /* Try ls-R, unless we're searching for texmf.cnf.  Our caller
	 (search), also tests first_search, and does the resetting.  */
      found = first_search
	? string_vector () : kpse_db_search (name, elt, all);

      /* Search the filesystem if (1) the path spec allows it, and either
         (2a) we are searching for texmf.cnf ; or
         (2b) no db exists; or
         (2c) no db's are relevant to this elt; or
         (3) MUST_EXIST && NAME was not in the db.
	 In (2*), `found' will be NULL.
	 In (3),  `found' will be an empty list. */

      if (allow_disk_search && found.empty ())
	{
	  str_llist_type *dirs = kpse_element_dirs (elt);

	  if (dirs && *dirs)
	    found = dir_list_search (dirs, name, all);
	}

      /* Did we find anything anywhere?  */
      if (! found.empty ())
	{
	  if (all)
	    ret_list.append (found);
	  else
	    {
	      ret_list.append (found[0]);
	      done = true;
	    }
	}
    }

  return ret_list;
}

/* Search PATH for ORIGINAL_NAME.  If ALL is false, or ORIGINAL_NAME is
   absolute_p, check ORIGINAL_NAME itself.  Otherwise, look at each
   element of PATH for the first readable ORIGINAL_NAME.

   Always return a list; if no files are found, the list will
   contain just NULL.  If ALL is true, the list will be
   terminated with NULL.  */

static string_vector
search (const std::string& path, const std::string& original_name,
	bool must_exist, bool all)
{
  string_vector ret_list;
  bool absolute_p;

  /* Make a leading ~ count as an absolute filename, and expand $FOO's.  */
  std::string name = kpse_expand (original_name);

  /* If the first name is absolute or explicitly relative, no need to
     consider PATH at all.  */
  absolute_p = kpse_absolute_p (name, true);

  if (KPSE_DEBUG_P (KPSE_DEBUG_SEARCH))
    DEBUGF4 ("start search (file=%s, must_exist=%d, find_all=%d, path=%s).\n",
             name.c_str (), must_exist, all, path.c_str ());

  /* Find the file(s). */
  ret_list = absolute_p ? absolute_search (name)
                        : path_search (path, name, must_exist, all);

  /* The very first search is for texmf.cnf.  We can't log that, since
     we want to allow setting TEXMFLOG in texmf.cnf.  */
  if (first_search)
    {
      first_search = false;
    }
  else
    {
      /* Record the filenames we found, if desired.  And wrap them in a
	 debugging line if we're doing that.  */

      if (KPSE_DEBUG_P (KPSE_DEBUG_SEARCH))
	DEBUGF1 ("search (%s) =>", original_name.c_str ());

      log_search (ret_list);

      if (KPSE_DEBUG_P (KPSE_DEBUG_SEARCH))
	putc ('\n', stderr);
    }

  return ret_list;
}

/* Search PATH for the first NAME.  */

std::string
kpse_path_search (const std::string& path, const std::string& name,
		  bool must_exist)
{
  string_vector ret_list = search (path, name, must_exist, false);

  return ret_list.empty () ? std::string () : ret_list[0];
}

/* Search all elements of PATH for files named NAME.  Not sure if it's
   right to assert `must_exist' here, but it suffices now.  */

string_vector
kpse_all_path_search (const std::string& path, const std::string& name)
{
  return search (path, name, true, true);
}

/* This is the hard case -- look in each element of PATH for each
   element of NAMES.  If ALL is false, return the first file found.
   Otherwise, search all elements of PATH.  */

static string_vector
path_find_first_of (const std::string& path_arg, const string_vector& names,
		    bool must_exist, bool all)
{
  char *elt;
  string_vector ret_list;
  bool done = false;

  const char *path = path_arg.c_str ();

  for (elt = kpse_path_element (path); !done && elt;
       elt = kpse_path_element (NULL))
    {
      str_llist_type *dirs;
      str_llist_elt_type *dirs_elt;
      string_vector found;
      bool allow_disk_search = true;

      if (*elt == '!' && *(elt + 1) == '!')
	{
	  /* Those magic leading chars in a path element means don't
	     search the disk for this elt.  And move past the magic to
	     get to the name.  */

	  allow_disk_search = false;
	  elt += 2;
	}

      /* Do not touch the device if present */

      if (NAME_BEGINS_WITH_DEVICE (elt))
	{
	  while (IS_DIR_SEP (*(elt + 2)) && IS_DIR_SEP (*(elt + 3)))
	    {
	      *(elt + 2) = *(elt + 1);
	      *(elt + 1) = *elt;
	      elt++;
	    }
	}
      else
	{
	  /* We never want to search the whole disk.  */
	  while (IS_DIR_SEP (*elt) && IS_DIR_SEP (*(elt + 1)))
	    elt++;
	}

      /* We have to search one directory at a time.  */
      dirs = kpse_element_dirs (elt);
      for (dirs_elt = *dirs; dirs_elt; dirs_elt = STR_LLIST_NEXT (*dirs_elt))
	{
	  const std::string dir = STR_LLIST (*dirs_elt);

	  int len = names.length ();
	  for (int i = 0; i < len && !done; i++)
	    {
	      std::string name = names[i];

	      /* Try ls-R, unless we're searching for texmf.cnf.  Our caller
		 (find_first_of), also tests first_search, and does the
		 resetting.  */
	      found = first_search
		? string_vector () : kpse_db_search (name, dir.c_str (), all);

	      /* Search the filesystem if (1) the path spec allows it,
		 and either

		   (2a) we are searching for texmf.cnf ; or
		   (2b) no db exists; or
		   (2c) no db's are relevant to this elt; or
		   (3) MUST_EXIST && NAME was not in the db.

		 In (2*), `found' will be NULL.
		 In (3),  `found' will be an empty list. */

	      if (allow_disk_search && found.empty ())
		{
		  static str_llist_type *tmp = 0;

		  if (! tmp)
		    {
		      tmp = new str_llist_type;
		      *tmp = NULL;
		      str_llist_add (tmp, "");
		    }

		  STR_LLIST (*(*tmp)) = dir;

		  found = dir_list_search (tmp, name, all);
		}

	      /* Did we find anything anywhere?  */
	      if (! found.empty ())
		{
		  if (all)
		    ret_list.append (found);
		  else
		    {
		      ret_list.append (found[0]);
		      done = true;
		    }
		}
	    }
	}
    }

  return ret_list;
}

static string_vector
find_first_of (const std::string& path, const string_vector& names,
	       bool must_exist, bool all)
{
  string_vector ret_list;

  if (KPSE_DEBUG_P (KPSE_DEBUG_SEARCH))
    {
      fputs ("start find_first_of ((", stderr);

      int len = names.length ();

      for (int i = 0; i < len; i++)
	{
	  if (i == 0)
	    fputs (names[i].c_str (), stderr);
	  else
	    fprintf (stderr, ", %s", names[i].c_str ());
	}

      fprintf (stderr, "), path=%s, must_exist=%d).\n",
	       path.c_str (), must_exist);
    }

  /* Find the file. */
  ret_list = path_find_first_of (path, names, must_exist, all);

  /* The very first search is for texmf.cnf.  We can't log that, since
     we want to allow setting TEXMFLOG in texmf.cnf.  */
  if (first_search)
    {
      first_search = false;
    }
  else
    {
      /* Record the filenames we found, if desired.  And wrap them in a
	 debugging line if we're doing that.  */

      if (KPSE_DEBUG_P (KPSE_DEBUG_SEARCH))
	{
	  fputs ("find_first_of (", stderr);

	  int len = names.length ();

	  for (int i = 0; i < len; i++)
	    {
	      if (i == 0)
		fputs (names[i].c_str (), stderr);
	      else
		fprintf (stderr, ", %s", names[i].c_str ());
	    }
	  fputs (") =>", stderr);
	}

      log_search (ret_list);

      if (KPSE_DEBUG_P (KPSE_DEBUG_SEARCH))
	putc ('\n', stderr);
    }

  return ret_list;
}

/* Search each element of PATH for each element of NAMES.  Return the
   first one found.  */

std::string
kpse_path_find_first_of (const std::string& path, const string_vector& names,
			 bool must_exist)
{
  string_vector ret_list = find_first_of (path, names, must_exist, false);

  return ret_list.empty () ? std::string () : ret_list[0];
}

/* Search each element of PATH for each element of NAMES and return a
   list containing everything found, in the order found.  */

string_vector
kpse_all_path_find_first_of (const std::string& path,
			     const string_vector& names)
{
  return find_first_of (path, names, true, true);
}

/* expand.c: general expansion.  Some of this file (the brace-expansion
   code from bash) is covered by the GPL; this is the only GPL-covered
   code in kpathsea.  The part of the file that I wrote (the first
   couple of functions) is covered by the LGPL.  */

/* If NAME has a leading ~ or ~user, Unix-style, expand it to the user's
   home directory, and return a new malloced string.  If no ~, or no
   <pwd.h>, just return NAME.  */

static std::string
kpse_tilde_expand (const std::string& name)
{
  std::string expansion;

  assert (! name.empty ());

  /* If no leading tilde, do nothing.  */
  if (name[0] != '~')
    {
      expansion = name;

      /* If a bare tilde, return the home directory or `.'.  (Very
	 unlikely that the directory name will do anyone any good, but
	 ...  */
    }
  else if (name.length () == 1)
    {
      expansion = octave_env::getenv ("HOME");

      if (expansion.empty ())
	expansion = ".";

      /* If `~/', remove any trailing / or replace leading // in $HOME.
	 Should really check for doubled intermediate slashes, too.  */
    }
  else if (IS_DIR_SEP (name[1]))
    {
      unsigned c = 1;
      std::string home = octave_env::getenv ("HOME");

      if (home.empty ())
	home = ".";

      size_t home_len = home.length ();

      /* handle leading // */
      if (home_len > 1 && IS_DIR_SEP (home[0]) && IS_DIR_SEP (home[1]))
	home = home.substr (1);

      /* omit / after ~ */
      if (IS_DIR_SEP (home[home_len - 1]))
	c++;

      expansion = home + name.substr (c);

      /* If `~user' or `~user/', look up user in the passwd database (but
	 OS/2 doesn't have this concept.  */
    }
  else
#ifdef HAVE_PWD_H
    {
      unsigned c = 2;

      /* find user name */
      while (name.length () > c && ! IS_DIR_SEP (name[c]))
        c++;

      std::string user = name.substr (1, c-1);

      /* We only need the cast here for (deficient) systems
         which do not declare `getpwnam' in <pwd.h>.  */
      octave_passwd p = octave_passwd::getpwnam (user);

      /* If no such user, just use `.'.  */
      std::string home = p ? p.dir () : std::string (".");

      if (home.empty ())
	home = ".";

      /* handle leading // */
      if (home.length () > 1 && IS_DIR_SEP (home[0]) && IS_DIR_SEP (home[1]))
        home = home.substr (1);

      /* If HOME ends in /, omit the / after ~user. */
      if (name.length () > c && IS_DIR_SEP (home[home.length () - 1]))
        c++;

      expansion = name.length () > c ? home : home + name.substr (c);
    }
#else /* not HAVE_PWD_H */
  expansion = name;
#endif /* not HAVE_PWD_H */

  return expansion;
}

/* Do variable expansion first so ~${USER} works.  (Besides, it's what the
   shells do.)  */

std::string
kpse_expand (const std::string& s)
{
  std::string var_expansion = kpse_var_expand (s);
  return kpse_tilde_expand (var_expansion);
}

/* Forward declarations of functions from the original expand.c  */
static char **brace_expand (const char *);
static void free_array (char **);

/* If $KPSE_DOT is defined in the environment, prepend it to any relative
   path components. */

static std::string
kpse_expand_kpse_dot (const std::string& path)
{
  std::string ret;
  char *elt;
  std::string kpse_dot = octave_env::getenv ("KPSE_DOT");

  if (kpse_dot.empty ())
    return path;

  char *tmp = xstrdup (path.c_str ());

  for (elt = kpse_path_element (tmp); elt; elt = kpse_path_element (NULL))
    {
      /* We assume that the !! magic is only used on absolute components.
	 Single "." get special treatment, as does "./" or its  equivalent.  */

      if (kpse_absolute_p (elt, false) || (elt[0] == '!' && elt[1] == '!'))
	ret += std::string (elt) + ENV_SEP_STRING;
      else if (elt[0] == '.' && elt[1] == 0)
	ret += std::string (kpse_dot) + ENV_SEP_STRING;
      else if (elt[0] == '.' && IS_DIR_SEP (elt[1]))
	ret += std::string (kpse_dot) + (elt + 1) + ENV_SEP_STRING;
      else
	ret += std::string (kpse_dot) + DIR_SEP_STRING + elt + ENV_SEP_STRING;
    }

  int len = ret.length ();

  if (len > 0)
    ret.resize (len - 1);

  return ret;
}

/* Do brace expansion on ELT; then do variable and ~ expansion on each
   element of the result; then do brace expansion again, in case a
   variable definition contained braces (e.g., $TEXMF).  Return a
   string comprising all of the results separated by ENV_SEP_STRING.  */

static std::string
kpse_brace_expand_element (const char *elt)
{
  unsigned i;
  char **expansions = brace_expand (elt);
  std::string ret;

  for (i = 0; expansions[i]; i++)
    {
      /* Do $ and ~ expansion on each element.  */
      std::string x = kpse_expand (expansions[i]);

      if (x != expansions[i])
	{
	  /* If we did any expansions, do brace expansion again.  Since
	     recursive variable definitions are not allowed, this recursion
	     must terminate.  (In practice, it's unlikely there will ever be
	     more than one level of recursion.)  */
	  x = kpse_brace_expand_element (x.c_str ());
	}

      ret += x + ENV_SEP_STRING;
    }

  free_array (expansions);

  ret.resize (ret.length () - 1);

  return ret;
}

/* Be careful to not waste all the memory we allocate for each element.  */

std::string
kpse_brace_expand (const char *path)
{
  char *elt;
  unsigned len;
  /* Must do variable expansion first because if we have
       foo = .:~
       TEXINPUTS = $foo
     we want to end up with TEXINPUTS = .:/home/karl.
     Since kpse_path_element is not reentrant, we must get all
     the path elements before we start the loop.  */
  std::string tmp = kpse_var_expand (path);
  const char *xpath = tmp.c_str ();
  std::string ret;

  for (elt = kpse_path_element (xpath); elt; elt = kpse_path_element (NULL))
    {
      /* Do brace expansion first, so tilde expansion happens in {~ka,~kb}.  */
      std::string expansion = kpse_brace_expand_element (elt);
      ret += expansion + ENV_SEP_STRING;
    }

  /* Waste the last byte by overwriting the trailing env_sep with a null.  */
  len = ret.length ();

  if (len > 0)
    ret.resize (len - 1);

  return kpse_expand_kpse_dot (ret);
}

/* Expand all special constructs in a path, and include only the actually
   existing directories in the result. */
std::string
kpse_path_expand (const std::string& path_arg)
{
  std::string ret;
  char *elt;
  unsigned len;

  const char *path = path_arg.c_str ();

  len = 0;

  /* Expand variables and braces first.  */
  std::string tmp = kpse_brace_expand (path);

  const char *xpath = tmp.c_str ();

  /* Now expand each of the path elements, printing the results */
  for (elt = kpse_path_element (xpath); elt; elt = kpse_path_element (NULL))
    {
      str_llist_type *dirs;

      /* Skip and ignore magic leading chars.  */
      if (*elt == '!' && *(elt + 1) == '!')
	elt += 2;

      /* Do not touch the device if present */
      if (NAME_BEGINS_WITH_DEVICE (elt))
	{
	  while (IS_DIR_SEP (*(elt + 2)) && IS_DIR_SEP (*(elt + 3)))
	    {
	      *(elt + 2) = *(elt + 1);
	      *(elt + 1) = *elt;
	      elt++;
	    }
	}
      else
	{
	  /* We never want to search the whole disk.  */
	  while (IS_DIR_SEP (*elt) && IS_DIR_SEP (*(elt + 1)))
	    elt++;
	}

    /* Search the disk for all dirs in the component specified.
       Be faster to check the database, but this is more reliable.  */
    dirs = kpse_element_dirs (elt);

    if (dirs && *dirs)
      {
	str_llist_elt_type *dir;

	for (dir = *dirs; dir; dir = STR_LLIST_NEXT (*dir))
	  {
	    const std::string thedir = STR_LLIST (*dir);
	    unsigned dirlen = thedir.length ();

	    /* Retain trailing slash if that's the root directory.  */
	    if (dirlen == 1 || (dirlen == 3 && NAME_BEGINS_WITH_DEVICE (thedir)
				&& IS_DIR_SEP (thedir[2])))
	      {
		ret += thedir + ENV_SEP_STRING;
		len += dirlen + 1;
		ret[len - 1] = ENV_SEP;
	      }
	    else
	      {
		ret += thedir;
		len += dirlen;
		ret [len - 1] = ENV_SEP;
	      }
	  }
      }
    }

  /* Get rid of trailing ':', if any. */
  if (len != 0)
    ret[len - 1] = 0;

  return ret;
}

/* braces.c -- code for doing word expansion in curly braces. Taken from
   bash 1.14.5.  [Ans subsequently modified for kpatshea.]

   Copyright (C) 1987,1991 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA.  */

#define brace_whitespace(c) (! (c) || (c) == ' ' || (c) == '\t' || (c) == '\n')

/* Basic idea:

   Segregate the text into 3 sections: preamble (stuff before an open brace),
   postamble (stuff after the matching close brace) and amble (stuff after
   preamble, and before postamble).  Expand amble, and then tack on the
   expansions to preamble.  Expand postamble, and tack on the expansions to
   the result so far.  */

/* The character which is used to separate arguments. */
static int brace_arg_separator = ',';

static int brace_gobbler (const char *, int *, int);
static char **expand_amble (const char *),
            **array_concat (char **, char **);

/* Return the length of ARRAY, a NULL terminated array of char *. */
static int
array_len (char **array)
{
  register int i;
  for (i = 0; array[i]; i++);
  return (i);
}

/* Free the contents of ARRAY, a NULL terminated array of char *. */
static void
free_array (char **array)
{
  register int i = 0;

  if (! array) return;

  while (array[i])
    free (array[i++]);

  free (array);
}

/* Allocate and return a new copy of ARRAY and its contents. */
static char **
copy_array (char **array)
{
  register int i;
  int len;
  char **new_array;

  len = array_len (array);

  new_array = (char **)xmalloc ((len + 1) * sizeof (char *));
  for (i = 0; array[i]; i++)
    new_array[i] = xstrdup (array[i]);
  new_array[i] = (char *)NULL;

  return (new_array);
}

/* Return an array of strings; the brace expansion of TEXT. */
static char **
brace_expand (const char *text)
{
  register int start;
  char *preamble, *amble;
  const char *postamble;
  char **tack, **result;
  int i, c;

  /* Find the text of the preamble. */
  i = 0;
  c = brace_gobbler (text, &i, '{');

  preamble = (char *) xmalloc (i + 1);
  strncpy (preamble, text, i);
  preamble[i] = 0;

  result = (char **) xmalloc (2 * sizeof (char *));
  result[0] = preamble;
  result[1] = NULL;

  /* Special case.  If we never found an exciting character, then
     the preamble is all of the text, so just return that. */
  if (c != '{')
    return (result);

  /* Find the amble.  This is the stuff inside this set of braces. */
  start = ++i;
  c = brace_gobbler (text, &i, '}');

  /* What if there isn't a matching close brace? */
  if (! c)
    {
      WARNING1 ("%s: Unmatched {", text);
      free (preamble);		/* Same as result[0]; see initialization. */
      result[0] = xstrdup (text);
      return (result);
    }

  amble = (char *) xmalloc (1 + (i - start));
  strncpy (amble, &text[start], (i - start));
  amble[i - start] = 0;

  postamble = &text[i + 1];

  tack = expand_amble (amble);
  result = array_concat (result, tack);
  free (amble);
  free_array (tack);

  tack = brace_expand (postamble);
  result = array_concat (result, tack);
  free_array (tack);

  return (result);
}

/* Expand the text found inside of braces.  We simply try to split the
   text at BRACE_ARG_SEPARATORs into separate strings.  We then brace
   expand each slot which needs it, until there are no more slots which
   need it. */
static char **
expand_amble (const char *text)
{
  char **result, **partial;
  char *tem;
  int start, i, c;

  result = NULL;

  for (start = 0, i = 0, c = 1; c; start = ++i)
    {
      int c0, c1;
      int i0, i1;
      i0 = i;
      c0 = brace_gobbler (text, &i0, brace_arg_separator);
      i1 = i;
      c1 = brace_gobbler (text, &i1, ENV_SEP);
      c = c0 | c1;
      i = (i0 < i1 ? i0 : i1);

      tem = (char *) xmalloc (1 + (i - start));
      strncpy (tem, &text[start], (i - start));
      tem[i- start] = 0;

      partial = brace_expand (tem);

      if (! result)
	result = partial;
      else
	{
	  register int lr = array_len (result);
	  register int lp = array_len (partial);
	  register int j;

	  result = (char **) xrealloc (result, (1 + lp + lr) * sizeof (char *));

	  for (j = 0; j < lp; j++)
	    result[lr + j] = partial[j];

	  result[lr + j] = NULL;
	  free (partial);
	}
      free (tem);
    }
  return (result);
}

/* Return a new array of strings which is the result of appending each
   string in ARR2 to each string in ARR1.  The resultant array is
   len (arr1) * len (arr2) long.  For convenience, ARR1 (and its contents)
   are free ()'ed.  ARR1 can be NULL, in that case, a new version of ARR2
   is returned. */
static char **
array_concat (char **arr1, char **arr2)
{
  register int i, j, len, len1, len2;
  register char **result;

  if (! arr1)
    return (copy_array (arr2));

  if (! arr2)
    return (copy_array (arr1));

  len1 = array_len (arr1);
  len2 = array_len (arr2);

  result = (char **) xmalloc ((1 + (len1 * len2)) * sizeof (char *));

  len = 0;
  for (i = 0; i < len2; i++)
    {
      int strlen_2 = strlen (arr2[i]);

      for (j = 0; j < len1; j++)
	{
	  int strlen_1 = strlen (arr1[j]);

	  result[len] = (char *) xmalloc (1 + strlen_1 + strlen_2);
	  strcpy (result[len], arr1[j]);
	  strcpy (result[len] + strlen_1, arr2[i]);
	  len++;
	}
    }
  free_array (arr1);

  result[len] = NULL;
  return (result);
}

/* Start at INDEX, and skip characters in TEXT. Set INDEX to the
   index of the character matching SATISFY.  This understands about
   quoting.  Return the character that caused us to stop searching;
   this is either the same as SATISFY, or 0. */
static int
brace_gobbler (const char *text, int *indx, int satisfy)
{
  register int i, c, quoted, level, pass_next;

  level = quoted = pass_next = 0;

  for (i = *indx; (c = text[i]); i++)
    {
      if (pass_next)
	{
	  pass_next = 0;
	  continue;
	}

      /* A backslash escapes the next character.  This allows backslash to
	 escape the quote character in a double-quoted string. */
      if (c == '\\' && (quoted == 0 || quoted == '"' || quoted == '`'))
        {
          pass_next = 1;
          continue;
        }

      if (quoted)
	{
	  if (c == quoted)
	    quoted = 0;
	  continue;
	}

      if (c == '"' || c == '\'' || c == '`')
	{
	  quoted = c;
	  continue;
	}

      if (c == satisfy && !level && !quoted)
	{
	  /* We ignore an open brace surrounded by whitespace, and also
	     an open brace followed immediately by a close brace, that
	     was preceded with whitespace.  */
	  if (c == '{' &&
	      ((! i || brace_whitespace (text[i - 1])) &&
	       (brace_whitespace (text[i + 1]) || text[i + 1] == '}')))
	    continue;
	  /* If this is being compiled as part of bash, ignore the `{'
	     in a `${}' construct */
	  if ((c != '{') || !i || (text[i - 1] != '$'))
	    break;
	}

      if (c == '{')
	level++;
      else if (c == '}' && level)
	level--;
    }

  *indx = i;
  return (c);
}

/* db.c: an external database to avoid filesystem lookups.  */

#ifndef DEFAULT_TEXMFDBS
#define DEFAULT_TEXMFDBS "/usr/local/share/texmf:/var/tmp/texfonts"
#endif

/* Perhaps we could use this for path values themselves; for now, we use
   it only for the program_enabled_p value.  */
typedef enum
{
  kpse_src_implicit,   /* C initialization to zero */
  kpse_src_compile,    /* configure/compile-time default */
  kpse_src_texmf_cnf,  /* texmf.cnf, the kpathsea config file */
  kpse_src_client_cnf, /* application config file, e.g., config.ps */
  kpse_src_env,        /* environment variable */
  kpse_src_x,          /* X Window System resource */
  kpse_src_cmdline     /* command-line option */
} kpse_src_type;

/* For each file format, we record the following information.  The main
   thing that is not part of this structure is the environment variable
   lists. They are used directly in tex-file.c. We could incorporate
   them here, but it would complicate the code a bit. We could also do
   it via variable expansion, but not now, maybe not ever:
   ${PKFONTS-${TEXFONTS-/usr/local/lib/texmf/fonts//}}.  */

typedef struct
{
  std::string type;	     /* Human-readable description.  */
  std::string path;	     /* The search path to use.  */
  std::string raw_path;	     /* Pre-$~ (but post-default) expansion.  */
  std::string path_source;   /* Where the path started from.  */
  std::string override_path; /* From client environment variable.  */
  std::string client_path;   /* E.g., from dvips's config.ps.  */
  std::string cnf_path;	     /* From texmf.cnf.  */
  std::string default_path;  /* If all else fails.  */
  const char **suffix;	     /* For kpse_find_file to check for/append.  */
  const char **alt_suffix;   /* More suffixes to check for.  */
  bool suffix_search_only;   /* Only search with a suffix?  */
  std::string program;	     /* ``mktexpk'', etc.  */
  std::string program_args;  /* Args to `program'.  */
  bool program_enabled_p;    /* Invoke `program'?  */
  kpse_src_type program_enable_level; /* Who said to invoke `program'.  */
  bool binmode;		     /* The files must be opened in binary mode. */
} kpse_format_info_type;

/* The sole variable of that type, indexed by `kpse_file_format_type'.
   Initialized by calls to `kpse_find_file' for `kpse_init_format'.  */
static kpse_format_info_type kpse_format_info;

#define DB_ENVS "TEXMFDBS"

/* And EXPAND_DEFAULT calls kpse_expand_default on try_path and the
   present info->path.  */
#define EXPAND_DEFAULT(try_path, source_string)			\
  if (! try_path.empty ())					\
    {								\
      info->raw_path = try_path;				\
      info->path = kpse_expand_default (try_path.c_str (), (info->path).c_str ());	\
      info->path_source = source_string;			\
    }

/* Find the final search path to use for the format entry INFO, given
   the compile-time default (DEFAULT_PATH), and the environment
   variables to check (the remaining arguments, terminated with NULL).
   We set the `path' and `path_source' members of INFO.  The
   `client_path' member must already be set upon entry.  */

static void
init_path (kpse_format_info_type *info, const char *default_path, ...)
{
  char *env_name;
  char *var = NULL;
  va_list ap;

  va_start (ap, default_path);

  info->default_path = default_path;

  /* First envvar that's set to a nonempty value will exit the loop.  If
     none are set, we want the first cnf entry that matches.  Find the
     cnf entries simultaneously, to avoid having to go through envvar
     list twice -- because of the PVAR?C macro, that would mean having
     to create a str_list and then use it twice.  Yuck.  */

  while ((env_name = va_arg (ap, char *)) != NULL)
    {
      /* Since sh doesn't like envvar names with `.', check PATH_prog
	 rather than PATH.prog.  */

      if (! var)
	{
	  /* Try simply PATH.  */
	  std::string env_value = octave_env::getenv (env_name);

	  if (! env_value.empty ())
	    var = env_name;
	}

      if (var && ! info->cnf_path.empty ())
	break;
    }

  va_end (ap);

  /* Expand any extra :'s.  For each level, we replace an extra : with
     the path at the next lower level.  For example, an extra : in a
     user-set envvar should be replaced with the path from the cnf file.
     things are complicated because none of the levels above the very
     bottom are guaranteed to exist.  */

  /* Assume we can reliably start with the compile-time default.  */
  info->path = info->raw_path = info->default_path;
  info->path_source = "compile-time paths.h";

  EXPAND_DEFAULT (info->cnf_path, "texmf.cnf");
  EXPAND_DEFAULT (info->client_path, "program config file");

  if (var)
    {
      std::string val = octave_env::getenv (var);
      EXPAND_DEFAULT (val, concat (var, " environment variable"));
    }

  EXPAND_DEFAULT (info->override_path, "application override variable");
  std::string tmp = kpse_brace_expand ((info->path).c_str ());
  info->path = tmp;
}

/* Some file types have more than one suffix.  */

static void
add_suffixes (const char ***list, ...)
{
  const char *s;
  unsigned count = 0;
  va_list ap;

  va_start (ap, list);

  while ((s = va_arg (ap, char *)) != NULL)
    {
      count++;
      XRETALLOC (*list, count + 1, const char *);
      (*list)[count - 1] = s;
    }

  va_end (ap);

  (*list)[count] = NULL;
}

static std::string
remove_dbonly (const std::string& path)
{
  std::string ret = path;
  size_t path_len = path.length ();

  size_t i = 0, j = 0;

  bool new_elt = true;

  while (i < path_len)
    {
      if (new_elt && i + 1 < path_len && path[i] == '!' && path[i+1] == '!')
	i += 2;
      else
	{
	  new_elt = (path[i] == ENV_SEP);
	  ret[j++] = path[i++];
	}
    }

  ret.resize (j);

  return ret;
}

/* Initialize everything for FORMAT.  */

static std::string
kpse_init_format (void)
{
  /* If we get called twice, don't redo all the work.  */
  if (! kpse_format_info.path.empty ())
    return kpse_format_info.path;

  kpse_format_info.type = "ls-R";
  init_path (&kpse_format_info, DEFAULT_TEXMFDBS, DB_ENVS, NULL);
  add_suffixes (&kpse_format_info.suffix, "ls-R", NULL);
  kpse_format_info.path = remove_dbonly (kpse_format_info.path);

#ifdef KPSE_DEBUG
#define MAYBE(member) \
  (kpse_format_info.member.empty () \
    ? "(none)" : kpse_format_info.member.c_str ())

  /* Describe the monster we've created.  */
  if (KPSE_DEBUG_P (KPSE_DEBUG_PATHS))
    {
      DEBUGF2 ("Search path for %s files (from %s)\n",
	       kpse_format_info.type.c_str (),
	       kpse_format_info.path_source.c_str ());

      DEBUGF1 ("  = %s\n", kpse_format_info.path.c_str ());

      DEBUGF1 ("  before expansion = %s\n",
	       kpse_format_info.raw_path.c_str ());

      DEBUGF1 ("  application override path = %s\n", MAYBE (override_path));

      DEBUGF1 ("  application config file path = %s\n", MAYBE (client_path));

      DEBUGF1 ("  texmf.cnf path = %s\n", MAYBE (cnf_path));

      DEBUGF1 ("  compile-time path = %s\n", MAYBE (default_path));

      DEBUGF  ("  default suffixes =");

      if (kpse_format_info.suffix)
	{
	  const char **ext;
	  for (ext = kpse_format_info.suffix; ext && *ext; ext++)
	    {
	      fprintf (stderr, " %s", *ext);
	    }
	  putc ('\n', stderr);
	}
      else
	{
	  fputs (" (none)\n", stderr);
	}

      DEBUGF  ("  other suffixes =");

      if (kpse_format_info.alt_suffix)
	{
	  const char **alt;
	  for (alt = kpse_format_info.alt_suffix; alt && *alt; alt++)
	    {
	      fprintf (stderr, " %s", *alt);
	    }
	  putc ('\n', stderr);
	}
      else
	{
	  fputs (" (none)\n", stderr);
	}

      DEBUGF1 ("  search only with suffix = %d\n",
	       kpse_format_info.suffix_search_only);

      DEBUGF1 ("  runtime generation program = %s\n", MAYBE (program));

      DEBUGF1 ("  extra program args = %s\n", MAYBE (program_args));

      DEBUGF1 ("  program enabled = %d\n",
	       kpse_format_info.program_enabled_p);

      DEBUGF1 ("  program enable level = %d\n",
	       kpse_format_info.program_enable_level);
    }
#endif /* KPSE_DEBUG */

  return kpse_format_info.path;
}

static hash_table_type db; /* The hash table for all the ls-R's.  */
/* SMALL: The old size of the hash table was 7603, with the assumption
   that a minimal ls-R bas about 3500 entries.  But a typical ls-R will
   be more like double that size.  */
#ifndef DB_HASH_SIZE
#define DB_HASH_SIZE 15991
#endif
#ifndef DB_NAME
#define DB_NAME "ls-R"
#endif

static hash_table_type alias_db;
#ifndef ALIAS_NAME
#define ALIAS_NAME "aliases"
#endif
#ifndef ALIAS_HASH_SIZE
#define ALIAS_HASH_SIZE 1009
#endif

static string_vector db_dir_list;

/* If DIRNAME contains any element beginning with a `.' (that is more
   than just `./'), return true.  This is to allow ``hidden''
   directories -- ones that don't get searched.  */

static bool
ignore_dir_p (const std::string& dirname_arg)
{
  const char *dirname = dirname_arg.c_str ();

  const char *dot_pos = dirname;

  while ((dot_pos = strchr (dot_pos + 1, '.')))
    {
      /* If / before and no / after, skip it. */
      if (IS_DIR_SEP (dot_pos[-1]) && dot_pos[1] && !IS_DIR_SEP (dot_pos[1]))
	return true;
    }

  return false;
}

static bool
read_line (FILE *f, std::string& line)
{
  bool read_something = false;

  int c;

  OSSTREAM buf;

  while ((c = getc (f)) != EOF)
    {
      read_something = true;

      if (c == '\n' || c == '\r')
	break;

      buf << static_cast<char> (c);
    }

  /* If we read anything, return it.  This can't represent a last
     ``line'' which doesn't end in a newline, but so what.  */
  if (read_something)
    {
      /* Absorb LF of a CRLF pair. */
      if (c == '\r')
	{
          c = getc (f);
          if (c != '\n')
	    ungetc (c, f);
	}

      buf << OSSTREAM_ENDS;
      line = OSSTREAM_STR (buf);
      OSSTREAM_FREEZE (buf);
    }

  return read_something;
}

/* If no DB_FILENAME, return false (maybe they aren't using this feature).
   Otherwise, add entries from DB_FILENAME to TABLE, and return true.  */

static bool
db_build (hash_table_type *table, const std::string& db_filename)
{
  std::string line;

  unsigned dir_count = 0, file_count = 0, ignore_dir_count = 0;

  unsigned len = db_filename.length () - sizeof (DB_NAME) + 1; /* Keep the /. */
  std::string top_dir = db_filename.substr (0, len);

  std::string cur_dir;

  FILE *db_file = xfopen (db_filename, "r");

  if (db_file)
    {
      while (read_line (db_file, line))
	{
	  len = line.length ();

	  /* A line like `/foo:' = new dir foo.  Allow both absolute (/...)
	     and explicitly relative (./...) names here.  It's a kludge to
	     pass in the directory name with the trailing : still attached,
	     but it doesn't actually hurt.  */
	  if (len > 0 && line[len - 1] == ':' && kpse_absolute_p (line, true))
	    {
	      /* New directory line.  */
	      if (! ignore_dir_p (line))
		{
		  /* If they gave a relative name, prepend full
		     directory name now.  */
		  line[len - 1] = DIR_SEP;

		  /* Skip over leading `./', it confuses `match' and
		     is just a waste of space, anyway.  This will lose
		     on `../', but `match' won't work there, either,
		     so it doesn't matter.  */

		  cur_dir = line[0] == '.' ? top_dir + line.substr (2) : line;

		  dir_count++;
		}
	      else
		{
		  cur_dir = std::string ();
		  ignore_dir_count++;
		}

	      /* Ignore blank, `.' and `..' lines.  */

	    }
	  else if (len > 0 && ! cur_dir.empty ()   /* a file line? */
		   && ! (line[0] == '.'
			 && (len == 1 || (len == 2 && line[1] == '.'))))
	    {
	      /* Make a new hash table entry with a key of `line' and
		 a data of `cur_dir'.  An already-existing identical
		 key is ok, since a file named `foo' can be in more
		 than one directory.  Share `cur_dir' among all its
		 files (and hence never free it). */
	      hash_insert (table, line, cur_dir);
	      file_count++;
	    }
	}

      xfclose (db_file, db_filename);

      if (file_count == 0)
	{
	  WARNING1 ("kpathsea: No usable entries in %s", db_filename.c_str ());
	  WARNING ("kpathsea: See the manual for how to generate ls-R");
	  db_file = NULL;
	}
      else
	db_dir_list.append (std::string (top_dir));

#ifdef KPSE_DEBUG
      if (KPSE_DEBUG_P (KPSE_DEBUG_HASH))
	{
	  /* Don't make this a debugging bit, since the output is so
	     voluminous, and being able to specify -1 is too useful.
	     Instead, let people who want it run the program under
	     a debugger and change the variable that way.  */
	  bool hash_summary_only = true;

	  DEBUGF4 ("%s: %u entries in %d directories (%d hidden).\n",
		   db_filename.c_str (), file_count, dir_count,
		   ignore_dir_count);

	  DEBUGF ("ls-R hash table:");
	  hash_print (*table, hash_summary_only);
	  fflush (stderr);
	}
#endif /* KPSE_DEBUG */
    }

  return db_file != NULL;
}

/* Insert FNAME into the hash table.  This is for files that get built
   during a run.  We wouldn't want to reread all of ls-R, even if it got
   rebuilt.  */

void
kpse_db_insert (const char *passed_fname)
{
  /* We might not have found ls-R, or even had occasion to look for it
     yet, so do nothing if we have no hash table.  */
  if (db.buckets)
    {
      const char *dir_part;
      char *fname = xstrdup (passed_fname);
      char *baseptr = xbasename (fname);
      const char *file_part = xstrdup (baseptr);

      *baseptr = '\0';  /* Chop off the filename.  */
      dir_part = fname; /* That leaves the dir, with the trailing /.  */

      hash_insert (&db, file_part, dir_part);
    }
}

/* Return true if FILENAME could be in PATH_ELT, i.e., if the directory
   part of FILENAME matches PATH_ELT.  Have to consider // wildcards, but
   $ and ~ expansion have already been done.  */

static bool
match (const std::string& filename_arg, const std::string& path_elt_arg)
{
  const char *filename = filename_arg.c_str ();
  const char *path_elt = path_elt_arg.c_str ();

  const char *original_filename = filename;
  bool matched = false;

  for (; *filename && *path_elt; filename++, path_elt++)
    {
      if (FILECHARCASEEQ (*filename, *path_elt)) /* normal character match */
	;

      else if (IS_DIR_SEP (*path_elt)  /* at // */
	       && original_filename < filename && IS_DIR_SEP (path_elt[-1]))
	{
	  while (IS_DIR_SEP (*path_elt))
	    path_elt++; /* get past second and any subsequent /'s */

	  if (*path_elt == 0)
	    {
	      /* Trailing //, matches anything. We could make this
		 part of the other case, but it seems pointless to do
		 the extra work.  */
	      matched = true;
	      break;
	    }
	  else
	    {
	      /* Intermediate //, have to match rest of PATH_ELT.  */
	      for (; !matched && *filename; filename++)
		{
		  /* Try matching at each possible character.  */
		  if (IS_DIR_SEP (filename[-1])
		      && FILECHARCASEEQ (*filename, *path_elt))
		    matched = match (filename, path_elt);
		}

	      /* Prevent filename++ when *filename='\0'. */
	      break;
	    }
	}
      else
	/* normal character nonmatch, quit */
	break;
    }

  /* If we've reached the end of PATH_ELT, check that we're at the last
     component of FILENAME, we've matched.  */
  if (! matched && *path_elt == 0)
    {
      /* Probably PATH_ELT ended with `vf' or some such, and FILENAME
	 ends with `vf/ptmr.vf'.  In that case, we'll be at a
	 directory separator.  On the other hand, if PATH_ELT ended
	 with a / (as in `vf/'), FILENAME being the same `vf/ptmr.vf',
	 we'll be at the `p'.  Upshot: if we're at a dir separator in
	 FILENAME, skip it.  But if not, that's ok, as long as there
	 are no more dir separators.  */

      if (IS_DIR_SEP (*filename))
	filename++;

      while (*filename && !IS_DIR_SEP (*filename))
	filename++;

      matched = *filename == 0;
    }

  return matched;
}

/* If DB_DIR is a prefix of PATH_ELT, return true; otherwise false.
   That is, the question is whether to try the db for a file looked up
   in PATH_ELT.  If PATH_ELT == ".", for example, the answer is no. If
   PATH_ELT == "/usr/local/lib/texmf/fonts//tfm", the answer is yes.

   In practice, ls-R is only needed for lengthy subdirectory
   comparisons, but there's no gain to checking PATH_ELT to see if it is
   a subdir match, since the only way to do that is to do a string
   search in it, which is all we do anyway.  */

static bool
elt_in_db (const std::string& db_dir, const std::string& path_elt)
{
  bool found = false;

  size_t db_dir_len = db_dir.length ();
  size_t path_elt_len = path_elt.length ();

  size_t i = 0;

  while (! found && FILECHARCASEEQ (db_dir[i], path_elt[i]))
    {
      i++;
      /* If we've matched the entire db directory, it's good.  */
      if (i == db_dir_len)
	found = true;

    /* If we've reached the end of PATH_ELT, but not the end of the db
       directory, it's no good.  */
      else if (i == path_elt_len)
	break;
    }

  return found;
}

/* If ALIAS_FILENAME exists, read it into TABLE.  */

static bool
alias_build (hash_table_type *table, const std::string& alias_filename)
{
  unsigned count = 0;

  FILE *alias_file = xfopen (alias_filename, "r");

  if (alias_file)
    {
      std::string line;

      while (read_line (alias_file, line))
	{
	  size_t len = line.length ();

	  /* comments or empty */
	  if (len == 0 || line[0] == '%' || line[0] == '#')
	    /* do nothing */ ;
	  else
	    {
	      size_t i = 0;

	      while (i < len && isspace (line[i]))
		i++;

	      size_t real_beg = i;

	      while (i < len && ! isspace (line[i]))
		i++;

	      size_t real_len = i - real_beg;

	      while (i < len && isspace (line[i]))
		i++;

	      size_t alias_beg = i;

	      while (i < len && ! isspace (line[i]))
		i++;

	      size_t alias_len = i - alias_beg;

	      /* Is the check for errors strong enough?  Should we
		 warn the user for potential errors?  */
	      if (real_len > 0 && alias_len > 0)
		{
		  hash_insert (table, line.substr (alias_beg, alias_len),
			       line.substr (real_beg, real_len));
		  count++;
		}
	    }
	}

#ifdef KPSE_DEBUG
      if (KPSE_DEBUG_P (KPSE_DEBUG_HASH))
	{
	  /* As with ls-R above ... */
	  bool hash_summary_only = true;
	  DEBUGF2 ("%s: %u aliases.\n", alias_filename.c_str (), count);
	  DEBUGF ("alias hash table:");
	  hash_print (*table, hash_summary_only);
	  fflush (stderr);
	}
#endif /* KPSE_DEBUG */

      xfclose (alias_file, alias_filename);
    }

  return alias_file != NULL;
}

/* Initialize the path for ls-R files, and read them all into the hash
   table `db'.  If no usable ls-R's are found, set db.buckets to NULL.  */

void
kpse_init_db (void)
{
  bool ok = false;
  const std::string db_path = kpse_init_format ();
  string_vector db_files = kpse_all_path_search (db_path.c_str (), DB_NAME);

  /* Must do this after the path searching (which ends up calling
    kpse_db_search recursively), so db.buckets stays NULL.  */
  db = hash_create (DB_HASH_SIZE);

  int len = db_files.length ();
  for (int i = 0; i < len; i++)
    {
      if (! db_files[i].empty ())
	{
	  if (db_build (&db, db_files[i]))
	    ok = true;
	}
    }

  if (! ok)
    {
      /* If db can't be built, leave `size' nonzero (so we don't
	 rebuild it), but clear `buckets' (so we don't look in it).  */
      free (db.buckets);
      db.buckets = NULL;
    }

  /* Add the content of any alias databases.  There may exist more than
     one alias file along DB_NAME files.  This duplicates the above code
     -- should be a function.  */
  ok = false;
  db_files = kpse_all_path_search (db_path.c_str (), ALIAS_NAME);

  alias_db = hash_create (ALIAS_HASH_SIZE);

  len = db_files.length ();
  for (int i = 0; i < len; i++)
    {
      if (! db_files[i].empty ())
	{
	  if (alias_build (&alias_db, db_files[i]))
	    ok = true;
	}
    }

  if (! ok)
    {
      free (alias_db.buckets);
      alias_db.buckets = NULL;
    }
}

/* Avoid doing anything if this PATH_ELT is irrelevant to the databases. */

string_vector
kpse_db_search (const std::string& name_arg,
		const std::string& orig_path_elt_arg, bool all)
{
  const char *last_slash;
  char *path_elt;
  bool done;
  string_vector ret;
  string_vector aliases;
  bool relevant = false;

  const char *name = name_arg.c_str ();
  const char *orig_path_elt = orig_path_elt_arg.c_str ();

  /* If we failed to build the database (or if this is the recursive
     call to build the db path), quit.  */
  if (db.buckets == NULL)
    return ret;

  /* When tex-glyph.c calls us looking for, e.g., dpi600/cmr10.pk, we
     won't find it unless we change NAME to just `cmr10.pk' and append
     `/dpi600' to PATH_ELT.  We are justified in using a literal `/'
     here, since that's what tex-glyph.c unconditionally uses in
     DPI_BITMAP_SPEC.  But don't do anything if the / begins NAME; that
     should never happen.  */
  last_slash = strrchr (name, '/');
  if (last_slash && last_slash != name)
    {
      unsigned len = last_slash - name + 1;
      char *dir_part = (char *) xmalloc (len);
      strncpy (dir_part, name, len - 1);
      dir_part[len - 1] = 0;
      path_elt = concat3 (orig_path_elt, "/", dir_part);
      name = last_slash + 1;
    }
  else
    path_elt = (char *) orig_path_elt;

  /* Don't bother doing any lookups if this `path_elt' isn't covered by
     any of database directories.  We do this not so much because the
     extra couple of hash lookups matter -- they don't -- but rather
     because we want to return NULL in this case, so path_search can
     know to do a disk search.  */
  for (int e = 0; !relevant && e < db_dir_list.length (); e++)
    relevant = elt_in_db (db_dir_list[e], path_elt);

  if (! relevant)
    return ret;

  /* If we have aliases for this name, use them.  */
  if (alias_db.buckets)
    aliases = hash_lookup (alias_db, name);

  /* Push aliases up by one and insert the original name at the front.  */
  int len = aliases.length ();
  aliases.resize (len+1);
  for (int i = len; i > 0; i--)
    aliases[i] = aliases[i - 1];
  aliases[0] = name;

  done = false;
  len = aliases.length ();
  for (int i = 0; i < len && !done; i++)
    {
      std::string atry = aliases[i];

      /* We have an ls-R db.  Look up `atry'.  */
      string_vector db_dirs = hash_lookup (db, atry);

      /* For each filename found, see if it matches the path element.  For
	 example, if we have .../cx/cmr10.300pk and .../ricoh/cmr10.300pk,
	 and the path looks like .../cx, we don't want the ricoh file.  */

      int db_dirs_len = db_dirs.length ();
      for (int j = 0; j < db_dirs_len && !done; j++)
	{
	  std::string db_file = db_dirs[j] + atry;
	  bool matched = match (db_file, path_elt);

#ifdef KPSE_DEBUG
	  if (KPSE_DEBUG_P (KPSE_DEBUG_SEARCH))
	    DEBUGF3 ("db:match (%s,%s) = %d\n", db_file.c_str (), path_elt, matched);
#endif

	  /* We got a hit in the database.  Now see if the file actually
	     exists, possibly under an alias.  */
	  if (matched)
	    {
	      std::string found;
	      std::string tmp = kpse_readable_file (db_file);
	      if (! tmp.empty ())
		found = db_file;
	      else
		{
		  /* The hit in the DB doesn't exist in disk.  Now try
		     all its aliases.  For example, suppose we have a
		     hierarchy on CD, thus `mf.bas', but ls-R contains
		     `mf.base'.  Find it anyway.  Could probably work
		     around this with aliases, but this is pretty easy
		     and shouldn't hurt.  The upshot is that if one of
		     the aliases actually exists, we use that.  */

		  int aliases_len = aliases.length ();

		  for (int k = 1; k < aliases_len && found.empty (); k++)
		    {
		      std::string atry = db_dirs[j] + aliases[k];
		      std::string tmp = kpse_readable_file (atry);
		      if (! tmp.empty ())
			found = atry;
		    }
		}

	      /* If we have a real file, add it to the list, maybe done.  */
	      if (! found.empty ())
		{
		  ret.append (found);

		  if (! (all || found.empty ()))
		    done = true;
		}
	    }
	}
    }

  /* If we had to break up NAME, free the temporary PATH_ELT.  */
  if (path_elt != orig_path_elt)
    free (path_elt);

  return ret;
}

/* kdefault.c: Expand extra colons.  */

/* Check for leading colon first, then trailing, then doubled, since
   that is fastest.  Usually it will be leading or trailing.  */

char *
kpse_expand_default (const char *path, const char *fallback)
{
  unsigned path_length;
  char *expansion;

  /* The default path better not be null.  */
  assert (fallback);

  if (path == NULL)
    expansion = xstrdup (fallback);

  /* Solitary or leading :?  */
  else if (IS_ENV_SEP (*path))
    {
      expansion = path[1] == 0 ? xstrdup (fallback) : concat (fallback, path);
    }

  /* Sorry about the assignment in the middle of the expression, but
     conventions were made to be flouted and all that.  I don't see the
     point of calling strlen twice or complicating the logic just to
     avoid the assignment (especially now that I've pointed it out at
     such great length).  */
  else if (path[(path_length = strlen (path)) - 1] == ENV_SEP)
    expansion = concat (path, fallback);

  /* OK, not leading or trailing.  Check for doubled.  */
  else
    {
      const char *loc;

      /* What we'll return if we find none.  */
      expansion = xstrdup (path);

      for (loc = path; *loc; loc++)
        {
          if (IS_ENV_SEP (loc[0]) && IS_ENV_SEP (loc[1]))
            { /* We have a doubled colon.  */
              expansion = (char *) xmalloc (path_length + strlen (fallback) + 1);

              /* Copy stuff up to and including the first colon.  */
              strncpy (expansion, path, loc - path + 1);
              expansion[loc - path + 1] = 0;

              /* Copy in FALLBACK, and then the rest of PATH.  */
              strcat (expansion, fallback);
              strcat (expansion, loc + 1);

	      break;
            }
        }
    }

  return expansion;
}

/* elt-dirs.c: Translate a path element to its corresponding
   director{y,ies}.  */

/* To avoid giving prototypes for all the routines and then their real
   definitions, we give all the subroutines first.  The entry point is
   the last routine in the file.  */

/* Make a copy of DIR (unless it's null) and save it in L.  Ensure that
   DIR ends with a DIR_SEP for the benefit of later searches.  */

static void
dir_list_add (str_llist_type *l, const std::string& dir)
{
  char last_char = dir[dir.length () - 1];
  std::string saved_dir = dir;
  if (IS_DIR_SEP (last_char) || IS_DEVICE_SEP (last_char))
    saved_dir += DIR_SEP_STRING;

  str_llist_add (l, saved_dir);
}

/* Return true if FN is a directory or a symlink to a directory,
   false if not. */

static bool
dir_p (const std::string& fn)
{
#ifdef WIN32
  unsigned int fa = GetFileAttributes (fn.c_str ());
  return (fa != 0xFFFFFFFF && (fa & FILE_ATTRIBUTE_DIRECTORY));
#else
  struct stat stats;
  return stat (fn.c_str (), &stats) == 0 && S_ISDIR (stats.st_mode);
#endif
}

/* If DIR is a directory, add it to the list L.  */

static void
checked_dir_list_add (str_llist_type *l, const std::string& dir)
{
  if (dir_p (dir))
    dir_list_add (l, dir);
}

/* The cache.  Typically, several paths have the same element; for
   example, /usr/local/lib/texmf/fonts//.  We don't want to compute the
   expansion of such a thing more than once.  Even though we also cache
   the dir_links call, that's not enough -- without this path element
   caching as well, the execution time doubles.  */

typedef struct
{
  const char *key;
  str_llist_type *value;
} cache_entry;

static cache_entry *the_cache = NULL;
static unsigned cache_length = 0;

/* Associate KEY with VALUE.  We implement the cache as a simple linear
   list, since it's unlikely to ever be more than a dozen or so elements
   long.  We don't bother to check here if PATH has already been saved;
   we always add it to our list.  We copy KEY but not VALUE; not sure
   that's right, but it seems to be all that's needed.  */

static void
cache (const char *key, str_llist_type *value)
{
  cache_length++;
  XRETALLOC (the_cache, cache_length, cache_entry);
  the_cache[cache_length - 1].key = xstrdup (key);
  the_cache[cache_length - 1].value = value;
}

/* To retrieve, just check the list in order.  */

static str_llist_type *
cached (const char *key)
{
  unsigned p;

  for (p = 0; p < cache_length; p++)
    {
      if (FILESTRCASEEQ (the_cache[p].key, key))
        return the_cache[p].value;
    }

  return NULL;
}

/* Handle the magic path constructs.  */

/* Declare recursively called routine.  */
static void expand_elt (str_llist_type *, const char *, unsigned);

/* POST is a pointer into the original element (which may no longer be
   ELT) to just after the doubled DIR_SEP, perhaps to the null.  Append
   subdirectories of ELT (up to ELT_LENGTH, which must be a /) to
   STR_LIST_PTR.  */

#ifdef WIN32
/* Shared across recursive calls, it acts like a stack. */
static char dirname[MAX_PATH];
#endif

static void
do_subdir (str_llist_type *str_list_ptr, const char *elt,
	   unsigned elt_length, const char *post)
{
#ifdef WIN32
  WIN32_FIND_DATA find_file_data;
  HANDLE hnd;
  int proceed;
#else
  DIR *dir;
  struct dirent *e;
#endif /* not WIN32 */

  std::string name (elt, elt_length);

  assert (IS_DIR_SEP (elt[elt_length - 1])
          || IS_DEVICE_SEP (elt[elt_length - 1]));

#if defined (WIN32)
  strcpy (dirname, name.c_str ());
  strcat (dirname, "/*.*");         /* "*.*" or "*" -- seems equivalent. */
  hnd = FindFirstFile (dirname, &find_file_data);

  if (hnd == INVALID_HANDLE_VALUE)
    return;

  /* Include top level before subdirectories, if nothing to match.  */
  if (*post == 0)
    dir_list_add (str_list_ptr, name);
  else {
    /* If we do have something to match, see if it exists.  For
       example, POST might be `pk/ljfour', and they might have a
       directory `$TEXMF/fonts/pk/ljfour' that we should find.  */
    name += post;
    expand_elt (str_list_ptr, name.c_str (), elt_length);
    name.resize (elt_length);
  }
  proceed = 1;
  while (proceed)
    {
      if (find_file_data.cFileName[0] != '.')
	{
	  /* Construct the potential subdirectory name.  */
	  name += find_file_data.cFileName;
	  if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	    {
	      /* It's a directory, so append the separator.  */
	      name += DIR_SEP_STRING;
	      unsigned potential_len = name.length ();
	      do_subdir (str_list_ptr, name.c_str (),
			 potential_len, post);
	    }
	  name.resize (elt_length);
	}
      proceed = FindNextFile (hnd, &find_file_data);
    }
  FindClose (hnd);

#else /* not WIN32 */

  /* If we can't open it, quit.  */
  dir = opendir (name.c_str ());
  if (dir == NULL)
    return;

  /* Include top level before subdirectories, if nothing to match.  */
  if (*post == 0)
    dir_list_add (str_list_ptr, name);
  else
    { /* If we do have something to match, see if it exists.  For
         example, POST might be `pk/ljfour', and they might have a
         directory `$TEXMF/fonts/pk/ljfour' that we should find.  */
      name += post;
      expand_elt (str_list_ptr, name.c_str (), elt_length);
      name.resize (elt_length);
    }

  while ((e = readdir (dir)) != NULL)
    { /* If it begins with a `.', never mind.  (This allows ``hidden''
         directories that the algorithm won't find.)  */
      if (e->d_name[0] != '.')
        {
          int links;

          /* Construct the potential subdirectory name.  */
          name += e->d_name;

          /* If we can't stat it, or if it isn't a directory, continue.  */
          links = dir_links (name.c_str ());

          if (links >= 0)
            {
              /* It's a directory, so append the separator.  */
              name += DIR_SEP_STRING;
              unsigned potential_len = name.length ();

              /* Should we recurse?  To see if the subdirectory is a
                 leaf, check if it has two links (one for . and one for
                 ..).  This means that symbolic links to directories do
                 not affect the leaf-ness.  This is arguably wrong, but
                 the only alternative I know of is to stat every entry
                 in the directory, and that is unacceptably slow.

                 The #ifdef here makes all this configurable at
                 compile-time, so that if we're using VMS directories or
                 some such, we can still find subdirectories, even if it
                 is much slower.  */
#ifdef ST_NLINK_TRICK
              if (links > 2)
#endif /* not ST_NLINK_TRICK */
                /* All criteria are met; find subdirectories.  */
                do_subdir (str_list_ptr, name.c_str (),
                           potential_len, post);
#ifdef ST_NLINK_TRICK
              else if (*post == 0)
                /* Nothing to match, no recursive subdirectories to
                   look for: we're done with this branch.  Add it.  */
                dir_list_add (str_list_ptr, name);
#endif
            }

          /* Remove the directory entry we just checked from `name'.  */
          name.resize (elt_length);
        }
    }

  xclosedir (dir);
#endif /* not WIN32 */
}

/* Assume ELT is non-empty and non-NULL.  Return list of corresponding
   directories (with no terminating NULL entry) in STR_LIST_PTR.  Start
   looking for magic constructs at START.  */

static void
expand_elt (str_llist_type *str_list_ptr, const char *elt, unsigned start)
{
  const char *dir = elt + start;
  const char *post;

  while (*dir != 0)
    {
      if (IS_DIR_SEP (*dir))
        {
          /* If two or more consecutive /'s, find subdirectories.  */
          if (IS_DIR_SEP (dir[1]))
            {
	      for (post = dir + 1; IS_DIR_SEP (*post); post++) ;
              do_subdir (str_list_ptr, elt, dir - elt + 1, post);
	      return;
            }

          /* No special stuff at this slash.  Keep going.  */
        }

      dir++;
    }

  /* When we reach the end of ELT, it will be a normal filename.  */
  checked_dir_list_add (str_list_ptr, elt);
}

/* Here is the entry point.  Returns directory list for ELT.  */

str_llist_type *
kpse_element_dirs (const char *elt)
{
  str_llist_type *ret;

  /* If given nothing, return nothing.  */
  if (! elt || !*elt)
    return NULL;

  /* If we've already cached the answer for ELT, return it.  */
  ret = cached (elt);
  if (ret)
    return ret;

  /* We're going to have a real directory list to return.  */
  ret = new str_llist_type;
  *ret = NULL;

  /* We handle the hard case in a subroutine.  */
  expand_elt (ret, elt, 0);

  /* Remember the directory list we just found, in case future calls are
     made with the same ELT.  */
  cache (elt, ret);

#ifdef KPSE_DEBUG
  if (KPSE_DEBUG_P (KPSE_DEBUG_EXPAND))
    {
      DEBUGF1 ("path element %s =>", elt);
      if (ret)
        {
          str_llist_elt_type *e;
          for (e = *ret; e; e = STR_LLIST_NEXT (*e))
            fprintf (stderr, " %s", (STR_LLIST (*e)).c_str ());
        }
      putc ('\n', stderr);
      fflush (stderr);
    }
#endif /* KPSE_DEBUG */

  return ret;
}

/* path-elt.c: Return the stuff between colons.  */

/* The static (but dynamically allocated) area we return the answer in,
   and how much we've currently allocated for it.  */
static char *elt = NULL;
static unsigned elt_alloc = 0;

/* The path we're currently working on.  */
static const char *path = NULL;

/* Upon entry, the static `path' is at the first (and perhaps last)
   character of the return value, or else NULL if we're at the end (or
   haven't been called).  I make no provision for caching the results;
   thus, we parse the same path over and over, on every lookup.  If that
   turns out to be a significant lose, it can be fixed, but I'm guessing
   disk accesses overwhelm everything else.  If ENV_P is true, use
   IS_ENV_SEP; else use IS_DIR_SEP.  */

char *
kpse_path_element (const char *passed_path)
{
  const char *p;
  char *ret;
  int brace_level;
  unsigned len;

  if (passed_path)
    path = passed_path;
  /* Check if called with NULL, and no previous path (perhaps we reached
     the end).  */
  else if (! path)
    return NULL;

  /* OK, we have a non-null `path' if we get here.  */
  assert (path);
  p = path;

  /* Find the next colon not enclosed by braces (or the end of the path).  */
  brace_level = 0;
  while (*p != 0  && ! (brace_level == 0 && IS_ENV_SEP (*p)))
    {
      if (*p == '{')
	++brace_level;
      else if (*p == '}')
	--brace_level;

      ++p;
    }

  /* Return the substring starting at `path'.  */
  len = p - path;

  /* Make sure we have enough space (including the null byte).  */
  if (len + 1 > elt_alloc)
    {
      elt_alloc = len + 1;
      elt = (char *) xrealloc (elt, elt_alloc);
    }

  strncpy (elt, path, len);
  elt[len] = 0;
  ret = elt;

  /* If we are at the end, return NULL next time.  */
  if (path[len] == 0)
    path = NULL;
  else
    path += len + 1;

  return ret;
}

#ifndef WIN32
void
xclosedir (DIR *d)
{
#ifdef CLOSEDIR_VOID
  closedir (d);
#else
  int ret = closedir (d);

  if (ret != 0)
    FATAL ("closedir failed");
#endif
}
#endif

/* dir.c: directory operations.  */

#ifndef WIN32

/* Return -1 if FN isn't a directory, else its number of links.
   Duplicate the call to stat; no need to incur overhead of a function
   call for that little bit of cleanliness. */

int
dir_links (const char *fn)
{
  std::map<std::string, long> link_table;

  long ret;

  if (link_table.find (fn) != link_table.end ())
    ret = link_table[fn];
  else
    {
      struct stat stats;

      ret = stat (fn, &stats) == 0 && S_ISDIR (stats.st_mode)
            ? stats.st_nlink : (unsigned) -1;

      link_table[fn] = ret;

#ifdef KPSE_DEBUG
      if (KPSE_DEBUG_P (KPSE_DEBUG_STAT))
        DEBUGF2 ("dir_links (%s) => %ld\n", fn, ret);
#endif
    }

  return ret;
}

#endif /* !WIN32 */

/* debug.c: Help the user discover what's going on.  */

#ifdef KPSE_DEBUG

unsigned int kpathsea_debug = 0;

/* If the real definitions of fopen or fclose are macros, we lose -- the
   #undef won't restore them. */

FILE *
fopen (const char *filename, const char *mode)
{
#undef fopen
  FILE *ret = fopen (filename, mode);

  if (KPSE_DEBUG_P (KPSE_DEBUG_FOPEN))
    DEBUGF3 ("fopen (%s, %s) => 0x%lx\n", filename, mode, (unsigned long) ret);

  return ret;
}

int
fclose (FILE *f)
{
#undef fclose
  int ret = fclose (f);

  if (KPSE_DEBUG_P (KPSE_DEBUG_FOPEN))
    DEBUGF2 ("fclose (0x%lx) => %d\n", (unsigned long) f, ret);

  return ret;
}

#endif

/* str-llist.c: Implementation of a linked list of strings.  */

/* Add the new string STR to the end of the list L.  */

void
str_llist_add (str_llist_type *l, const std::string& str)
{
  str_llist_elt_type *e;
  str_llist_elt_type *new_elt = new str_llist_elt_type;

  /* The new element will be at the end of the list.  */
  STR_LLIST (*new_elt) = str;
  STR_LLIST_MOVED (*new_elt) = 0;
  STR_LLIST_NEXT (*new_elt) = NULL;

  /* Find the current end of the list.  */
  for (e = *l; e && STR_LLIST_NEXT (*e); e = STR_LLIST_NEXT (*e))
    ;

  if (! e)
    *l = new_elt;
  else
    STR_LLIST_NEXT (*e) = new_elt;
}

/* Move an element towards the top. The idea is that when a file is
   found in a given directory, later files will likely be in that same
   directory, and looking for the file in all the directories in between
   is thus a waste.  */

void
str_llist_float (str_llist_type *l, str_llist_elt_type *mover)
{
  str_llist_elt_type *last_moved, *unmoved;

  /* If we've already moved this element, never mind.  */
  if (STR_LLIST_MOVED (*mover))
    return;

  /* Find the first unmoved element (to insert before).  We're
     guaranteed this will terminate, since MOVER itself is currently
     unmoved, and it must be in L (by hypothesis).  */
  for (last_moved = NULL, unmoved = *l; STR_LLIST_MOVED (*unmoved);
       last_moved = unmoved, unmoved = STR_LLIST_NEXT (*unmoved))
    ;

  /* If we are the first unmoved element, nothing to relink.  */
  if (unmoved != mover)
    { /* Remember `mover's current successor, so we can relink `mover's
         predecessor to it.  */
      str_llist_elt_type *before_mover;
      str_llist_elt_type *after_mover = STR_LLIST_NEXT (*mover);

      /* Find `mover's predecessor.  */
      for (before_mover = unmoved; STR_LLIST_NEXT (*before_mover) != mover;
           before_mover = STR_LLIST_NEXT (*before_mover))
        ;

      /* `before_mover' now links to `after_mover'.  */
      STR_LLIST_NEXT (*before_mover) = after_mover;

      /* Insert `mover' before `unmoved' and after `last_moved' (or at
         the head of the list).  */
      STR_LLIST_NEXT (*mover) = unmoved;
      if (! last_moved)
        *l = mover;
      else
        STR_LLIST_NEXT (*last_moved) = mover;
    }

  /* We've moved it.  */
  STR_LLIST_MOVED (*mover) = 1;
}

/* variable.c: variable expansion.  */

/* We have to keep track of variables being expanded, otherwise
   constructs like TEXINPUTS = $TEXINPUTS result in an infinite loop.
   (Or indirectly recursive variables, etc.)  Our simple solution is to
   add to a list each time an expansion is started, and check the list
   before expanding.  */

static std::map <std::string, bool> expansions;

static void
expanding (const std::string& var, bool xp)
{
  expansions[var] = xp;
}

/* Return whether VAR is currently being expanding.  */

static bool
expanding_p (const std::string& var)
{
  return (expansions.find (var) != expansions.end ())
    ? expansions[var] : false;
}

/* Append the result of value of `var' to EXPANSION, where `var' begins
   at START and ends at END.  If `var' is not set, do not complain.
   This is a subroutine for the more complicated expansion function.  */

static void
expand (std::string &expansion, const std::string& var)
{
  if (expanding_p (var))
    {
      WARNING1 ("kpathsea: variable `%s' references itself (eventually)",
		var.c_str ());
    }
  else
    {
      /* Check for an environment variable.  */
      std::string value = octave_env::getenv (var);

      if (! value.empty ())
	{
	  expanding (var, true);
	  std::string tmp = kpse_var_expand (value);
	  expanding (var, false);
	  expansion += tmp;
	}
    }
}

/* Can't think of when it would be useful to change these (and the
   diagnostic messages assume them), but ... */
#ifndef IS_VAR_START /* starts all variable references */
#define IS_VAR_START(c) ((c) == '$')
#endif
#ifndef IS_VAR_CHAR  /* variable name constituent */
#define IS_VAR_CHAR(c) (isalnum (c) || (c) == '_')
#endif
#ifndef IS_VAR_BEGIN_DELIMITER /* start delimited variable name (after $) */
#define IS_VAR_BEGIN_DELIMITER(c) ((c) == '{')
#endif
#ifndef IS_VAR_END_DELIMITER
#define IS_VAR_END_DELIMITER(c) ((c) == '}')
#endif

/* Maybe we should support some or all of the various shell ${...}
   constructs, especially ${var-value}.  */

std::string
kpse_var_expand (const std::string& src)
{
  std::string expansion;

  size_t src_len = src.length ();

  /* Copy everything but variable constructs.  */
  for (size_t i = 0; i < src_len; i++)
    {
      if (IS_VAR_START (src[i]))
	{
	  i++;

	  /* Three cases: `$VAR', `${VAR}', `$<anything-else>'.  */
	  if (IS_VAR_CHAR (src[i]))
	    {
	      /* $V: collect name constituents, then expand.  */
	      size_t var_end = i;

	      do
		{
		  var_end++;
		}
	      while (IS_VAR_CHAR (src[var_end]));

	      var_end--; /* had to go one past */
	      expand (expansion, src.substr (i, var_end - i + 1));
	      i = var_end;

	    }
	  else if (IS_VAR_BEGIN_DELIMITER (src[i]))
	    {
	      /* ${: scan ahead for matching delimiter, then expand.  */
	      size_t var_end = ++i;

	      while (var_end < src_len && !IS_VAR_END_DELIMITER (src[var_end]))
		var_end++;

	      if (var_end == src_len)
		{
		  WARNING1 ("%s: No matching } for ${", src.c_str ());
		  i = var_end - 1; /* will incr to eos at top of loop */
		}
	      else
		{
		  expand (expansion, src.substr (i, var_end - i));
		  i = var_end; /* will incr past } at top of loop*/
		}
	    }
	  else
	    {
	      /* $<something-else>: error.  */
	      WARNING2 ("%s: Unrecognized variable construct `$%c'",
			src.c_str (), src[i]);
	      /* Just ignore those chars and keep going.  */
	    }
	}
      else
	expansion += src[i];
    }

  return expansion;
}
