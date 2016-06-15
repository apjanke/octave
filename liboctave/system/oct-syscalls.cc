/*

Copyright (C) 1996-2015 John W. Eaton

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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <cerrno>
#include <cstdlib>
#include <cstring>

// We can't use csignal as kill is not in the std namespace, and picky
// compiler runtimes will also exclude it from global scope as well.

#include <signal.h>

#include "fcntl-wrappers.h"
#include "lo-utils.h"
#include "lo-sysdep.h"
#include "oct-syscalls.h"
#include "octave-popen2.h"
#include "str-vec.h"
#include "unistd-wrappers.h"

#define NOT_SUPPORTED(nm) \
  nm ": not supported on this system"

namespace octave
{
  namespace sys
  {
    int
    dup2 (int old_fd, int new_fd)
    {
      std::string msg;
      return octave::sys::dup2 (old_fd, new_fd, msg);
    }

    int
    dup2 (int old_fd, int new_fd, std::string& msg)
    {
      msg = "";

      int status = -1;

      status = octave_dup2_wrapper (old_fd, new_fd);

      if (status < 0)
        msg = gnulib::strerror (errno);

      return status;
    }

    int
    execvp (const std::string& file, const string_vector& argv)
    {
      std::string msg;
      return octave::sys::execvp (file, argv, msg);
    }

    int
    execvp (const std::string& file, const string_vector& args,
            std::string& msg)
    {
      msg = "";

      char **argv = args.c_str_vec ();

      int status = octave_execvp_wrapper (file.c_str (), argv);

      string_vector::delete_c_str_vec (argv);

      if (status < 0)
        msg = gnulib::strerror (errno);

      return status;
    }

    pid_t
    fork (std::string& msg)
    {
      pid_t status = -1;

#if defined (HAVE_FORK)
      status = octave_fork_wrapper ();

      if (status < 0)
        msg = gnulib::strerror (errno);
#else
      msg = NOT_SUPPORTED ("vfork");
#endif

      return status;
    }

    pid_t
    vfork (std::string& msg)
    {
      pid_t status = -1;

#if defined (HAVE_VFORK) || defined (HAVE_FORK)
#  if defined (HAVE_VFORK)
      status = octave_vfork_wrapper ();
#  else
      status = octave_fork_wrapper ();
#  endif

      if (status < 0)
        msg = gnulib::strerror (errno);
#else
      msg = NOT_SUPPORTED ("vfork");
#endif

      return status;
    }

    pid_t
    getpgrp (std::string& msg)
    {
      pid_t status = octave_getpgrp_wrapper ();

      if (status < 0)
        msg = gnulib::strerror (errno);

      return status;
    }

    pid_t
    getpid (void)
    {
      return octave_getpid_wrapper ();
    }

    pid_t
    getppid (void)
    {
      return octave_getppid_wrapper ();
    }

    gid_t
    getgid (void)
    {
      return octave_getgid_wrapper ();
    }

    gid_t
    getegid (void)
    {
      return octave_getegid_wrapper ();
    }

    uid_t
    getuid (void)
    {
      return octave_getuid_wrapper ();
    }

    uid_t
    geteuid (void)
    {
      return octave_geteuid_wrapper ();
    }

    int
    pipe (int *fildes)
    {
      std::string msg;
      return octave::sys::pipe (fildes, msg);
    }

    int
    pipe (int *fildes, std::string& msg)
    {
      msg = "";

      int status = -1;

      status = octave_pipe_wrapper (fildes);

      if (status < 0)
        msg = gnulib::strerror (errno);

      return status;
    }

    pid_t
    waitpid (pid_t pid, int *status, int options)
    {
      std::string msg;
      return octave::sys::waitpid (pid, status, options, msg);
    }

    pid_t
    waitpid (pid_t pid, int *status, int options,
             std::string& msg)
    {
      pid_t retval = -1;
      msg = "";

#if defined (HAVE_WAITPID)
      retval = ::octave_waitpid (pid, status, options);

      if (retval < 0)
        msg = gnulib::strerror (errno);
#else
      msg = NOT_SUPPORTED ("waitpid");
#endif

      return retval;
    }

    int
    kill (pid_t pid, int sig)
    {
      std::string msg;
      return octave::sys::kill (pid, sig, msg);
    }

    int
    kill (pid_t pid, int sig, std::string& msg)
    {
      msg = "";

      int status = -1;

#if defined (HAVE_KILL)
      status = ::kill (pid, sig);

      if (status < 0)
        msg = gnulib::strerror (errno);
#else
      msg = NOT_SUPPORTED ("kill");
#endif

      return status;
    }

    pid_t
    popen2 (const std::string& cmd, const string_vector& args,
            bool sync_mode, int *fildes)
    {
      std::string msg;
      bool interactive = false;
      return octave::sys::popen2 (cmd, args, sync_mode, fildes, msg,
                                       interactive);
    }

    pid_t
    popen2 (const std::string& cmd, const string_vector& args,
            bool sync_mode, int *fildes, std::string& msg)
    {
      bool interactive = false;
      return octave::sys::popen2 (cmd, args, sync_mode, fildes, msg,
                                       interactive);
    }

    pid_t
    popen2 (const std::string& cmd, const string_vector& args,
            bool sync_mode, int *fildes, std::string& msg,
            bool & /* interactive */)
    {
#if defined (__WIN32__) && ! defined (__CYGWIN__)
      // FIXME: this function could be combined with octave_popen2 in
      // liboctave/wrappers/octave-popen2.c.

      return octave::sys::win_popen2 (cmd, args, sync_mode, fildes, msg);
#else
      char **argv = args.c_str_vec ();

      pid_t pid = octave_popen2 (cmd.c_str (), argv, sync_mode, fildes);

      string_vector::delete_c_str_vec (argv);

      if (pid < 0)
        msg = gnulib::strerror (errno);

      return pid;
#endif
    }

    int
    fcntl (int fd, int cmd, long arg)
    {
      std::string msg;
      return octave::sys::fcntl (fd, cmd, arg, msg);
    }

    int
    fcntl (int fd, int cmd, long arg, std::string& msg)
    {
      msg = "";

      int status = -1;

      status = octave_fcntl_wrapper (fd, cmd, arg);

      if (status < 0)
        msg = gnulib::strerror (errno);

      return status;
    }
  }
}
