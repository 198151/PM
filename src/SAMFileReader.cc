#include "../inc/SAMFileReader.h"
#include <iostream>
#include <fstream>
#include "../lib/fse/fse.h"
#include "../lib/fse/huf.h"
#include <regex>
#include <algorithm>
#include <thread>
#include <future>
#include <experimental/filesystem>
#include <numeric>
#include <zstd.h>


#include <gzip/compress.hpp>
#include <gzip/config.hpp>
#include <gzip/decompress.hpp>
#include <gzip/utils.hpp>
#include <gzip/version.hpp>

SAMFileParser::SAMFileParser(std::string fileName) : m_FileName(fileName)
{
    m_Header.reserve(5000);
    for(int i = 0 ; i < 12 ; ++i) {
        m_SAMFields.reserve(50000);
    }

    experimantal_FieldsCompressed = new char*[13];
    for(int i = 0 ; i < 13 ; ++i)
    {
        experimantal_FieldsCompressed[i] = nullptr;
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

void SAMFileParser::compress_fse(void)
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

void SAMFileParser::compress_zstd(void)
{
    m_OriginalHeaderDataSize = m_Header.size() + 1;
    size_t headerDestinationCapacity = ZSTD_compressBound(m_Header.size() + 1);
    m_HeaderCompressed = new char[headerDestinationCapacity];
    m_CompressedHeaderDataSize = ZSTD_compress(m_HeaderCompressed, headerDestinationCapacity + 1, m_Header.c_str() , m_Header.size() + 1, 11);
    if(ZSTD_isError(m_CompressedHeaderDataSize))
    {
        std::cout << ZSTD_getErrorName(m_CompressedHeaderDataSize) << std::endl;
    }
    m_Header.clear();
    m_Header.shrink_to_fit();
    for(size_t i = 0 ; i < m_SAMFields.size() ; ++i)
    {
        m_OriginalFieldsDataSize[i] = m_SAMFields.at(i).size() + 1;
        size_t destinationCapacity = ZSTD_compressBound(m_SAMFields.at(i).size() + 1);
        m_FieldsCompressed[i] = new char[destinationCapacity];
        m_CompressedFieldsDataSize[i] = ZSTD_compress(m_FieldsCompressed[i], destinationCapacity, m_SAMFields.at(i).c_str() , m_SAMFields.at(i).size() + 1, 11);
        if(ZSTD_isError(m_CompressedFieldsDataSize[i]))
        {
            std::cout << ZSTD_getErrorName(m_CompressedFieldsDataSize[i]) << std::endl;
        }
        m_SAMFields.at(i).clear();
        m_SAMFields.at(i).shrink_to_fit();
    }
}

void SAMFileParser::compressionThread_zstd(int index)
{
    size_t destinationCapacity = 0;
    const char * data = nullptr;
    size_t dataSize = 0;
    if(index == -1) 
    {
        this->m_OriginalHeaderDataSize = this->m_Header.size() + 1;
        destinationCapacity = ZSTD_compressBound(m_Header.size() + 1);
        data = this->m_Header.c_str();
        dataSize = this->m_OriginalHeaderDataSize;
    }
    else
    {
        this->m_OriginalFieldsDataSize[index] = m_SAMFields.at(index).size() + 1;
        destinationCapacity = ZSTD_compressBound(m_SAMFields.at(index).size() + 1);
        data = this->m_SAMFields.at(index).c_str();
        dataSize = this->m_OriginalFieldsDataSize[index];
    }

    char * compressed_data = new char[destinationCapacity];
    auto compressedDataSize = ZSTD_compress(compressed_data, destinationCapacity + 1, data, dataSize, 11);
    if(ZSTD_isError(compressedDataSize))
    {
        std::cout << ZSTD_getErrorName(compressedDataSize) << "\n";
    }
    if(index == -1)
    {   
        this->m_HeaderCompressed = compressed_data;
        this->m_CompressedHeaderDataSize = compressedDataSize;
        this->m_Header.clear();
        this->m_Header.shrink_to_fit();

    }
    else
    {
        this->m_FieldsCompressed[index] = compressed_data;
        this->m_CompressedFieldsDataSize[index] = compressedDataSize;
        this->m_SAMFields.at(index).clear();
        this->m_SAMFields.at(index).shrink_to_fit();
    }
}

void SAMFileParser::compressionThread_fse(int index)
{
    size_t destinationCapacity = 0;
    const char * data = nullptr;
    size_t dataSize = 0;
    if(index == -1) 
    {
        this->m_OriginalHeaderDataSize = this->m_Header.size() + 1;
        destinationCapacity = FSE_compressBound(m_Header.size() + 1);
        data = this->m_Header.c_str();
        dataSize = this->m_OriginalHeaderDataSize;
    }
    else
    {
        this->m_OriginalFieldsDataSize[index] = m_SAMFields.at(index).size() + 1;
        destinationCapacity = FSE_compressBound(m_SAMFields.at(index).size() + 1);
        data = this->m_SAMFields.at(index).c_str();
        dataSize = this->m_OriginalFieldsDataSize[index];
    }

    char * compressed_data = new char[destinationCapacity];
    auto compressedDataSize = FSE_compress(compressed_data, destinationCapacity + 1, data, dataSize);
    if(FSE_isError(compressedDataSize))
    {
        std::cout << FSE_getErrorName(compressedDataSize) << "\n";
    }
    if(index == -1)
    {   
        this->m_HeaderCompressed = compressed_data;
        this->m_CompressedHeaderDataSize = compressedDataSize;
        this->m_Header.clear();
        this->m_Header.shrink_to_fit();

    }
    else
    {
        this->m_FieldsCompressed[index] = compressed_data;
        this->m_CompressedFieldsDataSize[index] = compressedDataSize;
        this->m_SAMFields.at(index).clear();
        this->m_SAMFields.at(index).shrink_to_fit();
    }
}

void SAMFileParser::compress_zstd_multithread(void)
{
    std::vector<std::future<void>> threads;
    for(int i = 0 ; i <= m_SAMFields.size() ; ++i)
    {
        threads.push_back(std::async(&SAMFileParser::compressionThread_zstd, this, i-1));
    }
}

void SAMFileParser::compressionThread_gzip(int index)
{
    if(index == -1) 
    {
        m_OriginalHeaderDataSize = m_Header.size();
        std::string compressed_data = gzip::compress(m_Header.c_str(), m_OriginalHeaderDataSize, Z_DEFAULT_COMPRESSION);
        m_HeaderCompressed = new char[compressed_data.size()];
        compressed_data.copy(m_HeaderCompressed, compressed_data.size(), 0);
        m_CompressedHeaderDataSize = compressed_data.size();
        m_Header.clear();
        m_Header.shrink_to_fit();
    }
    else
    {
        m_OriginalFieldsDataSize[index] = m_SAMFields.at(index).size();
        std::string compressed_field_data = gzip::compress(m_SAMFields.at(index).c_str(), m_OriginalFieldsDataSize[index], Z_DEFAULT_COMPRESSION);
        m_FieldsCompressed[index] = new char[compressed_field_data.size()];
        compressed_field_data.copy(m_FieldsCompressed[index], compressed_field_data.size(), 0);
        m_CompressedFieldsDataSize[index] = compressed_field_data.size();
        m_SAMFields.at(index).clear();
        m_SAMFields.at(index).shrink_to_fit();
    }
}

void SAMFileParser::compress_fse_multithread(void)
{
    std::vector<std::future<void>> threads;
    for(int i = 0 ; i <= m_SAMFields.size() ; ++i)
    {
        threads.push_back(std::async(&SAMFileParser::compressionThread_fse, this, i-1));
    }
}

void SAMFileParser::compress_gzip_multithread(void)
{
    std::vector<std::future<void>> threads;
    for(int i = 0 ; i <= m_SAMFields.size() ; ++i)
    {
        threads.push_back(std::async(&SAMFileParser::compressionThread_gzip, this, i-1));
    }
}

void SAMFileParser::compress_gzip(void)
{
    m_OriginalHeaderDataSize = m_Header.size();
    std::string compressed_data = gzip::compress(m_Header.c_str(), m_OriginalHeaderDataSize, Z_DEFAULT_COMPRESSION);
    m_HeaderCompressed = new char[compressed_data.size()];
    compressed_data.copy(m_HeaderCompressed, compressed_data.size(), 0);
    m_CompressedHeaderDataSize = compressed_data.size();
    m_Header.clear();
    m_Header.shrink_to_fit();
    for(size_t i = 0 ; i < m_SAMFields.size() ; ++i)
    {
        m_OriginalFieldsDataSize[i] = m_SAMFields.at(i).size();
        std::string compressed_field_data = gzip::compress(m_SAMFields.at(i).c_str(), m_OriginalFieldsDataSize[i], Z_DEFAULT_COMPRESSION);
        m_FieldsCompressed[i] = new char[compressed_field_data.size()];
        compressed_field_data.copy(m_FieldsCompressed[i], compressed_field_data.size(), 0);
        m_CompressedFieldsDataSize[i] = compressed_field_data.size();
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

void SAMFileParser::decompressionThread_fse(int index)
{
    const char * data = nullptr;
    size_t dataSize = 0;
    size_t decompressed_data_size = 0;
    if(index == -1) 
    {
        data = this->m_HeaderCompressed;
        dataSize = this->m_CompressedHeaderDataSize;
        decompressed_data_size = this->m_OriginalHeaderDataSize;
    }
    else
    {
        data = this->m_FieldsCompressed[index];
        dataSize = this->m_CompressedFieldsDataSize[index];
        decompressed_data_size = this->m_OriginalFieldsDataSize[index];
    }

    char * decompressed_data = new char[decompressed_data_size];
    auto decompressedDataSize = FSE_decompress(decompressed_data, decompressed_data_size, data, dataSize);
    if(FSE_isError(decompressedDataSize))
    {
        std::cout << FSE_getErrorName(decompressedDataSize) << "\n";
    }
    if(index == -1)
    {   
        std::istringstream headerStream(decompressed_data);
        headerStream.imbue(std::locale(headerStream.getloc(), new only_cr_is_whitespace));
        m_HeaderVec = {std::istream_iterator<std::string>{headerStream},
                       std::istream_iterator<std::string>{}};
        delete [] decompressed_data;
    }
    else
    {
        std::istringstream iss(decompressed_data);
        iss.imbue(std::locale(iss.getloc(), new tab_is_not_whitespace));
        m_SAMFieldsSplitted.at(index) = {std::istream_iterator<std::string>{iss},
                                         std::istream_iterator<std::string>{}};
        delete [] decompressed_data;
    }
}

void SAMFileParser::decompress_fse_multithread(void)
{
    std::vector<std::future<void>> threads;
    for(int i = 0 ; i <= m_SAMFields.size() ; ++i)
    {
        threads.push_back(std::async(&SAMFileParser::decompressionThread_fse, this, i-1));
    }
}


void SAMFileParser::decompress_fse(void)
{
    char *headerDecompressed;
    headerDecompressed = new char[m_OriginalHeaderDataSize];
    auto dataSize = FSE_decompress(headerDecompressed, m_OriginalHeaderDataSize, m_HeaderCompressed, m_CompressedHeaderDataSize);
    if(FSE_isError(dataSize))
    {
        std::cout << FSE_getErrorName(dataSize) << std::endl;
    }
    std::istringstream headerStream(headerDecompressed);
    headerStream.imbue(std::locale(headerStream.getloc(), new only_cr_is_whitespace));
    m_HeaderVec = {std::istream_iterator<std::string>{headerStream},
                  std::istream_iterator<std::string>{}};
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

void SAMFileParser::decompressionThread_zstd(int index)
{
    const char * data = nullptr;
    size_t dataSize = 0;
    size_t decompressed_data_size = 0;
    if(index == -1) 
    {
        data = this->m_HeaderCompressed;
        dataSize = this->m_CompressedHeaderDataSize;
        decompressed_data_size = this->m_OriginalHeaderDataSize;
    }
    else
    {
        data = this->m_FieldsCompressed[index];
        dataSize = this->m_CompressedFieldsDataSize[index];
        decompressed_data_size = this->m_OriginalFieldsDataSize[index];
    }

    char * decompressed_data = new char[decompressed_data_size];
    auto decompressedDataSize = ZSTD_decompress(decompressed_data, decompressed_data_size, data, dataSize);
    if(FSE_isError(decompressedDataSize))
    {
        std::cout << ZSTD_getErrorName(decompressedDataSize) << "\n";
    }
    if(index == -1)
    {   
        std::istringstream headerStream(decompressed_data);
        headerStream.imbue(std::locale(headerStream.getloc(), new only_cr_is_whitespace));
        m_HeaderVec = {std::istream_iterator<std::string>{headerStream},
                       std::istream_iterator<std::string>{}};
        delete [] decompressed_data;
    }
    else
    {
        std::istringstream iss(decompressed_data);
        iss.imbue(std::locale(iss.getloc(), new tab_is_not_whitespace));
        m_SAMFieldsSplitted.at(index) = {std::istream_iterator<std::string>{iss},
                                         std::istream_iterator<std::string>{}};
        delete [] decompressed_data;
    }
}

void SAMFileParser::decompress_zstd_multithread(void)
{
    std::vector<std::future<void>> threads;
    for(int i = 0 ; i <= m_SAMFields.size() ; ++i)
    {
        threads.push_back(std::async(&SAMFileParser::decompressionThread_zstd, this, i-1));
    }
}

void SAMFileParser::decompress_zstd(void)
{
    char *headerDecompressed;
    headerDecompressed = new char[m_OriginalHeaderDataSize];
    auto dataSize = ZSTD_decompress(headerDecompressed, m_OriginalHeaderDataSize, m_HeaderCompressed, m_CompressedHeaderDataSize);
    if(ZSTD_isError(dataSize))
    {
        std::cout << ZSTD_getErrorName(dataSize) << std::endl;
    }
    std::istringstream headerStream(headerDecompressed);
    headerStream.imbue(std::locale(headerStream.getloc(), new only_cr_is_whitespace));
    m_HeaderVec = {std::istream_iterator<std::string>{headerStream},
                  std::istream_iterator<std::string>{}};
    delete [] headerDecompressed;
    for(int i = 0 ; i < 12 ; ++i)
    {
        char *fieldDecompressed;
        fieldDecompressed = new char[m_OriginalFieldsDataSize[i]];
        auto dataSize = ZSTD_decompress(fieldDecompressed, m_OriginalFieldsDataSize[i], m_FieldsCompressed[i], m_CompressedFieldsDataSize[i]);
        if(ZSTD_isError(dataSize))
        {
            std::cout << ZSTD_getErrorName(dataSize) << std::endl;
        }
        std::istringstream iss(fieldDecompressed);
        iss.imbue(std::locale(iss.getloc(), new tab_is_not_whitespace));
        m_SAMFieldsSplitted.at(i) = {std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>{}};
        delete [] fieldDecompressed;
    }
}

void SAMFileParser::decompressionThread_gzip(int index)
{
    if(index == -1) 
    {
        std::istringstream headerStream(gzip::decompress(this->m_HeaderCompressed, this->m_CompressedHeaderDataSize));
        headerStream.imbue(std::locale(headerStream.getloc(), new only_cr_is_whitespace));
        this->m_HeaderVec = {std::istream_iterator<std::string>{headerStream},
                             std::istream_iterator<std::string>{}};
    }
    else
    {
        std::istringstream iss(gzip::decompress(this->m_FieldsCompressed[index], this->m_CompressedFieldsDataSize[index]));
        iss.imbue(std::locale(iss.getloc(), new only_space_is_whitespace));
        this->m_SAMFieldsSplitted.at(index) = {std::istream_iterator<std::string>{iss},
                                               std::istream_iterator<std::string>{}};
    }
}

void SAMFileParser::decompress_gzip_multithread(void)
{
    std::vector<std::future<void>> threads;
    for(int i = 0 ; i <= m_SAMFields.size() ; ++i)
    {
        threads.push_back(std::async(&SAMFileParser::decompressionThread_gzip, this, i-1));
    }
}

void SAMFileParser::decompress_gzip(void)
{
    std::istringstream headerStream(gzip::decompress(m_HeaderCompressed, m_CompressedHeaderDataSize));
    headerStream.imbue(std::locale(headerStream.getloc(), new only_cr_is_whitespace));
    m_HeaderVec = {std::istream_iterator<std::string>{headerStream},
                  std::istream_iterator<std::string>{}};
    for(int i = 0 ; i < 12 ; ++i)
    {
        std::istringstream iss(gzip::decompress(m_FieldsCompressed[i], m_CompressedFieldsDataSize[i]));
        iss.imbue(std::locale(iss.getloc(), new only_space_is_whitespace));
        m_SAMFieldsSplitted.at(i) = {std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>{}};
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
    std::string line;
    line.reserve (100000000);
    toFile.open(m_FileName + ".sam", std::ios::out | std::ios::app);
    for(int i = 0 ; i < m_HeaderVec.size() ; ++i)
    {
        line += m_HeaderVec.at(i) + "\n";
    }
    m_HeaderVec.clear();
    m_HeaderVec.shrink_to_fit();
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

std::string SAMFileParser::packOptionalTable()
{
    std::string ret;
    ret.reserve(1000);
    for(const auto &x : m_optionalHeaderTable)
    {
        std::string temp;
        temp += x.second;
        temp += x.first;
        temp += m_optionalTypeTable[x.first];
        ret += temp;
    }
    return ret;
}

void SAMFileParser::unpackOptionalTable(std::string table)
{
    int tableSize = table.size() / 4;
    std::string encodedHeader;
    std::string decodedHeader;
    std::string type;
    for(size_t i = 0 ; i < tableSize ; ++i )
    {
        for(int j = 0 ; j < 3 ; ++j)
        {
            encodedHeader = table.substr(i * 4 , 1);
            decodedHeader = table.substr(i * 4 + 1 , 2);
            type = table.substr(i * 4 + 3 , 1);
        }
        m_optionalDataTable[encodedHeader] = std::make_pair(decodedHeader, type);
    }
}

const void SAMFileParser::parseFile_experimental(void) {
    m_FileStream.open(m_FileName);
    std::string line;
    line.reserve(4000);
    while (std::getline(m_FileStream, line))
    {
        if(!isHeader(line)) 
        {

            splitLine_experimental(line);
        }
        else
        {
            m_Header.append(line + "\r");
        }
    }
}

void SAMFileParser::splitLine_experimental(std::string &line)
{
    std::vector<std::string> columns;
    std::string delimiter = "\t";
    uint8_t requiredNumber = 11;
    std::string column;
    column.reserve(200);
    std::string::size_type delimiterPosition = 0;
    for(size_t i = 0; i < requiredNumber; ++i)
    {
        delimiterPosition = line.find_first_of('\t');
        column = line.substr(0, delimiterPosition);
        columns.push_back(column);
        line.erase(0, delimiterPosition + delimiter.length());
    }
    if(columns.size() != 11) 
    {
        m_WrongLinesCount += 1;
        return;
    }
    for(int i = 0 ; i < columns.size() ; ++i)
    {
        experimental_SAMFields.at(i).append(" " + columns.at(i));
    } 
    columns.clear();
    delimiterPosition = line.find_first_of('\t');
    while(delimiterPosition != std::string::npos)
    {
        column = line.substr(0, delimiterPosition);
        columns.push_back(column);
        line.erase(0, delimiterPosition + delimiter.length());
        delimiterPosition = line.find_first_of('\t');
    }
    column = line.substr(0, delimiterPosition);
    columns.push_back(column);
    line.erase(0, delimiterPosition);
    if(!columns.empty())
    {
        std::string optionalHeaders;
        optionalHeaders.reserve(50);
        std::string optionalvalues;
        optionalvalues.reserve(1000);
        std::vector<std::string> elems;
        for(auto &x : columns)
        {
            delimiter = ":";
            for(size_t i = 0; i < 2; ++i)
            {
                delimiterPosition = x.find_first_of(':');
                column = x.substr(0, delimiterPosition);
                elems.push_back(column);
                x.erase(0, delimiterPosition + delimiter.length());
            }
            elems.push_back(x);
            optionalHeaders.append(encodeOptionalFieldHeader(elems.at(0), elems.at(1)));
            optionalvalues.append(elems.at(2) + "\r");
            elems.clear();
        }
        experimental_SAMFields.at(11).append(" " + optionalHeaders);
        experimental_SAMFields.at(12).append(" " + optionalvalues);
    }
    else
    {
        experimental_SAMFields.at(11).append(" 0");
        experimental_SAMFields.at(12).append(" 0");
    }
}

std::string SAMFileParser::encodeOptionalFieldHeader(const std::string &optionalHeader, const std::string &type)
{
    char retValue = '\0';
    if(m_optionalHeaderTable.find(optionalHeader) == m_optionalHeaderTable.end())
    {
        retValue = m_optionalsOffset++ + 48;
        m_optionalHeaderTable[optionalHeader] = retValue;
        m_optionalTypeTable[optionalHeader] = type;
    }
    else
    {
        retValue = m_optionalHeaderTable[optionalHeader];
    }
    return std::string(1,retValue);
}



void SAMFileParser::compress_fse_experimental(void)
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
    for(size_t i = 0 ; i < experimental_SAMFields.size() ; ++i)
    {
        experimental_OriginalFieldsDataSize[i] = experimental_SAMFields.at(i).size() + 1;
        size_t destinationCapacity = FSE_compressBound(experimental_SAMFields.at(i).size() + 1);
        experimantal_FieldsCompressed[i] = new char[destinationCapacity];
        experimental_CompressedFieldsDataSize[i] = FSE_compress(experimantal_FieldsCompressed[i], destinationCapacity, experimental_SAMFields.at(i).c_str() , experimental_SAMFields.at(i).size() + 1);
        if(FSE_isError(m_CompressedFieldsDataSize[i]))
        {
            std::cout << FSE_getErrorName(m_CompressedFieldsDataSize[i]) << std::endl;
        }
        experimental_SAMFields.at(i).clear();
        experimental_SAMFields.at(i).shrink_to_fit();
    }
}

void SAMFileParser::compress_zstd_experimental(void)
{
    m_OriginalHeaderDataSize = m_Header.size() + 1;
    size_t headerDestinationCapacity = ZSTD_compressBound(m_Header.size() + 1);
    m_HeaderCompressed = new char[headerDestinationCapacity];
    m_CompressedHeaderDataSize = ZSTD_compress(m_HeaderCompressed, headerDestinationCapacity + 1, m_Header.c_str() , m_Header.size() + 1, 11);
    if(ZSTD_isError(m_CompressedHeaderDataSize))
    {
        std::cout << ZSTD_getErrorName(m_CompressedHeaderDataSize) << std::endl;
    }
    m_Header.clear();
    m_Header.shrink_to_fit();
    for(size_t i = 0 ; i < experimental_SAMFields.size() ; ++i)
    {
        experimental_OriginalFieldsDataSize[i] = experimental_SAMFields.at(i).size() + 1;
        size_t destinationCapacity = ZSTD_compressBound(experimental_SAMFields.at(i).size() + 1);
        experimantal_FieldsCompressed[i] = new char[destinationCapacity];
        experimental_CompressedFieldsDataSize[i] = ZSTD_compress(experimantal_FieldsCompressed[i], destinationCapacity, experimental_SAMFields.at(i).c_str() , experimental_SAMFields.at(i).size() + 1, 11);
        if(ZSTD_isError(m_CompressedFieldsDataSize[i]))
        {
            std::cout << ZSTD_getErrorName(m_CompressedFieldsDataSize[i]) << std::endl;
        }
        experimental_SAMFields.at(i).clear();
        experimental_SAMFields.at(i).shrink_to_fit();
    }
}

void SAMFileParser::compress_gzip_experimental(void)
{
    m_OriginalHeaderDataSize = m_Header.size();
    std::string compressed_data = gzip::compress(m_Header.c_str(), m_OriginalHeaderDataSize, Z_DEFAULT_COMPRESSION);
    m_HeaderCompressed = new char[compressed_data.size()];
    compressed_data.copy(m_HeaderCompressed, compressed_data.size(), 0);
    m_CompressedHeaderDataSize = compressed_data.size();
    m_Header.clear();
    m_Header.shrink_to_fit();
    for(size_t i = 0 ; i < experimental_SAMFields.size() ; ++i)
    {
        experimental_OriginalFieldsDataSize[i] = experimental_SAMFields.at(i).size();
        std::string compressed_field_data = gzip::compress(experimental_SAMFields.at(i).c_str(), experimental_OriginalFieldsDataSize[i], Z_DEFAULT_COMPRESSION);
        experimantal_FieldsCompressed[i] = new char[compressed_field_data.size()];
        compressed_field_data.copy(experimantal_FieldsCompressed[i], compressed_field_data.size(), 0);
        experimental_CompressedFieldsDataSize[i] = compressed_field_data.size();
        experimental_SAMFields.at(i).clear();
        experimental_SAMFields.at(i).shrink_to_fit();
    }
}

void SAMFileParser::decompress_gzip_experimental(void)
{
    std::istringstream headerStream(gzip::decompress(m_HeaderCompressed, m_CompressedHeaderDataSize));
    headerStream.imbue(std::locale(headerStream.getloc(), new only_cr_is_whitespace));
    m_HeaderVec = {std::istream_iterator<std::string>{headerStream},
                  std::istream_iterator<std::string>{}};
    for(int i = 0 ; i < 13 ; ++i)
    {
        std::istringstream iss(gzip::decompress(experimantal_FieldsCompressed[i], experimental_CompressedFieldsDataSize[i]));
        iss.imbue(std::locale(iss.getloc(), new only_space_is_whitespace));
        experimental_SAMFieldsSplitted.at(i) = {std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>{}};
    }
}

// ntohl
void SAMFileParser::readCompressedDataFromFile_experimental(std::string fileName)
{
   std::ifstream inFile;
   inFile.open (fileName + ".test", std::ios::in | std::ios::binary);
   inFile.read(reinterpret_cast<char *>(&m_OriginalHeaderDataSize), sizeof(m_OriginalHeaderDataSize));
   inFile.read(reinterpret_cast<char *>(&experimental_OriginalFieldsDataSize[0]), 13*sizeof(size_t));
   inFile.read(reinterpret_cast<char *>(&m_CompressedHeaderDataSize), sizeof(m_CompressedHeaderDataSize));
   inFile.read(reinterpret_cast<char *>(&experimental_CompressedFieldsDataSize[0]), 13*sizeof(size_t));
   size_t optionalTableSize = 0;
   inFile.read(reinterpret_cast<char *>(&optionalTableSize), sizeof(size_t));
   char* optionalTable = new char[optionalTableSize];
   inFile.read(reinterpret_cast<char *>(&optionalTable[0]), optionalTableSize);
   unpackOptionalTable(optionalTable);
   delete [] optionalTable;
   m_HeaderCompressed = new char[m_CompressedHeaderDataSize];
   inFile.read(m_HeaderCompressed, m_CompressedHeaderDataSize);

   for(int i = 0 ; i < 13 ; ++i)
   {
    experimantal_FieldsCompressed[i] = new char[experimental_CompressedFieldsDataSize[i]];
    inFile.read(experimantal_FieldsCompressed[i], experimental_CompressedFieldsDataSize[i]);
  }
  inFile.close();   
}

void SAMFileParser::saveCompressedDataToFile_experimental(std::string fileName)
{
  std::ofstream toFile;
  toFile.open (fileName + ".test", std::ios::out | std::ios::app | std::ios::binary);
  toFile.write(reinterpret_cast<const char *>(&m_OriginalHeaderDataSize), sizeof(m_OriginalHeaderDataSize));
  toFile.write(reinterpret_cast<const char *>(&experimental_OriginalFieldsDataSize[0]), 13*sizeof(size_t));
  toFile.write(reinterpret_cast<const char *>(&m_CompressedHeaderDataSize), sizeof(m_CompressedHeaderDataSize));
  toFile.write(reinterpret_cast<const char *>(&experimental_CompressedFieldsDataSize[0]), 13*sizeof(size_t));
  auto optionalTable = packOptionalTable();
  size_t optionalTableSize = optionalTable.size();
  toFile.write(reinterpret_cast<const char *>(&optionalTableSize), sizeof(size_t));
  toFile.write(reinterpret_cast<const char *>(optionalTable.c_str()), optionalTableSize);

  toFile.write(m_HeaderCompressed, m_CompressedHeaderDataSize);
  delete [] m_HeaderCompressed;
  m_HeaderCompressed = nullptr;
  
  for(int i = 0 ; i < 13 ; ++i)
  {
    toFile.write(experimantal_FieldsCompressed[i], experimental_CompressedFieldsDataSize[i]);
    delete [] experimantal_FieldsCompressed[i];
    experimantal_FieldsCompressed[i] = nullptr;
  }
  toFile.close();
  printCompressionData_experimental(fileName + ".test");
}

void SAMFileParser::printCompressionData_experimental(std::string outFileName) const
{
    auto originalFileSize = std::experimental::filesystem::file_size(m_FileName);
    auto compressedFileSize = std::accumulate(experimental_CompressedFieldsDataSize, experimental_CompressedFieldsDataSize + 13, 0) + m_CompressedHeaderDataSize;
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
    for(int i = 0 ; i < 13 ; ++i)
    {
        std::cout << "=========================================" << std::endl;
        auto fieldFileRatio = ((experimental_OriginalFieldsDataSize[i]) / static_cast<double>(originalFileSize)) * 100;
        auto compressedFieldFileRatio = ((experimental_CompressedFieldsDataSize[i]) / static_cast<double>(compressedFileSize)) * 100;
        auto fieldCompressionRatio = ((experimental_CompressedFieldsDataSize[i]) / static_cast<double>(experimental_OriginalFieldsDataSize[i])) * 100;
        std::cout << "field " << i+1 << ") original size: " <<  experimental_OriginalFieldsDataSize[i] << " bytes" << std::endl;
        std::cout << "field " << i+1 << ") occupies " << fieldFileRatio << " % of the original file" << std::endl;
        std::cout << "field " << i+1 << ") compressed size: " <<  experimental_CompressedFieldsDataSize[i] << " bytes" << std::endl;
        std::cout << "field " << i+1 << ") occupies " << compressedFieldFileRatio << " % of the compressed file" << std::endl;
        std::cout << "field " << i+1 << ") Compression ratio: " << fieldCompressionRatio << " %" << std::endl;
    }
}

void SAMFileParser::decompress_fse_experimental(void)
{
    char *headerDecompressed;
    headerDecompressed = new char[m_OriginalHeaderDataSize];
    auto dataSize = FSE_decompress(headerDecompressed, m_OriginalHeaderDataSize, m_HeaderCompressed, m_CompressedHeaderDataSize);
    if(FSE_isError(dataSize))
    {
        std::cout << FSE_getErrorName(dataSize) << std::endl;
    }
    std::istringstream headerStream(headerDecompressed);
    headerStream.imbue(std::locale(headerStream.getloc(), new only_cr_is_whitespace));
    m_HeaderVec = {std::istream_iterator<std::string>{headerStream},
                  std::istream_iterator<std::string>{}};
    delete [] headerDecompressed;
    for(int i = 0 ; i < 13 ; ++i)
    {
        char *fieldDecompressed;
        fieldDecompressed = new char[experimental_OriginalFieldsDataSize[i]];
        auto dataSize = FSE_decompress(fieldDecompressed, experimental_OriginalFieldsDataSize[i], experimantal_FieldsCompressed[i], experimental_CompressedFieldsDataSize[i]);
        if(FSE_isError(dataSize))
        {
            std::cout << FSE_getErrorName(dataSize) << std::endl;
        }
        std::istringstream iss(fieldDecompressed);
        iss.imbue(std::locale(iss.getloc(), new only_space_is_whitespace));
        experimental_SAMFieldsSplitted.at(i) = {std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>{}};
        delete [] fieldDecompressed;
    }
}

void SAMFileParser::decompress_zstd_experimental(void)
{
    char *headerDecompressed;
    headerDecompressed = new char[m_OriginalHeaderDataSize];
    auto dataSize = ZSTD_decompress(headerDecompressed, m_OriginalHeaderDataSize, m_HeaderCompressed, m_CompressedHeaderDataSize);
    if(ZSTD_isError(dataSize))
    {
        std::cout << ZSTD_getErrorName(dataSize) << std::endl;
    }
    std::istringstream headerStream(headerDecompressed);
    headerStream.imbue(std::locale(headerStream.getloc(), new only_cr_is_whitespace));
    m_HeaderVec = {std::istream_iterator<std::string>{headerStream},
                  std::istream_iterator<std::string>{}};
    delete [] headerDecompressed;
    for(int i = 0 ; i < 13 ; ++i)
    {
        char *fieldDecompressed;
        fieldDecompressed = new char[experimental_OriginalFieldsDataSize[i]];
        auto dataSize = ZSTD_decompress(fieldDecompressed, experimental_OriginalFieldsDataSize[i], experimantal_FieldsCompressed[i], experimental_CompressedFieldsDataSize[i]);
        if(ZSTD_isError(dataSize))
        {
            std::cout << ZSTD_getErrorName(dataSize) << std::endl;
        }
        std::istringstream iss(fieldDecompressed);
        iss.imbue(std::locale(iss.getloc(), new only_space_is_whitespace));
        experimental_SAMFieldsSplitted.at(i) = {std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>{}};
        delete [] fieldDecompressed;
    }
}

void SAMFileParser::recreateFile_experimental(void)
{
    std::ofstream toFile;
    std::string line;
    line.reserve (100000000);
    toFile.open(m_FileName + ".sam", std::ios::out | std::ios::app);
    for(int i = 0 ; i < m_HeaderVec.size() ; ++i)
    {
        line += m_HeaderVec.at(i) + "\n";
    }
    m_HeaderVec.clear();
    m_HeaderVec.shrink_to_fit();
    std::string temp;
    temp.reserve(1000);
    std::string optionalsValue;
    std::string delimiter = "\r";
    optionalsValue.reserve(1000);
    for(int j = 0 ; j < experimental_SAMFieldsSplitted.at(0).size(); ++j)
    {
        for(int i = 0 ; i < 11 ; ++i) {
            line += experimental_SAMFieldsSplitted.at(i).at(j) + '\t'; // TODO: will be wrong if no optionals
        }
        if(experimental_SAMFieldsSplitted.at(11).at(j) == "0" && experimental_SAMFieldsSplitted.at(12).at(j) == "0")
        {
            line += "\n";
            continue;
        }
        else
        {
            for(int i = 0 ; i < experimental_SAMFieldsSplitted.at(11).at(j).size() ; ++i)
            {
                std::string encodedOptional = std::string(1,experimental_SAMFieldsSplitted.at(11).at(j).at(i));
                auto elem = m_optionalDataTable[encodedOptional];
                size_t pos = experimental_SAMFieldsSplitted.at(12).at(j).find_first_of('\r');
                optionalsValue = experimental_SAMFieldsSplitted.at(12).at(j).substr(0, pos);
                if(experimental_SAMFieldsSplitted.at(11).at(j).size() - 1 == i )
                {
                    temp += elem.first + ":" + elem.second + ":" + optionalsValue + '\n';
                }
                else
                {
                    temp += elem.first + ":" + elem.second + ":" + optionalsValue + '\t';
                }
                optionalsValue.clear();
                experimental_SAMFieldsSplitted.at(12).at(j).erase(0, pos + delimiter.length());
            }
        }
        line += temp;
        temp.clear();
    }
    toFile.write(line.c_str(), line.size());
    toFile.close();   
}

void SAMFileParser::compress_by_splitting(const char *src, char* dst, size_t dataSize)
{
    std::ofstream outFile("Data.log", std::ios::out | std::ios::app);
    size_t fieldOriginalSize = dataSize;
    size_t chunksNumber = fieldOriginalSize / 128000;
    char** dataChunks = new char*[chunksNumber + 1];
    outFile<< "Original File Size : " << fieldOriginalSize << std::endl;
    outFile << "Chunks : " << chunksNumber << std::endl;
    std::stringstream dataStream(src);
    for(int i = 0 ; i < chunksNumber ; ++i)
    {
        dataChunks[i] = new char[128000];
        dataStream.read(dataChunks[i], 128000);
    }
    char* temp = new char[128000];
    size_t blockSize = dataStream.readsome(temp, 128000);
    outFile << "Last Block Size: " << blockSize << std::endl;
    dataChunks[chunksNumber] = new char[blockSize];
    memcpy(dataChunks[chunksNumber], temp, blockSize);

    size_t fails = 0;
    size_t cumulativeSize = 0;
    char** compressedDataChunks = new char*[chunksNumber + 1];
    size_t generalCompressBound = HUF_compressBound(128000);
    for(int i = 0 ; i < chunksNumber ; ++i)
    {
        compressedDataChunks[i] = new char[generalCompressBound];
        size_t compressedChunkDataSize = HUF_compress(compressedDataChunks[i], generalCompressBound, dataChunks[i] , 128000);
        if(HUF_isError(compressedChunkDataSize))
        {   
            fails += 1;
            outFile << HUF_getErrorName(compressedChunkDataSize) << std::endl;
        }
        else
        {
            if(compressedChunkDataSize == 0 || compressedChunkDataSize == 1)
            {
                fails += 1;
            }
            outFile << "=========================================================" << std::endl;
            outFile << i+1 << ") Original chunk size : 128000 bytes" << std::endl;
            outFile << i+1 << ") Compressed chunkSize : " << compressedChunkDataSize << std::endl;
            outFile << i+1 << ") Chunk compression ratio : " << (compressedChunkDataSize / 128000.) * 100. << std::endl;;
            cumulativeSize += compressedChunkDataSize;
        }
    }

    size_t lastChunkCompressBound = HUF_compressBound(blockSize);
    compressedDataChunks[chunksNumber] = new char[lastChunkCompressBound];
    size_t compressedChunkDataSize = HUF_compress(compressedDataChunks[chunksNumber], lastChunkCompressBound, dataChunks[chunksNumber] , blockSize);
    if(HUF_isError(compressedChunkDataSize))
    {
        fails += 1;
        outFile << HUF_getErrorName(compressedChunkDataSize) << std::endl;
    }
    else
    {
        outFile << "=========================================================" << std::endl;
        outFile << chunksNumber+1 << ") Original chunk size : " << blockSize << " bytes" << std::endl;
        outFile << chunksNumber+1 << ") Compressed chunkSize : " << compressedChunkDataSize << std::endl;
        outFile << chunksNumber+1 << ") Chunk compression ratio : " << (compressedChunkDataSize / static_cast<double>(blockSize)) * 100. << std::endl;;
        cumulativeSize += compressedChunkDataSize;
    }

    outFile << "=========================================================" << std::endl;
    outFile << "Field original size : " << fieldOriginalSize << std::endl;
    outFile << "Field compressed size : " << cumulativeSize << std::endl;
    outFile << "Field compression ratio : " << (cumulativeSize / static_cast<double>(fieldOriginalSize)) * 100. << std::endl;;
    outFile << "Number of errors : " << fails << std::endl;
    outFile.close();

    for(int i = 0 ; i <= chunksNumber; ++i)
    {
        delete [] dataChunks[i];
    }
    delete [] dataChunks;

    for(int i = 0 ; i <= chunksNumber; ++i)
    {
        delete [] compressedDataChunks[i];
    }
    delete [] compressedDataChunks;
}
