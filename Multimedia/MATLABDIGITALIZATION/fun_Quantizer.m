% fun_LinearInterp  Linear Interpolation 
% ys: input vector 
% Quantizer: return a quantized versiopn of the input vector on L levels
% L: number of levels

function xq=fun_Quantizer(x,L)
Vm = min(x);
% We slightly inflate the max value to avoid problems when x==max(x) 
VM = max(x)*1.001; 

Delta = (VM-Vm)/L; 

xq = floor((x-Vm)/Delta)*Delta + Delta/2+ Vm;

