function [P,T] = loadMeshFile(fn)

fid = fopen(fn,'r');

P = loadDense(fid);
T = loadSparse(fid);

fclose(fid);
