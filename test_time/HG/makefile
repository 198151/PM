CXX = g++
CXXFLAGS =  -g -pthread -std=c++17 -DVER=3 -Ofast -march=native -lzstd -lz
INC=-I../../inc -I../../lib/fse -I../../lib/
SRC = ../../src
FSEDIR = ../../lib/fse

all: SAMFileReader_zstd_compress_multi SAMFileReader_fse_compress_multi SAMFileReader_gzip_compress_multi SAMFileReader_zstd_compress_single SAMFileReader_fse_compress_single SAMFileReader_gzip_compress_single SAMFileReader_zstd_decompress_multi SAMFileReader_fse_decompress_multi SAMFileReader_gzip_decompress_multi SAMFileReader_zstd_decompress_single SAMFileReader_fse_decompress_single SAMFileReader_gzip_decompress_single

SAMFileReader_zstd_compress_multi: SAMFileReader.o main_compress_zstd_multi.o
	$(CXX) -Ofast  -march=native -lzstd -flto -lz -g $(INC) SAMFileReader.o main_compress_zstd_multi.o  $(FSEDIR)/entropy_common.c $(FSEDIR)/huf_compress.c $(FSEDIR)/huf_decompress.c $(FSEDIR)/fse_compress.c $(FSEDIR)/fse_decompress.c $(FSEDIR)/hist.c -o  SAMFileReader_zstd_compress_multi -lstdc++fs -pthread
	
SAMFileReader_fse_compress_multi: SAMFileReader.o main_compress_fse_multi.o
	$(CXX) -Ofast  -march=native -lzstd -flto -lz -g $(INC) SAMFileReader.o main_compress_fse_multi.o  $(FSEDIR)/entropy_common.c $(FSEDIR)/huf_compress.c $(FSEDIR)/huf_decompress.c $(FSEDIR)/fse_compress.c $(FSEDIR)/fse_decompress.c $(FSEDIR)/hist.c -o  SAMFileReader_fse_compress_multi -lstdc++fs -pthread
	
SAMFileReader_gzip_compress_multi: SAMFileReader.o main_compress_gzip_multi.o
	$(CXX) -Ofast  -march=native -lzstd -flto -lz -g $(INC) SAMFileReader.o main_compress_gzip_multi.o  $(FSEDIR)/entropy_common.c $(FSEDIR)/huf_compress.c $(FSEDIR)/huf_decompress.c $(FSEDIR)/fse_compress.c $(FSEDIR)/fse_decompress.c $(FSEDIR)/hist.c -o  SAMFileReader_gzip_compress_multi -lstdc++fs -pthread
	
SAMFileReader_zstd_compress_single: SAMFileReader.o main_compress_zstd_single.o
	$(CXX) -Ofast  -march=native -lzstd -flto -lz -g $(INC) SAMFileReader.o main_compress_zstd_single.o  $(FSEDIR)/entropy_common.c $(FSEDIR)/huf_compress.c $(FSEDIR)/huf_decompress.c $(FSEDIR)/fse_compress.c $(FSEDIR)/fse_decompress.c $(FSEDIR)/hist.c -o  SAMFileReader_zstd_compress_single -lstdc++fs -pthread

SAMFileReader_fse_compress_single: SAMFileReader.o main_compress_fse_single.o
	$(CXX) -Ofast  -march=native -lzstd -flto -lz -g $(INC) SAMFileReader.o main_compress_fse_single.o  $(FSEDIR)/entropy_common.c $(FSEDIR)/huf_compress.c $(FSEDIR)/huf_decompress.c $(FSEDIR)/fse_compress.c $(FSEDIR)/fse_decompress.c $(FSEDIR)/hist.c -o  SAMFileReader_fse_compress_single -lstdc++fs -pthread
	
SAMFileReader_gzip_compress_single: SAMFileReader.o main_compress_gzip_single.o
	$(CXX) -Ofast  -march=native -lzstd -flto -lz -g $(INC) SAMFileReader.o main_compress_gzip_single.o  $(FSEDIR)/entropy_common.c $(FSEDIR)/huf_compress.c $(FSEDIR)/huf_decompress.c $(FSEDIR)/fse_compress.c $(FSEDIR)/fse_decompress.c $(FSEDIR)/hist.c -o  SAMFileReader_gzip_compress_single -lstdc++fs -pthread

SAMFileReader_zstd_decompress_multi: SAMFileReader.o main_decompress_zstd_multi.o
	$(CXX) -Ofast  -march=native -lzstd -flto -lz -g $(INC) SAMFileReader.o main_decompress_zstd_multi.o  $(FSEDIR)/entropy_common.c $(FSEDIR)/huf_compress.c $(FSEDIR)/huf_decompress.c $(FSEDIR)/fse_compress.c $(FSEDIR)/fse_decompress.c $(FSEDIR)/hist.c -o  SAMFileReader_zstd_decompress_multi -lstdc++fs -pthread
	
SAMFileReader_fse_decompress_multi: SAMFileReader.o main_decompress_fse_multi.o
	$(CXX) -Ofast  -march=native -lzstd -flto -lz -g $(INC) SAMFileReader.o main_decompress_fse_multi.o  $(FSEDIR)/entropy_common.c $(FSEDIR)/huf_compress.c $(FSEDIR)/huf_decompress.c $(FSEDIR)/fse_compress.c $(FSEDIR)/fse_decompress.c $(FSEDIR)/hist.c -o  SAMFileReader_fse_decompress_multi -lstdc++fs -pthread
	
SAMFileReader_gzip_decompress_multi: SAMFileReader.o main_decompress_gzip_multi.o
	$(CXX) -Ofast  -march=native -lzstd -flto -lz -g $(INC) SAMFileReader.o main_decompress_gzip_multi.o  $(FSEDIR)/entropy_common.c $(FSEDIR)/huf_compress.c $(FSEDIR)/huf_decompress.c $(FSEDIR)/fse_compress.c $(FSEDIR)/fse_decompress.c $(FSEDIR)/hist.c -o  SAMFileReader_gzip_decompress_multi -lstdc++fs -pthread
	
SAMFileReader_zstd_decompress_single: SAMFileReader.o main_decompress_zstd_single.o
	$(CXX) -Ofast  -march=native -lzstd -flto -lz -g $(INC) SAMFileReader.o main_decompress_zstd_single.o  $(FSEDIR)/entropy_common.c $(FSEDIR)/huf_compress.c $(FSEDIR)/huf_decompress.c $(FSEDIR)/fse_compress.c $(FSEDIR)/fse_decompress.c $(FSEDIR)/hist.c -o  SAMFileReader_zstd_decompress_single -lstdc++fs -pthread
	
SAMFileReader_fse_decompress_single: SAMFileReader.o main_decompress_fse_single.o
	$(CXX) -Ofast  -march=native -lzstd -flto -lz -g $(INC) SAMFileReader.o main_decompress_fse_single.o  $(FSEDIR)/entropy_common.c $(FSEDIR)/huf_compress.c $(FSEDIR)/huf_decompress.c $(FSEDIR)/fse_compress.c $(FSEDIR)/fse_decompress.c $(FSEDIR)/hist.c -o  SAMFileReader_fse_decompress_single -lstdc++fs -pthread
	
SAMFileReader_gzip_decompress_single: SAMFileReader.o main_decompress_gzip_single.o
	$(CXX) -Ofast  -march=native -lzstd -flto -lz -g $(INC) SAMFileReader.o main_decompress_gzip_single.o  $(FSEDIR)/entropy_common.c $(FSEDIR)/huf_compress.c $(FSEDIR)/huf_decompress.c $(FSEDIR)/fse_compress.c $(FSEDIR)/fse_decompress.c $(FSEDIR)/hist.c -o  SAMFileReader_gzip_decompress_single -lstdc++fs -pthread

main_compress_fse_multi.o: main_compress_fse_multi.cc
	$(CXX) -g $(CXXFLAGS) -c main_compress_fse_multi.cc
	
main_compress_zstd_multi.o: main_compress_zstd_multi.cc
	$(CXX) -g $(CXXFLAGS) -c main_compress_zstd_multi.cc
	
main_compress_gzip_multi.o: main_compress_gzip_multi.cc
	$(CXX) -g $(CXXFLAGS) -c main_compress_gzip_multi.cc
	
main_compress_fse_single.o: main_compress_fse_single.cc
	$(CXX) -g $(CXXFLAGS) -c main_compress_fse_single.cc
	
main_compress_zstd_single.o: main_compress_zstd_single.cc
	$(CXX) -g $(CXXFLAGS) -c main_compress_zstd_single.cc
	
main_compress_gzip_single.o: main_compress_gzip_single.cc
	$(CXX) -g $(CXXFLAGS) -c main_compress_gzip_single.cc
	
main_decompress_fse_multi.o: main_decompress_fse_multi.cc
	$(CXX) -g $(CXXFLAGS) -c main_decompress_fse_multi.cc
	
main_decompress_zstd_multi.o: main_decompress_zstd_multi.cc
	$(CXX) -g $(CXXFLAGS) -c main_decompress_zstd_multi.cc
	
main_decompress_gzip_multi.o: main_decompress_gzip_multi.cc
	$(CXX) -g $(CXXFLAGS) -c main_decompress_gzip_multi.cc
	
main_decompress_fse_single.o: main_decompress_fse_single.cc
	$(CXX) -g $(CXXFLAGS) -c main_decompress_fse_single.cc
	
main_decompress_zstd_single.o: main_decompress_zstd_single.cc
	$(CXX) -g $(CXXFLAGS) -c main_decompress_zstd_single.cc
	
main_decompress_gzip_single.o: main_decompress_gzip_single.cc
	$(CXX) -g $(CXXFLAGS) -c main_decompress_gzip_single.cc

SAMFileReader.o: $(SRC)/SAMFileReader.cc
	$(CXX) -g $(CXXFLAGS) $(INC) -c $(SRC)/SAMFileReader.cc

clean:
	rm -f *.o
