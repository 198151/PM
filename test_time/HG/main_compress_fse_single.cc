#include "../../inc/SAMFileReader.h"
#include <iostream>
#include <chrono>
#include <string>

inline bool checkiIfFileExists(const std::string& fileName) 
{
    FILE *file = NULL;
    if (file = fopen(fileName.c_str(), "rb")) 
    {
        fclose(file);
        return true;
    } 
    return false;
}

int main(int argc, char** argv) {

    std::string SAMFileName  = "../../../data/HG00096.chrom20.ILLUMINA.bwa.GBR.low_coverage.20120522.sam";
    std::string OutileName = "HG_OUT";
    
    SAMFileParser file(SAMFileName);
    std::chrono::steady_clock::time_point readBegin = std::chrono::steady_clock::now();
    file.parseFile_2();
    std::chrono::steady_clock::time_point readEnd = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point compressionBegin = std::chrono::steady_clock::now();
    file.compress_fse();
    std::chrono::steady_clock::time_point compressionEnd = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point saveBegin = std::chrono::steady_clock::now();
    file.saveCompressedDataToFile(OutileName);
    std::chrono::steady_clock::time_point saveEnd = std::chrono::steady_clock::now();
    
    std::cout << "Parsing: " << std::chrono::duration_cast<std::chrono::milliseconds>(readEnd - readBegin).count() << "[ms]\n";
    std::cout << "Compressing: " << std::chrono::duration_cast<std::chrono::milliseconds>(compressionEnd - compressionBegin).count() << "[ms]\n";
    std::cout << "Saving: " << std::chrono::duration_cast<std::chrono::milliseconds>(saveEnd - saveBegin).count() << "[ms]\n";
    std::cout << "All: " << std::chrono::duration_cast<std::chrono::milliseconds>((readEnd - readBegin) + (compressionEnd - compressionBegin) + (saveEnd - saveBegin)).count() << "[ms]\n";
    return 0;
}
