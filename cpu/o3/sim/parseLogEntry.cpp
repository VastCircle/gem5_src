#include "parse_log.h"

#include <regex>


// 正则表达式用于匹配日志格式
std::optional<LogEntry> parseLogEntry(const std::string& log) {
    std::regex pattern(R"(\[(\d+)\] pc=\[([0-9a-fA-F]+)\] W\[r\s?(\d+)=([0-9a-fA-F]+)\]\[(\d+)\] R\[r\s?(\d+)=([0-9a-fA-F]+)\]\[(\d+)\] R\[r\s?(\d+)=([0-9a-fA-F]+)\]\[(\d+)\] inst=\[([0-9a-fA-F]+)\] asm\[(.*?)\])");
    std::smatch match;
    
    if (std::regex_match(log, match, pattern)) {
        return LogEntry{
            static_cast<bool>(std::stoi(match[1])),
            std::stoull(match[2], nullptr, 16),  // pc 
            static_cast<uint8_t>(std::stoi(match[3])),  // writeReg 
            std::stoull(match[4], nullptr, 16),  // writeValue 
            static_cast<bool>(std::stoi(match[5])),  // writeExtra 
            static_cast<uint8_t>(std::stoi(match[6])),  // readReg1 
            std::stoull(match[7], nullptr, 16),  // readValue1 
            static_cast<bool>(std::stoi(match[8])),  // writeExtra 
            static_cast<uint8_t>(std::stoi(match[9])),  // readReg2 (转换为 uint8_t)
            std::stoull(match[10], nullptr, 16),  // readValue2 
            static_cast<bool>(std::stoi(match[11])),  // writeExtra 
            static_cast<uint32_t>(std::stoul(match[12], nullptr, 16)),
            match[13]  // asm_str
        };
    }
    return std::nullopt;  // 如果匹配失败，返回空
}


// #include <fstream>
// #include "parse_log.h"

// int main() {
//     // 示例：带偏移量的指令
//     // InstructionParser::InstructionInit();
//     std::ifstream file("log.txt");
//         if (!file) {
//             std::cerr << "Error opening file!" << std::endl;
//         }

//         std::string line;
//         int lineNumber = 0;
//         // 逐行读取文件
//         while (std::getline(file, line)) {
//             lineNumber++;
//             // 调用解析函数
//             auto entry = parseLogEntry(line);
//             // if (entry) { // 一条指令
//             //     // std::cout << std::hex << entry->pc << static_cast<int>(entry->writeReg) << std::endl;

//             // }
//             if (entry) {
//                 std::cout << "PC: " << std::hex << entry->pc << static_cast<int>(entry->writeReg) << "\n";
//                 } else {
//                     std::cout << "Failed to parse log entry." << std::endl;
//                 }

//         }
//         file.close();
        
//     return 0;
// }


// int main() {
//     // 示例日志行
//     //    std::string log = "[1] pc=[0000000080001cda] W[r13=0000000080001cdc][1] R[r 4=0000000000000000] R[r 5=0000000000000000] inst=[0000bfdd] asm[c.j     pc - 10]";
//     std::string log = "[1] pc=[0000000080000412] W[r 6=0000000000000040][1] R[r29=000000008002af14] R[r 0=0000000000000000] inst=[000ea303] asm[lw      t1, 0(t4)]";

//     auto logEntry = parseLogEntry(log);
//     if (logEntry) {
//         std::cout << "Index: " << logEntry->index << "\n"
//                   << "PC: " << std::hex << logEntry->pc << "\n"  // 以16进制输出 PC
//                   << "Write Reg: " << static_cast<int>(logEntry->writeReg) << "\n"
//                   << "Write Value: " << std::hex << logEntry->writeValue << "\n"  // 以16进制输出 Write Value
//                   << "Write Extra: " << logEntry->writeExtra << "\n"
//                   << "Read Reg1: " << static_cast<int>(logEntry->readReg1) << "\n"
//                   << "Read Value1: " << std::hex << logEntry->readValue1 << "\n"  // 以16进制输出 Read Value1
//                   << "Read Reg2: " << static_cast<int>(logEntry->readReg2) << "\n"
//                   << "Read Value2: " << std::hex << logEntry->readValue2 << "\n"  // 以16进制输出 Read Value2
//                   << "Instruction: " << std::hex << logEntry->inst << "\n"  // 以16进制输出 Instruction
//                   << "ASM: " << logEntry->asm_str << "\n";
//     } else {
//         std::cout << "Failed to parse log entry." << std::endl;
//     }

//     return 0;
// }

