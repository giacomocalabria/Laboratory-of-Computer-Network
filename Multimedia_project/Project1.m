close all;
clear all;
clc;

%% Parametri
N = 8; % Dimensione del blocco per la DCT
R = [10,12,14,16,18,20,22,30,40,50,60,70,80,90,93,95,96,97,98,99,99.5,99.9,100];
%R = 10:5:100; % Valori di R da 10 a 100 a passi di 10

% Loading an RGB image
rgb_in = imread("coffee.bmp");

% Inizializzazione dei vettori per i valori di PSNR
psnr = zeros(size(R));

% Definisco le funzioni per la DCT e la Inverse DCT
dctfun = @(block) dct2(block.data);
dctinvfun = @(block) idct2(block.data);

% Calcolo del PSNR per diversi valori di R
for i = 1:numel(R)
    clc;
    disp(i/numel(R)*100+" %");
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
    perc_y = prctile(abs(y_dct(:)), R(i));
    perc_cb = prctile(abs(cb_dct(:)), R(i));
    perc_cr = prctile(abs(cr_dct(:)), R(i));

    % Mette a zero la frazione dei coefficenti DCT calcolata
    y_dct(abs(y_dct) < perc_y) = 0;
    cb_dct(abs(cb_dct) < perc_cb) = 0;
    cr_dct(abs(cr_dct) < perc_cr) = 0;

    %Effettua la DCT inversa sui blocchi dopo sogliaggio, ottenendo la versione "compressa" della componente
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
    psnr(i) = 10 * log10((255^2) / mse_P);
end

% Tracciamento della curva del PSNR in funzione di R
figure;
plot(R, psnr, 'LineWidth', 2, 'DisplayName', 'PSNR');
xlabel('R (\%)','Interpreter','latex');
ylabel('PSNR (dB)','Interpreter','latex');
t = "PSNR in funzione di R per N = " + N;
title(t,'Interpreter','latex');
grid on;