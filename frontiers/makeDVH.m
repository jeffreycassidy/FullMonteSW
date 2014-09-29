% Creates a DVH from fluence and volume of elements
%
% Arguments
%   phi_v       Fluence vector
%   V           Volume vector
%
% Returns
%   phi_dvh     Fluence for DVH
%   v_dvh       Volume fraction for DVH
%   idx         Permutation indices


function [phi_dvh,v_dvh,idx] = makeDVH(phi_v,V);

[phi_dvh,idx] = sort(phi_v);
v_dvh = 100*(1-cumsum(V(idx))/sum(V));


