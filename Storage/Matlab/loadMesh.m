function [P,T] = loadMesh(fn)

fid = fopen(fn);

printf('================================================================================ File header\n');

N=1;
while (N==1)
    [comm,N] = fscanf(fid,'%%%[^\n]%[\n]',Inf);
    printf('%s\n',comm);
end

printf('================================================================================\n\n');

t = fscanf(fid,'%d',[1 2]);
Np = t(1);
Nd = t(2);

printf('Reading %d %d-D points\n',Np,Nd);

P = fscanf(fid,'%f',[Nd Np])';

t = fscanf(fid,'%d',[1 2]);
Nf = t(1);
Ns = t(2);

printf('Reading %d %d-tuple mesh elements\n',Nf,Ns);

T = fscanf(fid,'%d',[Ns Nf])';

fclose(fid);


