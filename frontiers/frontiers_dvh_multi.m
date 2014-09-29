% Plots many DVHs one over the other, illustrating the variance reduction effect
%
%   regions         Map of elements to region codes
%   phi_v           Fluence integrated over volume element
%   V               Volume of elements
%   region          Region to plot

function [phi_dvh,v_dvh,sigma_dvh] = frontiers_dvh_multi(regions,phi_v,V,region,varargin)

plotstyles = { 'r-','g-','b-','k-','m-' };

i=1;
while (i<length(varargin))
    if (strcmp(varargin{i},'TargetDose'))
        target=varargin{i+1};
        i=i+2;
    elseif (strcmp(varargin{i},'XLims'))
        xlims=varargin{i+1};
        i=i+2;
    elseif (strcmp(varargin{i},'Labels'))
        labels=varargin{i+1};
        i=i+2;
    else
        error(['Invalid argument: ' varargin{i}]);
    end
end

hold on;

for roi=1:length(region)
    idx = find(regions==region(roi));
    V_roi = V(idx);
    phi_v_roi = phi_v(idx,:);
    [Ne,Nr] = size(phi_v_roi);

    % get mean/variance
    mu = mean(phi_v_roi')';
    sigma = std(phi_v_roi')';

    for i=1:Nr
        [phi_dvh,v_dvh,idx] = makeDVH(phi_v_roi(:,i),V_roi);

        if (exist('target'))
            h = stairs(100*phi_dvh/target,v_dvh,plotstyles{roi});
        else
            h = stairs(phi_dvh,v_dvh,plotstyles{roi});
        end

        if (i==1)
            hobjs(roi)=h;
        end
    end
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

if (exist('labels'))
    legend(hobjs,labels{region+1});
else
    for i=1:length(region)
        labels{i} = sprintf('Region %d',region(i));
    end
    legend(hobjs,labels);
end
