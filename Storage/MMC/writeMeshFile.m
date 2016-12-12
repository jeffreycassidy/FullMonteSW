function writeMeshFile(fn,P,T)

fid = fopen(fn,'w');

[Np,Dp] = size(P);
if (Dp != 3)
    error('Incorrect dimensions for P array: should be Np x 3')
end

[Nt,Dt] = size(T);
if (Dt != 5)
    error('Incorrect dimensions for T array: should be Nt x 5')
end

fprintf(fid,'%d\n',Np);
fprintf(fid,'%d\n',Nt);

fprintf(fid,'%10.6f %10.6f %10.6f\n',P');
fprintf(fid,'%8d %8d %8d %8d %2d\n',T');

fclose(fid);
