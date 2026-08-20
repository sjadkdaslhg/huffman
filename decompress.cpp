#include "decompress.h"
#include "tree.h"
#include <fstream>
#include <vector>
#include <memory>
#include <stdexcept>
#include <iterator>


std::string decompress(const std::string& file_path, const std::string& output_path, const std::string& password) {
    std::ifstream input{file_path, std::ios::binary};
    if (!input.is_open())
        throw std::runtime_error("Cannot open input file");
    // 读取压缩文件标记并核对
    char identifier[8];
    input.read(identifier, 8);
    if (input.fail() || input.gcount() != 8)
        throw std::runtime_error("File is broken");
    if (std::string(identifier) != "HUFFMAN")
        throw std::runtime_error("Not compressed file");
    // 读取密码并核对
    std::vector<char> password_array(password.length());
    input.read(password_array.data(), static_cast<long long>(password.length()));
    if (input.fail() || input.gcount() != password.length())
        throw std::runtime_error("File is broken");
    input.read(identifier, 8);
    if (input.fail() || input.gcount() != 8)
        throw std::runtime_error("File is broken");
    if (std::string(password_array.begin(), password_array.end()) != password || std::string(identifier) != "HUFFMAN")
        throw std::runtime_error("Password is incorrect");
    // 读取文件后缀
    int extension_length = input.get();
    std::string extension;
    if (extension_length > 0) {
        std::vector<char> extension_array(extension_length);
        input.read(extension_array.data(), extension_length);
        if (input.fail() || input.gcount() != extension_length)
            throw std::runtime_error("File is broken");
        extension = std::string(extension_array.begin(), extension_array.end());
    }
    else
        extension = "";
    // 读取原始文件字节总数
    unsigned long long byte_total = 0;
    char total[8];
    input.read(total, 8);
    if (input.fail() || input.gcount() != 8)
        throw std::runtime_error("File is broken");
    for (int i = 7; i >= 0; i--)
        byte_total = byte_total << 8 | static_cast<unsigned char>(total[i]);
    // 读取原始文件所有字节出现次数
    char header[2048];
    input.read(header, 2048);
    if (input.fail() || input.gcount() != 2048)
        throw std::runtime_error("File is broken");
    std::vector<unsigned long long> counts(256);
    for (int i = 0; i < 256; i++) {
        unsigned long long count = 0;
        for (int j = 7; j >= 0; j--) {
            const char byte = header[i * 8 + j];
            count = count << 8 | static_cast<unsigned char>(byte);
        }
        counts[i] = count;
    }
    // 构建哈夫曼树，解压压缩文件
    std::shared_ptr<Node> root = buildTree(counts);
    std::string output_file;
    if (extension.empty())
        output_file = output_path;
    else
        output_file = output_path + '.' + extension;
    std::ofstream output{output_file, std::ios::binary};
    if (!output.is_open())
        throw std::runtime_error("Cannot open output file");
    // 原始文件为空
    if (!root)
        return extension;
    // 原始文件只有一种字节
    if (!root->left_child && !root->right_child) {
        unsigned long long byte_count = 0;
        std::vector<char> buffer(1024, static_cast<char>(root->byte_value));
        while (byte_count < byte_total) {
            if (byte_total - byte_count >= 1024) {
                output.write(buffer.data(), 1024);
                byte_count = byte_count + 1024;
            }
            else {
                std::vector<char> left(byte_total - byte_count, static_cast<char>(root->byte_value));
                output.write(left.data(), static_cast<long long>(left.size()));
                byte_count = byte_count + left.size();
            }
        }
        return extension;
    }
    // 原始文件有多种字节
    std::istreambuf_iterator<char> iterator{input};
    std::istreambuf_iterator<char> end_of_file;
    std::shared_ptr<Node> current = root;
    std::vector<char> buffer(1024);
    long long buffer_count = 0;
    unsigned long long byte_count = 0;
    while (iterator != end_of_file && byte_count < byte_total) {
        unsigned char byte = *iterator;
        ++iterator;
        for (int i = 0; i < 8; i++) {
            if (byte & static_cast<unsigned char>(128))
                current = current->right_child;
            else
                current = current->left_child;
            byte = byte << 1;
            if (!current->left_child && !current->right_child) {
                buffer[buffer_count++] = static_cast<char>(current->byte_value);
                current = root;
                ++byte_count;
                if (buffer_count == 1024) {
                    output.write(buffer.data(), buffer_count);
                    buffer_count = 0;
                }
            }
            if (byte_count == byte_total)
                break;
        }
    }
    if (buffer_count > 0)
        output.write(buffer.data(), buffer_count);
    input.close();
    output.close();
    return extension;
}
