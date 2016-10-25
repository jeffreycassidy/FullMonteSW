function P=loadDense(fid)

l = skipComments(fid);

[T,count] = sscanf(l,'%d %d',[1 2]);

if (count != 2)
	error(['Invalid dimension specifier, expection <N> <dim>, got ''' l '''']);
end

N = T(1);
dim = T(2);

[T,count] = fscanf(fid,'%f',[dim N]);

P=T';

printf('Read %d entries from a %dx%d dense matrix',count,dim,N);
