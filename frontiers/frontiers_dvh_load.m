region=1;

fid = fopen('../output/linesource_dvh100/dvh3197.regions.txt','r');
fgetl(fid);
fgetl(fid);
N = fscanf(fid,'%d',1);
T = fscanf(fid,'%d %f\n',[2 Inf])';
fclose(fid);

printf('reading region codes for %d elements\n',N);

regions=T(:,1);
vols=T(:,2);

if(length(regions) != N)
    error(sprintf('ERROR: Wrong number of regions (expected %d, read %d)',N,length(regions)));
end

idx = find(regions);

regions = regions(idx);
nnz = length(idx);

phi_v = zeros(nnz,100);
V = vols(idx);

clear vols

runs = 3098:3197;

for i=1:length(runs)
    printf('Reading data from run %d\n',runs(i));

    fflush(1);

    fn = sprintf('../output/linesource_dvh100/flt589.%d.phi_v.txt',runs(i));
    fid = fopen(fn,'r');

    T = fscanf(fid,'%f\n',[1 Inf])';

    phi_v(:,i) = T(idx);

    fclose(fid);

end



[Ne,Nr] = size(phi_v);

printf('Loaded data for region %d (%d elements, %d runs)\n',region,Ne,Nr);

clear nnz N Ne Nr T ans fid i idx region runs
