% 100x 100k-el line source run
runs = 3275:3374;
fn_fmt = '../output/ls_200k/flt592.%d.phi_v.txt';
fn_regions = '../output/ls_200k/dvh.regions.out';

frontiers_dvh_load
digilabels

frontiers_dvh_multi(regions,phi_v,V,[10 12 18],'TargetDose',0.75e4,'Labels',digilabels);
title('Dose-volume histogram for relevant organs, 100x 100k packet runs');
print('dvh100x100k.eps','-deps','-color','-tight','-r900');

frontiers_dvh_plotrange(regions,phi_v,V,18);
title('Variance reduction');
legend('Variance range of constituent element (\pm\sigma)','100 DVH runs');


clear


% 1M-el line source run
runs = 3098:3197;
fn_fmt = '../output/ls_1M_good/flt589.%d.phi_v.txt';
fn_regions = '../output/ls_1M_good/dvh.regions.out';

frontiers_dvh_load

