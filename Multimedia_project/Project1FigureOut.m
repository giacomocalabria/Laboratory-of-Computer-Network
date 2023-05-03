close all;
clear all;
clc;

%% Parametri
N = 64; % Dimensione del blocco per la DCT
R = 98; % Percentuale dei coefficienti DCT da mettere a zero

% Loading an RGB image
rgb_in = imread("colors.bmp");

% Visualizza immagine originale
figure(1); image(rgb_in); 
axis image; axis off; 
title('Original image','Interpreter','latex');

% Inizializzazione dei vettori per i valori di PSNR
psnr = zeros(size(R));

% Definisco le funzioni per la DCT e la Inverse DCT
dctfun = @(block) dct2(block.data);
dctinvfun = @(block) idct2(block.data);

% Converting the RGB image to YCbCr space
ycbcr_in = rgb2ycbcr(rgb_in);

% Extracting the Y, Cb and Cr components
y = ycbcr_in(:,:,1);
cb = ycbcr_in(:,:,2);
cr = ycbcr_in(:,:,3);

% Effettua la DCT bidimensionale delle componenti
y_dct = blockproc(y, [N N], dctfun);
cb_dct = blockproc(cb, [N N], dctfun);
cr_dct = blockproc(cr, [N N], dctfun);

% Calcola la frazione dei coefficenti DCT dell'intera componente da
% mettere a zero; prctile(A,p) return percentiles of elements in input
% data A for the percentages p in the interval [0,100].
perc_y = prctile(abs(y_dct(:)), R);
perc_cb = prctile(abs(cb_dct(:)), R);
perc_cr = prctile(abs(cr_dct(:)), R);

% Mette a zero la frazione dei coefficenti DCT calcolata
y_dct(abs(y_dct) < perc_y) = 0;
cb_dct(abs(cb_dct) < perc_cb) = 0;
cr_dct(abs(cr_dct) < perc_cr) = 0;

% Effettua la DCT inversa sui blocchi dopo sogliaggio, ottenendo la 
% versione "compressa" della componente
y_compressed = blockproc(y_dct, [N N], dctinvfun);
cb_compressed = blockproc(cb_dct, [N N], dctinvfun);
cr_compressed = blockproc(cr_dct, [N N], dctinvfun);

% Calcolo dell'MSE per la componente Y, Cb e Cr tra quella originale e
% la componente "compressa"
mse_y = immse(double(y(:)),double(y_compressed(:)));
mse_cb = immse(double(cb(:)),double(cb_compressed(:)));
mse_cr = immse(double(cr(:)),double(cr_compressed(:)));

% Calcolo dell'MSE pesato e del PSNR e meorizzazione dei valori di PSNR
mse_P = (3/4) * mse_y + (1/8) * mse_cb + (1/8) * mse_cr;
psnr = 10 * log10((255^2) / mse_P)
N

% Ricompone l'immagine dalle componenti compresse
comp = rgb_in;
comp(:,:,1) = y_compressed;
comp(:,:,2) = cb_compressed;
comp(:,:,3) = cr_compressed;
txt = "Compressed image $R="+R+"\%$ $N="+N+"$";
figure(2); image(ycbcr2rgb(comp)); 
axis image; axis off; 
title(txt,'Interpreter','latex')