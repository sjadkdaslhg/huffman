#include "compress.h"
#include "decompress.h"
#include <iostream>
#include <vector>
#include <string>


int main(const int argc, char* argv[]) {
    const std::vector<std::string> args(argv, argv + argc);
    if (argc < 3) {
        std::cerr << "--compress <file> (--password <password>) : Compress mode\n";
        std::cerr << "--decompress <file> (--password <password>) : Decompress mode\n";
        return 1;
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
        std::string file_name, file_type;
        if (last_dot == std::string::npos) {
            file_name = file;
            file_type = "";
        }
        else {
            file_name = file.substr(0, last_dot);
            file_type = file.substr(last_dot + 1);
        }
        std::string password;
        if (argc >= 5 && args[3] == "--password")
            password = args[4];
        if (file_type.length() > 255) {
            std::cerr << "Extension too long\n";
            return 1;
        }
        compress(file_path, directory_path + file_name + ".huffman", password, file_type);
        std::cout << "Output path : " << directory_path + file_name + ".huffman\n";
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
            file_name = file.substr(0, last_dot);
            file_type = file.substr(last_dot + 1);
        }
        if (file_type != "huffman") {
            std::cerr << "Not compressed file\n";
            return 1;
        }
        std::string password;
        if (argc >= 5 && args[3] == "--password")
            password = args[4];
        std::string extension = decompress(file_path, directory_path + file_name, password);
        if (extension.empty())
            std::cout << "Output path : " << directory_path + file_name << '\n';
        else
            std::cout << "Output path : " << directory_path + file_name + '.' + extension << '\n';
    }
    else {
        std::cerr << "--compress <file> (--password <password>) : Compress mode\n";
        std::cerr << "--decompress <file> (--password <password>) : Decompress mode\n";
        return 1;
    }
}