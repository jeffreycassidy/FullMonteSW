% recall region codes are offset by 1 when loaded into Matlab to accommodate region 0
region=18;

fid = fopen('../output/linesource_dvh100/dvh3197.regions.txt','r');
fgetl(fid);
fgetl(fid);
T = fscanf(fid,'%d %f\n',[2 Inf])';
fclose(fid);

regions=T(:,1);
vols=T(:,2);

idx = find(regions==region);

nnz = length(idx);

phi_v = zeros(nnz,100);

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


V = vols(idx);

[Ne,Nr] = size(phi_v);

printf('Loaded data for region $d (%d elements, %d runs)\n',region,Ne,Nr);
