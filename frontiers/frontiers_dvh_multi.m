% Plots many DVHs one over the other, illustrating the variance reduction effect
%
%   regions         Map of elements to region codes
%   phi_v           Fluence integrated over volume element
%   V               Volume of elements
%   region          Region to plot

function [phi_dvh,v_dvh,sigma_dvh] = frontiers_dvh_multi(regions,phi_v,V,region)

idx = find(regions==region);

V = V(idx);
phi_v = phi_v(idx,:);

[Ne,Nr] = size(phi_v);

figure;
hold on;

for i=1:Nr
    [phi_dvh,v_dvh,idx] = makeDVH(phi_v(:,i),V);

    plot(phi_dvh,v_dvh,'r.','MarkerSize',1);
end

set(gca,'XLim',[0 1e6],'YLim',[0 100]);
xlabel('Dose (au)');
ylabel('Volume (%)');
