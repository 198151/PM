CXX = g++
CXXFLAGS = -O3 -g -std=c++17
INC=-I./inc -I./lib/fse
SRC = ./src
FSEDIR = ./lib/fse

all: SAMFileReader

SAMFileReader: SAMFileReader.o main.o
	$(CXX) -O3 -g $(INC) SAMFileReader.o main.o $(FSEDIR)/entropy_common.c $(FSEDIR)/fse_compress.c $(FSEDIR)/fse_decompress.c $(FSEDIR)/hist.c -o  SAMFileReader -lstdc++fs

main.o: $(SRC)/main.cc
	$(CXX) -g $(CXXFLAGS) -c $(SRC)/main.cc

SAMFileReader.o: $(SRC)/SAMFileReader.cc
	$(CXX) -g $(CXXFLAGS) -c $(SRC)/SAMFileReader.cc

clean:
	rm -f *.o