/* db.h: lookups in an externally built db file.

Copyright (C) 1994, 95 Karl Berry.

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

#ifndef KPATHSEA_DB_H
#define KPATHSEA_DB_H

#include <kpathsea/c-proto.h>
#include <kpathsea/types.h>
#include <kpathsea/str-list.h>

/* Initialize the database.  Until this is called, no ls-R matches will
   be found.  */
extern void kpse_init_db P1H(void);

/* Return list of matches for NAME in the ls-R file matching PATH_ELT.  If
   ALL is set, return (null-terminated list) of all matches, else just
   the first.  If no matches, return a pointer to an empty list.  If no
   databases can be read, or PATH_ELT is not in any of the databases,
   return NULL.  */
extern str_list_type *kpse_db_search P3H(const_string name, 
                                         const_string path_elt, boolean all);

/* Insert the filename FNAME into the database.
   Called by MakeTeXPK et al.  */
extern void kpse_db_insert P1H(const_string fname);

#endif /* not KPATHSEA_DB_H */
