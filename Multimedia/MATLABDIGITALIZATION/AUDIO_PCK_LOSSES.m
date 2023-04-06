% Audio_Tests: Audio signal manipulation
%
% 1. Record a voice track
% 2. Indicate a packet loss probability
% 3. Here the sound of the voice recording with such losses

clear all
close all
% Default file to store the recorded voice track
namefile='myvoicerecording.mat';
% sample rate [samples/sec]
Fs = 8000;
% num. of bits per sample
N = 16;

chunktime = 30e-3; % duration of an audio chunk that goes in one single packet
pcksize = Fs*chunktime; %size of the packet payload, in # of samples of N bits each

% Choose with the test you wish to do by setting the corresponding flag
TestSubsampling = 1;
TestInterpolation =0;
TestQuantization =0;

% Target sampling rate of the interpolated sequence
Fstarget = 8e3;


%% RECORD VOICE TRACK (IF NEEDED)
% If the file does not exsit, create your own
if ~exist(namefile,'file')
    % recording... (you need a micro connected to your laptop)
    r = audiorecorder(Fs,N,1);
    disp('Press any key when ready to record');
    pause
    record(r);     % speak into microphone...
    
    disp('Stop the recording when you are done');
    pause
    stop(r);
    disp('Well done! Now I''ll playback what you''ve just recorded...')
    y = getaudiodata(r, 'double'); % get data as int16 array
    save(namefile)
    p = play(r);   % listen
else % If the file exswits, load it
    load(namefile)
    disp(['Loading pre-recorded audio trace'])
end


%% EXTRACT PARAMETERS
% Sampling period
T = 1/Fs;
% number of samples in the audiotrace
M = length(y);
% time interval for the audio trace
t =[0:M-1]*T;
% Parameters in the frequency domain
F = 1/(M*T);

% number of audio packets
npcks = floor(t(end)/chunktime);

%% Generating random packet losses
tryagain=1;
while tryagain
    %% Subsampling
    rep=1;
    while rep
        ploss = input('Specify packet loss probability from 0% to 100%: ');
        if ploss>100 || ploss<0, disp('The value must be in the interval [0,100]! Try again'), else rep=0; end
    end
    ploss=ploss/100;
    
    % generate random losses
    pckerrors = rand(1,npcks)>ploss;
    % cancel out lost samples
    sampleerrors =ones(ceil(pcksize),1) * pckerrors;
    sampleerrors=sampleerrors(:);
    yserror = y(1:length(sampleerrors)).*sampleerrors;
    yp=audioplayer(yserror,Fs);
    play(yp);
    pause(t(end)*0.9)
    % SAVE FILE FOR LATER USE (uncomment if you like)
    %audiowrite(['VoIPAliasing',num2str(round(d/1e3)),'.wav'],ys,d)
    tagain = input('Do you want to try again (Y/n)?','s');
    tryagain=not(strcmpi(tagain,'N')); % tryagain is 1 if the user types anything other than 'n'
end


