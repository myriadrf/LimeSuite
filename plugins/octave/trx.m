clear all;
LoadLimeSuite

% generate test signal
phase = pi/600;       % phase step
periods = 30;         % periods to generate
txSignal = 0.7*complex(sin(phase:phase:periods*2*pi), cos(phase:phase:periods*2*pi));

LimeInitialize();                  % open device

LimeLoadConfig('loop.ini');        % load configuration from file
%LimeLoadConfig('loop_mini.ini');  % load configuration from file
                                   % use LimeSuiteGUI to create configuration file                          
%Real-time sample streaming                                
fifoSize = 1024*1024                         % set library FIFO size to 1 MSample
LimeStartStreaming(fifoSize,["rx0"; "tx0"]); % start TX and RX channel 0

% send and receive samples to/from channel 0 
% discard first 30K samples in Rx
rxSignal1 = LimeTransceiveSamples(txSignal, 30000); 

for i=1:16
    rxSignal2 = LimeTransceiveSamples(txSignal); % send and receive samples to/from channel 0 
end
figure 1
plot(real(rxSignal1));    %plot first rx
figure 2
plot(real(rxSignal2));    %plot last rx
LimeGetStreamStatus()     %must run at least 1s to get data rate (B/s)
sleep(1);
LimeStopStreaming();      % stop streaming
LimeDestroy();            % close device



