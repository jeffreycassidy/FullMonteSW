function dvh(V,labels)

figure;
hold on;

formats = { 'r','g','b','c','m','y','k','r--','g--','b--','c--','m--','y--','k--','r-.','g-.','b-.','c-.','m-.','y-.','l-.' };

minval=1e-5;
maxval=1;

if (iscell(V))
    for i=1:length(V)
        [xs,ys] = stairs(max(minval,[0;V{i}(:,1)]),100*[1;V{i}(:,2)]);
        semilogx(xs,ys,formats{i});
    end
else
    [xs,ys] = stairs(max(minval,[0;V(:,1)]),100*[1;V(:,2)]);
    semilogx(xs,ys);
end

set(gca,'YLim',[0 100],'XLim',[minval maxval]);
xlabel('Fluence (J/cm^2)');
ylabel('% of volume');
title('Dose-volume histogram');
