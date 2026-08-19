#ifndef HUFFMAN_TREE
#define HUFFMAN_TREE


#include <memory>
#include <vector>

// 哈夫曼树结点
struct Node {
    // 字节值
    unsigned char byte_value;
    // 字节出现次数
    unsigned long long byte_count;
    // 左子结点
    std::shared_ptr<Node> left_child;
    // 右子结点
    std::shared_ptr<Node> right_child;
    // 构造函数
    Node(unsigned char byte_value, unsigned long long byte_count, std::shared_ptr<Node> left_child, std::shared_ptr<Node> right_child);
};

// 构建哈夫曼树，返回根结点和原始文件字节总数
std::shared_ptr<Node> buildTree(const std::vector<unsigned long long>& counts);


#endif
