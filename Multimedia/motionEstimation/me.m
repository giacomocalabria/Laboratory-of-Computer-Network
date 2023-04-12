function [mvf, MSE] = me(cur, ref, blockSizeRow, blockSizeCol, radius)
%ME Motion estimation
%    MVF = ME(cur, ref, brow, bcol, radius);
%    Computes a motion vector field between the current and reference
%    image, using a given block size and search area's radius
%    [MVF MSE] = ME(...)
%    Returns the MSE associated to the output MVF
%

[ROWS, COLS]=size(cur);
totalSSD = 0;

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
        SSDmin=blockSizeRow*blockSizeCol*256*256; 
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
                    SSD=sum(differences(:).^2); %Sum of squared diff's

                    % If current candidate is better than the previous
                    % best candidate, then update the best candidate
                    if (SSD<SSDmin) 
                        SSDmin=SSD;
                        bestDeltaCol=deltaCol;
                        bestDeltaRow=deltaRow;
                    end
                end % 
            end % 
        end % loop on candidate vectors
        % Store the best MV and the associated cost
        mvf(row:row+blockSizeRow-1,col:col+blockSizeCol-1,1)=bestDeltaRow;
        mvf(row:row+blockSizeRow-1,col:col+blockSizeCol-1,2)=bestDeltaCol;
        totalSSD = totalSSD + SSDmin;
    end  
end % loop on Blocks
MSE = totalSSD /ROWS /COLS;