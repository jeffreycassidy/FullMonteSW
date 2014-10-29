% Creates a DVH from fluence and volume of elements
%
% Arguments
%   phi_v       Fluence vector
%   V           Volume vector
%
% Returns
%   phi_dvh     Fluence for DVH
%   v_dvh       Volume % for DVH [0,100]
%   idx         Permutation indices into original vector


function [phi_dvh,v_dvh,idx,D90] = makeDVH(phi_v,V);

[Ne,Nr] = size(phi_v);

if (size(V) != [Ne 1])
    error(sprintf('Invalid size for volume vector, expecting %d x 1, got %d x %d',Ne,size(V)(1),size(V)(2)));
end

phi_dvh = zeros(Ne,Nr);
idx     = phi_dvh;
D90     = zeros(1,Nr);

for i=1:Nr
    [phi_dvh(:,i),idx(:,i)] = sort(phi_v(:,i));
    v_dvh(:,i) = 100*(1-cumsum(V(idx(:,i)))/sum(V(idx(:,i))));
    D90(i) = getD90(phi_dvh(:,i),v_dvh(:,i));
end
