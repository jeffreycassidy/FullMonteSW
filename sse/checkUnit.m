function a = checkUnit(fn,D)

fid=fopen(fn,'r');
a = fscanf(fid,'%f',[D Inf])';
fclose(fid);

subplot(2,2,1);
hist(sqrt(sum(a'.^2)));
title('Histogram of vector lengths');

subplot(2,2,2);

