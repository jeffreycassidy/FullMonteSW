% Creates a set of DVHs from a set of region elements
%
%
% Arguments
%   regions     Region codes for each fluence element
%   phi_v       Fluence elements
%   vols        Volumes for each element
%
% Returns
%   dvhArray    An array of DVHs (col1 = fluence, col2 = %vol)

function makeMultiDVH(regions,phi_v,vols,labels)

printf('creating multiDVH');

for r=1:max(regions)
    idx = find(regions==r);

    if (length(idx) != 0)
        [dvhArray{r}(:,1),dvhArray{r}(:,2)] = makeDVH(phi_v(idx),vols(idx));
        if (nargin >3)
            label = labels{r};
        else
            label = '?';
        end
        printf('  Region %d (%s): %d elements\n',r,label,length(dvhArray{r}(:,1)));
    else
        dvhArray{r} = zeros(0,2);
    end
end
