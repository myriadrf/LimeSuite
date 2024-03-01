%imports shared library functions
LoadLimeSuite

%Generate waveform
phase = pi/8;
src = 0.7*complex(sin(0:phase:1000*pi-phase), cos(0:phase:1000*pi-phase));

%%initialize LimeSuite
%if several boards are connected
%deviceList = LimeGetDeviceList();
%selectedDevice = 1;
%LimeInitialize(deviceList(selectedDevice));
%otherwise initialization uses first available device
LimeInitialize();

%load initial chip configuration
LimeLoadConfig('rxTest.ini');

%activate Tx and Rx streaming
LimeStartStreaming(1000000);

%Load waveform to be continuosly transmitted
LimeLoopWFMStart(src);

%receive samples
for i=1:10000
	samples = LimeReceiveSamples(1360);
	%plot(samples);
	pause(0.001);
end

%stop streaming
LimeLoopWFMStop();
LimeStopStreaming(); %also resets Rx Tx buffers



%deallocate resources
LimeDestroy();
