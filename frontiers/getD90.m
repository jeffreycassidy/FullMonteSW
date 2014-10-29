% D90 = getD90(phi,v)
% Returns the D90 for a given DVH
%
% Arguments
%   phi         Fluence vector
%   v           Cumulative volume vector
%
% Returns
%   D90         The minimum dose received by 90% of the organ volume

function D90 = getD90(phi,v);

i90 = find(v<90)(1);
D90 = phi(i90);
