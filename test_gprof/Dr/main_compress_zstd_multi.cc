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

    std::string SAMFileName  = "../../../data/Dracunculus_medinensis_SAMEA1889116.sam";
    std::string OutileName = "HG_OUT";
    
    SAMFileParser file(SAMFileName);
    file.parseFile();
    file.compress_zstd_multithread();
    file.saveCompressedDataToFile(OutileName);
/*
    else
    {
        SAMFileParser file(SAMFileName);
        std::chrono::steady_clock::time_point readBegin = std::chrono::steady_clock::now();
        file.readCompressedDataFromFile(OutileName);
        std::chrono::steady_clock::time_point readEnd = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point decompressBegin = std::chrono::steady_clock::now();
        file.decompress_zstd_multithread();
        std::chrono::steady_clock::time_point decompressEnd = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point recreateBegin = std::chrono::steady_clock::now();
        file.recreateFile();
        std::chrono::steady_clock::time_point recreateend = std::chrono::steady_clock::now();

        std::cout << "Reading: " << std::chrono::duration_cast<std::chrono::milliseconds>(readEnd - readBegin).count() << "[ms]" << std::endl;
        std::cout << "Decompressing: " << std::chrono::duration_cast<std::chrono::milliseconds>(decompressEnd - decompressBegin).count() << "[ms]" << std::endl;
        std::cout << "Recreating: " << std::chrono::duration_cast<std::chrono::milliseconds>(recreateend - recreateBegin).count() << "[ms]" << std::endl;

        std::cout << "All: " << std::chrono::duration_cast<std::chrono::milliseconds>((readEnd - readBegin) + (decompressEnd - decompressBegin) + (recreateend - recreateBegin)).count() << "[ms]" << std::endl;
    }
*/
    return 0;
}
