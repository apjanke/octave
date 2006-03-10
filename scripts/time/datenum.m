## -*- texinfo -*-
## @deftypefn {Function File} {} datenum(Y, M, D [, h , m [, s]])
## @deftypefnx {Function File} {} datenum('date' [, P])
## Returns the specified local time as a day number, with Jan 1, 0000
## being day 1. By this reckoning, Jan 1, 1970 is day number 719529.  
## The fractional portion, corresponds to the portion of the specified day.
##
## Years can be negative and/or fractional.
## Months below 1 are considered to be January.
## Days of the month start at 1.
## Days beyond the end of the month go into subsequent months.
## Days before the beginning of the month go to the previous month.
## Days can be fractional.
##
## WARNING: this function does not attempt to handle Julian
## calendars so dates before Octave 15, 1582 are wrong by as much
## as eleven days.  Also be aware that only Roman Catholic countries
## adopted the calendar in 1582.  It took until 1924 for it to be 
## adopted everywhere.  See the Wikipedia entry on the Gregorian 
## calendar for more details.
##
## WARNING: leap seconds are ignored.  A table of leap seconds
## is available on the Wikipedia entry for leap seconds.
## @seealso{date, clock, now, datestr, datevec, calendar, weekday}
## @end deftypefn

## Algorithm: Peter Baum (http://vsg.cape.com/~pbaum/date/date0.htm)
## Author: Paul Kienzle
## This program is granted to the public domain.

function [days,secs] = datenum(Y,M,D,h,m,s)
  ## Days until start of month assuming year starts March 1.
  persistent monthstart = [306;337;0;31;61;92;122;153;184;214;245;275];

  if nargin == 0 || (nargin > 2  && ischar(Y)) || nargin > 6
    usage("n=datenum('date' [, P]) or n=datenum(Y, M, D [, h, m [, s]])");
  endif
  if ischar(Y)
    if nargin < 2, M=[]; endif
    error('string form of dates not yet supported');
    ## [Y,M,D,h,m,s] = datevec(Y,M);
  else
    if nargin < 6, s = 0; endif
    if nargin < 5, m = 0; endif
    if nargin < 4, h = 0; endif
    if nargin == 1
      nc = columns(Y);
      if nc > 6 || nc < 3,
        error("expected date vector containing [Y,M,D,h,m,s]");
      endif
      s=m=h = 0;
      if nc >= 6, s = Y(:,6); endif
      if nc >= 5, m = Y(:,5); endif
      if nc >= 4, h = Y(:,4); endif
      D = Y(:,3);
      M = Y(:,2);
      Y = Y(:,1);
    endif 
  endif

  M(M<1) = 1; ## For compatibility.  Otherwise allow negative months.

  ## Set start of year to March by moving Jan. and Feb. to previous year.
  ## Correct for months > 12 by moving to subsequent years.
  Y += fix((M-14)/12);

  ## Lookup number of days since start of the current year.
  D += monthstart (mod (M-1,12) + 1) + 60;

  ## Add number of days to the start of the current year. Correct
  ## for leap year every 4 years except centuries not divisible by 400.
  D += 365*Y+floor(Y/4)-floor(Y/100)+floor(Y/400);

  ## Add fraction representing current second of the day.
  days = D + (h+(m+s/60)/60)/24;

  ## Output seconds if asked so that etime can be more accurate
  secs = 86400*D + h*3600 + m*60 + s;

endfunction

%!shared part
%! part = 0.514623842592593;
%!assert(datenum(2001,5,19), 730990)
%!assert(datenum([1417,6,12]), 517712)
%!assert(datenum([2001,5,19;1417,6,12]), [730990;517712])
%!assert(datenum(2001,5,19,12,21,3.5), 730990+part, eps)
%!assert(datenum([1417,6,12,12,21,3.5]), 517712+part, eps)
%!test
%! t = [2001,5,19,12,21,3.5; 1417,6,12,12,21,3.5];
%! n = [730990; 517712] + part;
%! assert(datenum(t), n, 2*eps);
