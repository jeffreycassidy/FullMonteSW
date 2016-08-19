N=1000;
theta_max=60*pi/180;	% 20 deg as rad

rnd = rand(N,2);

phi = 2*pi*rnd(:,1);
cosphi = cos(phi);
sinphi = sin(phi);
costheta = 1-(1-cos(theta_max))*rnd(:,2);
sintheta = sqrt(1-costheta.^2);

% Yields cone +/- theta_max degrees around (0,0,1) vector

P = [sintheta.*cosphi sintheta.*sinphi costheta];

plot3(P(:,1),P(:,2),P(:,3),'.');
set(gca,'XLim',[-1 1],'YLim',[-1 1],'ZLim',[0 1]);
