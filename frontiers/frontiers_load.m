% 100x 100k-el line source run
%fn_fmt_energy  = '../output/ls_100k/flt592.%d.E_v.txt';
fn_regions = '../output/ls_100k/dvh.regions.out';

[regions,vols,idx] = loadRegions(fn_regions);

phi_v_100k_good = frontiers_dvh_load(3275:3374,'../output/ls_100k/flt592.%d.phi_v.txt',idx);
E_v_100k_good = frontiers_dvh_load(3275:3374,'../output/ls_100k/flt592.%d.E_v.txt',idx);

phi_v_200k_good = frontiers_dvh_load(3199:3273,'../output/ls_200k/flt591.%d.phi_v.txt',idx);
E_v_200k_good = frontiers_dvh_load(3199:3273,'../output/ls_200k/flt591.%d.E_v.txt',idx);

phi_v_1M_good = frontiers_dvh_load(3098:3197,'../output/ls_1M/flt589.%d.phi_v.txt',idx);
E_v_1M_good = frontiers_dvh_load(3098:3197,'../output/ls_1M/flt589.%d.E_v.txt',idx);

phi_v_10M_bad = frontiers_dvh_load(3197,'../output/ls_bad_10M/flt590.%d.phi_v.txt',idx);

digilabels;

save frontiers.mat

