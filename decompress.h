#ifndef HUFFMAN_DECOMPRESS
#define HUFFMAN_DECOMPRESS


#include <string>

// 读取压缩文件，核对密码，解压得到原始文件，放入指定路径，返回原始文件后缀
std::string decompress(const std::string& file_path, const std::string& output_path, const std::string& password);


#endif
