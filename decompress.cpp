#include "decompress.h"
#include "tree.h"
#include <fstream>
#include <vector>
#include <memory>
#include <stdexcept>
#include <cstdio>
#include <iterator>


std::string decompress(const std::string& file_path, const std::string& output_path, const std::string& password) {
    std::ifstream input{file_path, std::ios::binary};
    if (!input.is_open())
        throw std::runtime_error("Cannot open input file");
    // 读取原始文件哈希
    unsigned long long hash_expect = 0;
    char hash_array[8];
    input.read(hash_array, 8);
    if (input.fail() || static_cast<unsigned long long>(input.gcount()) != 8)
        throw std::runtime_error("File is broken");
    for (int i = 7; i >= 0; i--)
        hash_expect = hash_expect << 8 | static_cast<unsigned char>(hash_array[i]);
    // 读取压缩文件标记并核对
    char identifier[8];
    input.read(identifier, 8);
    if (input.fail() || static_cast<unsigned long long>(input.gcount()) != 8)
        throw std::runtime_error("File is broken");
    if (std::string(identifier) != "HUFF1.0")
        throw std::runtime_error("Not compressed file");
    // 读取密码并核对
    std::vector<char> password_array(password.length());
    input.read(password_array.data(), static_cast<long long>(password.length()));
    if (input.fail() || static_cast<unsigned long long>(input.gcount()) != password.length())
        throw std::runtime_error("File is broken");
    input.read(identifier, 8);
    if (input.fail() || static_cast<unsigned long long>(input.gcount()) != 8)
        throw std::runtime_error("File is broken");
    for (char& ch: password_array)
        --ch; // 压缩文件写入密码时每个字符加 1
    if (std::string(password_array.begin(), password_array.end()) != password || std::string(identifier) != "HUFF1.0")
        throw std::runtime_error("Password is incorrect");
    // 读取文件后缀
    int extension_length = input.get();
    std::string extension;
    if (extension_length > 0) {
        std::vector<char> extension_array(extension_length);
        input.read(extension_array.data(), extension_length);
        if (input.fail() || static_cast<unsigned long long>(input.gcount()) != extension_length)
            throw std::runtime_error("File is broken");
        extension = std::string(extension_array.begin(), extension_array.end());
    }
    else
        extension = "";
    // 读取原始文件字节总数
    unsigned long long byte_total = 0;
    char total[8];
    input.read(total, 8);
    if (input.fail() || static_cast<unsigned long long>(input.gcount()) != 8)
        throw std::runtime_error("File is broken");
    for (int i = 7; i >= 0; i--)
        byte_total = byte_total << 8 | static_cast<unsigned char>(total[i]);
    // 读取原始文件所有字节出现次数
    char header[2048];
    input.read(header, 2048);
    if (input.fail() || static_cast<unsigned long long>(input.gcount()) != 2048)
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
    std::string output_temp = output_path + ".temp";
    std::ofstream output{output_temp, std::ios::binary};
    if (!output.is_open())
        throw std::runtime_error("Cannot open output file");
    try {
        unsigned long long hash = 14695981039346656037ULL;
        // 原始文件为空
        if (!root) {
            if (hash != hash_expect)
                throw std::runtime_error("File is broken");
            input.close();
            output.close();
            std::remove(output_file.c_str());
            std::rename(output_temp.c_str(), output_file.c_str());
            return extension;
        }
        // 原始文件只有一种字节
        if (!root->left_child && !root->right_child) {
            std::ifstream temp{file_path, std::ios::binary | std::ios::ate};
            if (!temp.is_open())
                throw std::runtime_error("Cannot open input file");
            if (static_cast<unsigned long long>(temp.tellg()) < byte_total / 8)
                throw std::runtime_error("File is broken");
            unsigned long long byte_count = 0;
            std::vector<char> buffer(1024, static_cast<char>(root->byte_value));
            while (byte_count < byte_total) {
                if (byte_total - byte_count >= 1024) {
                    output.write(buffer.data(), 1024);
                    byte_count = byte_count + 1024;
                    for (int i = 0; i < 1024; i++) {
                        hash = hash ^ root->byte_value;
                        hash = hash * 1099511628211ULL;
                    }
                }
                else {
                    std::vector<char> left(byte_total - byte_count, static_cast<char>(root->byte_value));
                    output.write(left.data(), static_cast<long long>(left.size()));
                    byte_count = byte_count + left.size();
                    for (int i = 0; i < static_cast<int>(left.size()); i++) {
                        hash = hash ^ root->byte_value;
                        hash = hash * 1099511628211ULL;
                    }
                }
            }
            if (hash != hash_expect)
                throw std::runtime_error("File is broken");
            input.close();
            output.close();
            std::remove(output_file.c_str());
            std::rename(output_temp.c_str(), output_file.c_str());
            return extension;
        }
        // 原始文件有多种字节
        std::ifstream temp{file_path, std::ios::binary | std::ios::ate};
        if (!temp.is_open())
            throw std::runtime_error("Cannot open input file");
        if (static_cast<unsigned long long>(temp.tellg()) < byte_total / 8)
            throw std::runtime_error("File is broken");
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
                    hash = hash ^ current->byte_value;
                    hash = hash * 1099511628211ULL;
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
        // 比较解压后字节总数和哈希是否与原始文件相同
        if (byte_count != byte_total || hash != hash_expect)
            throw std::runtime_error("File is broken");
    }
    catch (...) {
        input.close();
        output.close();
        std::remove(output_temp.c_str());
        throw;
    }
    input.close();
    output.close();
    std::remove(output_file.c_str());
    std::rename(output_temp.c_str(), output_file.c_str());
    return extension;
}
