DPDcontrol: DPDcontrol.cpp DPDcomm.o CFRcomm.o qadpd.o nrc.o
	g++ $(CFLAGS) DPDcontrol.cpp -I ./../protocols -I ./../lms7002m -I ./../lime -I ./.. -I ./../ConnectionRegistry -I /usr/local/lib/ -lLimeSuite -lm -o DPDcontrol DPDcomm.o CFRcomm.o qadpd.o nrc.o

DPDcomm.o: DPDcomm.cpp DPDcomm.h
	g++ $(CFLAGS) -c DPDcomm.cpp -I ./../protocols -I ./../lms7002m -I ./../lime -I ./.. -I ./../ConnectionRegistry -lLimeSuite  

CFRcomm.o: CFRcomm.cpp CFRcomm.h 
	g++ $(CFLAGS) -c CFRcomm.cpp -I ./../protocols -I ./../lms7002m -I ./../lime -I ./.. -I ./../ConnectionRegistry -lLimeSuite 

nrc.o: nrc.cpp nrc.h
	g++ $(CFLAGS) -c nrc.cpp 

qadpd.o: qadpd.cpp qadpd.h
	g++ $(CFLAGS) -c qadpd.cpp	

clean: 
	rm -f *.o DPDcontrol