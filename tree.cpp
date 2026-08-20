#include "tree.h"
#include <memory>
#include <vector>
#include <queue>
#include <stdexcept>


Node::Node(const unsigned char byte_value, const unsigned long long byte_count, std::shared_ptr<Node> left_child, std::shared_ptr<Node> right_child) :
    byte_value(byte_value), byte_count(byte_count), left_child(std::move(left_child)), right_child(std::move(right_child)) {}

// 先比较字节出现次数，再比较字节值
namespace {
    struct compare {
        bool operator()(const std::shared_ptr<Node>& left, const std::shared_ptr<Node>& right) const {
            if (left->byte_count != right->byte_count)
                return left->byte_count > right->byte_count; // 构建哈夫曼树，使用小根堆
            return left->byte_value < right->byte_value;
        }
    };
}


std::shared_ptr<Node> buildTree(const std::vector<unsigned long long>& counts) {
    if (counts.size() != 256)
        throw std::runtime_error("Invalid byte_count array");
    std::vector<std::shared_ptr<Node>> nodes;
    for (int i = 0; i < 256; i++)
        if (counts[i] > 0)
            nodes.push_back(std::make_shared<Node>(i, counts[i], nullptr, nullptr));
    std::priority_queue<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>, compare> priority_queue{nodes.begin(), nodes.end()};
    for (int i = 0; i < 255; i++) {
        std::shared_ptr<Node> left = priority_queue.top();
        priority_queue.pop();
        std::shared_ptr<Node> right = priority_queue.top();
        priority_queue.pop();
        priority_queue.push(std::make_shared<Node>(0, left->byte_count + right->byte_count, left, right));
    }
    return priority_queue.top();
}
