#include "../inc/SAMFileReader.h"
#include <iostream>
#include <fstream>
#include "../lib/fse/fse.h"
#include <regex>
#include <algorithm>
#include <experimental/filesystem>

SAMFileParser::SAMFileParser(std::string fileName) : m_FileName(fileName)
{
    m_Header.reserve(5000);
    for(int i = 0 ; i < 12 ; ++i) {
        m_SAMFields.reserve(50000);
    }
    m_FieldsCompressed = new char*[12];
    for(int i = 0 ; i < 12 ; ++i)
    {
        m_FieldsCompressed[i] = nullptr;
    }
}

SAMFileParser::~SAMFileParser()
{
    m_FileStream.close();
    for(int i = 0; i < 12 ; i++) {
       delete [] m_FieldsCompressed[i];
    }
    delete [] m_FieldsCompressed;
    delete [] m_HeaderCompressed;
}

const void SAMFileParser::parseFile(void) {
    m_FileStream.open(m_FileName);
    std::string line;
    line.reserve(4000);
    size_t noOfDelimiters = 0;
    while (std::getline(m_FileStream, line))
    {
        if(!isHeader(line)) 
        {
            noOfDelimiters =  std::count_if(line.begin(), line.end(), [](char c){ return c == '\t'; });
            m_AligmentDelimiters[noOfDelimiters] += 1;
            m_AligmentLinesCount += 1;
            splitLine(line, "\t");
        }
        else
        {
            noOfDelimiters = std::count_if(line.begin(), line.end(), [](char c){ return c == '\t'; });
            m_HeaderDelimiters[noOfDelimiters] += 1;
            m_HeaderLinesCount += 1;
            m_Header.append(line + "\r");
        }
    }
}

void SAMFileParser::compress(void)
{
    m_OriginalHeaderDataSize = m_Header.size() + 1;
    size_t headerDestinationCapacity = FSE_compressBound(m_Header.size() + 1);
    m_HeaderCompressed = new char[headerDestinationCapacity];
    m_CompressedHeaderDataSize = FSE_compress(m_HeaderCompressed, headerDestinationCapacity + 1, m_Header.c_str() , m_Header.size() + 1);
    if(FSE_isError(m_CompressedHeaderDataSize))
    {
        std::cout << FSE_getErrorName(m_CompressedHeaderDataSize) << std::endl;
    }
    m_Header.clear();
    m_Header.shrink_to_fit();
    for(size_t i = 0 ; i < m_SAMFields.size() ; ++i)
    {
        m_OriginalFieldsDataSize[i] = m_SAMFields.at(i).size() + 1;
        size_t destinationCapacity = FSE_compressBound(m_SAMFields.at(i).size() + 1);
        m_FieldsCompressed[i] = new char[destinationCapacity];
        m_CompressedFieldsDataSize[i] = FSE_compress(m_FieldsCompressed[i], destinationCapacity, m_SAMFields.at(i).c_str() , m_SAMFields.at(i).size() + 1);
        if(FSE_isError(m_CompressedFieldsDataSize[i]))
        {
            std::cout << FSE_getErrorName(m_CompressedFieldsDataSize[i]) << std::endl;
        }
        m_SAMFields.at(i).clear();
        m_SAMFields.at(i).shrink_to_fit();
    }
}

void SAMFileParser::printCompressionData(std::string outFileName) const
{
    auto originalFileSize = std::experimental::filesystem::file_size(m_FileName);
    auto compressedFileSize = std::experimental::filesystem::file_size(outFileName);
    std::cout << "=========================================" << std::endl;
    auto fileCompressionRatio = ((compressedFileSize) / static_cast<double>(originalFileSize)) * 100;
    std::cout << "Original file size : " << originalFileSize << " bytes"<< std::endl;
    std::cout << "Compressed file size: " <<  compressedFileSize << " bytes" << std::endl;
    std::cout << "File Compression ratio: " << fileCompressionRatio << std::endl;
    std::cout << "=========================================" << std::endl;
    auto headerFileRatio = ((m_OriginalHeaderDataSize) / static_cast<double>(originalFileSize)) * 100;
    auto headerCompressionRatio = ((m_CompressedHeaderDataSize) / static_cast<double>(m_OriginalHeaderDataSize)) * 100;
    auto compressedHeaderFileRatio = ((m_CompressedHeaderDataSize) / static_cast<double>(compressedFileSize)) * 100;
    std::cout << "Header original size: " <<  m_OriginalHeaderDataSize << " bytes" << std::endl;
    std::cout << "Header occupies " << headerFileRatio << " % of the original file size" << std::endl;
    std::cout << "Header compressed size: " <<  m_CompressedHeaderDataSize << " bytes" << std::endl;
    std::cout << "Header occupies " << compressedHeaderFileRatio << " % of the compressed file" << std::endl;
    std::cout << "Header Compression ratio: " << headerCompressionRatio << std::endl;

    for(int i = 0 ; i < 12 ; ++i)
    {
        std::cout << "=========================================" << std::endl;
        auto fieldFileRatio = ((m_OriginalFieldsDataSize[i]) / static_cast<double>(originalFileSize)) * 100;
        auto compressedFieldFileRatio = ((m_CompressedFieldsDataSize[i]) / static_cast<double>(compressedFileSize)) * 100;
        auto fieldCompressionRatio = ((m_CompressedFieldsDataSize[i]) / static_cast<double>(m_OriginalFieldsDataSize[i])) * 100;
        std::cout << "field " << i+1 << ") original size: " <<  m_OriginalFieldsDataSize[i] << " bytes" << std::endl;
        std::cout << "field " << i+1 << ") occupies " << fieldFileRatio << " % of the original file" << std::endl;
        std::cout << "field " << i+1 << ") compressed size: " <<  m_CompressedFieldsDataSize[i] << " bytes" << std::endl;
        std::cout << "field " << i+1 << ") occupies " << compressedFieldFileRatio << " % of the compressed file" << std::endl;
        std::cout << "field " << i+1 << ") Compression ratio: " << fieldCompressionRatio << " %" << std::endl;
    }
}

void SAMFileParser::decompress(void)
{
    char *headerDecompressed;
    headerDecompressed = new char[m_OriginalHeaderDataSize];
    auto dataSize = FSE_decompress(headerDecompressed, m_OriginalHeaderDataSize, m_HeaderCompressed, m_CompressedHeaderDataSize);
    if(FSE_isError(dataSize))
    {
        std::cout << FSE_getErrorName(dataSize) << std::endl;
    }
    m_Header = headerDecompressed;
    delete [] headerDecompressed;
    for(int i = 0 ; i < 12 ; ++i)
    {
        char *fieldDecompressed;
        fieldDecompressed = new char[m_OriginalFieldsDataSize[i]];
        auto dataSize = FSE_decompress(fieldDecompressed, m_OriginalFieldsDataSize[i], m_FieldsCompressed[i], m_CompressedFieldsDataSize[i]);
        if(FSE_isError(dataSize))
        {
            std::cout << FSE_getErrorName(dataSize) << std::endl;
        }
        std::istringstream iss(fieldDecompressed);
        iss.imbue(std::locale(iss.getloc(), new tab_is_not_whitespace));
        m_SAMFieldsSplitted.at(i) = {std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>{}};
        delete [] fieldDecompressed;
    }
}

void SAMFileParser::splitLine(std::string &line, const std::string &delimiter)
{
    std::vector<std::string> columns;
    uint8_t requiredNumber = 11;
    std::string column;
    column.reserve(200);
    std::string::size_type delimiterPosition = 0;
    for(size_t i = 0; i < requiredNumber; ++i)
    {
        delimiterPosition = line.find_first_of(delimiter);
        column = line.substr(0, delimiterPosition);
        columns.push_back(column);
        if(i == 9) // SEQ
        {
            if(column.size() > m_SeqMax)
            {
                m_SeqMax = column.size();
            }
            else if(column.size() < m_SeqMin)
            {
                m_SeqMin = column.size();
            }
        }
        line.erase(0, delimiterPosition + delimiter.length());
    }
    columns.push_back(line);

    if(columns.size() != 12) 
    {
        m_WrongLinesCount += 1;
        return;
    }

    for(int i = 0 ; i < columns.size() ; ++i)
    {
        m_SAMFields.at(i).append(" " + columns.at(i));
    }
}

void SAMFileParser::printSupportiveData(void) const
{
    std::cout << "Lines overall : " << m_AligmentLinesCount + m_HeaderLinesCount << std::endl;
    std::cout << "Header lines count : " << m_HeaderLinesCount << std::endl;
    std::cout << "Aligment lines count : " << m_AligmentLinesCount << std::endl;
    std::cout << "Lines whith not enough required fields : " << m_WrongLinesCount << std::endl;
    std::cout << "==\nHeader\nNumberOfFields\t\tNumberOfLines" << std::endl;
    for(const auto &x : m_HeaderDelimiters)
    {
        std::cout << x.first + 1 << "\t\t\t" << x.second << std::endl;
    }
    std::cout << "==\nAligment\nNumberOfFields\t\tNumberOfLines" << std::endl;
    for(const auto &x : m_AligmentDelimiters)
    {
        std::cout << x.first + 1 << "\t\t\t" << x.second << std::endl;
    }
    std::cout << "Min sequence size : " << m_SeqMin << "bp" << std::endl;
    std::cout << "Max sequence size : " << m_SeqMin << "bp" << std::endl;
}

// ntohl
void SAMFileParser::saveCompressedDataToFile(std::string fileName)
{
  std::ofstream toFile;
  toFile.open (fileName + ".test", std::ios::out | std::ios::app | std::ios::binary);
  toFile.write(reinterpret_cast<const char *>(&m_OriginalHeaderDataSize), sizeof(m_OriginalHeaderDataSize));
  toFile.write(reinterpret_cast<const char *>(&m_OriginalFieldsDataSize[0]), 12*sizeof(size_t));
  toFile.write(reinterpret_cast<const char *>(&m_CompressedHeaderDataSize), sizeof(m_CompressedHeaderDataSize));
  toFile.write(reinterpret_cast<const char *>(&m_CompressedFieldsDataSize[0]), 12*sizeof(size_t));
  toFile.write(m_HeaderCompressed, m_CompressedHeaderDataSize);
  delete [] m_HeaderCompressed;
  m_HeaderCompressed = nullptr;
  
  for(int i = 0 ; i < 12 ; ++i)
  {
    toFile.write(m_FieldsCompressed[i], m_CompressedFieldsDataSize[i]);
    delete [] m_FieldsCompressed[i];
    m_FieldsCompressed[i] = nullptr;
  }
  toFile.close();
  printCompressionData(fileName + ".test");
}

// ntohl
void SAMFileParser::readCompressedDataFromFile(std::string fileName)
{
   std::ifstream inFile;
   inFile.open (fileName + ".test", std::ios::in | std::ios::binary);
   inFile.read(reinterpret_cast<char *>(&m_OriginalHeaderDataSize), sizeof(m_OriginalHeaderDataSize));
   inFile.read(reinterpret_cast<char *>(&m_OriginalFieldsDataSize[0]), 12*sizeof(size_t));
   inFile.read(reinterpret_cast<char *>(&m_CompressedHeaderDataSize), sizeof(m_CompressedHeaderDataSize));
   inFile.read(reinterpret_cast<char *>(&m_CompressedFieldsDataSize[0]), 12*sizeof(size_t));

   m_HeaderCompressed = new char[m_CompressedHeaderDataSize];
   inFile.read(m_HeaderCompressed, m_CompressedHeaderDataSize);

   for(int i = 0 ; i < 12 ; ++i)
   {
    m_FieldsCompressed[i] = new char[m_CompressedFieldsDataSize[i]];
    inFile.read(m_FieldsCompressed[i], m_CompressedFieldsDataSize[i]);
  }
  inFile.close();   
}

void SAMFileParser::recreateFile(void)
{
    std::ofstream toFile;
    toFile.open(m_FileName + ".sam", std::ios::out | std::ios::app);
    std::string::size_type delimiterPosition = 0;
    std::string delimiter = "\r";
    while(true)
    {
        delimiterPosition = m_Header.find_first_of(delimiter);
        if(delimiterPosition == std::string::npos)
        {
            break;
        }
        else
        {
            std::string line = m_Header.substr(0, delimiterPosition);
            m_Header.erase(0, delimiterPosition + delimiter.length());
            if(m_Header.find_first_of(delimiter) != std::string::npos)
            {
                toFile << line << '\n';
            }
            else
            {
                toFile << line;
            }
        }
    }
    toFile << "\n";
    m_Header.clear();
    m_Header.shrink_to_fit();
    bool loopPredicate = true;
    delimiter = " ";
    std::string line;
    line.reserve (100000000);
    for(int j = 0 ; j < m_SAMFieldsSplitted.at(0).size(); ++j)
    {
        for(int i = 0 ; i < 11 ; ++i) {
            line += m_SAMFieldsSplitted.at(i).at(j) + '\t';
        }
        line += m_SAMFieldsSplitted.at(11).at(j) + "\n";
    }
    toFile.write(line.c_str(), line.size());
    toFile.close();   
}