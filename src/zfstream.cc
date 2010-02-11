/*

Copyright (C) 2005, 2007, 2008 Ludwig Schwardt, Kevin Ruland 


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

/*

 This file is adapted from the zlib 1.2.2 contrib/iostream3 code,
 written by

   Ludwig Schwardt <schwardt@sun.ac.za>
   original version by Kevin Ruland <kevin@rodin.wustl.edu>

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>

#include "zfstream.h"

#ifdef HAVE_ZLIB

#include <cstring>          // for strcpy, strcat, strlen (mode strings)
#include <cstdio>           // for BUFSIZ

// Internal buffer sizes (default and "unbuffered" versions)
#define STASHED_CHARACTERS 16
#define BIGBUFSIZE (256 * 1024 + STASHED_CHARACTERS) 
#define SMALLBUFSIZE 1

/*****************************************************************************/

// Default constructor
gzfilebuf::gzfilebuf()
: file(0), io_mode(std::ios_base::openmode(0)), own_fd(false),
  buffer(0), buffer_size(BIGBUFSIZE), own_buffer(true)
{
  // No buffers to start with
  this->disable_buffer();
}

// Destructor
gzfilebuf::~gzfilebuf()
{
  // Sync output buffer and close only if responsible for file
  // (i.e. attached streams should be left open at this stage)
  this->sync();
  if (own_fd)
    this->close();
  // Make sure internal buffer is deallocated
  this->disable_buffer();
}

// Set compression level and strategy
int
gzfilebuf::setcompression(int comp_level,
                          int comp_strategy)
{
  return gzsetparams(file, comp_level, comp_strategy);
}

// Open gzipped file
gzfilebuf*
gzfilebuf::open(const char *name,
                std::ios_base::openmode mode)
{
  // Fail if file already open
  if (this->is_open())
    return 0;
  // Don't support simultaneous read/write access (yet)
  if ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    return 0;

  // Build mode string for gzopen and check it [27.8.1.3.2]
  char char_mode[6] = "\0\0\0\0\0";
  if (!this->open_mode(mode, char_mode))
    return 0;

  // Attempt to open file
  if ((file = gzopen(name, char_mode)) == 0)
    return 0;

  // On success, allocate internal buffer and set flags
  this->enable_buffer();
  io_mode = mode;
  own_fd = true;
  return this;
}

// Attach to gzipped file
gzfilebuf*
gzfilebuf::attach(int fd,
                  std::ios_base::openmode mode)
{
  // Fail if file already open
  if (this->is_open())
    return 0;
  // Don't support simultaneous read/write access (yet)
  if ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    return 0;

  // Build mode string for gzdopen and check it [27.8.1.3.2]
  char char_mode[6] = "\0\0\0\0\0";
  if (!this->open_mode(mode, char_mode))
    return 0;

  // Attempt to attach to file
  if ((file = gzdopen(fd, char_mode)) == 0)
    return 0;

  // On success, allocate internal buffer and set flags
  this->enable_buffer();
  io_mode = mode;
  own_fd = false;
  return this;
}

// Close gzipped file
gzfilebuf*
gzfilebuf::close()
{
  // Fail immediately if no file is open
  if (!this->is_open())
    return 0;
  // Assume success
  gzfilebuf* retval = this;
  // Attempt to sync and close gzipped file
  if (this->sync() == -1)
    retval = 0;
  if (gzclose(file) < 0)
    retval = 0;
  // File is now gone anyway (postcondition [27.8.1.3.8])
  file = 0;
  own_fd = false;
  // Destroy internal buffer if it exists
  this->disable_buffer();
  return retval;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Convert int open mode to mode string
bool
gzfilebuf::open_mode(std::ios_base::openmode mode,
                     char* c_mode) const
{
  // FIXME -- do we need testb?
  // bool testb = mode & std::ios_base::binary;
  bool testi = mode & std::ios_base::in;
  bool testo = mode & std::ios_base::out;
  bool testt = mode & std::ios_base::trunc;
  bool testa = mode & std::ios_base::app;

  // Check for valid flag combinations - see [27.8.1.3.2] (Table 92)
  // Original zfstream hardcoded the compression level to maximum here...
  // Double the time for less than 1% size improvement seems
  // excessive though - keeping it at the default level
  // To change back, just append "9" to the next three mode strings
  if (!testi && testo && !testt && !testa)
    strcpy(c_mode, "w");
  if (!testi && testo && !testt && testa)
    strcpy(c_mode, "a");
  if (!testi && testo && testt && !testa)
    strcpy(c_mode, "w");
  if (testi && !testo && !testt && !testa)
    strcpy(c_mode, "r");
  // No read/write mode yet
//  if (testi && testo && !testt && !testa)
//    strcpy(c_mode, "r+");
//  if (testi && testo && testt && !testa)
//    strcpy(c_mode, "w+");

  // Mode string should be empty for invalid combination of flags
  if (strlen(c_mode) == 0)
    return false;

  strcat(c_mode, "b");

  return true;
}

// Determine number of characters in internal get buffer
std::streamsize
gzfilebuf::showmanyc()
{
  // Calls to underflow will fail if file not opened for reading
  if (!this->is_open() || !(io_mode & std::ios_base::in))
    return -1;
  // Make sure get area is in use
  if (this->gptr() && (this->gptr() < this->egptr()))
    return std::streamsize(this->egptr() - this->gptr());
  else
    return 0;
}

// Puts back a character to the stream in two cases. Firstly, when there
// is no putback position available, and secondly when the character putback
// differs from the one in the file. We can only support the first case 
// with gzipped files.
gzfilebuf::int_type
gzfilebuf::pbackfail (gzfilebuf::int_type c)
{
  if (this->is_open())
    {
      if (gzseek (file, this->gptr() - this->egptr() - 1, SEEK_CUR) < 0)
        return traits_type::eof();
  
      // Invalidates contents of the buffer
      enable_buffer ();

      // Attempt to fill internal buffer from gzipped file
      // (buffer must be guaranteed to exist...)
      int bytes_read = gzread(file, buffer, buffer_size);
      // Indicates error or EOF
      if (bytes_read <= 0)
        {
          // Reset get area
          this->setg(buffer, buffer, buffer);
          return traits_type::eof();
        }

      // Make all bytes read from file available as get area
      this->setg(buffer, buffer, buffer + bytes_read);

      // If next character in get area differs from putback character
      // flag a failure
      gzfilebuf::int_type ret = traits_type::to_int_type(*(this->gptr()));
      if (ret != c)
        return traits_type::eof();
      else
        return ret;
    }
  else
    return traits_type::eof();
}

// Fill get area from gzipped file
gzfilebuf::int_type
gzfilebuf::underflow()
{
  // If something is left in the get area by chance, return it
  // (this shouldn't normally happen, as underflow is only supposed
  // to be called when gptr >= egptr, but it serves as error check)
  if (this->gptr() && (this->gptr() < this->egptr()))
    return traits_type::to_int_type(*(this->gptr()));

  // If the file hasn't been opened for reading, produce error
  if (!this->is_open() || !(io_mode & std::ios_base::in))
    return traits_type::eof();

  // Copy the final characters to the front of the buffer
  int stash = 0;
  if (this->eback() && buffer && buffer_size > STASHED_CHARACTERS)
    {
      char_type *ptr1 = buffer;
      char_type *ptr2 = this->egptr() - STASHED_CHARACTERS + 1;
      if (ptr2 > this->eback())
        while (stash++ <= STASHED_CHARACTERS)
          *ptr1++ = *ptr2++;
    }

  // Attempt to fill internal buffer from gzipped file
  // (buffer must be guaranteed to exist...)
  int bytes_read = gzread(file, buffer + stash, buffer_size - stash);

  // Indicates error or EOF
  if (bytes_read <= 0)
  {
    // Reset get area
    this->setg(buffer, buffer, buffer);
    return traits_type::eof();
  }
  // Make all bytes read from file plus the stash available as get area
  this->setg(buffer, buffer + stash, buffer + bytes_read + stash);

  // Return next character in get area
  return traits_type::to_int_type(*(this->gptr()));
}

// Write put area to gzipped file
gzfilebuf::int_type
gzfilebuf::overflow(int_type c)
{
  // Determine whether put area is in use
  if (this->pbase())
  {
    // Double-check pointer range
    if (this->pptr() > this->epptr() || this->pptr() < this->pbase())
      return traits_type::eof();
    // Add extra character to buffer if not EOF
    if (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *(this->pptr()) = traits_type::to_char_type(c);
      this->pbump(1);
    }
    // Number of characters to write to file
    int bytes_to_write = this->pptr() - this->pbase();
    // Overflow doesn't fail if nothing is to be written
    if (bytes_to_write > 0)
    {
      // If the file hasn't been opened for writing, produce error
      if (!this->is_open() || !(io_mode & std::ios_base::out))
        return traits_type::eof();
      // If gzipped file won't accept all bytes written to it, fail
      if (gzwrite(file, this->pbase(), bytes_to_write) != bytes_to_write)
        return traits_type::eof();
      // Reset next pointer to point to pbase on success
      this->pbump(-bytes_to_write);
    }
  }
  // Write extra character to file if not EOF
  else if (!traits_type::eq_int_type(c, traits_type::eof()))
  {
    // If the file hasn't been opened for writing, produce error
    if (!this->is_open() || !(io_mode & std::ios_base::out))
      return traits_type::eof();
    // Impromptu char buffer (allows "unbuffered" output)
    char_type last_char = traits_type::to_char_type(c);
    // If gzipped file won't accept this character, fail
    if (gzwrite(file, &last_char, 1) != 1)
      return traits_type::eof();
  }

  // If you got here, you have succeeded (even if c was EOF)
  // The return value should therefore be non-EOF
  if (traits_type::eq_int_type(c, traits_type::eof()))
    return traits_type::not_eof(c);
  else
    return c;
}

// Assign new buffer
std::streambuf*
gzfilebuf::setbuf(char_type* p,
                  std::streamsize n)
{
  // First make sure stuff is sync'ed, for safety
  if (this->sync() == -1)
    return 0;
  // If buffering is turned off on purpose via setbuf(0,0), still allocate one...
  // "Unbuffered" only really refers to put [27.8.1.4.10], while get needs at
  // least a buffer of size 1 (very inefficient though, therefore make it bigger?)
  // This follows from [27.5.2.4.3]/12 (gptr needs to point at something, it seems)
  if (!p || !n)
  {
    // Replace existing buffer (if any) with small internal buffer
    this->disable_buffer();
    buffer = 0;
    buffer_size = 0;
    own_buffer = true;
    this->enable_buffer();
  }
  else
  {
    // Replace existing buffer (if any) with external buffer
    this->disable_buffer();
    buffer = p;
    buffer_size = n;
    own_buffer = false;
    this->enable_buffer();
  }
  return this;
}

// Write put area to gzipped file (i.e. ensures that put area is empty)
int
gzfilebuf::sync()
{
  return traits_type::eq_int_type(this->overflow(), traits_type::eof()) ? -1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Allocate internal buffer
void
gzfilebuf::enable_buffer()
{
  // If internal buffer required, allocate one
  if (own_buffer && !buffer)
  {
    // Check for buffered vs. "unbuffered"
    if (buffer_size > 0)
    {
      // Allocate internal buffer
      buffer = new char_type[buffer_size];
      // Get area starts empty and will be expanded by underflow as need arises
      this->setg(buffer, buffer, buffer);
      // Setup entire internal buffer as put area.
      // The one-past-end pointer actually points to the last element of the buffer,
      // so that overflow(c) can safely add the extra character c to the sequence.
      // These pointers remain in place for the duration of the buffer
      this->setp(buffer, buffer + buffer_size - 1);
    }
    else
    {
      // Even in "unbuffered" case, (small?) get buffer is still required
      buffer_size = SMALLBUFSIZE;
      buffer = new char_type[buffer_size];
      this->setg(buffer, buffer, buffer);
      // "Unbuffered" means no put buffer
      this->setp(0, 0);
    }
  }
  else
  {
    // If buffer already allocated, reset buffer pointers just to make sure no
    // stale chars are lying around
    this->setg(buffer, buffer, buffer);
    this->setp(buffer, buffer + buffer_size - 1);
  }
}

// Destroy internal buffer
void
gzfilebuf::disable_buffer()
{
  // If internal buffer exists, deallocate it
  if (own_buffer && buffer)
  {
    // Preserve unbuffered status by zeroing size
    if (!this->pbase())
      buffer_size = 0;
    delete[] buffer;
    buffer = 0;
    this->setg(0, 0, 0);
    this->setp(0, 0);
  }
  else
  {
    // Reset buffer pointers to initial state if external buffer exists
    this->setg(buffer, buffer, buffer);
    if (buffer)
      this->setp(buffer, buffer + buffer_size - 1);
    else
      this->setp(0, 0);
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Seek functions
gzfilebuf::pos_type
gzfilebuf::seekoff(off_type off, std::ios_base::seekdir way, 
                   std::ios_base::openmode)
{
  pos_type ret = pos_type (off_type (-1));

  if (this->is_open())
    {
      off_type computed_off = off;

      if ((io_mode & std::ios_base::in) && way == std::ios_base::cur)
        computed_off += this->gptr() - this->egptr();

      if (way == std::ios_base::beg)
        ret = pos_type (gzseek (file, computed_off, SEEK_SET));
      else if (way == std::ios_base::cur)
        ret = pos_type (gzseek (file, computed_off, SEEK_CUR));
      else
        // Can't seek from end of a gzipped file, so this will give -1
        ret = pos_type (gzseek (file, computed_off, SEEK_END));
  
      if (io_mode & std::ios_base::in)
        // Invalidates contents of the buffer
        enable_buffer ();
      else
        // flush contents of buffer to file
        overflow ();
    }

  return ret;
}

gzfilebuf::pos_type
gzfilebuf::seekpos(pos_type sp, std::ios_base::openmode)
{
  pos_type ret = pos_type (off_type (-1));

  if (this->is_open ())
    {
      ret = pos_type (gzseek (file, sp, SEEK_SET));

      if (io_mode & std::ios_base::in)
        // Invalidates contents of the buffer
        enable_buffer ();
      else
        // flush contents of buffer to file
        overflow ();
    }

  return ret;
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzifstream::gzifstream()
: std::istream(0), sb()
{ this->init(&sb); }

// Initialize stream buffer and open file
gzifstream::gzifstream(const char* name,
                       std::ios_base::openmode mode)
: std::istream(0), sb()
{
  this->init(&sb);
  this->open(name, mode);
}

// Initialize stream buffer and attach to file
gzifstream::gzifstream(int fd,
                       std::ios_base::openmode mode)
: std::istream(0), sb()
{
  this->init(&sb);
  this->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
void
gzifstream::open(const char* name,
                 std::ios_base::openmode mode)
{
  if (!sb.open(name, mode | std::ios_base::in))
    this->setstate(std::ios_base::failbit);
  else
    this->clear();
}

// Attach to file and go into fail() state if unsuccessful
void
gzifstream::attach(int fd,
                   std::ios_base::openmode mode)
{
  if (!sb.attach(fd, mode | std::ios_base::in))
    this->setstate(std::ios_base::failbit);
  else
    this->clear();
}

// Close file
void
gzifstream::close()
{
  if (!sb.close())
    this->setstate(std::ios_base::failbit);
}

/*****************************************************************************/

// Default constructor initializes stream buffer
gzofstream::gzofstream()
: std::ostream(0), sb()
{ this->init(&sb); }

// Initialize stream buffer and open file
gzofstream::gzofstream(const char* name,
                       std::ios_base::openmode mode)
: std::ostream(0), sb()
{
  this->init(&sb);
  this->open(name, mode);
}

// Initialize stream buffer and attach to file
gzofstream::gzofstream(int fd,
                       std::ios_base::openmode mode)
: std::ostream(0), sb()
{
  this->init(&sb);
  this->attach(fd, mode);
}

// Open file and go into fail() state if unsuccessful
void
gzofstream::open(const char* name,
                 std::ios_base::openmode mode)
{
  if (!sb.open(name, mode | std::ios_base::out))
    this->setstate(std::ios_base::failbit);
  else
    this->clear();
}

// Attach to file and go into fail() state if unsuccessful
void
gzofstream::attach(int fd,
                   std::ios_base::openmode mode)
{
  if (!sb.attach(fd, mode | std::ios_base::out))
    this->setstate(std::ios_base::failbit);
  else
    this->clear();
}

// Close file
void
gzofstream::close()
{
  if (!sb.close())
    this->setstate(std::ios_base::failbit);
}

#endif // HAVE_ZLIB
