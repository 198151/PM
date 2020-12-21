#include "../inc/SAMFileReader.h"
#include <iostream>
#include <fstream>
#include "../lib/fse/fse.h"
#include <regex>
#include <algorithm>

SAMFileParser::SAMFileParser(std::string fileName) : m_FileName(fileName)
{
    m_FileStream.open(m_FileName);
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
    printSupportiveData();
}

void SAMFileParser::compress(void)
{
    m_OriginalHeaderDataSize = m_Header.size() + 1;
    std::cout << "Header Original Size : " << m_Header.size() + 1 << std::endl;
    size_t headerDestinationCapacity = FSE_compressBound(m_Header.size() + 1);
    m_HeaderCompressed = new char[headerDestinationCapacity];
    m_CompressedHeaderDataSize = FSE_compress(m_HeaderCompressed, headerDestinationCapacity + 1, m_Header.c_str() , m_Header.size() + 1);
    std::cout << "Header: => size : ";
    if(FSE_isError(m_CompressedHeaderDataSize))
    {
        std::cout << FSE_getErrorName(m_CompressedHeaderDataSize) << std::endl;
    }
    else
    {
        std::cout << m_CompressedHeaderDataSize << std::endl;
    }
    m_Header.clear();
    m_Header.shrink_to_fit();
    for(size_t i = 0 ; i < m_SAMFields.size() ; ++i)
    {
        m_OriginalFieldsDataSize[i] = m_SAMFields.at(i).size() + 1;
        size_t destinationCapacity = FSE_compressBound(m_SAMFields.at(i).size() + 1);
        m_FieldsCompressed[i] = new char[destinationCapacity];
        m_CompressedFieldsDataSize[i] = FSE_compress(m_FieldsCompressed[i], destinationCapacity, m_SAMFields.at(i).c_str() , m_SAMFields.at(i).size() + 1);
        std::cout << "field no: " << i+1 << " => size : "; 
        if(FSE_isError(m_CompressedFieldsDataSize[i]))
        {
            std::cout << FSE_getErrorName(m_CompressedFieldsDataSize[i]) << std::endl;
        }
        else
        {
            std::cout << m_CompressedFieldsDataSize[i] << std::endl;
        }
        m_SAMFields.at(i).clear();
        m_SAMFields.at(i).shrink_to_fit();
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
        m_SAMFields[i] = fieldDecompressed;
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

void SAMFileParser::saveCompressedDataToFile(std::string fileName)
{
  std::ofstream toFile;
  toFile.open (fileName + ".test", std::ios::out | std::ios::app | std::ios::binary);
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
}

void SAMFileParser::readCompressedDataFromFile(std::string fileName)
{
   std::ifstream inFile;
   inFile.open (fileName + ".test", std::ios::in | std::ios::binary);
   m_HeaderCompressed = new char[m_CompressedHeaderDataSize];
   inFile.read(m_HeaderCompressed, m_CompressedHeaderDataSize);
   for(int i = 0 ; i < 12 ; ++i)
   {
    m_FieldsCompressed[i] = new char[m_CompressedFieldsDataSize[i]];
    inFile.read(m_FieldsCompressed[i], m_CompressedFieldsDataSize[i]);
  }
  inFile.close();   
}

void SAMFileParser::recreateFile(std::string fileName)
{
    std::ofstream toFile;
    toFile.open(fileName + ".sam", std::ios::out | std::ios::app);
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
    m_Header.clear();
    m_Header.shrink_to_fit();
    bool loopPredicate = true;
    delimiter = " ";
    while(loopPredicate)
    {
        for(int i = 0 ; i < 12 ; ++i)
        {
          delimiterPosition = m_SAMFields.at(i).find_first_of(delimiter);
          if(delimiterPosition == std::string::npos)
          {
              loopPredicate = false;
              break;
          }
          if(i != 11)
          {
            toFile << m_SAMFields.at(i).substr(0, delimiterPosition) << '\t';
          }
          else
          {
            toFile << m_SAMFields.at(i).substr(0, delimiterPosition) << '\n';
          }
          m_SAMFields.at(i).erase(0, delimiterPosition + delimiter.length());
        }
    }
    toFile.close();   
}
