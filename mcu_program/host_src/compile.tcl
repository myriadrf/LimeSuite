rm *.o
g++ -c mcuHexBin.cpp
g++ main2.cpp -o main2 mcuHexBin.o

