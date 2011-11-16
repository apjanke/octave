/*

Copyright (C) 2000-2011 Kai Habel

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
20. Augiust 2000 - Kai Habel: first release
*/

/*
2003-12-14 Rafael Laboissiere <rafael@laboissiere.net>
Added optional second argument to pass options to the underlying
qhull command
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstdio>

#include <list>

#include "lo-ieee.h"

#include "Cell.h"
#include "defun-dld.h"
#include "error.h"
#include "oct-obj.h"

#ifdef HAVE_QHULL
extern "C" {
#include <qhull/qhull_a.h>
}

#ifdef NEED_QHULL_VERSION
char qh_version[] = "__voronoi__.oct 2007-07-24";
#endif
#endif

DEFUN_DLD (__voronoi__, args, ,
        "-*- texinfo -*-\n\
@deftypefn  {Loadable Function} {@var{C}, @var{F} =} __voronoi__ (@var{pts})\n\
@deftypefnx {Loadable Function} {@var{C}, @var{F} =} __voronoi__ (@var{pts}, @var{options})\n\
@deftypefnx {Loadable Function} {@var{C}, @var{F}, @var{Inf_Pts} =} __voronoi__ (@dots{})\n\
Undocumented internal function.\n\
@end deftypefn")
{
  octave_value_list retval;

#ifdef HAVE_QHULL

  retval(0) = 0.0;

  int nargin = args.length ();
  if (nargin < 1 || nargin > 2)
    {
      print_usage ();
      return retval;
    }

  std::string cmd = "qhull v";

  if (nargin == 2 && ! args(1).is_empty ())
    {
      if (args(1).is_string ())
        cmd += " " + args(1).string_value ();
      else if (args(1).is_cellstr ())
        {
          Array<std::string> tmp = args(1).cellstr_value ();

          for (octave_idx_type i = 0; i < tmp.numel (); i++)
            cmd += " " + tmp(i);
        }
      else
        {
          error ("__voronoi__: OPTIONS argument must be a string, cell array of strings, or empty");
          return retval;
        }
    }

  Matrix points (args(0).matrix_value ());
  const octave_idx_type dim = points.columns ();
  const octave_idx_type num_points = points.rows ();

  points = points.transpose ();

  boolT ismalloc = false;

  // Replace the 0 pointer with stdout for debugging information
  FILE *outfile = 0;
  FILE *errfile = stderr;

  // Qhull flags and points arguments are not const...

  OCTAVE_LOCAL_BUFFER (char, cmd_str, cmd.length () + 1);

  strcpy (cmd_str, cmd.c_str ());

  int exitcode = qh_new_qhull (dim, num_points, points.fortran_vec (),
                               ismalloc, cmd_str, outfile, errfile);
  if (! exitcode) 
    {
      // Calling findgood_all provides the number of Voronoi vertices
      // (sets qh num_good).

      qh_findgood_all (qh facet_list);

      octave_idx_type num_voronoi_regions
        = qh num_vertices - qh_setsize (qh del_vertices);

      octave_idx_type num_voronoi_vertices = qh num_good;

      // Find the voronoi centers for all facets.

      qh_setvoronoi_all ();

      facetT *facet;
      vertexT *vertex;
      octave_idx_type k;

      // Find the number of Voronoi vertices for each Voronoi cell and
      // store them in NI so we can use them later to set the dimensions
      // of the RowVector objects used to collect them.

      FORALLfacets
        {
          facet->seen = false;
        }
      
      OCTAVE_LOCAL_BUFFER (octave_idx_type, ni, num_voronoi_regions);
      for (octave_idx_type i = 0; i < num_voronoi_regions; i++)
        ni[i] = 0;

      k = 0;

      FORALLvertices
        {
          if (qh hull_dim == 3)
            qh_order_vertexneighbors (vertex);
          
          bool infinity_seen = false;

          facetT *neighbor, **neighborp;

          FOREACHneighbor_ (vertex)
            {
              if (neighbor->upperdelaunay)
                {
                  if (! infinity_seen)
                    {
                      infinity_seen = true;
                      ni[k]++;
                    }
                }
              else
                {
                  neighbor->seen = true;
                  ni[k]++;
                }
            }

          k++;
        }

      // If Qhull finds fewer regions than points, we will pad the end
      // of the at_inf and C arrays so that they always contain at least
      // as many elements as the given points array.

      // FIXME -- is it possible (or does it make sense) for
      // num_voronoi_regions to ever be larger than num_points?

      octave_idx_type nr = (num_points > num_voronoi_regions
                            ? num_points : num_voronoi_regions);

      boolMatrix at_inf (nr, 1, false);

      // The list of Voronoi vertices.  The first element is always
      // Inf.
      Matrix F (num_voronoi_vertices+1, dim);

      for (octave_idx_type d = 0; d < dim; d++)
        F(0,d) = octave_Inf;

      // The cell array of vectors of indices into F that represent the
      // vertices of the Voronoi regions (cells).

      Cell C (nr, 1);

      // Now loop through the list of vertices again and store the
      // coordinates of the Voronoi vertices and the lists of indices
      // for the cells.

      FORALLfacets
        {
          facet->seen = false;
        }

      octave_idx_type i = 0;
      k = 0;

      FORALLvertices
        {
          if (qh hull_dim == 3)
            qh_order_vertexneighbors (vertex);

          bool infinity_seen = false;

          octave_idx_type idx = qh_pointid (vertex->point);

          octave_idx_type num_vertices = ni[k++];

          // Qhull seems to sometimes produces regions with a single
          // vertex.  Is that a bug?  How can a region have just one
          // vertex?  Let's skip it.

          if (num_vertices == 1)
            continue;

          RowVector facet_list (num_vertices);

          octave_idx_type m = 0;

          facetT *neighbor, **neighborp;

          FOREACHneighbor_(vertex)
            {
              if (neighbor->upperdelaunay)
                {
                  if (! infinity_seen)
                    {
                      infinity_seen = true;
                      facet_list(m++) = 1;
                      at_inf(idx) = true;
                    }
                }
              else
                {
                  if (! neighbor->seen)
                    {
                      i++;
                      for (octave_idx_type d = 0; d < dim; d++)
                        F(i,d) = neighbor->center[d];

                      neighbor->seen = true;
                      neighbor->visitid = i;
                    }

                  facet_list(m++) = neighbor->visitid + 1;
                }
            }

          C(idx) = facet_list;
        }

      retval(2) = at_inf;
      retval(1) = C;
      retval(0) = F;
    }
  else
    error ("__voronoi__: qhull failed");

  // Free memory from Qhull
  qh_freeqhull (! qh_ALL);

  int curlong, totlong;
  qh_memfreeshort (&curlong, &totlong);

  if (curlong || totlong)
    warning ("__voronoi__: did not free %d bytes of long memory (%d pieces)",
             totlong, curlong);

#else
  error ("__voronoi__: not available in this version of Octave");
#endif

  return retval;
}

/*

## No test needed for internal helper function.
%!assert (1)

*/
