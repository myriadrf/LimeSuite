clear all;

LoadLimeSuite

%generate test signal
phase = pi/6;           %phase step
periods = 3000;         %periods to generate
src = 0.7*complex(sin(phase:phase:periods*2*pi), cos(phase:phase:periods*2*pi));

LimeInitialize();               % open device
LimeLoadConfig('trxTest.ini');  % load configuration from file
                                % use LimeSuiteGUI to create configuration file
                                
%Real-time sample streaming                                
fifoSize = 1024*1024        %set library FIFO size to 1 MSample
LimeStartStreaming(fifoSize,"tx0"); % start TX to channel 0
for i=1:100
    LimeTransmitSamples(src,0); % send samples to TX channel 0
end
LimeGetStreamStatus()     %must run at least 1s to get data rate (B/s)
sleep(1);
LimeStopStreaming();    % stop streaming

%Waveform playback from device RAM
LimeLoopWFMStart(src);  % Load samples to device RAM, for looping to TX
sleep(10);
LimeLoopWFMStop();      % stop looping TX samples from device RAM

LimeDestroy();          % close device



