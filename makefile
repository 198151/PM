CXX = g++
CXXFLAGS =  -g -pthread -std=c++17 -DVER=3 -Ofast -march=native -lzstd -lz
INC=-I./inc -I./lib/fse -I./lib/
SRC = ./src
FSEDIR = ./lib/fse

all: SAMFileReader

SAMFileReader: SAMFileReader.o main.o
	$(CXX) -Ofast  -march=native -lzstd -flto -lz -g $(INC) SAMFileReader.o main.o  $(FSEDIR)/entropy_common.c $(FSEDIR)/huf_compress.c $(FSEDIR)/huf_decompress.c $(FSEDIR)/fse_compress.c $(FSEDIR)/fse_decompress.c $(FSEDIR)/hist.c -o  SAMFileReader -lstdc++fs -pthread

main.o: $(SRC)/main.cc
	$(CXX) -g $(CXXFLAGS) -c $(SRC)/main.cc

SAMFileReader.o: $(SRC)/SAMFileReader.cc
	$(CXX) -g $(CXXFLAGS) $(INC) -c $(SRC)/SAMFileReader.cc

clean:
	rm -f *.o