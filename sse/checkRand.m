function r = checkRand(fn)

fid=fopen(fn,'r');
r = fscanf(fn,'%f',[1 Inf])';
fclose(fid);

x = linspace(0,1,length(r));

subplot(2,2,1);
plot(x,sort(r),'.');
xlabel('Pr(X < x)');
ylabel('x');


