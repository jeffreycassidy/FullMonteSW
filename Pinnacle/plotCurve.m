function plotCurve(C,i)

if (length(i)==1)
    plot3(C{i}(:,1),C{i}(:,2),C{i}(:,3));
else
    figure; hold on;
    for j=1:length(i)
        plot3(C{i(j)}(:,1),C{i(j)}(:,2),C{i(j)}(:,3));
    end
end



