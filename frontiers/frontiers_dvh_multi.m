% Plots many DVHs one over the other, illustrating the variance reduction effect
%
%   regions         Map of elements to region codes
%   phi_v           Fluence integrated over volume element
%   V               Volume of elements
%   region          Regions to plot (target first if applicable)

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

set(gcf,'Visible','Off');

for roi=1:length(region)
    idx = find(regions==region(roi));
    V_roi = V(idx);
    phi_v_roi = phi_v(idx,:);
    [Ne,Nr] = size(phi_v_roi);

    % get mean/variance
    mu = mean(phi_v_roi')';
    sigma = std(phi_v_roi')';

    for i=1:Nr
        [phi_dvh,v_dvh,idx,D90] = makeDVH(phi_v_roi(:,i),V_roi);

        if (roi == 1)
            D90s(i) = D90;
        end

        if (exist('target'))
            h = stairs(100*phi_dvh/target,v_dvh,plotstyles{roi});
        else
            k = 100/getD90(phi_dvh,v_dvh);
            h = stairs(k*phi_dvh,v_dvh,plotstyles{roi},'LineWidth',0.1);
        end

        if (i==1)
            hobjs(roi)=h;
        end
    end
end

mu_d90 = mean(D90s);
sigma_d90 = std(D90s);

printf('Target region D90 mean=%f std=%f cv=%f\n',mu_d90,sigma_d90,sigma_d90/mu_d90);


set(gca,'YLim',[0 100]);

if (exist('target'))
    xlabel('Fluence dose (% of target)');
    set(gca,'XLim',[0 2e2]);
elseif(exist('xlims'))
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

line((mu_d90 + sigma_d90*[-1 1])/target*100,[90 90],'LineWidth',5,'Color','k');
set(gcf,'Visible','On');
