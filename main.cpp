#include "compress.h"
#include "decompress.h"
#include <iostream>
#include <vector>
#include <string>


int main(const int argc, char* argv[]) {
    const std::vector<std::string> args(argv + 1, argv + argc);
    if (argc < 3) {
        std::cerr << "--compress <file> : Compress mode\n";
        std::cerr << "--decompress <file> : Decompress mode\n";
        return 0;
    }
    if (args[1] == "--compress") {
        const std::string& file_path = args[2];
        std::string directory_path, file;
        const size_t last_slash = file_path.find_last_of('/');
        if (last_slash == std::string::npos) {
            directory_path = "";
            file = file_path;
        }
        else {
            directory_path = file_path.substr(0, last_slash + 1);
            file = file_path.substr(last_slash + 1);
        }
        const size_t last_dot = file.find_last_of('.');
        std::string file_name;
        if (last_dot == std::string::npos)
            file_name = file;
        else
            file_name = file.substr(last_dot);
        compress(file_path, directory_path + file_name + ".huffman");
    }
    else if (args[1] == "--decompress") {
        const std::string& file_path = args[2];
        std::string directory_path, file;
        const size_t last_slash = file_path.find_last_of('/');
        if (last_slash == std::string::npos) {
            directory_path = "";
            file = file_path;
        }
        else {
            directory_path = file_path.substr(0, last_slash + 1);
            file = file_path.substr(last_slash + 1);
        }
        const size_t last_dot = file.find_last_of('.');
        std::string file_name, file_type;
        if (last_dot == std::string::npos) {
            file_name = file;
            file_type = "";
        }
        else {
            file_name = file.substr(last_dot);
            file_type = file.substr(last_dot + 1);
        }
        if (file_type != "huffman") {
            std::cerr << "Not compressed file\n";
            return 0;
        }
        decompress(file_path, directory_path + file_name + ".txt");
    }
    else {
        std::cerr << "Unknown command\n";
        return 0;
    }
}