#include "../inc/SAMFileReader.h"
#include <iostream>
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
    m_FieldsCompressed = new void*[12];
}

SAMFileParser::~SAMFileParser()
{
    m_FileStream.close();
    for(int i = 0; i < 12 ; i++) {
       operator delete(m_FieldsCompressed[i]);
    }
    delete [] m_FieldsCompressed;
    operator delete(m_HeaderCompressed);
}

const void SAMFileParser::parseFile() {
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
            m_Header.append(line);
        }
    }
    printSupportiveData();
}

void SAMFileParser::compress(void)
{
    size_t headerDestinationCapacity = FSE_compressBound(m_Header.size() + 1);
    m_HeaderCompressed = operator new(headerDestinationCapacity);
    auto headerDataSize = FSE_compress(m_HeaderCompressed, headerDestinationCapacity, m_Header.c_str() , m_Header.size() + 1);
    std::cout << "Header: => size : ";
    if(FSE_isError(headerDataSize))
    {
        std::cout << FSE_getErrorName(headerDataSize) << std::endl;
    }
    else
    {
        std::cout << headerDataSize << std::endl;
    }

    for(size_t i = 0 ; i < m_SAMFields.size() ; ++i)
    {
        size_t destinationCapacity = FSE_compressBound(m_SAMFields.at(i).size() + 1);
        m_FieldsCompressed[i] = operator new(destinationCapacity);
        auto dataSize = FSE_compress(m_FieldsCompressed[i], destinationCapacity, m_SAMFields.at(i).c_str() , m_SAMFields.at(i).size() + 1);
        std::cout << "field no: " << i+1 << " => size : "; 
        if(FSE_isError(dataSize))
        {
            std::cout << FSE_getErrorName(dataSize) << std::endl;
        }
        else
        {
            std::cout << dataSize << std::endl;
        }
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