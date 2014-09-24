% loadDVH.m
%
% load a dose-volume histogram from a text file produced by (among other things) DVH.cpp
%
% D=loadDVH(fn)
%
%
% Args
%   fn      File name
%
% Returns
%   D       Nr x 1 Cell array with DVHs for each region; each element is Ne x 2 (fluence, volume%)
%
% NOTE: stairs() function extends points rightwards until the next x value
% DVH.hpp continues with that convention

function D = loadDVH(fn,opts)

if (nargin>1)
    with_var=1;
    Nc=3;
else
    with_var=0;
    Nc=2;
end

fid = fopen(fn,'r');

fscanf(fid,'#%[^\n]\n',Inf);

Nr = fscanf(fid,'%d\n',1);
N  = sscanf(fgetl(fid),'%d\n',Nr);

printf('Reading DVH from %s: %d regions with total %d elements\n',fn,Nr,sum(N));

if (Nc==3)
    T = fscanf(fid,'%f %f %f\n',[3 Inf])';
else
    T = fscanf(fid,'%f %f\n',[2 Inf])';
end

totals = cumsum(N)';

start_idx = [1 totals(1:end-1)+1];
end_idx   = totals;

header=zeros(1,Nc);
header(1,2)=1;

for i=0:Nr-1
    D{i+1} = [header;T(start_idx(i+1):end_idx(i+1),:)];
end

fclose(fid);
