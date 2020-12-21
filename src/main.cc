#include "../inc/SAMFileReader.h"
#include <iostream>

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
    if (argc == 2) 
    {
        SAMFileName = *(argv + 1);
    }
    else
    {
        std::cout << "Wrong number of parameters" << std::endl;
    }

    if(!checkiIfFileExists(SAMFileName))
    {
        std::cout << "File not found : " << SAMFileName << std::endl;
        return 0;
    }

    SAMFileParser file(SAMFileName);
    file.parseFile();
    file.compress();
    file.saveCompressedDataToFile("fileTest");
    file.readCompressedDataFromFile("fileTest");
    file.decompress();
    file.recreateFile("out");
    return 0;
}