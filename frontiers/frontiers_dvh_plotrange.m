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

function [phi_dvh,v_dvh,sigma_dvh] = frontiers_dvh_plotrange(regions,phi_v,V,rois,varargin)

i=1;

ylims = [0 100];
xlims = [0 200];

vpoints = 60:10:140;

% scale factor
kx = 1;

while (i<length(varargin))
    if (strcmp(varargin{i},'TargetDose'))
        kx=100/varargin{i+1};
        i=i+2;
    elseif (strcmp(varargin{i},'XLims'))
        xlims=varargin{i+1};
        i=i+2;
    elseif (strcmp(varargin{i},'YLims'))
        ylims=varargin{i+1};
        i=i+2;
    else
        error(['Invalid argument: ' varargin{i}]);
    end
end

colours = { 'b-','g-','k-' };

hold on;

for r=1:length(rois)
    idx = find(regions==rois(r));
    
    mu    = mean(phi_v(idx,:)')';
    sigma = std(phi_v(idx,:)')';
    
    [phi_dvh,v_dvh,i] = makeDVH(mu,V(idx));
    
    phi_dvh = mu(i);
    sigma_dvh = sigma(i);
    
    v_mids = ([100;v_dvh(1:end-1)]+v_dvh)/2;
    
    phi_plus  = kx*(phi_dvh+sigma_dvh);
    phi_minus = kx*(phi_dvh-sigma_dvh);
    
    Vplot = [phi_plus phi_minus v_mids];

    i_clip = find(and(xlims(1) < kx*phi_dvh,kx*phi_dvh < xlims(2)));

    Vplot=Vplot(i_clip,:);

    plotX = Vplot(:,1:2)';
    plotY = repmat(Vplot(:,3),[1 2])';
    
    plot(plotX,plotY,'r-','LineWidth',0.1);
    
    [Ne,Nr] = size(phi_v(idx,:));
    
    for i=1:Nr
        [phi_dvh,v_dvh] = makeDVH(phi_v(idx,i),V(idx));
        phi_dvh = kx*phi_dvh;
        i_clip = find(and(xlims(1) < phi_dvh,phi_dvh < xlims(2)));
        %phi_dvh = [xlims(1);phi_dvh(i_clip)];
        %v_dvh = [ylims(2);v_dvh(i_clip)];
        stairs(phi_dvh,v_dvh,colours{r},'LineWidth',0.1);

        for j=1:length(vpoints)
            vvals(j,i) = getV(phi_dvh,v_dvh,vpoints(j));
        end
    end

end

set(gca,'XLim',xlims);
set(gca,'YLim',ylims);

if (kx==1)
    xlabel('Fluence dose (au)');
else
    xlabel('Fluence dose (% of target)');
end


ylabel('Volume (%)');


for i=1:length(vpoints)
    mean_v = mean(vvals(i,:));
    std_v = std(vvals(i,:));
    line([vpoints(i) vpoints(i)],[mean_v-std_v mean_v+std_v],'LineWidth',5,'Color','k');
    printf('V%d=%f +- %f (%f%%)\n',vpoints(i),mean_v,std_v,std_v/mean_v*100);
end
