tumour_regions = loadTextMatrix(fn_regions);

[Ne,Nc] = size(tumour_regions);

printf('read region codes for %d elements\n',Ne);

regions=tumour_regions(:,1);
vols=tumour_regions(:,2);

%if(length(regions) != N)
%    error(sprintf('ERROR: Wrong number of regions (expected %d, read %d)',N,length(regions)));
%end

% Squeeze out values not relevant to the current ROI
idx = find(regions);
regions = regions(idx);
nnz = length(idx);

phi_v = zeros(nnz,length(runs));
V = vols(idx);

clear vols

for i=1:length(runs)
    printf('Reading data from run %d\n',runs(i));

    fflush(1);

    fn = sprintf(fn_fmt,runs(i));
    fid = fopen(fn,'r');

    T = fscanf(fid,'%f\n',[1 Inf])';

    phi_v(:,i) = T(idx);

    fclose(fid);

end


for i=1:max(regions)
    Ne = length(find(regions==i));
    if (Ne > 0)
        printf('  Region %d: %d elements\n',i,length(find(regions==i)));
    end
end



[Ne,Nr] = size(phi_v);

printf('Loaded data: %d elements, %d runs\n',Ne,Nr);

clear nnz N Ne Nr T ans fid i idx runs Nc fn_fmt fn_regions
