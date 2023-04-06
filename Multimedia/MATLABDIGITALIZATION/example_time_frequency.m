% Esempio time-frequency

clear all
%close all
Tmax = 0.2; % Signal duration [s]
T = 1e-4; % Sampling period;
% Sample instants in a periodic interval in the time axis
t=0:T:Tmax;

% line style
col='b:.';


% Time-varying discete signal. This signal is not actually periodic, but
% it is assumed to be such with respect to the time interval contained in
% the vector t
a=100;
x = exp(-t).*sinc(a*t).*(t>=0);




%% Plot x over time
figure(1)
clf
plot(t,x,col);

hold on
grid on
xlabel('time [s]')
ylabel('signal (markers represent actual samples)')



%% COMPUTE FT of x(t)
% Remark: FFT computes the Fourier Transform with respect ot the periodic
% interval [0, Fp], where Fp=1/T.
% fftshift shifts the FT in the periodic interval [-Fp/2,Fp/2].

% total number of samples
nsamples = length(x);
% Step in the frequency axis
F = 1./(max(t)-min(t));

% samples in a periodic interval the frequency axis
f = 0:F:(nsamples-1)*F;
% periodic interval across the origin
f= f - max(f)/2;

% Remark:
% The FT of f(t) = exp(-t)*u(t) is 
% F(f) = 1/(1+j*2*pi* f)
% The FT of f(t) = sinc(-a*t) is 
% F(f) = rect(f/a)/a; where rect(x)=1 if |x|<=0.5, and 0 otherwise.
%
% In our case, the FT of x(t)= exp(-t)*sinc(a*t)*u(t) would be the
% convolution of the two functions that, after some algebra, yields the
% following expression

Xtrue = 1/(1i*4*a*pi)*log((1+(2*pi*(f+a/2)).^2)./(1+(2*pi*(f-a/2)).^2))+ 1/(2*pi*a)*(atan(2*pi*(f+a/2))-atan(2*pi*(f-a/2))); 

% We now compute the FFT from the digital samples of the signal
X =fftshift(fft(x))*T;

% We can now compare the two spectra
% Plot module of X(f)
figure(2)
%clf
plot(f,abs(X),col); hold on
% Plot the exact FT of the unconstrained function exp(-t)*sinc(a*t)*u(t)
plot(f,abs(Xtrue),'k')
xlim([-100 100])
xlabel('Frequency [Hz]')
ylabel('|Y|')
hold on
grid on