#ifndef INSTRUCTIONPARSER_H
#define INSTRUCTIONPARSER_H

#include <cstdint>
#include <unordered_set>
#include <iostream>
#include <string>

class InstructionParser {
private:
    static std::unordered_set<std::string> loadInstructions;
    static std::unordered_set<std::string> branchInstructions;
    static std::unordered_set<std::string> addiInstructions;
public:
    struct Instruction {
        std::string name;
        int immediateValue;
    };
    static Instruction instruction;
    static void InstructionInit() ;
    // 判断是否为压缩指令
    static bool isLoadInstruction(const std::string& asm_instruction);
    static bool isBranchInstruction(const std::string& asm_instruction);
    static bool isAddiInstruction(const std::string& asm_instruction);
};
#endif // INSTRUCTIONPARSER_H


