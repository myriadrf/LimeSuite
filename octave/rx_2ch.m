clear all;

LoadLimeSuite

LimeInitialize();               % open first device

LimeLoadConfig('trxTest.ini');  % load configuration from file
                                % use LimeSuiteGUI to create configuration file

readCnt = 1024*64;          %samples to read (64K)
fifoSize = 1024*1024        %set library FIFO size to 1 MSample

LimeStartStreaming(fifoSize,["rx0"; "rx1"]); % start RX from channels 0 and 1

%receive samples, overwrite the same array
for i=1:40
    samplesCh0 = LimeReceiveSamples(readCnt,0); % read samples from RX channel 0
    samplesCh1 = LimeReceiveSamples(readCnt,1); % read samples from RX channel 1
end
LimeGetStreamStatus()     %must run at least 1s to get data rate (B/s)
%stop streaming
LimeStopStreaming();      % stop streaming
LimeDestroy();            % close device
%plot samples
figure(1)
plot(real(samplesCh0));
figure(2)
plot(real(samplesCh1));

