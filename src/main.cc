#include "../inc/SAMFileReader.h"
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

    std::string SAMFileName;
    std::string OutileName;
    if (argc == 4) 
    {
        if(std::string(argv[1]) == "-c")
        {
            SAMFileName = argv[2];
            OutileName = argv[3];
        }
        else if(std::string(argv[1]) == "-d")
        {
            SAMFileName = argv[3];
            OutileName = argv[2];        
        }
        else
        {
            std::cout << "Wrong option : " << argv[1] << std::endl;
            return 0;
        }
    }
    else
    {
        std::cout << "Wrong number of parameters" << std::endl;
    }
    
    /*
    if(!checkiIfFileExists(SAMFileName))
    {
        std::cout << "File not found : " << SAMFileName << std::endl;
        return 0;
    }

    if(!checkiIfFileExists(OutileName))
    {
        std::cout << "File not found : " << OutileName << std::endl;
        return 0;
    }
    */

    if(std::string(argv[1]) == "-c")
    {
        SAMFileParser file(SAMFileName);
        std::chrono::steady_clock::time_point parseBegin = std::chrono::steady_clock::now();
        file.parseFile();
        std::chrono::steady_clock::time_point parseEnd = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point compressBegin = std::chrono::steady_clock::now();
        file.compress();
        std::chrono::steady_clock::time_point compressEnd = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point saveBegin = std::chrono::steady_clock::now();
        file.saveCompressedDataToFile(OutileName);
        std::chrono::steady_clock::time_point saveEnd = std::chrono::steady_clock::now();

        std::cout << "Parsing: " << std::chrono::duration_cast<std::chrono::milliseconds>(parseEnd - parseBegin).count() << "[ms]" << std::endl;
        std::cout << "Compressing: " << std::chrono::duration_cast<std::chrono::milliseconds>(compressEnd - compressBegin).count() << "[ms]" << std::endl;
        std::cout << "Saving: " << std::chrono::duration_cast<std::chrono::milliseconds>(saveEnd - saveBegin).count() << "[ms]" << std::endl;

        std::cout << "All: " << std::chrono::duration_cast<std::chrono::milliseconds>((saveEnd - saveBegin) + (parseEnd - parseBegin) + (compressEnd - compressBegin)).count() << "[ms]" << std::endl;

    }
    else
    {
        SAMFileParser file(SAMFileName);
        std::chrono::steady_clock::time_point readBegin = std::chrono::steady_clock::now();
        file.readCompressedDataFromFile(OutileName);
        std::chrono::steady_clock::time_point readEnd = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point decompressBegin = std::chrono::steady_clock::now();
        file.decompress();
        std::chrono::steady_clock::time_point decompressEnd = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point recreateBegin = std::chrono::steady_clock::now();
        file.recreateFile();
        std::chrono::steady_clock::time_point recreateend = std::chrono::steady_clock::now();

        std::cout << "Reading: " << std::chrono::duration_cast<std::chrono::milliseconds>(readEnd - readBegin).count() << "[ms]" << std::endl;
        std::cout << "Decompressing: " << std::chrono::duration_cast<std::chrono::milliseconds>(decompressEnd - decompressBegin).count() << "[ms]" << std::endl;
        std::cout << "Recreating: " << std::chrono::duration_cast<std::chrono::milliseconds>(recreateend - recreateBegin).count() << "[ms]" << std::endl;

        std::cout << "All: " << std::chrono::duration_cast<std::chrono::milliseconds>((readEnd - readBegin) + (decompressEnd - decompressBegin) + (recreateend - recreateBegin)).count() << "[ms]" << std::endl;
    }

    return 0;
}