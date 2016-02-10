import matplotlib.pyplot as plt
import numpy as np
import sys

samps = np.fromfile(sys.argv[1], np.float32)
timeScale = 1000/4333333.333333333

t = np.arange(0.0, timeScale*samps.size, timeScale)
s = samps#np.sin(2*np.pi*t)
plt.plot(t, s)

plt.xlabel('time (ms)')
plt.ylabel('Amplitude')
#plt.title('About as simple as it gets, folks')
plt.grid(True)
#plt.savefig("test.png")
plt.show()
