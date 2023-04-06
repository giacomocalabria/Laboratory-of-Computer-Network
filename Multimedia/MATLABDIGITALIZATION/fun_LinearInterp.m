% fun_LinearInterp  Linear Interpolation 
% Replace missing samples with linear interpolation of previous and next
% ones
% ys: input vector 
% FS: sampling frequency of the input vector
% FStarget: sampling frequency of the output vector

function yi=fun_LinearInterp(ys,FS,FStarget)

fact = round(FStarget/FS);
if not(FS*fact == FStarget)
    error('Target frequency must be a multiple integer of the original frequency')
end


FS2 = FS * fact; 
disp(['Interpolating from ',num2str(FS),' Hz to ',num2str(FS2),' Hz using Linear interpolation'])


% represent ys as a raw vector 
ys = reshape(ys,1,length(ys));
ysp = [ys(2:end),0]; 

% slope for each intermediate sample
slope = (ysp-ys)/fact;
% delete last element which is not meaningful nor useful
slope(end)=[]; 

% create a matrix where the j-th column contains the increments between
% consecutive interpolated samples starting from the j-th value of
% the original vector
D = [0:fact-1]'*slope;

% create a matrix with "fact" rows all equal to ys
yy = ones(fact,1) * ys(1:end-1) ;
yx = yy+D;

% Create a vector with all the elements of yx read columnwise
yi = [yx(:);ys(end)]; 
