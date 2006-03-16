function sparseimages(dirc,typ)
  ## XXX FIXME XXX 
  ## How do we set terminal and direct the output to /dev/null without
  ## gnuplot? Note that due to replot in print.m, the redirection to
  ## /dev/null effectively doesn't work at the moment.
  __gnuplot_set__ term dumb
  [status, dummy] = fileattrib("/dev/null");
  if (status)
    __gnuplot_set__ output '/dev/null'
  endif
  plot(1) # FIXME bypass 2.9.4 bug!!
  if (strcmp(typ,"txt"))
    txtimages(15,dirc,typ);
  else
    otherimages(200,dirc,typ);
    gplotimages("gplot",dirc,typ);
    femimages("grid",dirc,typ);
  endif
endfunction

function gplotimages(nm,dirc,typ)
  A = sparse([2,6,1,3,2,4,3,5,4,6,1,5],
	     [1,1,2,2,3,3,4,4,5,5,6,6],1,6,6);
  xy = [0,4,8,6,4,2;5,0,5,7,5,7]';
  gplot(A,xy)
  print(strcat(dirc,filesep,nm,".",typ),strcat("-d",typ))
endfunction

function txtimages(n,dirc,typ)
  a = 10*speye(n) + sparse(1:n,ceil([1:n]/2),1,n,n) + ...
      sparse(ceil([1:n]/2),1:n,1,n,n);
  printsparse(a,strcat(dirc,filesep,"spmatrix.",typ));
  if (!isempty(findstr(octave_config_info ("DEFS"),"HAVE_COLAMD")) &&
      !isempty(findstr(octave_config_info ("DEFS"),"HAVE_CHOLMOD")))
    r1 = chol(a);
    printsparse(r1,strcat(dirc,filesep,"spchol.",typ));
    [r2,p2,q2]=chol(a);
    printsparse(r2,strcat(dirc,filesep,"spcholperm.",typ));
    printf("Text NNZ: Matrix %d, Chol %d, PermChol %d\n",nnz(a),nnz(r1),nnz(r2));
  endif
endfunction

function otherimages(n,dirc,typ)
  a = 10*speye(n) + sparse(1:n,ceil([1:n]/2),1,n,n) + ...
      sparse(ceil([1:n]/2),1:n,1,n,n);
  spy(a);
  axis("ij")
  print(strcat(dirc,filesep,"spmatrix.",typ),strcat("-d",typ))
  if (!isempty(findstr(octave_config_info ("DEFS"),"HAVE_COLAMD")) &&
      !isempty(findstr(octave_config_info ("DEFS"),"HAVE_CHOLMOD")))
    r1 = chol(a);
    spy(r1);
    axis("ij")
    print(strcat(dirc,filesep,"spchol.",typ),strcat("-d",typ))
    [r2,p2,q2]=chol(a);
    spy(r2);
    axis("ij")
    print(strcat(dirc,filesep,"spcholperm.",typ),strcat("-d",typ))
    printf("Image NNZ: Matrix %d, Chol %d, PermChol %d\n",nnz(a),nnz(r1),nnz(r2));
    axis("xy")
  endif
endfunction

function printsparse(a,nm)
  fid = fopen (nm,"wt");
  for i = 1:size(a,1)
    if (rem(i,5) == 0)
      fprintf (fid,"         %2d - ", i);
    else
      fprintf (fid,"            | ");
    endif
    for j = 1:size(a,2)
      if (a(i,j) == 0)
	fprintf(fid,"  ")
      else
	fprintf(fid," *")
      endif
    endfor
    fprintf(fid,"\n")
  endfor
  fprintf(fid,"            |-");
  for j=1:size(a,2)
    if (rem(j,5)==0)
      fprintf(fid,"-|");
    else
      fprintf(fid,"--");
    endif
  endfor
  fprintf(fid,"\n")
  fprintf(fid,"              ");
  for j=1:size(a,2)
    if (rem(j,5)==0)
      fprintf(fid,"%2d",j);
    else
      fprintf(fid,"  ");
    endif
  endfor
  fclose(fid);
endfunction

function femimages (nm,dirc,typ)
  if (!isempty(findstr(octave_config_info ("DEFS"),"HAVE_COLAMD")) &&
      !isempty(findstr(octave_config_info ("DEFS"),"HAVE_CHOLMOD")) &&
      !isempty(findstr(octave_config_info ("DEFS"),"HAVE_UMFPACK")))
    ## build a rectangle
    node_y = [1;1.2;1.5;1.8;2]*ones(1,11);
    node_x = ones(5,1)*[1,1.05,1.1,1.2,1.3,1.5,1.7,1.8,1.9,1.95,2];
    nodes = [node_x(:), node_y(:)];

    [h,w] = size(node_x);
    elems = [];
    for idx = 1:w-1
      widx = (idx-1)*h;
      elems = [elems; widx+[(1:h-1);(2:h);h+(1:h-1)]']; 
      elems = [elems; widx+[(2:h);h+(2:h);h+(1:h-1)]']; 
    endfor

    E = size(elems,1);  #No. of elements
    N = size(nodes,1);  #No. of elements
    D = size(elems,2);  #dimentions+1

    ## Plot FEM Geometry
    elemx = elems(:,[1,2,3,1])';
    xelems = reshape( nodes(elemx, 1), 4, E);
    yelems = reshape( nodes(elemx, 2), 4, E);

    ## Set element conductivity
    conductivity = [1*ones(1,16),2*ones(1,48),1*ones(1,16)];

    ## Dirichlet boundary conditions
    D_nodes = [1:5, 51:55]; 
    D_value = [10*ones(1,5), 20*ones(1,5)]; 
  
    ## Neumann boundary conditions
    ## Note that N_value must be normalized by the boundary
    ##   length and element conductivity
    N_nodes = [];
    N_value = [];

    ## Calculate connectivity matrix
    C = sparse((1:D*E), reshape(elems',D*E,1),1, D*E, N);

    ## Calculate stiffness matrix
    Siidx = floor([0:D*E-1]'/D)*D*ones(1,D) + ones(D*E,1)*(1:D) ;
    Sjidx = [1:D*E]'*ones(1,D);
    Sdata = zeros(D*E,D);
    dfact = prod(2:(D-1));
    for j = 1:E
      a = inv([ ones(D,1), nodes( elems(j,:), : ) ]);
      const = conductivity(j)*2/dfact/abs(det(a));
      Sdata(D*(j-1)+(1:D),:)= const * a(2:D,:)'*a(2:D,:);
    endfor

    ## Element-wise system matrix
    SE = sparse(Siidx,Sjidx,Sdata);
    ## Global system matrix
    S = C'* SE *C;

    ## Set Dirichlet boundary
    V = zeros(N,1);
    V(D_nodes) = D_value;
    idx = 1:N;
    idx(D_nodes) = [];

    ## Set Neumann boundary
    Q = zeros(N,1);
    Q(N_nodes) = N_value; # FIXME

    V(idx) = S(idx,idx)\( Q(idx) - S(idx,D_nodes)*V(D_nodes) );

    velems = reshape( V(elemx), 4, E);

    sz = size(xelems,2);
    ## FIXME How can I do this without a gnuplot specific commands? plot3 anyone?
    unwind_protect
      __gnuplot_set__  parametric;
      __gnuplot_raw__ ("set nohidden3d;\n");
      tmp = [([xelems; NaN*ones(1,sz)])(:), ([yelems; NaN*ones(1,sz)])(:), ([velems; NaN*ones(1,sz)])(:)];
      __gnuplot_splot__(tmp);
      __gnuplot_raw__ ("set view 80,10;\n")
      print(strcat(dirc,filesep,nm,".",typ),strcat("-d",typ))
    unwind_protect_cleanup
      __gnuplot_set__ noparametric; 
    end_unwind_protect
  endif
endfunction
