% Verifies the properties of a line source's directional output
% Reads in packet directions, one line per packet:
% dx dy dz ax ay az bx by bz
%
% Verifies the following properties:
%   Orthonormality (d,a,b unit and mutually orthohonal)
%   Uniform distribution of d around fiber axis
%   d orthogonal to fiber axis

fid = fopen('output/linesource.directions.out.txt','r');
T = fscanf(fid,'%f',[9 Inf]);

d = T(1:3,:);
a = T(4:6,:);
b = T(7:9,:);

% have taken the line source orientation to be the b vector
uv = b(:,1);

% generate auxiliary vectors, arbitrarily using cross(uv,i)
n0 = cross(uv,[1 0 0]);
n0 = n0/norm(n0);

n1 = cross(n0,uv);

printf('Aux vectors: %f %f %f\n',n0);
printf('             %f %f %f\n',n1);

subplot(3,3,1);
hist(dot(d,a));
subplot(3,3,2);
hist(dot(d,b));
subplot(3,3,3);
hist(dot(a,b));

subplot(3,3,4);
hist(dot(d,d));
subplot(3,3,5);
hist(dot(a,a));
subplot(3,3,6);
hist(dot(b,b));

subplot(3,3,7);
hist(real(asin(n0'*d)));
title('acos(n0 dot d)');
subplot(3,3,8);
hist(real(asin(n1'*d)));
title('acos(n1 dot d)');
subplot(3,3,9);
