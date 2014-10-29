% v = loadTextMatrix(fn)
%
% Strips leading comments (lines whose first character is '%' or '#')
%
%
% Arguments
%   fn      File name
%
% Returns
%   M       Matrix read from file

function v = loadTextMatrix(fn)

fid=fopen(fn,'r');
%comm = stripComments(fid);
fgetl(fid);
fgetl(fid);
[T,count] = fscanf(fid,'%d %d\n',[2 1]);

if (count != 2)
    error('Failed to read matrix size');
end

R=T(1);
C=T(2);

printf('Reading %dx%d matrix\n',R,C);
[v,count] = fscanf(fid,'%f %f\n',[C R]);
fclose(fid);

v=v';

if(size(v) != [R,C])
    error(sprintf('Incorrect number of values loaded from file %s: %d read, expecting %dx%d\n',fn,count,R,C));
end
