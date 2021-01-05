#ifndef FILE_READER_H
#define FILE_READER_H

#include <string>
#include <fstream>
#include <vector>
#include <limits>
#include <unordered_map>
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

struct only_colon_is_delimiter : std::ctype<char> 
{
    static const mask* make_table()
    {
        static std::vector<mask> v(classic_table(), classic_table() + table_size);
        v[' '] &= ~space;
        v['\n'] &= ~space;
        v['\t'] &= ~space;
        v['\v'] &= ~space;
        v['\f'] &= ~space;
        v['\r'] &= ~space;
        v[':'] |= std::ctype_base::space;
        return &v[0];
    }
    only_colon_is_delimiter(std::size_t refs = 0) : ctype(make_table(), false, refs) {}
};

struct only_space_is_whitespace : std::ctype<char> 
{
    static const mask* make_table()
    {
        static std::vector<mask> v(classic_table(), classic_table() + table_size);
        v['\t'] &= ~space;
        v['\n'] &= ~space;
        v['\r'] &= ~space;
        v['\v'] &= ~space;
        v['\f'] &= ~space;
        return &v[0];
    }
    only_space_is_whitespace(std::size_t refs = 0) : ctype(make_table(), false, refs) {}
};

typedef std::pair<std::string, std::string> name_type;

class SAMFileParser
{
public:
    SAMFileParser(std::string fileName);
    ~SAMFileParser();
    void saveCompressedDataToFile(std::string fileName);
    void saveCompressedDataToFile_experimental(std::string fileName);
    void readCompressedDataFromFile(std::string fileName);
    void readCompressedDataFromFile_experimental(std::string fileName);
    void recreateFile(void);
    void recreateFile_experimental(void);
    const void parseFile();
    const void parseFile_experimental();


    void compress_fse(void);
    void decompress_fse(void);

    void compress_zstd(void);
    void decompress_zstd(void);

    void compress_gzip(void);
    void decompress_gzip(void);

    void compress_fse_experimental(void);
    void decompress_fse_experimental(void);

    void compress_zstd_experimental(void);
    void decompress_zstd_experimental(void);

    void compress_gzip_experimental(void);
    void decompress_gzip_experimental(void);

private:
    void compress_by_splitting(const char *src, char* dst, size_t dataSize);
    void printSupportiveData(void) const;
    void printCompressionData(std::string outFileName) const;
    void printCompressionData_experimental(std::string outFileName) const;
    void splitLine(std::string &line, const std::string &delimiter);
    void splitLine_experimental(std::string &line);
    std::string packOptionalTable();
    void unpackOptionalTable(std::string table); 
    inline bool isHeader(const std::string &line) const
    {
        return *line.begin() == '@';
    }

    std::string encodeOptionalFieldHeader(const std::string &optionalHeader, const std::string &type);

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

    char **experimantal_FieldsCompressed = nullptr;
    size_t experimental_CompressedFieldsDataSize[13] = {0};
    size_t experimental_OriginalFieldsDataSize[13] = {0};
    std::vector<std::string> experimental_SAMFields {13};
    std::vector<std::vector<std::string>> experimental_SAMFieldsSplitted{13};
    int m_optionalsOffset = 0;
    int m_additionalOptionals = 0;
    std::unordered_map<std::string, char> m_optionalHeaderTable;
    std::unordered_map<std::string, std::string> m_optionalTypeTable;
    std::map<std::string, name_type> m_optionalDataTable;
};

#endif