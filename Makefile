CFLAGS = -Wall -Wextra -Wno-write-strings -DSWAP_BYTES \
         -fdiagnostics-show-option $(curl-config --cflags) 

all: triggercounter

triggercounter: triggercounter.o PZdabFile.o PZdabWriter.o MD5Checksum.o
	g++ $(CFLAGS) -o triggercounter triggercounter.o PZdabFile.o PZdabWriter.o MD5Checksum.o

caencounter: caencounter.o PZdabFile.o PZdabWriter.o MD5Checksum.o
	g++ $(CFLAGS) -o caencounter caencounter.o PZdabFile.o PZdabWriter.o MD5Checksum.o

triggercounter.o: triggercounter.cpp struct.h
	g++ -c triggercounter.cpp $(CFLAGS)

caencounter.o: caencounter.cpp struct.h
	g++ -c caencounter.cpp $(CFLAGS)

PZdabFile.o: PZdabFile.cxx
	g++ -c PZdabFile.cxx $(CFLAGS) 

PZdabWriter.o: PZdabWriter.cxx
	g++ -c PZdabWriter.cxx $(CFLAGS) 

MD5Checksum.o: MD5Checksum.cxx
	g++ -c MD5Checksum.cxx $(CFLAGS)

clean:
	rm -f triggercounter triggercounter.o PZdabFile.o PZdabWriter.o MD5Checksum.o
