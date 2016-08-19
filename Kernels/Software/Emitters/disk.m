N=1000;

rnd=rand(N,2);

% Cover a disk randomly using r * [ sin(2 pi theta), cos(2 pi theta) ]
% Where theta is U[0,1) and r is sqrt(U[0,1))

P = [cos(2*pi*rnd(:,1)) sin(2*pi*rnd(:,1))] .* repmat(sqrt(rnd(:,2)),1,2);

plot(P(:,1),P(:,2),'.');
