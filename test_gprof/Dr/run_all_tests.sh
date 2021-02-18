bash total_cleaner.sh
make

echo "STARTING"
## MULTI
echo "MULTITHREAD START"
# FSE
echo "FSE COMPRESS START"
./SAMFileReader_fse_compress_multi
gprof SAMFileReader_fse_compress_multi gmon.out > ./out/COMPRESS_FSE_MULTI.prof
rm -rf gmon.out
echo "FSE COMPRESS DONE"
echo "FSE DECOMPRESS START"
./SAMFileReader_fse_decompress_multi
gprof SAMFileReader_fse_decompress_multi gmon.out > ./out/DECOMPRESS_FSE_MULTI.prof
echo "FSE DECOMPRESS DONE"
bash cleaner.sh

# ZSTD
echo "ZSTD COMPRESS START"
./SAMFileReader_zstd_compress_multi
gprof SAMFileReader_zstd_compress_multi gmon.out > ./out/COMPRESS_ZSTD_MULTI.prof
rm -rf gmon.out
echo "ZSTD COMPRESS DONE"
echo "ZSTD DECOMPRESS START"
./SAMFileReader_zstd_decompress_multi
gprof SAMFileReader_zstd_decompress_multi gmon.out > ./out/DECOMPRESS_ZSTD_MULTI.prof
echo "ZSTD DECOMPRESS DONE"
bash cleaner.sh

# GZIP
echo "GZIP COMPRESS START"
./SAMFileReader_gzip_compress_multi
gprof SAMFileReader_gzip_compress_multi gmon.out > ./out/COMPRESS_GZIP_MULTI.prof
rm -rf gmon.out
echo "GZIP COMPRESS DONE"
echo "GZIP DECOMPRESS START"
./SAMFileReader_gzip_decompress_multi
gprof SAMFileReader_gzip_decompress_multi gmon.out > ./out/DECOMPRESS_GZIP_MULTI.prof
echo "GZIP DECOMPRESS DONE"
bash cleaner.sh
echo "MULTITHREAD DONE"

## SINGLE
echo "SINGLE START"
# FSE
echo "FSE COMPRESS START"
./SAMFileReader_fse_compress_single
gprof SAMFileReader_fse_compress_single gmon.out > ./out/COMPRESS_FSE_SINGLE.prof
rm -rf gmon.out
echo "FSE COMPRESS DONE"
echo "FSE DECOMPRESS START"
./SAMFileReader_fse_decompress_single
gprof SAMFileReader_fse_decompress_single gmon.out > ./out/DECOMPRESS_FSE_SINGLE.prof
echo "FSE DECOMPRESS DONE"
bash cleaner.sh

# ZSTD
echo "ZSTD COMPRESS START"
./SAMFileReader_zstd_compress_single
gprof SAMFileReader_zstd_compress_single gmon.out > ./out/COMPRESS_ZSTD_SINGLE.prof
rm -rf gmon.out
echo "ZSTD COMPRESS DONE"
echo "ZSTD DECOMPRESS START"
./SAMFileReader_zstd_decompress_single
gprof SAMFileReader_zstd_decompress_single gmon.out > ./out/DECOMPRESS_ZSTD_SINGLE.prof
echo "ZSTD DECOMPRESS DONE"
bash cleaner.sh

# GZIP
echo "GZIP COMPRESS START"
./SAMFileReader_gzip_compress_single
gprof SAMFileReader_gzip_compress_single gmon.out > ./out/COMPRESS_GZIP_SINGLE.prof
rm -rf gmon.out
echo "GZIP COMPRESS DONE"
echo "GZIP DECOMPRESS START"
./SAMFileReader_gzip_decompress_single
gprof SAMFileReader_gzip_decompress_single gmon.out > ./out/DECOMPRESS_GZIP_SINGLE.prof
echo "GZIP DECOMPRESS DONE"
bash cleaner.sh
echo "SINGLE DONE"

echo "ALL DONE"
bash total_cleaner.sh
