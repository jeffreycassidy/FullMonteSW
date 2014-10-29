% Needs data loaded first

figure;
mu_E = mean(E_v_100k_good')';
var_E = var(E_v_100k_good')';
std_E = std(E_v_100k_good')';

loglog(mu_E,var_E,'+');

xlabel('Energy received (photon weight)');
ylabel('Variance of energy received (photon weight^2)');

% plot fit line for values 10^-2 and up

lb=1;
i = find(mu_E > lb);
C = polyfit(log(mu_E(i)),log(var_E(i)),1);
printf('Polynomial fit on loglog with constants %f %f\n',C(1),C(2));

% fit is of form log y = c1 log x + c2 => y = c2*x^c1 

hold on;
line([lb max(mu_E(i))],exp(C(2))*[lb^C(1) max(mu_E(i))^C(1)],'LineWidth',5,'Color','g');

ub=1e-5;
i = find(and(mu_E < ub,mu_E>0));
C = polyfit(log(mu_E(i)),log(var_E(i)),1);
printf('Polynomial fit on loglog with constants %f %f\n',C(1),C(2));
line([min(mu_E(i)) ub],exp(C(2))*[min(mu_E(i))^C(1) ub^C(1)],'LineWidth',5,'Color','r');


figure;
semilogx(mu_E,std_E./mu_E,'r+');
set(gca,'YLim',[0 1]);
xlabel('Mean \mu');
ylabel('Coefficient of variation (\sigma/\mu)');

