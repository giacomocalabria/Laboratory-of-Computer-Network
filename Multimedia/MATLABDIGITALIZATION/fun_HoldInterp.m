% fun_HoldInterp  Holding Interpolation 
% ys: input vector 
% FS: sampling frequency of the input vector
% FStarget: sampling frequency of the output vector

function yi=fun_HoldInterp(ys,FS,FStarget)

fact = round(FStarget/FS);
if not(FS*fact == FStarget),
    error('Target frequency must be a multiple integer of the original frequency')
end


FS2 = FS * fact; 
disp(['Interpolating from ',num2str(FS),' Hz to ',num2str(FS2),' Hz using Holding interpolation'])

% represent ys as a raw vector
ys = reshape(ys,1,length(ys));

% create a matrix with "fact" rows all equal to ys
yy = ones(fact,1) * ys ;

% Create a vector with all the elements of yy read columnwise
yi = yy(:); 
