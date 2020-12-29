#ifndef FILE_READER_H
#define FILE_READER_H

#include <string>
#include <fstream>
#include <vector>
#include <limits>
#include <map>

struct tab_is_not_whitespace : std::ctype<char> 
{
    static const mask* make_table()
    {
        static std::vector<mask> v(classic_table(), classic_table() + table_size);
        v['\t'] &= ~space;
        return &v[0];
    }
    tab_is_not_whitespace(std::size_t refs = 0) : ctype(make_table(), false, refs) {}
};

struct only_cr_is_whitespace : std::ctype<char> 
{
    static const mask* make_table()
    {
        static std::vector<mask> v(classic_table(), classic_table() + table_size);
        v[' '] &= ~space;
        v['\n'] &= ~space;
        v['\t'] &= ~space;
        v['\v'] &= ~space;
        v['\f'] &= ~space;
        return &v[0];
    }
    only_cr_is_whitespace(std::size_t refs = 0) : ctype(make_table(), false, refs) {}
};

class SAMFileParser
{
public:
    SAMFileParser(std::string fileName);
    ~SAMFileParser();
    void saveCompressedDataToFile(std::string fileName);
    void saveCompressedDataToFile_experimental(std::string fileName);
    void readCompressedDataFromFile(std::string fileName);
    void recreateFile(void);
    const void parseFile();
    void compress(void);
    void decompress(void);

private:
    void compress_experimental(const char *src, char* dst, size_t dataSize);
    void printSupportiveData(void) const;
    void printCompressionData(std::string outFileName) const;
    void splitLine(std::string &line, const std::string &delimiter);
    inline bool isHeader(const std::string &line) const
    {
        return *line.begin() == '@';
    }

    std::map<uint32_t, uint32_t> m_HeaderDelimiters;
    std::map<uint32_t, uint32_t> m_AligmentDelimiters;
    std::vector<std::string> m_SAMFields{12};
    std::vector<std::vector<std::string>> m_SAMFieldsSplitted{12};
    uint32_t m_WrongLinesCount = 0;
    uint32_t m_AligmentLinesCount = 0;
    uint32_t m_HeaderLinesCount = 0;
    std::string m_FileName;
    std::ifstream m_FileStream;
    std::string m_Header;
    std::vector<std::string> m_HeaderVec;
    uint64_t m_SeqMin = std::numeric_limits<uint64_t>::max();
    uint64_t m_SeqMax = 0;
    char **m_FieldsCompressed = nullptr;
    char *m_HeaderCompressed = nullptr;
    size_t m_CompressedHeaderDataSize = 0;
    size_t m_OriginalHeaderDataSize = 0;
    size_t m_CompressedFieldsDataSize[12] = {0};
    size_t m_OriginalFieldsDataSize[12] = {0};
};

#endif