% F(x) == Pr(X <= x)
%
% xi    Input x data
% N     Take every Nth point
% name  Distribution name

function plotcdf(xi,N,name)

x=sort(xi);
F=(1:length(x))/length(x);

x_sub=x(1:N:end);
F_sub=(1:N:length(x))/length(x);

F_ref = 1-exp(-x_sub);

figure;
plot(x_sub,F_sub,'r+',x_sub,F_ref,'b-');
legend('Received','Expected');

if (nargin > 2)
    title(['CDF for ' name]);
else
    title('CDF');
end
xlabel('Value x');
ylabel('Pr(X <= x)');

%figure; 
%plot(x_sub,F_sub-F_ref);
%title('Deviation');
%xlabel('Value x');
%ylabel('CDF - expected CDF');
