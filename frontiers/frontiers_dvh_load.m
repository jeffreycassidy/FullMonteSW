% Load a set of fluence (or energy values)
%
% Arguments
%   runs        The run IDs to load
%   fnfmt       Format of file name; must contain a %d somewhere for run IDs
%   idxs        Indices to load (can use to mask out some data)
%
% Returns
%   D           An Ne x Nr matrix of results (rows are elements, columns are runs)

function D = frontiers_dvh_load(runs,fnfmt,idxs);

% Squeeze out values not relevant to the current ROI
if (nargin>2)
    D = zeros(length(idxs),length(runs));
else
    D = zeros(0,length(runs));
end


printf('Reading data: ');

for i=1:length(runs)
    printf('%d ',runs(i));
    fflush(1);

    fn = sprintf(fnfmt,runs(i));
    fid = fopen(fn,'r');

    T = fscanf(fid,'%f\n',[1 Inf])';

    if (nargin>2)
        D(:,i) = T(idxs);
    else
        D(:,i) = T;
    end

    fclose(fid);

end

[Ne,Nr] = size(D);
printf('Loaded data: %d elements, %d runs\n',Ne,Nr);
