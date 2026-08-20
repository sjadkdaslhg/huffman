#include "decompress.h"
#include "tree.h"
#include <fstream>
#include <vector>
#include <memory>
#include <stdexcept>
#include <iterator>


void decompress(const std::string& file_path, const std::string& output_path) {
    std::ifstream input{file_path, std::ios::binary};
    unsigned long long byte_total = 0;
    char total[8];
    input.read(total, 8);
    for (int i = 7; i >= 0; i--)
        byte_total = byte_total << 8 | static_cast<unsigned char>(total[i]);
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
    std::shared_ptr<Node> root = buildTree(counts);
    std::ofstream output{output_path, std::ios::binary};
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
        return;
    }
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
        }
    }
    if (buffer_count > 0)
        output.write(buffer.data(), buffer_count);
    input.close();
    output.close();
}
