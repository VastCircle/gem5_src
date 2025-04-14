#ifndef __ARCH_RISCV_VECTORIZER_HH__
#define __ARCH_RISCV_VECTORIZER_HH__

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

#include "arch/generic/decode_cache.hh"
#include "arch/generic/decoder.hh"
#include "arch/riscv/insts/vector.hh"
#include "arch/riscv/types.hh"
#include "base/logging.hh"
#include "base/types.hh"
#include "cpu/static_inst.hh"
#include "debug/Decode.hh"
#include "params/RiscvDecoder.hh"

namespace gem5 {

namespace RiscvISA {

class Vectorizer {
    public:
        Vectorizer();
        // 识别指令并返回向量指令信息
        struct VectorMapping {
            std::string vector_name;
            uint32_t vector_match;
            uint32_t vector_mask;
        };
        std::pair<bool , VectorMapping> get_vector_mapping(ExtMachInst instr);
        bool get_vector(ExtMachInst &instr);
    private:
        struct ScalarInstruction {
            std::string name;
            uint32_t match;
            uint32_t mask;
            bool is_imm;  // 是否是立即数指令
        };

        // 标量指令表 (按opcode分组)
        std::unordered_map<uint32_t, std::vector<ScalarInstruction>> scalar_instructions;
    
        // 标量到向量指令映射表
        std::unordered_map<std::string, VectorMapping> scalar_to_vector_map;

        void init_scalar_instructions();
        void init_scalar_to_vector_map();
        ScalarInstruction identify_instruction(ExtMachInst instr);

};

}

}

#endif