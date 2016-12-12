% Load an MMC mesh, returning the points (P), tetras (T), and faces (F)
% Format is described at mcx.sourceforge.net

function [P,T,F] = loadMMCMesh(fn)

load(fn)

% Point array: { x y z } Np
P = node;
[Np,Dp] = size(P);
if (Dp != 3)
    error('Invalid dimensions on node array: expecting Np x 3');
end

% Tetra array: {IDp0 IDp1 IDp2 IDp3 IDm}Nt
T = elem;
[Nt,Dt] = size(T);
if (Dt != 5)
    error('Invalid dimensions on elem array: expecting Nt x 5');
end

% Surface face array: {IDp0 IDp1 IDp2 IDsurf}Nf
F = face;
[Nf,Df] = size(F);
if (Df != 4)
    error('Invalid dimensions on face array: expecting Nf x 4');
end

clear node elem face
