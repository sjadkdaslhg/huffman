#include "compress.h"
#include "tree.h"
#include <utility>
#include <fstream>
#include <vector>
#include <iterator>
#include <string>
#include <stdexcept>
#include <memory>
#include <stack>


// 读取原始文件，返回字节出现次数数组和字节总数
static std::pair<std::vector<unsigned long long>, unsigned long long> countBytes(std::ifstream input) {
    unsigned long long count = 0;
    std::vector<unsigned long long> counts(256);
    std::istreambuf_iterator<char> iterator{input};
    const std::istreambuf_iterator<char> end_of_file;
    while (iterator != end_of_file) {
        const char byte = *iterator;
        ++iterator;
        ++counts[byte & 0xFF];
        ++count;
    }
    return {counts, count};
}

// 根据哈夫曼树，对每个字节编码
static std::vector<std::string> encode(const std::shared_ptr<Node>& root) {
    if (!root)
        return std::vector<std::string>{};
    std::vector<std::string> codes(256);
    // 原始文件只有一种字节，哈夫曼树只有一个结点，编码为 1
    if (!root->left_child && !root->right_child) {
        codes[root->byte_value] = "1";
        return codes;
    }
    std::stack<std::pair<std::shared_ptr<Node>, std::string>> stack;
    stack.emplace(root, "");
    while (!stack.empty()) {
        std::pair<std::shared_ptr<Node>, std::string> top = stack.top();
        stack.pop();
        if (!top.first->left_child && !top.first->right_child)
            codes[top.first->byte_value] = top.second;
        else { // 往左子树编码为 0，往右子树编码为 1
            stack.emplace(top.first->left_child, top.second + "0");
            stack.emplace(top.first->right_child, top.second + "1");
        }
    }
    return codes;
}


void compress(const std::string& file_path, const std::string& output_path, const std::string& password, const std::string& extension) {
    std::ifstream input{file_path, std::ios::binary};
    if (!input.is_open())
        throw std::runtime_error("Cannot open input file");
    std::pair<std::vector<unsigned long long>, unsigned long long> pair = countBytes(std::move(input));
    std::shared_ptr<Node> root = buildTree(pair.first);
    std::vector<std::string> codes = encode(root);
    std::ofstream output{output_path, std::ios::binary};
    if (!output.is_open())
        throw std::runtime_error("Cannot open output file");
    /* 压缩文件头部信息：
        H U F F M A N \0：压缩文件标记
        ****************：密码
        H U F F M A N \0：密码结束标记
        length..........：文件后缀长度和文件后缀
        头部信息均为未压缩的字符
    */
    // 写入压缩文件标记
    char identifier[8] = "HUFFMAN";
    output.write(identifier, 8);
    // 写入密码和文件后缀
    std::vector<char> password_array(password.begin(), password.end());
    output.write(password_array.data(), static_cast<long long>(password_array.size()));
    output.write(identifier, 8); // 再次写入压缩文件标记，表示密码结束
    output.put(static_cast<char>(extension.length())); // 写入文件后缀长度
    output.write(extension.data(), static_cast<long long>(extension.length()));
    // 压缩文件记录原始文件字节总数
    std::vector<char> total(8);
    for (int i = 0; i < 8; i++) {
        unsigned char byte = pair.second & 0xFF;
        pair.second = pair.second >> 8;
        total[i] = static_cast<char>(byte);
    }
    output.write(total.data(), 8);
    // 压缩文件记录原始文件所有字节出现次数
    std::vector<char> counts(2048);
    size_t header_count = 0;
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 8; j++) {
            unsigned char byte = pair.first[i] & 0xFF;
            pair.first[i] = pair.first[i] >> 8;
            counts[header_count++] = static_cast<char>(byte);
        }
    }
    if (header_count != 2048)
        throw std::runtime_error("Header error");
    output.write(counts.data(), 2048);
    // 再次读取原始文件，进行压缩
    std::ifstream new_input{file_path, std::ios::binary};
    if (!new_input.is_open())
        throw std::runtime_error("Cannot open input file");
    std::istreambuf_iterator<char> iterator{new_input};
    std::istreambuf_iterator<char> end_of_file;
    unsigned char bit = 0;
    size_t bit_count = 0;
    std::vector<char> buffer(1024);
    long long buffer_count = 0;
    while (iterator != end_of_file) {
        char byte = *iterator;
        ++iterator;
        for (char ch : codes[byte & 0xFF]) {
            bit = bit << 1 | ch - '0';
            ++bit_count;
            if (bit_count == 8) {
                buffer[buffer_count++] = static_cast<char>(bit);
                bit_count = 0;
                bit = 0;
            }
            if (buffer_count == 1024) {
                output.write(buffer.data(), buffer_count);
                buffer_count = 0;
            }
        }
    }
    if (bit_count > 0) {
        bit = bit << (8 - bit_count);
        buffer[buffer_count++] = static_cast<char>(bit);
    }
    if (buffer_count > 0)
        output.write(buffer.data(), buffer_count);
    new_input.close();
    output.close();
}
