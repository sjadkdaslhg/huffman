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
    }
    return {counts, count};
}

// 根据哈夫曼树，对每个字节编码
static std::vector<std::string> encode(const std::shared_ptr<Node>& root) {
    if (!root)
        throw std::runtime_error("Empty tree");
    std::vector<std::string> codes(256);
    size_t count = 0;
    if (!root->left_child && !root->right_child) {
        codes[root->byte_value] = "1";
        return codes;
    }
    std::stack<std::pair<std::shared_ptr<Node>, std::string>> stack;
    stack.emplace(root, "");
    std::string code;
    while (!stack.empty()) {
        std::pair<std::shared_ptr<Node>, std::string> top = stack.top();
        if (!top.first->left_child && !top.first->right_child) {
            codes[top.first->byte_value] = top.second;
            ++count;
        }
        else {
            stack.emplace(top.first->left_child, top.second + "0");
            stack.emplace(top.first->right_child, top.second + "1");
        }
    }
    if (count != 256)
        throw std::runtime_error("Encoding failed");
    return codes;
}


void compress(const std::string& file_path, const std::string& output_path) {
    std::ifstream input{file_path, std::ios::binary};
    std::pair<std::vector<unsigned long long>, unsigned long long> pair = countBytes(std::move(input));
    std::shared_ptr<Node> root = buildTree(pair.first);
    std::vector<std::string> codes = encode(root);
    std::ofstream output{output_path, std::ios::binary};
    std::vector<char> total(8);
    for (int i = 0; i < 8; i++) {
        unsigned char byte = pair.second & 0xFF;
        pair.second = pair.second >> 8;
        total[i] = static_cast<char>(byte);
    }
    output.write(total.data(), 8);
    std::vector<char> counts(2048);
    size_t header_count = 0;
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 8; j++) {
            unsigned char byte = pair.first[i] & 0xFF;
            pair.first[i] = pair.first[i] >> 8;
            counts[header_count++] = static_cast<char>(byte);
        }
    }
    output.write(counts.data(), 2048);
    std::istreambuf_iterator<char> iterator{input};
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
            }
        }
        if (buffer_count == 2024) {
            output.write(buffer.data(), buffer_count);
            buffer_count = 0;
        }
    }
    if (bit_count > 0) {
        bit = bit << (8 - bit_count);
        buffer[buffer_count++] = static_cast<char>(bit);
    }
    if (buffer_count > 0)
        output.write(buffer.data(), buffer_count);
}
