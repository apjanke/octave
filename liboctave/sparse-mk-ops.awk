BEGIN {
  declare_types = 0;
  generate_ops = 0;
  ntypes = 0;
} {
  if (NR == 1 && make_inclusive_header)
    {
      print "// DO NOT EDIT -- generated by sparse-mk-ops";
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

      if (NF == 6)
        {
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
      if (NF >= 5)
        {
          result_tag_1 = $1;
          result_tag_2 = $2;
          lhs_tag = $3;
          rhs_tag = $4;
	  op_type = $5;

	  bin_ops = index (op_type, "B") != 0;
	  cmp_ops = index (op_type, "C") != 0;
	  eqne_ops = index (op_type, "E") != 0;
	  bool_ops = index (op_type, "L") != 0;

          n = 5;

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

	  result_num_1 = rev_tag[result_tag_1];
	  result_num_2 = rev_tag[result_tag_2];
	  lhs_num = rev_tag[lhs_tag];
	  rhs_num = rev_tag[rhs_tag];

	  result_type_1 = type[result_num_1];
	  result_type_2 = type[result_num_2];
	  lhs_type = type[lhs_num];
          rhs_type = type[rhs_num];

	  result_scalar_zero_val_1 = scalar_zero_val[result_num_1];
	  result_scalar_zero_val_2 = scalar_zero_val[result_num_2];
          lhs_scalar_zero_val = scalar_zero_val[lhs_num];
          rhs_scalar_zero_val = scalar_zero_val[rhs_num];

	  result_header_1 = header[result_num_1];
	  result_header_2 = header[result_num_2];
	  lhs_header = header[lhs_num];
          rhs_header = header[rhs_num];

	  lhs_class = class[lhs_num];
	  rhs_class = class[rhs_num];

	  print "// DO NOT EDIT -- generated by sparse-mk-ops" > h_file;

	  printf ("#if !defined (%s)\n", h_guard) >> h_file;
	  printf ("#define %s 1\n", h_guard) >> h_file;

          if (result_header_1)
	    {
	      if (result_fwd_decl_ok)
	        printf ("class %s\n", result_type_1) >> h_file;
	      else
	        printf ("#include \"%s\"\n", result_header_1) >> h_file;
	    }

          if (result_header_2 && ! (result_header_2 == result_header_1))
	    {
	      if (result_fwd_decl_ok)
	        printf ("class %s\n", result_type_2) >> h_file;
	      else
	        printf ("#include \"%s\"\n", result_header_2) >> h_file;
	    }

          if (lhs_header && ! (lhs_header == result_header_1 || lhs_header == result_header_2))
	    {
	      if (result_fwd_decl_ok)
	        printf ("class %s\n", lhs_type) >> h_file;
	      else
	        printf ("#include \"%s\"\n", lhs_header) >> h_file;
	    }

          if (rhs_header && ! (rhs_header == lhs_header || rhs_header == result_header_1 || rhs_header == result_header_2))
	    {
	      if (result_fwd_decl_ok)
	        printf ("class %s\n", rhs_type) >> h_file;
	      else
	        printf ("#include \"%s\"\n", rhs_header) >> h_file;
	    }

          printf ("#include \"Sparse-op-defs.h\"\n") >> h_file;

          if (bin_ops)
            printf ("SPARSE_%s%s_BIN_OP_DECLS (%s, %s, %s, %s)\n", lhs_class,
		    rhs_class, result_type_1, result_type_2, lhs_type, 
		    rhs_type) >> h_file

          if (cmp_ops)
            printf ("SPARSE_%s%s_CMP_OP_DECLS (%s, %s)\n", lhs_class,
		    rhs_class, lhs_type, rhs_type) >> h_file

          if (eqne_ops)
            printf ("SPARSE_%s%s_EQNE_OP_DECLS (%s, %s)\n", lhs_class,
		    rhs_class, lhs_type, rhs_type) >> h_file

          if (bool_ops)
            printf ("SPARSE_%s%s_BOOL_OP_DECLS (%s, %s)\n", lhs_class,
		    rhs_class, lhs_type, rhs_type) >> h_file


          print "#endif" >> h_file;

	  close (h_file);


	  print "// DO NOT EDIT -- generated by sparse-mk-ops" > cc_file;

	  ## print "#ifdef HAVE_CONFIG_H" >> cc_file;
	  print "#include <config.h>" >> cc_file;
	  ## print "#endif" >> cc_file;

	  print "#include \"Array-util.h\"" >> cc_file;
	  print "#include \"quit.h\"" >> cc_file;

	  printf ("#include \"%s\"\n", h_file) >> cc_file;

	  for (i in bool_headers)
	    {
	      printf ("#include \"%s\"\n", bool_headers[i]) >> cc_file;
	      delete bool_headers[i];
	    }

          if (result_header_1)
	    printf ("#include \"%s\"\n", result_header_1) >> cc_file;

          if (result_header_2 && ! (result_header_2 == result_header_1))
	    printf ("#include \"%s\"\n", result_header_2) >> cc_file;

          if (lhs_header && ! (lhs_header == result_header_1 || lhs_header == result_header_2))
	    printf ("#include \"%s\"\n", lhs_header) >> cc_file;

          if (rhs_header && ! (rhs_header == lhs_header || rhs_header == result_header_1 || rhs_heaer == result_header_2))
	    printf ("#include \"%s\"\n", rhs_header) >> cc_file;

	  if (bin_ops)
            printf ("SPARSE_%s%s_BIN_OPS (%s, %s, %s, %s)\n", lhs_class, 
		    rhs_class, result_type_1, result_type_2, lhs_type, 
		    rhs_type) >> cc_file

          if (cmp_ops)
            printf ("SPARSE_%s%s_CMP_OPS (%s, %s, %s, %s, %s, %s)\n", 
		    lhs_class, rhs_class, lhs_type, lhs_scalar_zero_val,
		    lhs_conv, rhs_type, rhs_scalar_zero_val, rhs_conv) >> cc_file

          if (eqne_ops)
            printf ("SPARSE_%s%s_EQNE_OPS (%s, %s, %s, %s, %s, %s)\n", 
		    lhs_class, rhs_class, lhs_type, lhs_scalar_zero_val,
		    lhs_conv, rhs_type, rhs_scalar_zero_val, rhs_conv) >> cc_file

          if (bool_ops)
            printf ("SPARSE_%s%s_BOOL_OPS2 (%s, %s, %s, %s)\n", lhs_class, 
		    rhs_class, lhs_type, rhs_type, lhs_scalar_zero_val,
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
