%imports shared library functions
LoadLimeSuite

%Generate waveform
src = 0.7*complex(sin(0:pi/4:1000*pi-pi/4), cos(0:pi/4:1000*pi-pi/4));

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
LimeStartStreaming();

%Load waveform to be continuosly transmitted
LimeLoopWFMStart(src);

%receive samples
for i=1:1000
	samples = LimeReceiveSamples(1360);
	plot(samples);
	disp(i);
	pause(0.001);
end

%stop streaming
LimeLoopWFMStop();
LimeStopStreaming(); %also resets Rx Tx buffers



%deallocate resources
LimeDestroy();
