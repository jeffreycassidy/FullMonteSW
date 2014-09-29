% plotdvhs(dvhArray,[labels])
%
% Plots several dose-volume histograms including labels
%
% dvhArray      Cell array with each element Ne x 2 (or x3)
%
%

function plotdvhs(dvhArray,labels);

[Ne,Nc] = size(dvhArray{1});

if (Nc == 2)
    printf('DVH with two columns (no variance)\n');
elseif (Nc == 3)
    printf('DVH with three columns (incl variance)\n');
end

idx_nz = [];

for i=1:length(dvhArray)
    [Ne,Nc] = size(dvhArray{i});
    if (Ne > 1)
        idx_nz = [idx_nz i];
    end
end

printf('Nonzero elements: ');
printf('%d ',idx_nz);
printf('\n');
printf('Nonzero labels: ');
printf('%s ',labels{idx_nz});
printf('\n');

figure;
title('Dose-volume histograms');
hold on;

plotstyle = { 'r-','g-','b-','m-','y-','k-','r+','g+','b+','m+','y+','k+' };

printf('Regions with nonzero fluence:\n');

for i=1:length(idx_nz)
    plot(dvhArray{idx_nz(i)}(:,1),dvhArray{idx_nz(i)}(:,2)*100,plotstyle{i});
    [Ne,Nc] = size(dvhArray{idx_nz(i)});
    printf('  Region %d (%s): %d elements\n',idx_nz(i)-1,labels{idx_nz(i)},Ne-1);
end
set(gca,'XLim',[0 1e6],'YLim',[0 100]);
xlabel('Dose (au)');
ylabel('Volume (%)');

legend(labels{idx_nz});
