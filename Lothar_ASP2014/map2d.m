fid = fopen('map2d.csv','r');

mu_eff=sqrt(3*0.526*(0.526+10.0));

T = fscanf(fid,'%f,%f,%f\n',[3 Inf])';

r = reshape(T(:,1),40,20);
z = reshape(T(:,2),40,20);
f = reshape(T(:,3),40,20);

imagesc(r,z,log10(f/max(f(:))));

theta=linspace(-pi/2,pi/2,100);
ucirc_y = sin(theta);
ucirc_x = cos(theta);

R = 1/mu_eff;

hold on;

plot(1*R*ucirc_x,1*R*ucirc_y,'r-');
plot(2*R*ucirc_x,2*R*ucirc_y,'r-');
plot(3*R*ucirc_x,3*R*ucirc_y,'r-');
plot(4*R*ucirc_x,4*R*ucirc_y,'r-');

axis square
axis equal

title('Relative detection constant \mu_{eff}=4.0cm^{-1}, albedo 95%');
xlabel('Radius r (cm)');
ylabel('Axial distance z (cm)');
colorbar;

fclose(fid);

print 'map2d.png' '-dpng'


figure;

indices=[6 11];

z_line = z(:,indices);
z_pos  = z(21:end,indices);
z_neg  = flipud(z(1:20,indices));

f_line = f(:,indices);
f_pos  = f(21:end,indices);
f_neg  = flipud(f(1:20,indices));

lw = 3;

semilogy(z_pos(:,1),f_pos(:,1),'Linewidth',lw,-z_neg(:,1),f_neg(:,1),'Linewidth',lw,z_pos(:,2),f_pos(:,2),'Linewidth',lw,-z_neg(:,2),f_neg(:,2),'Linewidth',lw);

xlabel('Axial separation |z| (cm)');
ylabel('Detection efficiency (au)');
title('Positional sensitivity of axial sensor placement');
legend('z+ r=1 MFP','z- r=1 MFP','z+ r=2 MFP','z- r=2 MFP');

