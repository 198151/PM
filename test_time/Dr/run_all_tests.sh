bash total_cleaner.sh
make

echo "STARTING"
## MULTI
echo "MULTITHREAD START"
# FSE
echo "FSE COMPRESS START"
./SAMFileReader_fse_compress_multi > ./out/FSE_COMPESS_MULTI.log
echo "FSE COMPRESS DONE"
echo "FSE DECOMPRESS START"
./SAMFileReader_fse_decompress_multi > ./out/FSE_DECOMPESS_MULTI.log
echo "FSE DECOMPRESS DONE"
bash cleaner.sh

# ZSTD
echo "ZSTD COMPRESS START"
./SAMFileReader_zstd_compress_multi > ./out/ZSTD_COMPESS_MULTI.log
echo "ZSTD COMPRESS DONE"
echo "ZSTD DECOMPRESS START"
./SAMFileReader_zstd_decompress_multi  > ./out/ZSTD_DECOMPESS_MULTI.log
echo "ZSTD DECOMPRESS DONE"
bash cleaner.sh

# GZIP
echo "GZIP COMPRESS START"
./SAMFileReader_gzip_compress_multi  > ./out/GZIP_COMPESS_MULTI.log
echo "GZIP COMPRESS DONE"
echo "GZIP DECOMPRESS START"
./SAMFileReader_gzip_decompress_multi  > ./out/GZIP_DECOMPESS_MULTI.log
echo "GZIP DECOMPRESS DONE"
bash cleaner.sh
echo "MULTITHREAD DONE"

## SINGLE
echo "SINGLE START"
# FSE
echo "FSE COMPRESS START"
./SAMFileReader_fse_compress_single  > ./out/FSE_COMPESS_SINGLE.log
echo "FSE COMPRESS DONE"
echo "FSE DECOMPRESS START"
./SAMFileReader_fse_decompress_single  > ./out/FSE_DECOMPESS_SINGLE.log
echo "FSE DECOMPRESS DONE"
bash cleaner.sh

# ZSTD
echo "ZSTD COMPRESS START"
./SAMFileReader_zstd_compress_single  > ./out/ZSTD_COMPESS_SINGLE.log
echo "ZSTD COMPRESS DONE"
echo "ZSTD DECOMPRESS START"
./SAMFileReader_zstd_decompress_single  > ./out/ZSTD_DECOMPESS_SINGLE.log
echo "ZSTD DECOMPRESS DONE"
bash cleaner.sh

# GZIP
echo "GZIP COMPRESS START"
./SAMFileReader_gzip_compress_single  > ./out/GZIP_COMPESS_SINGLE.log
echo "GZIP COMPRESS DONE"
echo "GZIP DECOMPRESS START"
./SAMFileReader_gzip_decompress_single  > ./out/GZIP_DECOMPESS_SINGLE.log
echo "GZIP DECOMPRESS DONE"
bash cleaner.sh
echo "SINGLE DONE"

echo "ALL DONE"
bash total_cleaner.sh
