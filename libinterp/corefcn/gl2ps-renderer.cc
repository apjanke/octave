/*

Copyright (C) 2009-2015 Shai Ayal

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

#include "error.h"
#include "gl2ps-renderer.h"
#include "txt-eng-ft.h"

#ifdef HAVE_GL2PS_H

#include <cstdio>

#include "lo-mappers.h"
#include "oct-locbuf.h"
#include "unwind-prot.h"

#include "gl2ps.h"
#include "sysdep.h"

void
glps_renderer::draw (const graphics_object& go, const std::string& print_cmd)
{
  static bool in_draw = false;
  static std::string old_print_cmd;

  if (!in_draw)
    {
      unwind_protect frame;

      frame.protect_var (in_draw);

      in_draw = true;

      GLint gl2ps_term;
      if (term.find ("eps") != std::string::npos)
        gl2ps_term = GL2PS_EPS;
      else if (term.find ("pdf") != std::string::npos)
        gl2ps_term = GL2PS_PDF;
      else if (term.find ("ps") != std::string::npos)
        gl2ps_term = GL2PS_PS;
      else if (term.find ("svg") != std::string::npos)
        gl2ps_term = GL2PS_SVG;
      else if (term.find ("pgf") != std::string::npos)
        gl2ps_term = GL2PS_PGF;
      else if (term.find ("tex") != std::string::npos)
        gl2ps_term = GL2PS_TEX;
      else
        {
          error ("gl2ps-renderer::draw: Unknown terminal %s", term.c_str ());
          return;
        }

      GLint gl2ps_text = 0;
      if (term.find ("notxt") != std::string::npos)
        gl2ps_text = GL2PS_NO_TEXT;

      // Default sort order optimizes for 3D plots
      GLint gl2ps_sort = GL2PS_BSP_SORT;

      // For 2D plots we can use a simpler Z-depth sorting algorithm
      if (term.find ("is2D") != std::string::npos)
        gl2ps_sort = GL2PS_SIMPLE_SORT;

      GLint state = GL2PS_OVERFLOW;
      GLint buffsize = 0;

      while (state == GL2PS_OVERFLOW)
        {
          // For LaTeX output the fltk print process uses 2 drawnow() commands.
          // The first one is for the pdf/ps/eps graph to be included.  The
          // print_cmd is saved as old_print_cmd.  Then the second drawnow()
          // outputs the tex-file and the graphic filename to be included is
          // extracted from old_print_cmd.

          std::string include_graph;

          size_t found_redirect = old_print_cmd.find (">");

          if (found_redirect != std::string::npos)
            include_graph = old_print_cmd.substr (found_redirect + 1);
          else
            include_graph = old_print_cmd;

          size_t n_begin = include_graph.find_first_not_of (" ");

          if (n_begin != std::string::npos)
            {
              size_t n_end = include_graph.find_last_not_of (" ");
              include_graph = include_graph.substr (n_begin,
                                                    n_end - n_begin + 1);
            }
          else
            include_graph = "foobar-inc";

          buffsize += 1024*1024;

          // GL2PS_SILENT was removed to allow gl2ps printing errors on stderr
          GLint ret = gl2psBeginPage ("glps_renderer figure", "Octave", 0,
                                      gl2ps_term, gl2ps_sort,
                                      (GL2PS_NO_BLENDING
                                       | GL2PS_OCCLUSION_CULL
                                       | GL2PS_BEST_ROOT
                                       | gl2ps_text
                                       | GL2PS_NO_PS3_SHADING
                                       | GL2PS_USE_CURRENT_VIEWPORT),
                                      GL_RGBA, 0, 0, 0, 0, 0,
                                      buffsize, fp, include_graph.c_str ());
          if (ret == GL2PS_ERROR)
            {
              old_print_cmd.clear ();
              error ("gl2ps-renderer::draw: gl2psBeginPage returned GL2PS_ERROR");
              return;
            }

          old_print_cmd = print_cmd;

          opengl_renderer::draw (go);

          // Without glFinish () there may be primitives missing in the
          // gl2ps output.
          glFinish ();

          state = gl2psEndPage ();

          if (state == GL2PS_ERROR)
            {
              old_print_cmd.clear ();
              error ("gl2ps-renderer::draw: gl2psEndPage returned GL2PS_ERROR");
              return;
            }

          // Don't check state for GL2PS_UNINITIALIZED (should never happen)
          // GL2PS_OVERFLOW (see while loop) or GL2PS_SUCCESS
        }
    }
  else
    opengl_renderer::draw (go);
}

int
glps_renderer::alignment_to_mode (int ha, int va) const
{
  int gl2psa = GL2PS_TEXT_BL;

  if (ha == 0)
    {
      if (va == 0 || va == 3)
        gl2psa=GL2PS_TEXT_BL;
      else if (va == 2)
        gl2psa=GL2PS_TEXT_TL;
      else if (va == 1)
        gl2psa=GL2PS_TEXT_CL;
    }
  else if (ha == 2)
    {
      if (va == 0 || va == 3)
        gl2psa=GL2PS_TEXT_BR;
      else if (va == 2)
        gl2psa=GL2PS_TEXT_TR;
      else if (va == 1)
        gl2psa=GL2PS_TEXT_CR;
    }
  else if (ha == 1)
    {
      if (va == 0 || va == 3)
        gl2psa=GL2PS_TEXT_B;
      else if (va == 2)
        gl2psa=GL2PS_TEXT_T;
      else if (va == 1)
        gl2psa=GL2PS_TEXT_C;
    }

  return gl2psa;
}

void
glps_renderer::fix_strlist_position (double x, double y, double z,
                                     Matrix box, double rotation,
                                     std::list<ft_render::ft_string>& lst)
{
  for (std::list<ft_render::ft_string>::iterator p = lst.begin ();
       p != lst.end (); p++)
    {
      // Get pixel coordinates
      ColumnVector coord_pix = get_transform ().transform (x, y, z, false);

      // Translate and rotate
      double rot = rotation * 4.0 * atan (1.0) / 180;
      coord_pix(0) += ((*p).get_x () + box(0))*cos (rot)
                      - ((*p).get_y () + box(1))*sin (rot);
      coord_pix(1) -= ((*p).get_y () + box(1))*cos (rot)
                      + ((*p).get_x () + box(0))*sin (rot);;

      // Turn coordinates back into current gl coordinates
      ColumnVector coord =
        get_transform ().untransform (coord_pix(0), coord_pix(1),
                                      coord_pix(2), false);
      (*p).set_x (coord(0));
      (*p).set_y (coord(1));
      (*p).set_z (coord(2));
    }
}

static std::string
code_to_symbol (uint32_t code)
{
  std::string retval;

  uint32_t idx = code - 945;
  if (idx < 25)
    {
      std::string characters("abgdezhqiklmnxoprVstufcyw");
      retval = characters[idx];
      return retval;
    }

  idx = code - 913;
  if (idx < 25)
    {
      std::string characters("ABGDEZHQIKLMNXOPRVSTUFCYW");
      retval = characters[idx];
    }
  else if (code == 978)
    retval = std::string ("U");
  else if (code == 215)
    retval = std::string ("\xb4");
  else if (code == 177)
    retval = std::string ("\xb1");
  else if (code == 8501)
    retval = std::string ("\xc0");
  else if (code == 8465)
    retval = std::string ("\xc1");
  else if (code == 8242)
    retval = std::string ("\xa2");
  else if (code == 8736)
    retval = std::string ("\xd0");
  else if (code == 172)
    retval = std::string ("\xd8");
  else if (code == 9829)
    retval = std::string ("\xa9");
  else if (code == 8472)
    retval = std::string ("\xc3");
  else if (code == 8706)
    retval = std::string ("\xb6");
  else if (code == 8704)
    retval = std::string ("\x22");
  else if (code == 9827)
    retval = std::string ("\xa7");
  else if (code == 9824)
    retval = std::string ("\xaa");
  else if (code == 8476)
    retval = std::string ("\xc2");
  else if (code == 8734)
    retval = std::string ("\xa5");
  else if (code == 8730)
    retval = std::string ("\xd6");
  else if (code == 8707)
  retval = std::string ("\x24");
  else if (code == 9830)
    retval = std::string ("\xa8");
  else if (code == 8747)
    retval = std::string ("\xf2");
  else if (code == 8727)
    retval = std::string ("\x2a");
  else if (code == 8744)
    retval = std::string ("\xda");
  else if (code == 8855)
    retval = std::string ("\xc4");
  else if (code == 8901)
    retval = std::string ("\xd7");
  else if (code == 8728)
    retval = std::string ("\xb0");
  else if (code == 8745)
    retval = std::string ("\xc7");
  else if (code == 8743)
    retval = std::string ("\xd9");
  else if (code == 8856)
    retval = std::string ("\xc6");
  else if (code == 8729)
    retval = std::string ("\xb7");
  else if (code == 8746)
    retval = std::string ("\xc8");
  else if (code == 8853)
    retval = std::string ("\xc5");
  else if (code == 8804)
    retval = std::string ("\xa3");
  else if (code == 8712)
    retval = std::string ("\xce");
  else if (code == 8839)
    retval = std::string ("\xca");
  else if (code == 8801)
    retval = std::string ("\xba");
  else if (code == 8773)
    retval = std::string ("\x40");
  else if (code == 8834)
    retval = std::string ("\xcc");
  else if (code == 8805)
    retval = std::string ("\xb3");
  else if (code == 8715)
    retval = std::string ("\x27");
  else if (code == 8764)
    retval = std::string ("\x7e");
  else if (code == 8733)
    retval = std::string ("\xb5");
  else if (code == 8838)
    retval = std::string ("\xcd");
  else if (code == 8835)
    retval = std::string ("\xc9");
  else if (code == 8739)
    retval = std::string ("\xbd");
  else if (code == 8776)
    retval = std::string ("\xbb");
  else if (code == 8869)
    retval = std::string ("\x5e");
  else if (code == 8656)
    retval = std::string ("\xdc");
  else if (code == 8592)
    retval = std::string ("\xac");
  else if (code == 8658)
    retval = std::string ("\xde");
  else if (code == 8594)
    retval = std::string ("\xae");
  else if (code == 8596)
    retval = std::string ("\xab");
  else if (code == 8593)
    retval = std::string ("\xad");
  else if (code == 8595)
    retval = std::string ("\xaf");
  else if (code == 8970)
    retval = std::string ("\xeb");
  else if (code == 8971)
    retval = std::string ("\xfb");
  else if (code == 10216)
    retval = std::string ("\xe1");
  else if (code == 10217)
    retval = std::string ("\xf1");
  else if (code == 8968)
    retval = std::string ("\xe9");
  else if (code == 8969)
    retval = std::string ("\xf9");
  else if (code == 8800)
    retval = std::string ("\xb9");
  else if (code == 8230)
    retval = std::string ("\xbc");
  else if (code == 176)
    retval = std::string ("\xb0");
  else if (code == 8709)
    retval = std::string ("\xc6");
  else if (code == 169)
    retval = std::string ("\xd3");

  if (retval.empty ())
    warning ("print: unhandled symbol %d", code);

  return retval;
}


static std::string
select_font (caseless_str fn, bool isbold, bool isitalic)
{
  std::transform (fn.begin (), fn.end (), fn.begin (), ::tolower);
  std::string fontname;
  if (fn == "times" || fn == "times-roman")
    {
      if (isitalic && isbold)
        fontname = "Times-BoldItalic";
      else if (isitalic)
        fontname = "Times-Italic";
      else if (isbold)
        fontname = "Times-Bold";
      else
        fontname = "Times-Roman";
    }
  else if (fn == "courier")
    {
      if (isitalic && isbold)
        fontname = "Courier-BoldOblique";
      else if (isitalic)
        fontname = "Courier-Oblique";
      else if (isbold)
        fontname = "Courier-Bold";
      else
        fontname = "Courier";
    }
  else if (fn == "symbol")
    fontname = "Symbol";
  else if (fn == "zapfdingbats")
    fontname = "ZapfDingbats";
  else
    {
      if (isitalic && isbold)
        fontname = "Helvetica-BoldOblique";
      else if (isitalic)
        fontname = "Helvetica-Oblique";
      else if (isbold)
        fontname = "Helvetica-Bold";
      else
        fontname = "Helvetica";
    }
  return fontname;
}

static void
escape_character (const std::string chr, std::string& str)
{
  std::size_t idx = str.find (chr);
  while (idx != std::string::npos)
    {
      str.insert (idx, "\\");
      idx = str.find (chr, idx + 2);
    }
}

Matrix
glps_renderer::render_text (const std::string& txt,
                            double x, double y, double z,
                            int ha, int va, double rotation)
{
  std::string saved_font = fontname;

  if (txt.empty ())
    return Matrix (1, 4, 0.0);

  // We have no way to get a bounding box from gl2ps, so we parse the raw
  // string using freetype
  Matrix bbox;
  std::string str = txt;
  std::list<ft_render::ft_string> lst;

  text_to_strlist (str, lst, bbox, ha, va, rotation);

  // When using "tex" or when the string has only one line and no
  // special characters, use gl2ps for alignment
  if (lst.empty () || term.find ("tex") != std::string::npos
      || (lst.size () == 1 && ! lst.front ().get_code ()))
    {
      std::string name = fontname;
      int sz = fontsize;
      if (! lst.empty () && term.find ("tex") == std::string::npos)
        {
          ft_render::ft_string s = lst.front ();
          name = select_font (s.get_name (), s.get_weight () == "bold",
                              s.get_angle () == "italic");
          set_color (s.get_color ());
          str = s.get_string ();
          sz = s.get_size ();
        }

      glRasterPos3d (x, y, z);

      // Escape parenthesis until gl2ps does it (see bug ##45301).
      if (term.find ("svg") == std::string::npos
          && term.find ("tex") == std::string::npos)
        {
          escape_character ("(", str);
          escape_character (")", str);
        }

      gl2psTextOpt (str.c_str (), name.c_str (), sz,
                    alignment_to_mode (ha, va), rotation);
      return bbox;
    }

  // Translate and rotate coordinates in order to use bottom-left alignment
  fix_strlist_position (x, y, z, bbox, rotation, lst);

  for (std::list<ft_render::ft_string>::iterator p = lst.begin ();
       p != lst.end (); p++)
    {
      fontname = select_font ((*p).get_name (),
                              (*p).get_weight () == "bold",
                              (*p).get_angle () == "italic");
      if ((*p).get_code ())
        {
          // This is only one character represented by a uint32 (utf8) code.
          // We replace it by the corresponding character in the
          // "Symbol" font except for svg which has built-in utf8 support.
          if (term.find ("svg") == std::string::npos)
            {
              fontname = "Symbol";
              str = code_to_symbol ((*p).get_code ());
            }
          else
            {
              std::stringstream ss;
              ss << (*p).get_code ();
              str = "&#" + ss.str () + ";";
            }
        }
      else
        {
          str = (*p).get_string ();
          // Escape parenthesis until gl2ps does it (see bug ##45301).
          if (term.find ("svg") == std::string::npos)
            {
              escape_character ("(", str);
              escape_character (")", str);
            }
        }

      set_color ((*p).get_color ());
      glRasterPos3d ((*p).get_x (), (*p).get_y (), (*p).get_z ());
      gl2psTextOpt (str.c_str (), fontname.c_str (), (*p).get_size (),
                    GL2PS_TEXT_BL, rotation);
    }

  fontname = saved_font;
  return bbox;
}

void
glps_renderer::set_font (const base_properties& props)
{
  opengl_renderer::set_font (props);

  // Set the interpreter so that text_to_pixels can parse strings properly
  if (props.has_property ("interpreter"))
    set_interpreter (props.get ("interpreter").string_value ());

  fontsize = props.get ("fontsize_points").double_value ();

  caseless_str fn = props.get ("fontname").xtolower ().string_value ();
  bool isbold =
    (props.get ("fontweight").xtolower ().string_value () == "bold");
  bool isitalic =
    (props.get ("fontangle").xtolower ().string_value () == "italic");

  fontname = select_font (fn, isbold, isitalic);
}

template <typename T>
static void
draw_pixels (GLsizei w, GLsizei h, GLenum format, const T *data, float maxval)
{
  OCTAVE_LOCAL_BUFFER (GLfloat, a, 3*w*h);

  // Convert to GL_FLOAT as it is the only type gl2ps accepts.
  for (int i = 0; i < 3*w*h; i++)
    a[i] = data[i] / maxval;

  gl2psDrawPixels (w, h, 0, 0, format, GL_FLOAT, a);
}

void
glps_renderer::draw_pixels (GLsizei w, GLsizei h, GLenum format,
                            GLenum type, const GLvoid *data)
{
  // gl2psDrawPixels only supports the GL_FLOAT type.
  // Other formats, such as uint8, must be converted first.
  if (type == GL_UNSIGNED_BYTE)
    ::draw_pixels (w, h, format, static_cast<const GLubyte *> (data), 255.0f);
  else if (type == GL_UNSIGNED_SHORT)
    ::draw_pixels (w, h, format, static_cast<const GLushort *> (data), 65535.0f);
  else
    gl2psDrawPixels (w, h, 0, 0, format, type, data);
}

void
glps_renderer::draw_text (const text::properties& props)
{
  if (props.get_string ().is_empty ())
    return;

  // First set font properties: freetype will use them to compute
  // coordinates and gl2ps will retrieve the color directly from the
  // feedback buffer
  set_font (props);
  set_color (props.get_color_rgb ());

  std::string saved_font = fontname;

  // Alignment
  int halign = 0;
  int valign = 0;

  if (props.horizontalalignment_is ("center"))
    halign = 1;
  else if (props.horizontalalignment_is ("right"))
    halign = 2;

  if (props.verticalalignment_is ("top"))
    valign = 2;
  else if (props.verticalalignment_is ("baseline"))
    valign = 3;
  else if (props.verticalalignment_is ("middle"))
    valign = 1;

  // FIXME: handle margin and surrounding box

  Matrix bbox;
  const Matrix pos = get_transform ().scale (props.get_data_position ());
  std::string str = props.get_string ().all_strings ().join ("\n");

  render_text (str, pos(0), pos(1), pos.numel () > 2 ? pos(2) : 0.0,
               halign, valign, props.get_rotation ());
}

static void
safe_pclose (FILE *f)
{
  if (f)
    octave_pclose (f);
}

#endif

void
gl2ps_print (const graphics_object& fig, const std::string& cmd,
             const std::string& term)
{
#ifdef HAVE_GL2PS_H

  FILE *fp = octave_popen (cmd.c_str (), "w");

  if (fp)
    {
      unwind_protect frame;

      frame.add_fcn (safe_pclose, fp);

      glps_renderer rend (fp, term);

      rend.draw (fig, cmd);
    }
  else
    error ("print: failed to open pipe for gl2ps renderer");

#else

  error ("print: printing not available without gl2ps library");

#endif
}
