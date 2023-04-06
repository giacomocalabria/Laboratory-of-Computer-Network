function bits=AChuffTables(run,cat)

if run<0,
    error('Negative run');
elseif cat<0
    error('Negative category')
elseif cat>10
    error('Category cannot be larger than 10')
end

tab{1,1}       ='1010'            ;
tab{1,2}      ='00'              ;
tab{1,3}      ='01'              ;
tab{1,4}       ='100'             ;
tab{1,5}       ='1011'            ;
tab{1,6}       ='11010'           ;
tab{1,7}       ='1111000'         ;
tab{1,8}       ='11111000'        ;
tab{1,9}       ='1111110110'      ;
tab{1,10}       ='1111111110000010';
tab{1,11}      ='1111111110000011';
tab{2,2}      ='1100'            ;
tab{2,3}      ='11011'           ;
tab{2,4}       ='1111001'         ;
tab{2,5}       ='111110110'       ;
tab{2,6}       ='11111110110'     ;
tab{2,7}       ='1111111110000100';
tab{2,8}       ='1111111110000101';
tab{2,9}       ='1111111110000110';
tab{2,10}       ='1111111110000111';
tab{2,11}      ='1111111110001000';
tab{3,2}      ='11100'           ;
tab{3,3}      ='11111001'        ;
tab{3,4}       ='1111110111'      ;
tab{3,5}       ='111111110100'    ;
tab{3,6}       ='1111111110001001';
tab{3,7}       ='1111111110001010';
tab{3,8}       ='1111111110001011';
tab{3,9}       ='1111111110001100';
tab{3,10}       ='1111111110001101';
tab{3,11}      ='1111111110001110';
tab{4,2}      ='111010'          ;
tab{4,3}      ='111110111'       ;
tab{4,4}       ='111111110101'    ;
tab{4,5}       ='1111111110001111';
tab{4,6}       ='1111111110010000';
tab{4,7}       ='1111111110010001';
tab{4,8}       ='1111111110010010';
tab{4,9}       ='1111111110010011';
tab{4,10}       ='1111111110010100';
tab{4,11}       ='111111110010101';
tab{5,2}      ='111011'          ;
tab{5,3}      ='1111111000'      ;
tab{5,4}       ='1111111110010110';
tab{5,5}       ='1111111110010111';
tab{5,6}       ='1111111110011000';
tab{5,7}       ='1111111110011001';
tab{5,8}       ='1111111110011010';
tab{5,9}       ='1111111110011011';
tab{5,10}       ='1111111110011100';
tab{5,11}      ='1111111110011101';
tab{6,2}      ='1111010'         ;
tab{6,3}      ='11111110111'     ;
tab{6,4}       ='1111111110011110';
tab{6,5}       ='1111111110011111';
tab{6,6}       ='1111111110100000';
tab{6,7}       ='1111111110100001';
tab{6,8}       ='1111111110100010';
tab{6,9}       ='1111111110100011';
tab{6,10}       ='1111111110100100';
tab{6,11}      ='1111111110100101';
tab{7,2}      ='1111011'         ;
tab{7,3}      ='111111110110'    ;
tab{7,4}       ='1111111110100110';
tab{7,5}       ='1111111110100111';
tab{7,6}       ='1111111110101000';
tab{7,7}       ='1111111110101001';
tab{7,8}       ='1111111110101010';
tab{7,9}       ='1111111110101011';
tab{7,10}       ='1111111110101100';
tab{7,11}      ='1111111110101101';
tab{8,2}      ='11111010'        ;
tab{8,3}      ='111111110111'    ;
tab{8,4}       ='1111111110101110';
tab{8,5}       ='1111111110101111';
tab{8,6}       ='1111111110110000';
tab{8,7}       ='1111111110110001';
tab{8,8}       ='1111111110110010';
tab{8,9}       ='1111111110110011';
tab{8,10}       ='1111111110110100';
tab{8,11}      ='1111111110110101';
tab{9,2}      ='111111000';
tab{9,3}      ='111111111000000';
tab{9,4}       ='1111111110110110';
tab{9,5}       ='1111111110110111';
tab{9,6}       ='1111111110111000';
tab{9,7}       ='1111111110111001';
tab{9,8}       ='1111111110111010';
tab{9,9}       ='1111111110111011';
tab{9,10}       ='1111111110111100';
tab{9,11}      ='1111111110111101';
tab{10,2}      ='111111001';
tab{10,3}      ='1111111110111110';
tab{10,4}       ='1111111110111111';
tab{10,5}       ='1111111111000000';
tab{10,6}       ='1111111111000001';
tab{10,7}       ='1111111111000010';
tab{10,8}       ='1111111111000011';
tab{10,9}       ='1111111111000100';
tab{10,10}       ='1111111111000101';
tab{10,11}      ='1111111111000110';
tab{11,2}      ='111111010';
tab{11,3}      ='1111111111000111';
tab{11,4}       ='1111111111001000';
tab{11,5}       ='1111111111001001';
tab{11,6}       ='1111111111001010';
tab{11,7}       ='1111111111001011';
tab{11,8}       ='1111111111001100';
tab{11,9}       ='1111111111001101';
tab{11,10}       ='1111111111001110';
tab{11,11}      ='1111111111001111';
tab{12,2}      ='1111111001';
tab{12,3}      ='1111111111010000';
tab{12,4}       ='1111111111010001';
tab{12,5}       ='1111111111010010';
tab{12,6}       ='1111111111010011';
tab{12,7}       ='1111111111010100';
tab{12,8}       ='1111111111010101';
tab{12,9}       ='1111111111010110';
tab{12,10}       ='1111111111010111';
tab{12,11}      ='1111111111011000';
tab{13,2}      ='1111111010';
tab{13,3}      ='1111111111011001';
tab{13,4}       ='1111111111011010';
tab{13,5}       ='1111111111011011';
tab{13,6}       ='1111111111011100';
tab{13,7}       ='1111111111011101';
tab{13,8}       ='1111111111011110';
tab{13,9}       ='1111111111011111';
tab{13,10}       ='1111111111100000';
tab{13,11}      ='1111111111100001';
tab{14,2}      ='11111111000';
tab{14,3}      ='1111111111100010';
tab{14,4}       ='1111111111100011';
tab{14,5}       ='1111111111100100';
tab{14,6}       ='1111111111100101';
tab{14,7}       ='1111111111100110';
tab{14,8}       ='1111111111100111';
tab{14,9}       ='1111111111101000';
tab{14,10}       ='1111111111101001';
tab{14,11}      ='1111111111101010';
tab{15,2}      ='1111111111101011';
tab{15,3}      ='1111111111101100';
tab{15,4}       ='1111111111101101';
tab{15,5}       ='1111111111101110';
tab{15,6}       ='1111111111101111';
tab{15,7}       ='1111111111110000';
tab{15,8}       ='1111111111110001';
tab{15,9}       ='1111111111110010';
tab{15,10}       ='1111111111110011';
tab{15,11}      ='1111111111110100';
tab{16,1}  ='11111111001';
tab{16,2}      ='1111111111110101';
tab{16,3}      ='1111111111110110';
tab{16,4}       ='1111111111110111';
tab{16,5}       ='1111111111111000';
tab{16,6}       ='1111111111111001';
tab{16,7}       ='1111111111111010';
tab{16,8}       ='1111111111111011';
tab{16,9}       ='1111111111111100';
tab{16,10}       ='1111111111111101';
tab{16,11}      ='1111111111111110';
if run>15
    tmp = AChuffTables(run-15,cat);
    bits= [tab{16,1}, tmp];
else
    bits= tab{run+1,cat+1};
end
