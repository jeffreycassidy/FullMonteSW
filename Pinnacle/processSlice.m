% processSlice(C)
%
% Arguments
%   C       Cell array containing 1 or more curves
%
%

function processSlice(C)

if (!iscell(C))
    M = C;
    C = cell(1);
    C{1}=M;
end

size(C)

P=zeros(0,3);

figure; hold on;
for i=1:length(C)
    x = C{i}(:,1);
    y = C{i}(:,2);
    plot([x;x(1)],[y;y(1)],'LineWidth',5);

    P=[P;C{i}];

%    TRI = delaunay(x,y);
%    triplot(TRI,x,y);
end

P2 = P(:,1:2);
TRI=delaunay(P(:,1),P(:,2));

figure;
triplot(TRI,P2(:,1),P2(:,2));


fid = fopen('test.poly','w');
fprintf(fid,'%d 2 0 1\n',length(P2));
fprintf(fid,'%d %f %f 1\n',[(1:length(P2))' P2]);

for i=1:length(C)
    Np = length(C{i});
    fprintf(fid,'%d %d %d 1\n',[(1:length(P2))' (2:length(P2)+1)']');
end

% no holes (otherwise N, following by {hole# x y}N }
fprintf(fid,'%d\n',0);

% additional constraints if any
%% NONE SO FAR
% <region#> <x> <y> <attribute> <max area>
