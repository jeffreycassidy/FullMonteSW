% Plots a DVH with error bars on the horizontal axis
%
% plotdvhvar (regions,phi_v,V,region)
%
%   regions         Map of elements to region codes
%   phi_v           Fluence integrated over volume element
%   V               Volume of elements
%   region          Region(s) to plot

function [phi_dvh,v_dvh,sigma_dvh] = plotdvhvar(regions,phi_v,V,region)

idx = find(regions==region);

mu    = mean(phi_v(idx,:)')';
sigma = std(phi_v(idx,:)')';

[phi_dvh,v_dvh,i] = makeDVH(mu,V(idx));

phi_dvh = mu(i);
sigma_dvh = sigma(i);

figure;
hold on;
stairs(phi_dvh,v_dvh,'b-');

%stairs(phi_dvh-2*sigma_dvh,v_dvh,'b-');
%stairs(phi_dvh,v_dvh,'r-','LineWidth',2);
plot([phi_dvh-Nsigmas*sigma_dvh phi_dvh+Nsigmas*sigma_dvh]',repmat(([1;v_dvh(1:end-1)]+v_dvh)/2,[1 2])','r-');

set(gca,'XLim',[0 1e6],'YLim',[0 100]);
xlabel('Dose (au)');
ylabel('Volume (%)');
