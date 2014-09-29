% Plots a DVH with +/- sigma bars on the horizontal axis
%
% frontiers_dvh_plotrange(regions,phi_v,V,region,opts...)
%
%   regions         Map of elements to region codes
%   phi_v           Fluence integrated over volume element
%   V               Volume of elements
%   region          Region(s) to plot
%
%   opts            Options
%       'TargetDose'    Scale x axis relative to target dose and display 0..200%
%       'XLim'          Specify x limits

function [phi_dvh,v_dvh,sigma_dvh] = frontiers_dvh_plotrange(regions,phi_v,V,region,varargin)

i=1;
while (i<length(varargin))
    if (varargin{i} == 'TargetDose')
        target=varargin{i+1};
        i=i+2;
    elseif (varargin{i} == 'XLims')
        xlims=varargin{i+1};
        i=i+2;
    else
        error(['Invalid argument: ' varargin{i}]);
    end
end


idx = find(regions==region);

mu    = mean(phi_v(idx,:)')';
sigma = std(phi_v(idx,:)')';

[phi_dvh,v_dvh,i] = makeDVH(mu,V(idx));

phi_dvh = mu(i);
sigma_dvh = sigma(i);

%stairs(phi_dvh,v_dvh,'b-');
%stairs(phi_dvh-2*sigma_dvh,v_dvh,'b-');
%stairs(phi_dvh+2*sigma_dvh,v_dvh,'r-','LineWidth',2);

if(exist('target'))
    plot(100*[phi_dvh-sigma_dvh phi_dvh+sigma_dvh]'/target,repmat(([1;v_dvh(1:end-1)]+v_dvh)/2,[1 2])','r-');
else
    plot([phi_dvh-sigma_dvh phi_dvh+sigma_dvh]',repmat(([1;v_dvh(1:end-1)]+v_dvh)/2,[1 2])','r-');
end

set(gca,'YLim',[0 100]);

if (exist('target'))
    xlabel('Fluence dose (% of target)');
    set(gca,'XLim',[0 2e2]);
elseif(exist('XLim'))
    set(gca,'XLim',xlims);
    xlabel('Fluence dose (au)');
end

ylabel('Volume (%)');
