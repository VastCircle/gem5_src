#include "InstructionParser.h"

#include <regex>

std::unordered_set<std::string> InstructionParser::loadInstructions;
std::unordered_set<std::string> InstructionParser::branchInstructions;
std::unordered_set<std::string> InstructionParser::addiInstructions;



InstructionParser::Instruction InstructionParser::instruction;

void InstructionParser::InstructionInit() {
    loadInstructions = {
        "lb", "lh", "lw", "ld", "lbu", "lhu", "lwu",
        "c.lw", "c.ld", "c.lq", "c.flw", "c.fld", "c.fsq",
    };
    // jal 就算了
    branchInstructions = {
        "beq", "bne", "blt", "bge", "bltu", "bgeu",
        "beqz", "bnez", "bltz", "bgez", "bltz", "bgez",
        "c.beqz", "c.bnez" ,
    };

    addiInstructions = {
        "addi", "addiw", "c.addi", "c.addiw"
    };
}

bool InstructionParser::isLoadInstruction(const std::string& asm_instruction) {
    std::regex pattern(R"(\s*([a-zA-Z0-9\.]+)\s+([a-zA-Z0-9]+)\s*,\s*(-?\d+)\(([a-zA-Z0-9]+)\))");
    std::smatch match;
    if (std::regex_search(asm_instruction, match, pattern)) {
        // 提取指令和操作数
        instruction.name = match[1];   // 指令名，如 "lbu"
        instruction.immediateValue = std::stoi(match[3]);   // 立即数，如 0
        return loadInstructions.find(instruction.name) != loadInstructions.end();
    }
    return false;
}

bool InstructionParser::isBranchInstruction(const std::string& asm_instruction) {
    std::regex pattern(R"(^\s*([a-zA-Z0-9\.]+)\b.*?\bpc\s*([\+\-])\s*(\d+))");
    std::smatch match;
    if (std::regex_search(asm_instruction, match, pattern)) {
        instruction.name = match[1];   // 指令名，如 "lbu"
        instruction.immediateValue = match[2] == "+" ? std::stoi(match[3]) : -std::stoi(match[3]);   // 立即数，如 0
        return branchInstructions.find(instruction.name) != branchInstructions.end();
    }
    return false;
}

bool InstructionParser::isAddiInstruction(const std::string& asm_instruction) {
    std::regex pattern(R"(^\s*([a-zA-Z0-9\.]+)\b.*?\s*(-?\d+)\s*$)");
    std::smatch match;
    if (std::regex_search(asm_instruction, match, pattern)) {
        instruction.name = match[1];   // 指令名，如 "lbu"
        instruction.immediateValue = std::stoi(match[2]);   // 立即数，如 0
        return addiInstructions.find(instruction.name) != addiInstructions.end();
    }
    return false;
}

// #include <fstream>
// #include "parse_log.h"

// int main() {
//     // 示例：带偏移量的指令
//     InstructionParser::InstructionInit();
//     std::ifstream file("test.txt");
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
//             if (entry) { // 一条指令
//                 std::string asm_instruction = entry->asm_str;
//                 if (InstructionParser::isAddiInstruction(asm_instruction)) {
//                     std::cout << "Instruction: " << InstructionParser::instruction.name  <<" imm " << InstructionParser::instruction.immediateValue << std::endl;
//                 }
//             }
//         }
//         file.close();
//     return 0;
// }

// // 判断是否为压缩指令 
// bool InstructionParser::isCompressedInstruction(uint32_t instruction) {
//     uint32_t opcode = instruction & 0x3;  
//     return (opcode == 0x03);  
// }

// // 判断是否为非压缩的 load 指令（例如 lw, lb, lh, lbu, lhu）
// bool InstructionParser::isLoadInstruction(uint32_t instruction) {
//     bool is_compressed = isCompressedInstruction(instruction);
//     if (is_compressed) {
//         uint32_t opcode = instruction & 0x7f;  // 获取最低 7 位 opcode
//         if (opcode != 0x03) {
//             return false;  // 只有 load/store 指令才继续判断
//         }
//         // 获取 funct3（取出 12 到 14 位）
//         uint32_t funct3 = (instruction >> 12) & 0x07;
//         // 判断是否是 load 类型的指令：lb, lh, lw, lbu, lhu
//         return (funct3 == 0x0 || funct3 == 0x1 || funct3 == 0x2 || funct3 == 0x4 || funct3 == 0x5);
//     }
//     else {
//         uint32_t opcode = instruction & 0x03;  // 获取最低 7 位 opcode
//         if (opcode == 0) {
//             uint16_t funct3 = (instruction >> 13) & 0x07;
//             // 判断是否是 C.LB, C.LH, C.LW（funct3 = 0x0, 0x1, 0x2）
//             return (funct3 == 0x0 || funct3 == 0x1 || funct3 == 0x2);
//         }
//         else   
//             return false;
//     }
// }