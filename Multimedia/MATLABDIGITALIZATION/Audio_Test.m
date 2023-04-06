% Audio_Tests: Audio signal manipulation
%
% load a PCM signal
% Subsample the signal as to mimic a sampling rate equal to
% Fs*[1/2,1/4,1/8] and save the subsampled trace
% Interpolate the subsampled trace with holding interpolation
% Interpolate the subsampled trace with linear interpolation

clear all
close all

% sample rate [samples/sec]
Fs = 16000; % Hertz = 1/s

% num. of bits per sample
N = 16;

% Choose the test you wish to do by setting the corresponding flag
TestSubsampling   =0;
TestInterpolation =0;
TestQuantization  =1;

namefile='myvoicerecording16kHz.mat';
%% SAMPLING PARAMETERS: Sampling frequencies [Hz]
Fsub = [8 4 2 1]*1e3;

%% INTERPOLATION parameters:
% Sampling frequency of the subsampled signal
Fstarting = 2e3;
% Target sampling rate of the interpolated sequence
Fstarget = Fs;

%% QUANTIZATION parameters
% Number of quantization bits
qq = [8 6 4 2 1];


%% RECORD VOICE TRACK (IF NEEDED)

% If the file does not exsit, create your own
if ~exist(namefile,'file')
    % recording... (you need a micro connected to your laptop)
    r = audiorecorder(Fs,N,1);
    disp('Press any key when ready to record...')
    pause
    record(r);     % speak into the microphone...
    disp('Recording... Press any key when done!')
    % the following "pause" instruction is to block the execution of the
    % program while you record your voice. It's exited at key press
    pause
    stop(r); % stop recording
    disp('Well done! Now I''ll playback what you''ve just recorded...')
    y = getaudiodata(r, 'double'); % get data as int16 array
    save(namefile,'Fs','y','N')
    %p = play(r);   % listen
else % If the file exists, load it
    load(namefile,'Fs','y','N')
    disp(['Loading audiotrace at Fs = ',num2str(Fs/1e3), 'KHz'])
end


%% EXTRACT PARAMETERS
% Sampling period
T = 1/Fs;
% number of samples in the audiotrace
M = length(y);
% time interval for the audio trace
t =[0:M-1]*T;
%% Parameters in the frequency domain
% Sample step in the frequency domain
F = 1/(M*T);
% Signal semi-band
B = M/2*F;
% Support of the signal spectrum [Hz], centered in the origin
f = [0:M-1]*F; f = f-B;
% scale to express in kHz
f = f/1e3;
% Module of the signal secptrum (centered in zero)
Y = abs(fftshift(T*fft(y,length(y))));

%% plot signal over time
figure(1)
clf
subplot(2,1,1)
plot(t,y,'b');
xlim([2 4])
xlabel('time [s]')
ylabel('original signal amplitude')

%% plot signal spectrum
figure(2)
clf
subplot(2,1,1)
plot(f,Y);
xlabel('frequency [kHz]')
hold on
xlabel('frequency [kHz]')
ylabel('|Ys|')
drawnow

%% SUBSAMPLING

% Inline function definition. See "help @" and "help function_handle"
mysampling=@(y,step) y(1:step:end);


if TestSubsampling
    %% Subsampling
    
    
    % Playing recorded audio trace
    yp=audioplayer(y,Fs);
    play(yp)
    disp('Press any key to continue')
    pause
    ccolor=char('r','g','m','k','b','y','c','m','k','b','r','g');
    clear yy
    for id=1:length(Fsub)
        d=Fsub(id);

        step = round(Fs/d);
        % Computing subsampled version of the input vector
        ys = mysampling(y,step);
        % and related time and frequency support
        ts = mysampling(t,step);
        fs = linspace(-d/2,d/2,length(f))/1e3;
        YS = abs(fftshift((step*T*fft(ys,length(f)))));
        
        figure(1)
        subplot(2,1,2)
        h=plot(ts,ys,'r.');
        xlim([2 4])
        disp(['Signal sampled at Fs=',num2str(d/1e3),' KHz'])
        xlabel('time [s]')
        ylabel('Sampled signal amplitude')
        
        figure(2)
        subplot(2,1,2)
        h2=plot(fs,YS,'r.');
        xlim([min(f),max(f)])
        disp(['Spectrum of signal sampled at Fs=',num2str(d/1e3),' KHz'])
        xlabel('frequency [kHz]')
        xlabel('frequency [kHz]')
        ylabel('|Ys|')
        drawnow
        
        disp(['Now listen to the recorded audio when subsampled at Fs=[',num2str(d/1e3),'] kHz'])
        yp=audioplayer(ys,d);
        play(yp);        
        % SAVE FILE FOR LATER USE (uncomment if you like)
        %audiowrite(['VoIPAliasing',num2str(round(d/1e3)),'.wav'],ys,d)
        disp('Press any key')
        pause
        
        
        %% PREFILTERING
        % 
        
        % Compute the number of samples up to 10/Fs
        mm = sum(t<10/Fsub(id)); 
        % create the filter impulse response function. Since we want to
        % have an almost rectangualr filter in the frequency domain, it's
        % impulse response should be a sinc in the time domain
        % 1/B*rect(f/B) --> sinc(Bt)
        hh=sinc(Fsub(id)*(t(1:mm)-t(mm)/2));
        % filer the original signal by this bandpass filter
        yh = filter(hh,1,y)/sum(hh);
        
        % subsample the pre-filtered signal
        
        % Computing subsampled version of the input vector
        yhs = mysampling(yh,step);
        % and related time and frequency support
        Yh = abs(fftshift((T*step*fft(yhs,length(f)))));
        
        figure(1)
        subplot(2,1,2)
        h=plot(ts,yhs,'r.');
        
        xlim([2 4])
        disp(['Signal sampled at Fs=',num2str(d/1e3),' KHz'])
        xlabel('time [s]')
        ylabel('Sampled signal amplitude')
        
        figure(2)
        subplot(2,1,2)
        h2=plot(fs,Yh,'k.');
        xlim([min(f),max(f)])
        disp(['Spectrum of signal sampled at Fs=',num2str(d/1e3),' KHz'])
        xlabel('frequency [kHz]')
        xlabel('frequency [kHz]')
        ylabel('|Ys|')
        drawnow
        
         
        disp(['Now listen to the recorded audio when prefiltrered and subsampled at Fs=[',num2str(d/1e3),'] kHz'])
        yph=audioplayer(yhs,d);
        play(yph);        
        % SAVE FILE FOR LATER USE (uncomment if you like)
        %audiowrite(['VoIPAliasing',num2str(round(d/1e3)),'.wav'],ys,d)
        disp('Press any key')
        pause
    end
end


if TestInterpolation
    %%  INTERPOLATION
    
    % Number of samples in the interpoalted trace for each sample of the
    % starting trace
    fact = round(Fstarget./Fstarting);
    
    % Let first subsample the vector (again... just to be sure we start with
    % the right subsampled trace
    ystart = mysampling(y,fact);
    
    
    p=audioplayer(ystart,Fstarting);
    play(p)
    disp('You are now listening to the subsampled audiotrace. Please any key when it is ended...')
    pause
    
    % HOLDING
    % Interpolate the sampled vector by repeating each sample on the new
    % points
    yi = fun_HoldInterp(ystart,Fstarting,Fstarget);
    
    p=audioplayer(yi,Fstarget);
    play(p)
    disp(['Playing out interpolated audio track with holding'])
    
    disp('press any key to continue')
    pause
    
    %%
    % LINEAR INTERPOLATION
    yi = fun_LinearInterp(ystart,Fstarting,Fstarget);
    
    p=audioplayer(yi,Fstarget);
    play(p)
    disp(['Playing out interpolated audio track with linear interpolation'])
    
    disp('press any key to continue')
    pause
    
end

%% QUANTIZATION
if TestQuantization
    
    disp('Now listen to the recorded audio when changing the # of quantization levels')
    ccolor=char('r','g','m','k','b','y','c','m','k','b','r','g');
    clear yy
    for iq=1:length(qq)
        q=qq(iq);
        yq = fun_Quantizer(y,2^q);
        figure(1)
        subplot(2,1,2)
        if exist('h'), delete(h), end
        h=plot(t,yq,'r-.');
        xlabel('time [s]')
        ylabel('Quantized signal')
        disp(['Signal quantized with n =',num2str(q),' bits'])
        
        figure(2)
        subplot(2,1,2)
        if exist('h2'), delete(h2), end
        h2=plot(f,fftshift(T*abs(fft(yq))),'r-.');
        xlabel('frequency [kHz]')
        ylabel('|Yq|')
        disp(['Signal quantized with n =',num2str(q),' bits'])
        
        yp=audioplayer(yq,Fs);
        play(yp);
        
        % SAVE FILE FOR LATER USE
        %audiowrite(['VoIPQuantization',num2str(q),'.wav'],ys,Fs)
        
        disp('Press any key')
        pause
        
    end
end

