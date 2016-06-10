LoadLimeSuite

src = 0.7*complex(sin(0:pi/4:1000*pi-pi/4), cos(0:pi/4:1000*pi-pi/4));

LimeInitialize()
LimeLoadConfig('rxTest.ini');
LimeStartStreaming();

	samples = LimeReceiveSamples(1360*16);

LimeStopStreaming();
