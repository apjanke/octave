/* xmalloc.c: malloc with error checking.

Copyright (C) 1992, 93 Free Software Foundation, Inc.

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

/* Don't include config.h or all our other usual includes, since
   it's useful to just throw this file into other programs.  */

#include <stdio.h>
extern char *malloc ();


void *
xmalloc (size)
    unsigned size;
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
