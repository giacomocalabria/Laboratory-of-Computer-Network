% fun_Quantizer_balanced New quantization function 
% ys: input vector 
% Quantizer: return a quantized versiopn of the input vector on L levels
% L: number of levels

function xq=fun_Quantizer_balanced(x,L)

% Here we balance the quantization alphabet and add the zero symbol. 
% The quantization alphabet will then be 
% Q={0,+/-Delta, +/- 2*Delta, ..., +/- (L-2)/2*Delta }
% Overall, we will use L-1 symbols, out of the L possible ones (one symbol
% remains unused)
% We split the range in L-1 intervals and, whitin each interval, we round
% the value of the signal to the closest multiple of Delta. 

delta = (max(x)-min(x))/(L-1);
xq = round(x/delta)*delta;


% OTHER OPTION % % 
% Vmean = mean(x); Vstd = std(x);
% xs2 = sort(x.^2,'descend');
% SatThreshold = 0.8;
% z=sum(cumsum(xs2./sum(x.^2))<=1-SatThreshold);
% a=sqrt(abs(xs2(z)));
% Vm = -a;
% % We slightly inflate the max value to avoid problems when x==max(x) 
% VM = a; 
% 
% x=min(x,VM)*0.999;
% x=max(x,Vm)*0.999;
% Delta = (VM-Vm)/L; 
% 
% xq = floor((x-Vm)/Delta)*Delta + Delta/2+ Vm;

