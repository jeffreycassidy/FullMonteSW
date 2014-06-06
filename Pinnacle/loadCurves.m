% Load a text representation of a curve
%
% Arguments
%   fn          Filename
%
% Returns
%   P           Point vector (all points concatenated)
%   C           Curves (Nc x 1 cell array of curves, each curve Np x 3)
%   z           z-coordinate of first point of each curve


function [P,C,z] = loadCurves(fn);

fid = fopen(fn,'r');
[comments,commentlines] = fscanf(fid,'#%[^\n]\n',Inf);

printf('Read %d comment lines\n',commentlines);

[Nc,N] = fscanf(fid,'%d\n',1);
if (N != 1)
    error("Failed to read number of lines");
else
    printf('Reading %d curves\n',Nc);
end

[Np,N] = fscanf(fid,'%d',Nc);
if (N != Nc)
    error('Failed to read correct number of curve lengths\n');
else
    printf('  Total points: %d\n',sum(Np));
end

[P,N] = fscanf(fid,'%f %f %f\n',[3 sum(Np)]);
P=P';
if (N != sum(Np)*3)
    error(sprintf('Failed to read correct number of points (%d, expecting %d)',N/3,sum(Np)));
else
    printf('  Read OK\n');
end

fclose(fid);

Np_cs = cumsum([1;Np]);

figure; hold on;

for i=1:length(Np_cs)-1
%for i=2:2
    z(i) = P(Np_cs(i),3);
    C{i} = P(Np_cs(i):Np_cs(i+1)-1,:);

    x = C{i}(:,1);
    y = C{i}(:,2);
    z = C{i}(:,3);

%    x = P(Np_cs(i):(Np_cs(i+1)-1),1);
%    y = P(Np_cs(i):(Np_cs(i+1)-1),2);
%    z = P(Np_cs(i):(Np_cs(i+1)-1),3);
    plot3([x;x(1)],[y;y(1)],[z;z(1)],'-');
end
