function [regions,V,idx] = loadRegions(fn);

T = loadTextMatrix(fn);

[Ne,Nc] = size(T);

printf('read region codes for %d elements from %s\n',Ne,fn);

regions=T(:,1);
vols=T(:,2);

idx = find(regions > 0);
V = vols(idx);
regions = regions(idx);
nnz = length(idx);

printf('Mesh statistics: Total %d elements, %f volume; DVH area %d elements, %f volume\n',Ne-1,sum(vols),nnz,sum(V));

for i=1:max(regions)
    idxt = find(regions==i);
    Ne = length(idxt);
    if (Ne > 0)
        printf('  Region %d: %d elements, %f volume\n',i,length(idxt),sum(V(idxt)));
    end
end
