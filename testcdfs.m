sse_exp = loadVectN('unitexp.sse.out',1)';
plotcdf(sse_exp,1000,'SSE unit exponential');

avx_exp = loadVectN('unitexp.avx.out',1)';
plotcdf(avx_exp,1000,'AVX unit exponential');
