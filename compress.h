#ifndef HUFFMAN_COMPRESS
#define HUFFMAN_COMPRESS


#include <string>

// 读取原始文件，在指定路径创建压缩文件
void compress(const std::string& file_path, const std::string& output_path, const std::string& password, const std::string& extension);


#endif
