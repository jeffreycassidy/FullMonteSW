load out.mat

% calculate elementwise mean and stddev
mean_e = mean(volume_energy')';
std_e  = std(volume_energy')';

% trim out zero values
idx_nz = find(mean_e);
printf('There are %d nonzero elements (of %d)\n',length(idx_nz),length(mean_e));
mean_e = mean_e(idx_nz);
std_e  = std_e(idx_nz);

% plot it
subplot(2,1,1);

%% plot: mean vs stdev
plot(mean_e,std_e,'.');
xlabel('Mean value');
ylabel('Standard deviation');

%% plot: mean vs. coeff of variation
subplot(2,1,2);
loglog(mean_e,std_e./mean_e,'.');
xlabel('Mean value');
ylabel('Coefficient of variation \mu/\sigma');
set(gca,'YLim',[0.001 1]);


