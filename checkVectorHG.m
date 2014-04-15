% checkVectorHG(fn)
%
% Checks Henyey-Greenstein output for errors
%
% Text output:
%   Non-unit vectors
%   <cos(theta)> which should equal g
%
%
% 5 Columns of input:
%   random number for HG [-1,1)
%   cos(theta)  deflection angle
%   sin(theta)
%   cos(phi)    azimuthal angle
%   sin(phi)

function checkVectorHG(fn)

T = loadVectN(fn,5)';

figure; subplot(2,2,1);
hist(T(:,1)); title('Random number distribution (should be -1,1)');

subplot(2,2,2);
hist(T(:,2),100); title('HG cosine distribution');

subplot(2,2,3);
hist(atan2(T(:,4),T(:,5)),100); title('Azimuthal angle distribution');

subplot(2,2,4);
hist(T(:,2).^2+T(:,3).^2,100); title('Deflection UV length');

printf('<cos(theta)> = %f\n',mean(T(:,2)));

eps = 1e-5;
errDefl = abs(sum(T(:,2:3)'.^2)-1) > eps;
errAz   = abs(sum(T(:,4:5)'.^2)-1) > eps;

printf('Tolerance errors at eps=%f\n  Deflection: %d/%d\n  Azimuth: %d/%d\n',eps,nnz(errDefl),length(errDefl),nnz(errAz),length(errAz));
