
regionID=10;

figure;
hold on;

for i=1:24
    D = loadDVH(sprintf('Test/DVH/poisson_bootstrap%d.dvh.txt',i-1));
    [x,y] = stairs(D{regionID+1}(:,1),D{regionID+1}(:,2));
    semilogx(x,y,'r');
end

% load it with variance
Dv = loadDVH('Test/DVH/poisson_var.dvh.txt',1);

mean_fluence = Dv{regionID+1}(:,1);
vol_cdf      = Dv{regionID+1}(:,2);
std_fluence  = sqrt(Dv{regionID+1}(:,3));

%errorbar(mean_fluence,vol_cdf,std_fluence,'>');

X = [mean_fluence+std_fluence mean_fluence-std_fluence]';
Y = [vol_cdf vol_cdf]';

X = max(X,1);

semilogx(X(:,1:50:end),Y(:,1:50:end),'b-');

