# Copyright (C) 2003, 2004, 2006, 2007, 2009 John W. Eaton
#
# This file is part of Octave.
# 
# Octave is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 3 of the License, or (at
# your option) any later version.
# 
# Octave is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Octave; see the file COPYING.  If not, see
# <http://www.gnu.org/licenses/>.

BEGIN {
  declare_types = 0;
  generate_ops = 0;
  ntypes = 0;
} {
  if (NR == 1 && make_inclusive_header)
    {
      print "// DO NOT EDIT -- generated by mk-ops";
      tmp = make_inclusive_header;
      gsub (/[\.-]/, "_", tmp);
      printf ("#if !defined (octave_%s)\n", tmp);
      printf ("#define octave_%s 1\n", tmp);
    }
}
/^#/ {
  if ($2 == "types")
    declare_types = 1;
  else if ($2 == "ops")
    {
      generate_ops = 1;
      declare_types = 0;
    }
  next;
} {
  if (declare_types)
    {
      ntypes++;

      if (NF == 6 || NF == 7)
        {
	  if (NF == 7)
	    core_type[ntypes] = $7;

          scalar_zero_val[ntypes] = $6;
          fwd_decl_ok[ntypes] = $5 == "YES";
          header[ntypes] = $4 == "NONE" ? "" : $4;
          class[ntypes] = $3;
          type[ntypes] = $2;
          tag[ntypes] = $1;
          rev_tag[$1] = ntypes;
        }
      else
        printf ("skipping line %d: %s\n", NR, $0); 
    }
  else if (generate_ops)
    {
      if (NF >= 4)
        {
          result_tag = $1;
          lhs_tag = $2;
          rhs_tag = $3;
	  op_type = $4;

	  bin_ops = index (op_type, "B") != 0;
	  cmp_ops = index (op_type, "C") != 0;
	  bool_ops = index (op_type, "L") != 0;

          n = 4;

          lhs_conv = cmp_ops ? $(++n) : "";
          rhs_conv = cmp_ops ? $(++n) : "";

	  if (lhs_conv == "NONE")
	    lhs_conv = "";

	  if (rhs_conv == "NONE")
	    rhs_conv = "";

	  k = 0
	  while (NF > n)
	    bool_headers[k++] = $(++n);

	  cc_file = sprintf ("%s-%s-%s.cc", prefix, lhs_tag, rhs_tag);
	  h_file = sprintf ("%s-%s-%s.h", prefix, lhs_tag, rhs_tag);

	  if (list_cc_files)
	    {
	      print cc_file;
	      next;
	    }

	  if (list_h_files)
	    {
	      print h_file;
	      next;
	    }

	  if (make_inclusive_header)
	    {
              printf ("#include \"%s\"\n", h_file);
              next;
            }

	  h_guard = sprintf ("octave_%s_%s_%s_h", prefix, lhs_tag, rhs_tag);

	  result_num = rev_tag[result_tag];
	  lhs_num = rev_tag[lhs_tag];
	  rhs_num = rev_tag[rhs_tag];

	  result_type = type[result_num];
	  lhs_type = type[lhs_num];
          rhs_type = type[rhs_num];

	  lhs_core_type = core_type[lhs_num];
	  rhs_core_type = core_type[rhs_num];

	  result_scalar_zero_val = scalar_zero_val[result_num];
          lhs_scalar_zero_val = scalar_zero_val[lhs_num];
          rhs_scalar_zero_val = scalar_zero_val[rhs_num];

	  result_header = header[result_num];
	  lhs_header = header[lhs_num];
          rhs_header = header[rhs_num];

	  lhs_class = class[lhs_num];
	  rhs_class = class[rhs_num];

	  print "// DO NOT EDIT -- generated by mk-ops" > h_file;

	  printf ("#if !defined (%s)\n", h_guard) >> h_file;
	  printf ("#define %s 1\n", h_guard) >> h_file;

          if (result_header)
	    {
	      if (result_fwd_decl_ok)
	        printf ("class %s\n", result_type) >> h_file;
	      else
	        printf ("#include \"%s\"\n", result_header) >> h_file;
	    }

          if (lhs_header && ! (lhs_header == result_header))
	    {
	      if (result_fwd_decl_ok)
	        printf ("class %s\n", lhs_type) >> h_file;
	      else
	        printf ("#include \"%s\"\n", lhs_header) >> h_file;
	    }

          if (rhs_header && ! (rhs_header == lhs_header || rhs_header == result_header))
	    {
	      if (result_fwd_decl_ok)
	        printf ("class %s\n", rhs_type) >> h_file;
	      else
	        printf ("#include \"%s\"\n", rhs_header) >> h_file;
	    }

          printf ("#include \"mx-op-decl.h\"\n") >> h_file;

          if (bin_ops)
            printf ("%s%s_BIN_OP_DECLS (%s, %s, %s, OCTAVE_API)\n", lhs_class,
		    rhs_class, result_type, lhs_type, rhs_type) >> h_file

          if (cmp_ops)
            printf ("%s%s_CMP_OP_DECLS (%s, %s, OCTAVE_API)\n", lhs_class,
		    rhs_class, lhs_type, rhs_type) >> h_file

          if (bool_ops)
            printf ("%s%s_BOOL_OP_DECLS (%s, %s, OCTAVE_API)\n", lhs_class,
		    rhs_class, lhs_type, rhs_type) >> h_file


          print "#endif" >> h_file;

	  close (h_file);


	  print "// DO NOT EDIT -- generated by mk-ops" > cc_file;

	  print "#ifdef HAVE_CONFIG_H" >> cc_file;
	  print "#include <config.h>" >> cc_file;
	  print "#endif" >> cc_file;

	  print "#include \"Array-util.h\"" >> cc_file;

	  printf ("#include \"%s\"\n", h_file) >> cc_file;

          printf ("#include \"mx-op-defs.h\"\n") >> cc_file;

	  for (i in bool_headers)
	    {
	      printf ("#include \"%s\"\n", bool_headers[i]) >> cc_file;
	      delete bool_headers[i];
	    }

          if (result_header)
	    printf ("#include \"%s\"\n", result_header) >> cc_file;

          if (lhs_header && ! (lhs_header == result_header))
	    printf ("#include \"%s\"\n", lhs_header) >> cc_file;

          if (rhs_header && ! (rhs_header == lhs_header || rhs_header == result_header))
	    printf ("#include \"%s\"\n", rhs_header) >> cc_file;

	  if (bin_ops)
            {
              if ((lhs_class == "DM" && rhs_class == "M") || (lhs_class == "M" && rhs_class == "DM"))
                printf ("%s%s_BIN_OPS (%s, %s, %s, %s)\n",
		        lhs_class, rhs_class, result_type,
		        lhs_type, rhs_type, result_scalar_zero_val) >> cc_file
              else
                printf ("%s%s_BIN_OPS (%s, %s, %s)\n",
		        lhs_class, rhs_class, result_type,
			lhs_type, rhs_type) >> cc_file
            }

          if (cmp_ops)
	    {
	      if (lhs_class == "S" || rhs_class == "S")
	        {
		  if (lhs_core_type)
		    {
		      if (rhs_core_type)
			printf ("%s%s_CMP_OPS2 (%s, %s, %s, %s, %s, %s)\n",
				lhs_class, rhs_class, lhs_type, lhs_conv,
				rhs_type, rhs_conv,
				lhs_core_type, rhs_core_type) >> cc_file
		      else
			printf ("%s%s_CMP_OPS1 (%s, %s, %s, %s, %s)\n",
				lhs_class, rhs_class, lhs_type, lhs_conv,
				rhs_type, rhs_conv, lhs_core_type) >> cc_file
		    }
		  else
		    {
		      if (rhs_core_type)
			printf ("%s%s_CMP_OPS1 (%s, %s, %s, %s, %s)\n",
				lhs_class, rhs_class, lhs_type, lhs_conv,
				rhs_type, rhs_conv, rhs_core_type) >> cc_file
		      else
			printf ("%s%s_CMP_OPS (%s, %s, %s, %s)\n",
				lhs_class, rhs_class, lhs_type, lhs_conv,
				rhs_type, rhs_conv) >> cc_file
		    }
		}
	      else
		printf ("%s%s_CMP_OPS (%s, %s, %s, %s)\n",
			lhs_class, rhs_class, lhs_type, lhs_conv,
			rhs_type, rhs_conv) >> cc_file
	    }

          if (bool_ops)
            printf ("%s%s_BOOL_OPS2 (%s, %s, %s, %s)\n", lhs_class, rhs_class,
	            lhs_type, rhs_type, lhs_scalar_zero_val,
	            rhs_scalar_zero_val) >> cc_file


          close (cc_file);
        }
      else
        printf ("skipping line %d: %s\n", NR, $0); 
    }
}
END {
  if (make_inclusive_header)
    print "#endif";
}
