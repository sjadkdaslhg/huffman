# Huffman 压缩
基于 Huffman 编码的文件压缩

## 使用方法
使用CMake构建项目  
通过命令行参数指定压缩或解压、文件路径和密码
- `--compress <file> (--password <password>)` 压缩文件，可选密码
- `--decompress <file> (--password <password>)` 解压文件，压缩时如果设置密码，可以输入密码尝试解锁