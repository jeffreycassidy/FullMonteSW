function T = loadSparse(fid)

l = skipComments(fid);

if (!feof(fid) && l(1) != '#')
	[T,count] = sscanf(l,'%d/%d %d',[3 1]);
	if (count != 3)
		error(['Invalid sparse dimension specifier, expecting <nnz>/<N> <dim>, got ''' l ''''])
	end
end

nnz=T(1);
N=T(2);
D=T(3);

[tmp,count] = fscanf(fid,'%f',[D nnz]);

T = tmp(2:4,:)';

printf('Read sparse matrix with nnz=%d N=%d and %d data columns\n',nnz,N,D);
