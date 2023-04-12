function [mvf] = meReg(cur, ref, blockSizeRow, blockSizeCol, radius, lambda)
%MEREG Regularized Motion estimation
%    MVF = meReg(cur, ref, brow, bcol, radius, lambda);
%    Computes a motion vector field between the current and reference
%    image, using a given block size and search area's radius
%    The criterion is SSD + lambda* Distance
%    Where Distance is the distance between current candidate and the
%    average of (available) top and left neighbors
%

[ROWS, COLS]=size(cur);
mvf = zeros(ROWS,COLS,2);

% Blocks scan
for row=1:blockSizeRow:ROWS
    for col=1:blockSizeCol:COLS        
        % Block from current image
        % B is a block with top-left pixel in position (row,col); the size
        % of B is blockSizeRow x blockSizeCol
        B=cur(row:row+blockSizeRow-1,col:col+blockSizeCol-1);
        % Initialization of the best displacement
        bestDeltaCol=0; bestDeltaRow=0;
        % Best cost initialized at the highest possible value
        Jmin=blockSizeRow*blockSizeCol*256*256; 
        % loop on candidate motion vectors v = (deltaCol,deltaRow) 
        % It is a full search in [-radius:radius]^2
        for deltaCol=-radius:radius
            for deltaRow=-radius:radius
                % Check: the candidate vector must point inside the image
                if ((row+deltaRow>0)&&(row+deltaRow+blockSizeRow-1<=ROWS)&& ...
                        (col+deltaCol>0)&&(col+deltaCol+blockSizeCol-1<=COLS))
                    % Reference Block
                    R=ref(row+deltaRow:row+deltaRow+blockSizeRow-1, ...
                        col+deltaCol:col+deltaCol+blockSizeCol-1);
                    differences = B-R;
                    MSE=sum(differences(:).^2); %Sum of squared diff's

                    % Regularization

                    % 1. Find the predictor of the current candidate
                    if col==1 && row ==1
                        NM = [0; 0];
                        weight = 0;
                    elseif col==1
                        NM = mvf(row-1,col,:);
                        weight = lambda/2;
                    elseif row==1
                        NM = mvf(row,col-1,:);
                        weight = lambda/2;
                    else
                        NM = (mvf(row-1,col,:)+mvf(row,col-1,:))/2;
                        weight=lambda;
                    end

                    % 2. The regularization cost is the distance btw 
                    % predictor candidate

                    distance  = norm([deltaRow; deltaCol] -NM(:));
                    J= MSE + weight*distance;


                    % If current candidate is better than the previous
                    % best candidate, then update the best candidate
                    if (J<Jmin) 
                        Jmin=J;
                        bestDeltaCol=deltaCol;
                        bestDeltaRow=deltaRow;
                    end

                end % 
            end % 
        end % loop on candidate vectors
        % Store the best MV and the associated cost
        mvf(row:row+blockSizeRow-1,col:col+blockSizeCol-1,1)=bestDeltaRow;
        mvf(row:row+blockSizeRow-1,col:col+blockSizeCol-1,2)=bestDeltaCol;
        
    end  
end % loop on Blocks
