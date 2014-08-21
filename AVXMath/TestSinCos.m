T=loadVectN('sincos.avx.txt',5)';

figure; plot(T(1:10000,1),T(1:10000,2:3),'.'); title('Reference');
figure; plot(T(1:10000,1),T(1:10000,4:5),'.'); title('AVX');

printf('%d/%d deviate by more than 1e-6\n',length(find(abs(T(:,2:3)-T(:,4:5))>1e-6)),2*length(T));

