% script to test the output of results2matlab

load output.mat
whos

sum_sparse = sum(sparse_values);
sum_dense  = sum(dense);
sum_diff   = 100*(sum_sparse/sum_dense-1);
printf('Sums: sparse %f dense %f, difference %f%% ',sum_sparse,sum_dense,sum_diff);

if (abs(sum_diff)<0.1)
    printf('OK\n');
else
    printf(' **ERROR**\n');
end

i = find(dense);
nnz_sparse = length(sparse_indices);
nnz_dense  = length(i);

printf('NNZ: sparse %d dense %d ',nnz_sparse,nnz_dense);

if (nnz_sparse==nnz_dense)
    printf('OK\n');
else
    printf(' **ERROR**\n');
end

nid = length(find(sparse_indices+1 ~= find(dense)));

printf('%d index discrepancies ',nid);
if(nid == 0)
    printf('OK\n');
else
    printf(' **ERROR**\n');
end


nvd = find(abs(dense(sparse_indices+1)-sparse_values)>1e-6);

printf('%d value discrepancies greater than 1e-6 ',length(nvd));

if(length(nvd) == 0)
    printf('OK\n');
else
    printf(' **ERROR**\n');
end
