## Copyright (C) 2010-2012 John W. Eaton
## Copyright (C) 2010 VZLU Prague
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or (at
## your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {Function File} {@var{txt} =} unimplemented (@var{fcn})
## Return specific help text for the unimplemented function @var{fcn}.
## This is usually a suggestion for an existing compatible function to use in
## place of @var{fcn}.
##
## This function is not called by users, but by the Octave interpreter when
## it fails to recognize an input string as a valid function name.  See
## @code{missing_function_hook} for using a different handler for this event.
## @seealso{missing_function_hook}
## @end deftypefn


function txt = __unimplemented__ (fcn)

  if (nargin != 1)
    print_usage ();
  endif

  is_matlab_function = true;

  ## Some smarter cases, add more as needed.
  switch (fcn)
    case {"avifile", "aviinfo", "aviread"}
      txt = ["Basic video file support is provided in the video package.  ", ...
             "See @url{http://octave.sf.net/video/}."];

    case "exifread"
      txt = ["exifread is deprecated.  " ...
             "The functionality is available in the imfinfo function."];

    case "gsvd"
      txt = ["gsvd is not currently part of core Octave.  ", ...
             "See the linear-algebra package at ", ...
             "@url{http://octave.sourceforge.net/linear-algebra/}."];

    case "funm"
      txt = ["funm is not currently part of core Octave.  ", ...
             "See the linear-algebra package at ", ...
             "@url{http://octave.sourceforge.net/linear-algebra/}."];

    case "griddedInterpolant"
      txt = ["griddedInterpolant is not implemented.  ", ...
             "Consider using griddata."];

    case "integral"
      txt = ["Octave provides many routines for 1-D numerical integration.  ", ...
             "Consider quadcc, quad, quadv, quadl, quadgk."];

    case "integral2"
      txt = ["integral2 is not implemented.  Consider using dblquad."];

    case "integral3"
      txt = ["integral3 is not implemented.  Consider using triplequad"];

    case "linprog"
      txt = ["Octave does not currently provide linprog.  ", ...
             "Linear programming problems may be solved using @code{glpk}.  ", ...
             "Try @code{help glpk} for more info."];

    case "matlabrc"
      txt = ["matlabrc is not implemented.  ", ...
             'Octave uses the file ".octaverc" instead.'];

    case {"ode113", "ode15i", "ode15s", "ode23", "ode23s", "ode23t", ...
          "ode23tb", "ode45", "odeget", "odeset"}
      txt = ["Octave provides lsode for solving differential equations.  ", ...
             "For more information try @code{help lsode}.  ", ...
             "Matlab-compatible ODE functions are provided by the odepkg ", ...
             "package.  See @url{http://octave.sourceforge.net/odepkg/}."];

    case "startup"
      txt = ["startup is not implemented.  ", ...
             'Octave uses the file ".octaverc" instead.'];

    case "quad2d"
      txt = ["quad2d is not implemented.  Consider using dblquad."];

    case {"xlsread", "xlsfinfo", "xlswrite", "wk1read", "wk1finfo", "wk1write"}
      txt = ["Functions for spreadsheet style I/O ", ...
             "(.xls .xlsx .sxc .ods .dbf .wk1 etc.) " , ...
             "are provided in the io package. ", ...
             "See @url{http://octave.sf.net/io/}."];

    ## control system
    case {"absorbDelay", "allmargin", "append", "augstate", "balreal", ...
          "balred", "balredOptions", "bandwidth", "bdschur", "bode", ...
          "bodemag", "bodeoptions", "bodeplot", "c2d", "c2dOptions", ...
          "canon", "care", "chgFreqUnit", "chgTimeUnit", "connect", ...
          "connectOptions", "covar", "ctrb", "ctrbf", "ctrlpref", "d2c", ...
          "d2cOptions", "d2d", "d2dOptions", "damp", "dare", "db2mag", ...
          "dcgain", "delay2z", "delayss", "dlqr", "dlyap", "dlyapchol", ...
          "drss", "dsort", "dss", "dssdata", "esort", "estim", "evalfr", ...
          "feedback", "filt", "frd", "frdata", "freqresp", "gcare", "gdare", ...
          "genfrd", "genmat", "gensig", "genss", "get", "getBlockValue", ...
          "getDelayModel", "getGainCrossover", "getIOTransfer", ...
          "getLFTModel", "getLoopTransfer", "getNominal", "getoptions", ...
          "getPeakGain", "getSwitches", "getValue", "gram", "hasdelay", ...
          "hasInternalDelay", "hsvd", "hsvdOptions", "hsvoptions", ...
          "hsvplot", "imp2exp", "impulse", "impulseplot", "initial", ...
          "initialplot", "iopzmap", "iopzplot", "isct", "isdt", "isempty", ...
          "isfinite", "isParametric", "isproper", "isreal", "issiso", ...
          "isstable", "isstatic", "kalman", "kalmd", "lft", "loopswitch", ...
          "lqg", "lqgreg", "lqgtrack", "lqi", "lqr", "lqrd", "lqry", "lsim", ...
          "lsiminfo", "lsimplot", "ltiview", "lyap", "lyapchol", "mag2db", ...
          "margin", "minreal", "modred", "modsep", "nblocks", "ndims", ...
          "ngrid", "nichols", "nicholsoptions", "nicholsplot", "nmodels", ...
          "norm", "nyquist", "nyquistoptions", "nyquistplot", "obsv", ...
          "obsvf", "order", "pade", "parallel", "permute", "pid", "piddata", ...
          "pidstd", "pidstddata", "pidtool", "pidtune", "pidtuneOptions", ...
          "place", "pole", "prescale", "pzmap", "pzoptions", "pzplot", ...
          "realp", "reg", "replaceBlock", "repsys", "reshape", "rlocus", ...
          "rlocusplot", "rss", "series", "set", "setBlockValue", ...
          "setDelayModel", "setoptions", "setValue", "sgrid", ...
          "showBlockValue", "showTunable", "sigma", "sigmaoptions", ...
          "sigmaplot", "sisoinit", "sisotool", "size", "sminreal", "ss", ...
          "ss2ss", "ssdata", "stabsep", "stabsepOptions", "stack", "step", ...
          "stepDataOptions", "stepinfo", "stepplot", "sumblk", "tf", ...
          "tfdata", "thiran", "timeoptions", "totaldelay", "tzero", ...
          "updateSystem", "upsample", "xperm", "zero", "zgrid", "zpk", ...
          "zpkdata"}
      txt = check_package (fcn, "control");

    ## communications
    case {"algdeintrlv", "algintrlv", "alignsignals", "amdemod", "ammod", ...
          "arithdeco", "arithenco", "awgn", "bchdec", "bchenc", ...
          "bchgenpoly", "bchnumerr", "berawgn", "bercoding", "berconfint", ...
          "berfading", "berfit", "bersync", "bertool", "bi2de", "bin2gray", ...
          "biterr", "bsc", "cma", "commscope", "compand", "convdeintrlv", ...
          "convenc", "convintrlv", "convmtx", "cosets", "cyclgen", ...
          "cyclpoly", "de2bi", "decode", "deintrlv", "dfe", "dftmtx", ...
          "distspec", "doppler", "dpcmdeco", "dpcmenco", "dpcmopt", ...
          "dpskdemod", "dpskmod", "dvbs2ldpc", "encode", "equalize", ...
          "eyediagram", "EyeScope", "finddelay", "fmdemod", "fmmod", ...
          "fskdemod", "fskmod", "gaussdesign", "gen2par", "genqamdemod", ...
          "genqammod", "gf", "gfadd", "gfconv", "gfcosets", "gfdeconv", ...
          "gfdiv", "gffilter", "gflineq", "gfminpol", "gfmul", "gfpretty", ...
          "gfprimck", "gfprimdf", "gfprimfd", "gfrank", "gfrepcov", ...
          "gfroots", "gfsub", "gftable", "gftrunc", "gftuple", "gfweight", ...
          "gray2bin", "hammgen", "heldeintrlv", "helintrlv", ...
          "helscandeintrlv", "helscanintrlv", "huffmandeco", "huffmandict", ...
          "huffmanenco", "intdump", "intrlv", "iscatastrophic", ...
          "isprimitive", "istrellis", "legacychannelsim", "lineareq", ...
          "lloyds", "lms", "log", "lteZadoffChuSeq", "marcumq", ...
          "mask2shift", "matdeintrlv", "matintrlv", "minpol", "mldivide", ...
          "mlseeq", "modnorm", "muxdeintrlv", "muxintrlv", "noisebw", ...
          "normlms", "oct2dec", "oqpskdemod", "oqpskmod", "pamdemod", ...
          "pammod", "pmdemod", "pmmod", "poly2trellis", "primpoly", ...
          "pskdemod", "pskmod", "qamdemod", "qammod", "qfunc", "qfuncinv", ...
          "quantiz", "randdeintrlv", "randerr", "randintrlv", "randsrc", ...
          "rayleighchan", "rcosdesign", "rectpulse", "reset", "ricianchan", ...
          "rls", "rsdec", "rsenc", "rsgenpoly", "rsgenpolycoeffs", ...
          "scatterplot", "sdrexamples", "sdrfgensysace", "sdrfroot", ...
          "sdrinfo", "sdrload", "sdrsetup", "semianalytic", "shift2mask", ...
          "signlms", "ssbdemod", "ssbmod", "stdchan", ...
          "supportPackageInstaller", "symerr", "syndtable", "varlms", ...
          "vec2mat", "vitdec", "wgn"}
      txt = check_package (fcn, "comm");

    ## finance
    case {"abs2active", "accrfrac", "acrubond", "acrudisc", "active2abs", ...
          "addEquality", "addEquality", "addEquality", "addGroupRatio", ...
          "addGroupRatio", "addGroupRatio", "addGroups", "addGroups", ...
          "addGroups", "addInequality", "addInequality", "addInequality", ...
          "adline", "adosc", "amortize", "annurate", "annuterm", ...
          "arith2geom", "ascii2fts", "beytbill", "binprice", "blkimpv", ...
          "blkprice", "blsdelta", "blsgamma", "blsimpv", "blslambda", ...
          "blsprice", "blsrho", "blstheta", "blsvega", "bndconvp", ...
          "bndconvy", "bnddurp", "bnddury", "bndkrdur", "bndprice", ...
          "bndspread", "bndtotalreturn", "bndyield", "bolling", "bollinger", ...
          "boxcox", "busdate", "busdays", "candle", "cdai", "cdprice", ...
          "cdyield", "cfamounts", "cfbyzero", "cfconv", "cfdates", "cfdur", ...
          "cfplot", "cfport", "cfprice", "cfspread", "cftimes", "cfyield", ...
          "chaikosc", "chaikvolat", "chartfts", "checkFeasibility", ...
          "checkFeasibility", "checkFeasibility", "chfield", "convert2sur", ...
          "convertto", "corr2cov", "cov2corr", "cpncount", "cpndaten", ...
          "cpndatenq", "cpndatep", "cpndaysp", "cpnpersz", "createholidays", ...
          "cumsum", "cur2frac", "cur2str", "date2time", "dateaxis", ...
          "datedisp", "datefind", "datemnth", "datewrkdy", "day", "days360", ...
          "days360e", "days360isda", "days360psa", "days365", "daysact", ...
          "daysadd", "daysdif", "dec2thirtytwo", "depfixdb", "depgendb", ...
          "deprdv", "depsoyd", "depstln", "diff", "disc2zero", "discrate", ...
          "ecmmvnrfish", "ecmmvnrmle", "ecmmvnrobj", "ecmmvnrstd", ...
          "ecmnfish", "ecmnhess", "ecmninit", "ecmnmle", "ecmnobj", ...
          "ecmnstd", "effrr", "elpm", "emaxdrawdown", "end", "eomdate", ...
          "estimateAssetMoments", "estimateBounds", "estimateBounds", ...
          "estimateBounds", "estimateFrontier", "estimateFrontier", ...
          "estimateFrontier", "estimateFrontierByReturn", ...
          "estimateFrontierByReturn", "estimateFrontierByReturn", ...
          "estimateFrontierByRisk", "estimateFrontierByRisk", ...
          "estimateFrontierByRisk", "estimateFrontierLimits", ...
          "estimateFrontierLimits", "estimateFrontierLimits", ...
          "estimateMaxSharpeRatio", "estimatePortMoments", ...
          "estimatePortReturn", "estimatePortReturn", "estimatePortReturn", ...
          "estimatePortRisk", "estimatePortRisk", "estimatePortRisk", ...
          "estimatePortStd", "estimatePortStd", "estimatePortVaR", ...
          "estimateScenarioMoments", "estimateScenarioMoments", "ewstats", ...
          "exp", "extfield", "fbusdate", "fetch", "fieldnames", "fillts", ...
          "fints", "floatdiscmargin", "floatmargin", "fpctkd", "frac2cur", ...
          "freqnum", "freqstr", "frontcon", "frontier", "fts2ascii", ...
          "fts2mat", "ftsbound", "ftsgui", "ftsinfo", "ftstool", "ftsuniq", ...
          "fvdisc", "fvfix", "fvvar", "fwd2zero", "geom2arith", ...
          "getAssetMoments", "getBounds", "getBounds", "getBounds", ...
          "getBudget", "getBudget", "getBudget", "getCosts", "getCosts", ...
          "getCosts", "getEquality", "getEquality", "getEquality", ...
          "getGroupRatio", "getGroupRatio", "getGroupRatio", "getGroups", ...
          "getGroups", "getGroups", "getInequality", "getInequality", ...
          "getInequality", "getnameidx", "getOneWayTurnover", ...
          "getOneWayTurnover", "getOneWayTurnover", "getScenarios", ...
          "getScenarios", "hhigh", "highlow", "holdings2weights", ...
          "holidays", "horzcat", "hour", "inforatio", "irr", "isbusday", ...
          "iscompatible", "isempty", "isfield", "issorted", "kagi", "lagts", ...
          "lbusdate", "leadts", "length", "linebreak", "llow", "log", ...
          "log10", "log2", "lpm", "lweekdate", "m2xdate", "macd", ...
          "maxdrawdown", "medprice", "merge", "minus", "minute", "mirr", ...
          "month", "months", "movavg", "mrdivide", "mtimes", "mvnrfish", ...
          "mvnrmle", "mvnrobj", "mvnrstd", "nancov", "nanmax", "nanmean", ...
          "nanmedian", "nanmin", "nanstd", "nansum", "nanvar", "negvolidx", ...
          "nomrr", "nweekdate", "nyseclosures", "onbalvol", "opprofit", ...
          "payadv", "payodd", "payper", "payuni", "pcalims", "pcgcomp", ...
          "pcglims", "pcpval", "peravg", "periodicreturns", "plotFrontier", ...
          "plotFrontier", "plotFrontier", "plus", "pointfig", "portalloc", ...
          "portalpha", "portcons", "Portfolio", "PortfolioCVaR", ...
          "PortfolioMAD", "portopt", "portrand", "portror", "portsim", ...
          "portstats", "portvar", "portvrisk", "posvolidx", "power", ...
          "prbyzero", "prcroc", "prdisc", "priceandvol", "prmat", "prtbill", ...
          "pvfix", "pvtrend", "pvvar", "pyld2zero", "rdivide", "renko", ...
          "resamplets", "ret2tick", "rmfield", "rsindex", "selectreturn", ...
          "setAssetList", "setAssetList", "setAssetList", "setAssetMoments", ...
          "setBounds", "setBounds", "setBounds", "setBudget", "setBudget", ...
          "setBudget", "setCosts", "setCosts", "setCosts", ...
          "setDefaultConstraints", "setDefaultConstraints", ...
          "setDefaultConstraints", "setEquality", "setEquality", ...
          "setEquality", "setGroupRatio", "setGroupRatio", "setGroupRatio", ...
          "setGroups", "setGroups", "setGroups", "setInequality", ...
          "setInequality", "setInequality", "setInitPort", "setInitPort", ...
          "setInitPort", "setOneWayTurnover", "setOneWayTurnover", ...
          "setOneWayTurnover", "setProbabilityLevel", "setScenarios", ...
          "setScenarios", "setSolver", "setSolver", "setSolver", ...
          "setTurnover", "setTurnover", "setTurnover", "sharpe", ...
          "simulateNormalScenariosByData", "simulateNormalScenariosByData", ...
          "simulateNormalScenariosByMoments", ...
          "simulateNormalScenariosByMoments", "size", "smoothts", "sortfts", ...
          "spctkd", "stochosc", "subsasgn", "subsref", "targetreturn", ...
          "taxedrr", "tbilldisc2yield", "tbillprice", "tbillrepo", ...
          "tbillval01", "tbillyield", "tbillyield2disc", "tbl2bond", ...
          "thirdwednesday", "thirtytwo2dec", "tick2ret", "time2date", ...
          "times", "toannual", "todaily", "today", "todecimal", "tomonthly", ...
          "toquarterly", "toquoted", "tosemi", "totalreturnprice", ...
          "toweekly", "tr2bonds", "transprob", "transprobbytotals", ...
          "transprobfromthresholds", "transprobgrouptotals", ...
          "transprobprep", "transprobtothresholds", "tsaccel", "tsmom", ...
          "tsmovavg", "typprice", "ugarch", "ugarchllf", "ugarchpred", ...
          "ugarchsim", "uicalendar", "uminus", "uplus", "vertcat", ...
          "volarea", "volroc", "wclose", "weeknum", "weights2holdings", ...
          "willad", "willpctr", "wrkdydif", "x2mdate", "xirr", "year", ...
          "yeardays", "yearfrac", "ylddisc", "yldmat", "yldtbill", ...
          "zbtprice", "zbtyield", "zero2disc", "zero2fwd", "zero2pyld"}
      txt = check_package (fcn, "financial");

    ## image processing
    case {"activecontour", "adapthisteq", "affine2d", "affine3d", ...
          "analyze75info", "analyze75read", "applycform", "applylut", ...
          "axes2pix", "bestblk", "blockproc", "bwarea", "bwareaopen", ...
          "bwboundaries", "bwconncomp", "bwconvhull", "bwdist", ...
          "bwdistgeodesic", "bweuler", "bwhitmiss", "bwlabel", "bwlabeln", ...
          "bwlookup", "bwmorph", "bwpack", "bwperim", "bwselect", ...
          "bwtraceboundary", "bwulterode", "bwunpack", "checkerboard", ...
          "col2im", "colfilt", "conndef", "convmtx2", "corner", ...
          "cornermetric", "corr2", "cp2tform", "cpcorr", "cpselect", ...
          "cpstruct2pairs", "dct2", "dctmtx", "deconvblind", "deconvlucy", ...
          "deconvreg", "deconvwnr", "decorrstretch", "demosaic", ...
          "dicomanon", "dicomdict", "dicominfo", "dicomlookup", "dicomread", ...
          "dicomuid", "dicomwrite", "edge", "edgetaper", "entropy", ...
          "entropyfilt", "fan2para", "fanbeam", "findbounds", "fitgeotrans", ...
          "fliptform", "freqz2", "fsamp2", "fspecial", "ftrans2", "fwind1", ...
          "fwind2", "getheight", "getimage", "getimagemodel", "getline", ...
          "getneighbors", "getnhood", "getpts", "getrect", "getsequence", ...
          "graycomatrix", "graycoprops", "graydist", "grayslice", ...
          "graythresh", "hdrread", "hdrwrite", "histeq", "hough", ...
          "houghlines", "houghpeaks", "iccfind", "iccread", "iccroot", ...
          "iccwrite", "idct2", "ifanbeam", "im2bw", "im2col", "im2double", ...
          "im2int16", "im2java2d", "im2single", "im2uint16", "im2uint8", ...
          "imabsdiff", "imadd", "imadjust", "ImageAdapter", "imageinfo", ...
          "imapplymatrix", "imapprox", "imattributes", "imbothat", ...
          "imclearborder", "imclose", "imcolormaptool", "imcomplement", ...
          "imcontour", "imcontrast", "imcrop", "imdilate", "imdisplayrange", ...
          "imdistline", "imdivide", "imellipse", "imerode", "imextendedmax", ...
          "imextendedmin", "imfill", "imfilter", "imfindcircles", ...
          "imfreehand", "imfuse", "imgca", "imgcf", "imgetfile", ...
          "imgradient", "imgradientxy", "imhandles", "imhist", ...
          "imhistmatch", "imhmax", "imhmin", "imimposemin", "imlincomb", ...
          "imline", "immagbox", "immovie", "immultiply", "imnoise", ...
          "imopen", "imoverview", "imoverviewpanel", "impixel", ...
          "impixelinfo", "impixelinfoval", "impixelregion", ...
          "impixelregionpanel", "implay", "impoint", "impoly", "improfile", ...
          "impyramid", "imquantize", "imreconstruct", "imrect", "imref2d", ...
          "imref3d", "imregconfig", "imregionalmax", "imregionalmin", ...
          "imregister", "imregtform", "imresize", "imroi", "imrotate", ...
          "imsave", "imscrollpanel", "imsharpen", "imshowpair", ...
          "imsubtract", "imtool", "imtophat", "imtransform", "imwarp", ...
          "interfileinfo", "interfileread", "intlut", "iptaddcallback", ...
          "iptcheckconn", "iptcheckhandle", "iptgetapi", ...
          "iptGetPointerBehavior", "iptgetpref", "ipticondir", ...
          "iptPointerManager", "iptprefs", "iptremovecallback", ...
          "iptSetPointerBehavior", "iptsetpref", "iptwindowalign", "iradon", ...
          "isflat", "isicc", "isrset", "lab2double", "lab2uint16", ...
          "lab2uint8", "label2rgb", "labelmatrix", "makecform", ...
          "makeConstrainToRectFcn", "makehdr", "makelut", "makeresampler", ...
          "maketform", "mat2gray", "mean2", "medfilt2", "montage", ...
          "multithresh", "nitfinfo", "nitfread", "nlfilter", "normxcorr2", ...
          "openrset", "ordfilt2", "otf2psf", "padarray", "para2fan", ...
          "phantom", "poly2mask", "projective2d", "psf2otf", "qtdecomp", ...
          "qtgetblk", "qtsetblk", "radon", "rangefilt", "reflect", ...
          "regionprops", "rgb2gray", "rgb2ycbcr", "roicolor", "roifill", ...
          "roifilt2", "roipoly", "rsetwrite", "std2", "stdfilt", "strel", ...
          "stretchlim", "subimage", "tformarray", "tformfwd", "tforminv", ...
          "tonemap", "translate", "truesize", "viscircles", "warp", ...
          "watershed", "whitepoint", "wiener2", "xyz2double", "xyz2uint16", ...
          "ycbcr2rgb"}
      txt = check_package (fcn, "image");

    ## signal processing
    case {"ac2poly", "ac2rc", "angle", "arburg", "arcov", "armcov", ...
          "aryule", "bandpower", "barthannwin", "besselap", "besself", ...
          "bilinear", "bitrevorder", "blackmanharris", "bohmanwin", ...
          "buffer", "buttap", "butter", "buttord", "cceps", "cconv", ...
          "cell2sos", "cfirpm", "cheb1ap", "cheb1ord", "cheb2ap", ...
          "cheb2ord", "chebwin", "cheby1", "cheby2", "chirp", "convmtx", ...
          "corrmtx", "cpsd", "czt", "db", "db2mag", "db2pow", "dct", ...
          "decimate", "demod", "design", "designmethods", "designopts", ...
          "dfilt", "dftmtx", "digitrevorder", "diric", "downsample", "dpss", ...
          "dpssclear", "dpssdir", "dpssload", "dspdata", "dspfwiz", ...
          "dutycycle", "ellip", "ellipap", "ellipord", "enbw", "equiripple", ...
          "falltime", "fdatool", "fdesign", "filt2block", "filterbuilder", ...
          "filternorm", "filtfilt", "filtic", "filtord", "findpeaks", ...
          "fir1", "fir2", "fircls", "fircls1", "firls", "firpm", "firpmord", ...
          "firrcos", "firtype", "flattopwin", "freqs", "freqsamp", "fvtool", ...
          "fwht", "gauspuls", "gaussdesign", "gaussfir", "gausswin", ...
          "gmonopuls", "goertzel", "grpdelay", "hann", "hilbert", "icceps", ...
          "idct", "ifwht", "impinvar", "impz", "impzlength", "interp", ...
          "intfilt", "invfreqs", "invfreqz", "is2rc", "isallpass", ...
          "islinphase", "ismaxphase", "isminphase", "isstable", "kaiser", ...
          "kaiserord", "kaiserwin", "lar2rc", "latc2tf", "latcfilt", ...
          "levinson", "lp2bp", "lp2bs", "lp2hp", "lp2lp", "lpc", "lsf2poly", ...
          "mag2db", "marcumq", "maxflat", "medfilt1", "midcross", ...
          "modulate", "mscohere", "nuttallwin", "overshoot", "parzenwin", ...
          "pburg", "pcov", "peak2peak", "peak2rms", "peig", "phasedelay", ...
          "phasez", "pmcov", "pmtm", "pmusic", "poly2ac", "poly2lsf", ...
          "poly2rc", "polyscale", "polystab", "pow2db", "prony", ...
          "pulseperiod", "pulsesep", "pulsewidth", "pulstran", "pwelch", ...
          "pyulear", "rc2ac", "rc2is", "rc2lar", "rc2poly", "rceps", ...
          "rcosdesign", "realizemdl", "rectpuls", "rectwin", "resample", ...
          "residuez", "risetime", "rlevinson", "rms", "rooteig", ...
          "rootmusic", "rssq", "sawtooth", "schurrc", "seqperiod", ...
          "setspecs", "settlingtime", "sfdr", "sgolay", "sgolayfilt", ...
          "shiftdata", "sigwin", "sinad", "slewrate", "snr", "sos2cell", ...
          "sos2ss", "sos2tf", "sos2zp", "sosfilt", "spectrogram", ...
          "spectrum", "sptool", "square", "ss2sos", "ss2tf", "ss2zp", ...
          "statelevels", "stepz", "stmcb", "strips", "taylorwin", "tf2latc", ...
          "tf2sos", "tf2ss", "tf2zp", "tf2zpk", "tfestimate", "thd", "toi", ...
          "triang", "tripuls", "tukeywin", "udecode", "uencode", ...
          "undershoot", "unshiftdata", "upfirdn", "upsample", ...
          "validstructures", "vco", "window", "wintool", "wvtool", "xcorr", ...
          "xcorr2", "xcov", "yulewalk", "zerophase", "zp2sos", "zp2ss", ...
          "zp2tf", "zplane"}
      txt = check_package (fcn, "signal");

    ## statistics
    case {"addedvarplot", "addlevels", "addTerms", "addTerms", "adtest", ...
          "andrewsplot", "anova1", "anova2", "anovan", "ansaribradley", ...
          "aoctool", "barttest", "bbdesign", "betafit", "betalike", ...
          "betastat", "binofit", "binostat", "biplot", "bootci", "bootstrp", ...
          "boxplot", "candexch", "candgen", "canoncorr", "capability", ...
          "capaplot", "caseread", "casewrite", "ccdesign", "cdf", "cdf", ...
          "cdfplot", "cell2dataset", "chi2gof", "chi2stat", "cholcov", ...
          "ClassificationBaggedEnsemble", "ClassificationDiscriminant", ...
          "ClassificationEnsemble", "ClassificationKNN", ...
          "ClassificationPartitionedEnsemble", ...
          "ClassificationPartitionedModel", "ClassificationTree", ...
          "classify", "classregtree", "cluster", "clusterdata", "cmdscale", ...
          "coefCI", "coefCI", "coefCI", "coefCI", "coefTest", "coefTest", ...
          "coefTest", "coefTest", "combnk", "compact", ...
          "CompactClassificationDiscriminant", ...
          "CompactClassificationEnsemble", "CompactClassificationTree", ...
          "CompactRegressionEnsemble", "CompactRegressionTree", ...
          "CompactTreeBagger", "compare", "confusionmat", "controlchart", ...
          "controlrules", "cophenet", "copulacdf", "copulafit", ...
          "copulaparam", "copulapdf", "copularnd", "copulastat", "cordexch", ...
          "corrcov", "covarianceParameters", "coxphfit", "createns", ...
          "crosstab", "crossval", "cvpartition", "datasample", "dataset", ...
          "dataset2cell", "dataset2struct", "dataset2table", "datasetfun", ...
          "daugment", "dcovary", "dendrogram", "designMatrix", ...
          "devianceTest", "dfittool", "disp", "disp", "disp", "disp", ...
          "disttool", "droplevels", "dummyvar", "dwtest", "dwtest", "ecdf", ...
          "ecdfhist", "evalclusters", "evcdf", "evfit", "evinv", "evlike", ...
          "evpdf", "evrnd", "evstat", "ExhaustiveSearcher", "expfit", ...
          "explike", "export", "expstat", "factoran", "feval", "feval", ...
          "feval", "ff2n", "fitdist", "fitensemble", "fitglm", "fitlm", ...
          "fitlme", "fitlmematrix", "fitnlm", "fitted", "fixedEffects", ...
          "fracfact", "fracfactgen", "friedman", "fsurfht", "fullfact", ...
          "gagerr", "gamfit", "gamlike", "gamstat", ...
          "GeneralizedLinearModel", "geomean", "geostat", "getlabels", ...
          "getlevels", "gevcdf", "gevfit", "gevinv", "gevlike", "gevpdf", ...
          "gevrnd", "gevstat", "gline", "glmfit", "glmval", "glyphplot", ...
          "gmdistribution", "gname", "gpcdf", "gpfit", "gpinv", "gplike", ...
          "gplotmatrix", "gppdf", "gprnd", "gpstat", "grp2idx", "grpstats", ...
          "gscatter", "haltonset", "harmmean", "hist3", "histfit", ...
          "hmmdecode", "hmmestimate", "hmmgenerate", "hmmtrain", ...
          "hmmviterbi", "hougen", "hygestat", "icdf", "icdf", ...
          "inconsistent", "interactionplot", "invpred", "islevel", ...
          "ismissing", "isundefined", "iwishrnd", "jackknife", "jbtest", ...
          "johnsrnd", "join", "KDTreeSearcher", "kmeans", "knnsearch", ...
          "kruskalwallis", "ksdensity", "kstest", "kstest2", "labels", ...
          "lasso", "lassoglm", "lassoPlot", "levelcounts", "leverage", ...
          "lhsdesign", "lhsnorm", "lillietest", "LinearMixedModel", ...
          "LinearModel", "linhyptest", "linkage", "lognfit", "lognlike", ...
          "lognstat", "lsline", "mad", "mahal", "maineffectsplot", ...
          "makedist", "manova1", "manovacluster", "mat2dataset", "mdscale", ...
          "mergelevels", "mhsample", "mle", "mlecov", "mnpdf", "mnrfit", ...
          "mnrnd", "mnrval", "multcompare", "multivarichart", "mvncdf", ...
          "mvnpdf", "mvnrnd", "mvregress", "mvregresslike", "mvtcdf", ...
          "mvtpdf", "mvtrnd", "NaiveBayes", "nancov", "nanmax", "nanmean", ...
          "nanmedian", "nanmin", "nanstd", "nansum", "nanvar", "nbinfit", ...
          "nbinstat", "ncfcdf", "ncfinv", "ncfpdf", "ncfrnd", "ncfstat", ...
          "nctcdf", "nctinv", "nctpdf", "nctrnd", "nctstat", "ncx2cdf", ...
          "ncx2inv", "ncx2pdf", "ncx2rnd", "ncx2stat", "negloglik", ...
          "negloglik", "nlinfit", "nlintool", "nlmefit", "nlmefitsa", ...
          "nlparci", "nlpredci", "nnmf", "nominal", "NonLinearModel", ...
          "normfit", "normlike", "normplot", "normspec", "normstat", ...
          "optimalleaforder", "ordinal", "parallelcoords", "paramci", ...
          "paretotails", "partialcorr", "partialcorri", "pca", "pcacov", ...
          "pcares", "pdf", "pdf", "pdist", "pdist2", "pearsrnd", ...
          "perfcurve", "plotAdded", "plotAdjustedResponse", ...
          "plotDiagnostics", "plotDiagnostics", "plotDiagnostics", ...
          "plotEffects", "plotInteraction", "plotResiduals", ...
          "plotResiduals", "plotResiduals", "plotResiduals", "plotSlice", ...
          "plotSlice", "plotSlice", "plsregress", "poissfit", "poisstat", ...
          "polyconf", "polytool", "ppca", "predict", "predict", "predict", ...
          "predict", "predict", "predict", "predict", "predict", "princomp", ...
          "ProbDistUnivKernel", "ProbDistUnivParam", "probplot", ...
          "procrustes", "proflik", "qrandset", "qrandstream", "random", ...
          "random", "random", "random", "random", "random", "randomEffects", ...
          "randsample", "randtool", "rangesearch", "ranksum", "raylcdf", ...
          "raylfit", "raylinv", "raylpdf", "raylrnd", "raylstat", "rcoplot", ...
          "refcurve", "refline", "regress", "RegressionBaggedEnsemble", ...
          "RegressionEnsemble", "RegressionPartitionedEnsemble", ...
          "RegressionPartitionedModel", "RegressionTree", "regstats", ...
          "relieff", "removeTerms", "removeTerms", "residuals", "response", ...
          "ridge", "robustdemo", "robustfit", "rotatefactors", "rowexch", ...
          "rsmdemo", "rstool", "runstest", "sampsizepwr", "scatterhist", ...
          "sequentialfs", "setlabels", "signrank", "signtest", "silhouette", ...
          "slicesample", "sobolset", "squareform", "statget", "statset", ...
          "step", "step", "stepwise", "stepwisefit", "stepwiseglm", ...
          "stepwiselm", "struct2dataset", "surfht", "svmclassify", ...
          "svmtrain", "table2dataset", "tabulate", "tblread", "tblwrite", ...
          "tdfread", "tiedrank", "TreeBagger", "trimmean", "truncate", ...
          "tstat", "ttest", "ttest2", "unidstat", "unifit", "unifstat", ...
          "vartest", "vartest2", "vartestn", "wblfit", "wbllike", "wblplot", ...
          "wblstat", "wishrnd", "x2fx", "xlsread", "xptread", "ztest"}
      txt = check_package (fcn, "statistics");

    ## optimization
    case {"bintprog", "color", "fgoalattain", "fmincon", "fminimax", ...
          "fminsearch", "fseminf", "fzmult", "gangstr", "ktrlink", ...
          "linprog", "lsqcurvefit", "lsqlin", "lsqnonlin", "optimoptions", ...
          "optimtool", "quadprog"}
      txt = check_package (fcn, "optim");

    otherwise
      if (ismember (fcn, missing_functions ()))
        txt = ["the '" fcn "' function is not yet implemented in Octave"];
      else
        is_matlab_function = false;
        txt = "";
      endif
  endswitch

  if (is_matlab_function)
    txt = [txt, "\n\n@noindent\nPlease read ", ...
           "@url{http://www.octave.org/missing.html} to learn how ", ...
           "you can contribute missing functionality."];
    txt = __makeinfo__ (txt);
  endif

  if (nargout == 0)
    warning ("Octave:missing-function", "%s", txt);
  endif

endfunction

function txt = check_package (fcn, name)
  txt = sprintf ("the '%s' function belongs to the %s package from Octave Forge",
                 fcn, name);

  [~, status] = pkg ("describe", name);
  switch (tolower (status{1}))
    case "loaded",
      txt = sprintf ("%s but has not yet been implemented.", txt);
    case "not loaded",
      txt = sprintf (["%s which you have installed but not loaded. To ", ...
                      "load the package, run `pkg load %s' from the ", ...
                      "Octave prompt."], txt, name);
    otherwise
      ## this includes "not installed" and anything else if pkg changes
      ## the output of describe
      txt = sprintf ("%s which seems to not be installed in your system.", txt);
  endswitch
endfunction

function list = missing_functions ()
  persistent list = {
  "MException",
  "RandStream",
  "Tiff",
  "VideoReader",
  "VideoWriter",
  "align",
  "alim",
  "alpha",
  "alphamap",
  "annotation",
  "audiodevinfo",
  "audioinfo",
  "audioplayer",
  "audioread",
  "audiorecorder",
  "audiowrite",
  "bar3",
  "bar3h",
  "bench",
  "bicgstabl",
  "brush",
  "builddocsearchdb",
  "bvp4c",
  "bvp5c",
  "bvpget",
  "bvpinit",
  "bvpset",
  "bvpxtend",
  "callSoapService",
  "calllib",
  "camdolly",
  "cameratoolbar",
  "camlight",
  "camlookat",
  "camorbit",
  "campan",
  "campos",
  "camproj",
  "camroll",
  "camtarget",
  "camup",
  "camva",
  "camzoom",
  "cdf2rdf",
  "cdfepoch",
  "cdfinfo",
  "cdfread",
  "cdfwrite",
  "cellplot",
  "checkin",
  "checkcode",
  "checkout",
  "cholinc",
  "clearvars",
  "clipboard",
  "cmopts",
  "colordef",
  "colormapeditor",
  "commandhistory",
  "commandwindow",
  "condeig",
  "coneplot",
  "containers.Map",
  "contourslice",
  "createClassFromWsdl",
  "createSoapMessage",
  "customverctrl",
  "datacursormode",
  "dbmex",
  "dde23",
  "ddeget",
  "ddensd",
  "ddesd",
  "ddeset",
  "decic",
  "delaunayTriangulation",
  "depdir",
  "depfun",
  "deval",
  "dialog",
  "dither",
  "docsearch",
  "dragrect",
  "dynamicprops",
  "echodemo",
  "evalc",
  "export2wsdlg",
  "figurepalette",
  "filebrowser",
  "fill3",
  "fitsdisp",
  "fitsinfo",
  "fitsread",
  "fitswrite",
  "flow",
  "frame2im",
  "freqspace",
  "funm",
  "gammaincinv",
  "getframe",
  "getpixelposition",
  "gobjects",
  "grabcode",
  "graymon",
  "griddedInterpolant",
  "gsvd",
  "guidata",
  "guide",
  "guihandles",
  "handle",
  "h5create",
  "h5disp",
  "h5info",
  "h5read",
  "h5readatt",
  "h5write",
  "h5writeatt",
  "hdfinfo",
  "hdfread",
  "hgexport",
  "hgload",
  "hgsave",
  "hgsetget",
  "hgtransform",
  "ichol",
  "ilu",
  "im2frame",
  "im2java",
  "imapprox",
  "import",
  "inmem",
  "inputParser",
  "inspect",
  "instrcallback",
  "instrfind",
  "instrfindall",
  "integral",
  "integral2",
  "integral3",
  "interpstreamspeed",
  "iscom",
  "isinterface",
  "isjava",
  "isocaps",
  "isstudent",
  "javachk",
  "ldl",
  "libfunctions",
  "libfunctionsview",
  "libisloaded",
  "libpointer",
  "libstruct",
  "light",
  "lightangle",
  "lighting",
  "linkaxes",
  "linkdata",
  "listfonts",
  "loadlibrary",
  "lscov",
  "lsqr",
  "makehgtform",
  "material",
  "matfile",
  "matlabrc",
  "memmapfile",
  "memory",
  "metaclass",
  "methodsview",
  "minres",
  "mlintrpt",
  "mmfileinfo",
  "movegui",
  "movie",
  "movie2avi",
  "multibandread",
  "multibandwrite",
  "native2unicode",
  "nccreate",
  "ncdisp",
  "ncinfo",
  "ncread",
  "ncreadatt",
  "ncwrite",
  "ncwriteatt",
  "ncwriteschema",
  "noanimate",
  "notebook",
  "ode113",
  "ode15i",
  "ode15s",
  "ode23",
  "ode23s",
  "ode23t",
  "ode23tb",
  "ode45",
  "odeget",
  "odeset",
  "odextend",
  "open",
  "openfig",
  "opengl",
  "openvar",
  "ordeig",
  "ordqz",
  "ordschur",
  "padecoef",
  "pan",
  "parseSoapResponse",
  "pathtool",
  "pcode",
  "pdepe",
  "pdeval",
  "plotbrowser",
  "plotedit",
  "plottools",
  "printdlg",
  "printopt",
  "printpreview",
  "profsave",
  "propedit",
  "propertyeditor",
  "psi",
  "publish",
  "qmr",
  "quad2d",
  "rbbox",
  "reducepatch",
  "reducevolume",
  "readasync",
  "rng",
  "rotate",
  "rotate3d",
  "scatteredInterpolant",
  "selectmoveresize",
  "sendmail",
  "serial",
  "serialbreak",
  "setpixelposition",
  "showplottool",
  "smooth3",
  "snapnow",
  "sound",
  "soundsc",
  "ss2tf",
  "startup",
  "stopasync",
  "stream2",
  "stream3",
  "streamline",
  "streamparticles",
  "streamribbon",
  "streamslice",
  "streamtube",
  "strings",
  "subvolume",
  "superclasses",
  "surf2patch",
  "symmlq",
  "syntax",
  "texlabel",
  "textwrap",
  "tfqmr",
  "timer",
  "timeseries",
  "todatenum",
  "toolboxdir",
  "triangulation",
  "tscollection",
  "tstool",
  "uibuttongroup",
  "uicontextmenu",
  "uicontrol",
  "uigetpref",
  "uiimport",
  "uiopen",
  "uipanel",
  "uipushtool",
  "uiresume",
  "uisave",
  "uisetcolor",
  "uisetfont",
  "uisetpref",
  "uistack",
  "uitable",
  "uitoggletool",
  "uitoolbar",
  "uiwait",
  "undocheckout",
  "unicode2native",
  "unloadlibrary",
  "unmesh",
  "userpath",
  "validateattributes",
  "verctrl",
  "verLessThan",
  "viewmtx",
  "visdiff",
  "volumebounds",
  "web",
  "whatsnew",
  "winopen",
  "winqueryreg",
  "workspace",
  "xmlread",
  "xmlwrite",
  "xslt",
  "zoom",
  };
endfunction


%!test
%! str = __unimplemented__ ("no_name_function");
%! assert (isempty (str));
%! str = __unimplemented__ ("quad2d");
%! assert (str(1:51), "quad2d is not implemented.  Consider using dblquad.");
%! str = __unimplemented__ ("MException");
%! assert (str(1:58), "the 'MException' function is not yet implemented in Octave");

