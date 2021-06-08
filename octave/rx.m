LoadLimeSuite

LimeInitialize()
LimeLoadConfig('rxTest.ini');
samplesToBuffer = 1024*64; %octave has array limit around 20M

batches = 32;
FIFOsize = samplesToBuffer*batches % total number of samples to be buffered inside DLL, they will be contiguous
LimeStartStreaming(FIFOsize);

%must use separate arrays, otherwise octave array index limit will be reached
samplesBatch1 = LimeReceiveSamples(samplesToBuffer);
samplesBatch2 = LimeReceiveSamples(samplesToBuffer);
samplesBatch3 = LimeReceiveSamples(samplesToBuffer);
samplesBatch4 = LimeReceiveSamples(samplesToBuffer);
samplesBatch5 = LimeReceiveSamples(samplesToBuffer);
samplesBatch6 = LimeReceiveSamples(samplesToBuffer);

%or overwrite the same array
for i=1:batches
    samplesBatch = LimeReceiveSamples(samplesToBuffer);
    %do some processing on samplesBatch
end

LimeStopStreaming();
LimeDestroy();

plot(real(samplesBatch1));
%plot(real(samplesBatch2));
%plot(real(samplesBatch3));
