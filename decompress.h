#ifndef HUFFMAN_DECOMPRESS
#define HUFFMAN_DECOMPRESS


#include <string>

// 读取压缩文件，解压得到原始文件，放入指定路径
void decompress(const std::string& file_path, const std::string& output_path);


#endif
