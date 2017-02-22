/*

Copyright (C) 1996-2017 John W. Eaton

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

#if ! defined (octave_str_vec_h)
#define octave_str_vec_h 1

#include "octave-config.h"

#include <iosfwd>
#include <list>
#include <string>

#include "Array.h"

class
OCTAVE_API
string_vector : public Array<std::string>
{
public:

  string_vector (void) : Array<std::string> () { }

  explicit string_vector (octave_idx_type n)
    : Array<std::string> (dim_vector (n, 1)) { }

  string_vector (const char *s)
    : Array<std::string> (dim_vector (1, 1), s) { }

  string_vector (const std::string& s)
    : Array<std::string> (dim_vector (1, 1), s) { }

  string_vector (const string_vector& s) : Array<std::string> (s) { }

  //! Constructor for STL containers of std::string
  /*!
    Templated constructor for any template class with std::string as the
    first parameter, and begin, end, and size methods, i.e., a class with
    similar interface as the STL containers.
  */
  template<template <typename...> class String_Container, typename... Other>
  string_vector (const String_Container<std::string, Other...>& lst);

  string_vector (const Array<std::string>& s)
    : Array<std::string> (s.as_column ()) { }

  string_vector (const char * const *s);

  string_vector (const char * const *s, octave_idx_type n);

  string_vector& operator = (const string_vector& s)
  {
    if (this != &s)
      Array<std::string>::operator = (s);

    return *this;
  }

  ~string_vector (void) { }

  bool empty (void) const { return numel () == 0; }

  octave_idx_type max_length (void) const
  {
    octave_idx_type n = numel ();
    octave_idx_type longest = 0;

    for (octave_idx_type i = 0; i < n; i++)
      {
        octave_idx_type tmp = elem (i).length ();

        if (tmp > longest)
          longest = tmp;
      }

    return longest;
  }

  void resize (octave_idx_type n, const std::string& rfv = "")
  {
    Array<std::string>::resize (dim_vector (n, 1), rfv);
  }

  std::string& operator[] (octave_idx_type i)
  { return Array<std::string>::elem (i); }

  std::string operator[] (octave_idx_type i) const
  { return Array<std::string>::elem (i); }

  string_vector& sort (bool make_uniq = false);

  string_vector& uniq (void);

  string_vector& append (const std::string& s);

  string_vector& append (const string_vector& sv);

  std::string join (const std::string& sep = "") const;

  char **c_str_vec (void) const;

  std::list<std::string> std_list (void) const;

  static void delete_c_str_vec (const char * const*);

  std::ostream&
  list_in_columns (std::ostream&, int width = 0,
                   const std::string& prefix = "") const;
};


template<template <typename...> class String_Container, typename... Other>
string_vector::string_vector (const String_Container<std::string, Other...>&
                              lst)
  : Array<std::string> ()
{
  resize (lst.size ());

  octave_idx_type i = 0;
  for (const std::string& s : lst)
    elem(i++) = s;
}

#endif
