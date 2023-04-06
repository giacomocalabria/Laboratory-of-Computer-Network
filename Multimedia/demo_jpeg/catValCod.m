function bits = catValCod(cat,dcPredErr)
%JPEG DC coding

% Category coding
switch cat
    case 0
        nb = 2;
        bits = '00';
    case 1
        nb = 3;
        bits = '010';
    case 2
        nb = 3;
        bits = '011';
    case 3
        nb = 3;
        bits = '100';
    case 4
        nb = 3;
        bits = '101';
    case 5
        nb = 3;
        bits = '110';
    case 6
        nb = 4;
        bits = '1110';

    case 7
        nb = 5;
        bits = '11110';
    case 8
        nb = 6;
        bits = '111110';
    case 9
        nb = 7;
        bits = '1111110';
    case 10
        nb = 8;
        bits = '11111110';
    case 11
        nb = 9;
        bits = '111111110';
    otherwise
        error('Wrong DC category')
end

% Value coding
if dcPredErr >0
    bits(1,nb+1:nb+cat)= dec2bin(dcPredErr,cat);
elseif dcPredErr < 0
    bits(1,nb+1:nb+cat)= 97-(dec2bin(-dcPredErr,cat));
end