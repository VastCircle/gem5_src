#include "arch/riscv/vectorizer.hh"
#include "arch/riscv/decoder.hh"
#include "arch/riscv/isa.hh"
#include "arch/riscv/types.hh"
#include "base/bitfield.hh"
#include "debug/Decode.hh"

#include "arch/riscv/encoding.out.h"

namespace gem5 {
namespace RiscvISA {

Vectorizer::Vectorizer() {
    init_scalar_instructions();
    init_scalar_to_vector_map();
}

// 初始化标量指令表
void Vectorizer::init_scalar_instructions() {
    // 算术指令
    scalar_instructions[0x33 & 0x7f].push_back({"add", MATCH_ADD, MASK_ADD, false});
    scalar_instructions[0x3b & 0x7f].push_back({"addw", MATCH_ADDW, MASK_ADDW, false});
    scalar_instructions[0x13 & 0x7f].push_back({"addi", MATCH_ADDI, MASK_ADDI, true});
    scalar_instructions[0x1b & 0x7f].push_back({"addiw", MATCH_ADDIW, MASK_ADDIW, true});
    scalar_instructions[0x40000033 & 0x7f].push_back({"sub", MATCH_SUB, MASK_SUB, false});
    scalar_instructions[0x4000003b & 0x7f].push_back({"subw", MATCH_SUBW, MASK_SUBW, false});
    
    // 逻辑指令
    scalar_instructions[0x7033 & 0x7f].push_back({"and", MATCH_AND, MASK_AND, false});
    scalar_instructions[0x7013 & 0x7f].push_back({"andi", MATCH_ANDI, MASK_ANDI, true});
    scalar_instructions[0x6033 & 0x7f].push_back({"or", MATCH_OR, MASK_OR, false});
    scalar_instructions[0x6013 & 0x7f].push_back({"ori", MATCH_ORI, MASK_ORI, true});
    scalar_instructions[0x4033 & 0x7f].push_back({"xor", MATCH_XOR, MASK_XOR, false});
    scalar_instructions[0x4013 & 0x7f].push_back({"xori", MATCH_XORI, MASK_XORI, true});
    
    // 移位指令
    scalar_instructions[0x1033 & 0x7f].push_back({"sll", MATCH_SLL, MASK_SLL, false});
    scalar_instructions[0x1013 & 0x7f].push_back({"slli", MATCH_SLLI, MASK_SLLI, true});
    scalar_instructions[0x101b & 0x7f].push_back({"slliw", MATCH_SLLIW, MASK_SLLIW, true});
    scalar_instructions[0x103b & 0x7f].push_back({"sllw", MATCH_SLLW, MASK_SLLW, false});
    scalar_instructions[0x5033 & 0x7f].push_back({"srl", MATCH_SRL, MASK_SRL, false});
    scalar_instructions[0x5013 & 0x7f].push_back({"srli", MATCH_SRLI, MASK_SRLI, true});
    scalar_instructions[0x501b & 0x7f].push_back({"srliw", MATCH_SRLIW, MASK_SRLIW, true});
    scalar_instructions[0x503b & 0x7f].push_back({"srlw", MATCH_SRLW, MASK_SRLW, false});
    scalar_instructions[0x40005033 & 0x7f].push_back({"sra", MATCH_SRA, MASK_SRA, false});
    scalar_instructions[0x40005013 & 0x7f].push_back({"srai", MATCH_SRAI, MASK_SRAI, true});
    scalar_instructions[0x4000501b & 0x7f].push_back({"sraiw", MATCH_SRAIW, MASK_SRAIW, true});
    scalar_instructions[0x4000503b & 0x7f].push_back({"sraw", MATCH_SRAW, MASK_SRAW, false});
    
    // 比较指令
    scalar_instructions[0x2033 & 0x7f].push_back({"slt", MATCH_SLT, MASK_SLT, false});
    scalar_instructions[0x2013 & 0x7f].push_back({"slti", MATCH_SLTI, MASK_SLTI, true});
    scalar_instructions[0x3033 & 0x7f].push_back({"sltu", MATCH_SLTU, MASK_SLTU, false});
    scalar_instructions[0x3013 & 0x7f].push_back({"sltiu", MATCH_SLTIU, MASK_SLTIU, true});
    
    // 乘除指令
    scalar_instructions[0x2000033 & 0x7f].push_back({"mul", MATCH_MUL, MASK_MUL, false});
    scalar_instructions[0x200003b & 0x7f].push_back({"mulw", MATCH_MULW, MASK_MULW, false});
    scalar_instructions[0x2001033 & 0x7f].push_back({"mulh", MATCH_MULH, MASK_MULH, false});
    scalar_instructions[0x2002033 & 0x7f].push_back({"mulhsu", MATCH_MULHSU, MASK_MULHSU, false});
    scalar_instructions[0x2003033 & 0x7f].push_back({"mulhu", MATCH_MULHU, MASK_MULHU, false});
    scalar_instructions[0x2004033 & 0x7f].push_back({"div", MATCH_DIV, MASK_DIV, false});
    scalar_instructions[0x200403b & 0x7f].push_back({"divw", MATCH_DIVW, MASK_DIVW, false});
    scalar_instructions[0x2005033 & 0x7f].push_back({"divu", MATCH_DIVU, MASK_DIVU, false});
    scalar_instructions[0x200503b & 0x7f].push_back({"divuw", MATCH_DIVUW, MASK_DIVUW, false});
    scalar_instructions[0x2006033 & 0x7f].push_back({"rem", MATCH_REM, MASK_REM, false});
    scalar_instructions[0x200603b & 0x7f].push_back({"remw", MATCH_REMW, MASK_REMW, false});
    scalar_instructions[0x2007033 & 0x7f].push_back({"remu", MATCH_REMU, MASK_REMU, false});
    scalar_instructions[0x200703b & 0x7f].push_back({"remuw", MATCH_REMUW, MASK_REMUW, false});
    
    // 压缩指令
    scalar_instructions[0x1 & 0x7f].push_back({"c_addi", MATCH_C_ADDI, MASK_C_ADDI, true});
    scalar_instructions[0x9002 & 0x7f].push_back({"c_add", MATCH_C_ADD, MASK_C_ADD, false});
    scalar_instructions[0x2001 & 0x7f].push_back({"c_addiw", MATCH_C_ADDIW, MASK_C_ADDIW, true});
    scalar_instructions[0x9c21 & 0x7f].push_back({"c_addw", MATCH_C_ADDW, MASK_C_ADDW, false});
    scalar_instructions[0x8c61 & 0x7f].push_back({"c_and", MATCH_C_AND, MASK_C_AND, false});
    scalar_instructions[0x8801 & 0x7f].push_back({"c_andi", MATCH_C_ANDI, MASK_C_ANDI, true});
    scalar_instructions[0x8002 & 0x7f].push_back({"c_mv", MATCH_C_MV, MASK_C_MV, false});
    scalar_instructions[0x8c41 & 0x7f].push_back({"c_or", MATCH_C_OR, MASK_C_OR, false});
    scalar_instructions[0x2 & 0x7f].push_back({"c_slli", MATCH_C_SLLI, MASK_C_SLLI, true});
    scalar_instructions[0x8401 & 0x7f].push_back({"c_srai", MATCH_C_SRAI, MASK_C_SRAI, true});
    scalar_instructions[0x8001 & 0x7f].push_back({"c_srli", MATCH_C_SRLI, MASK_C_SRLI, true});
    scalar_instructions[0x8c01 & 0x7f].push_back({"c_sub", MATCH_C_SUB, MASK_C_SUB, false});
    scalar_instructions[0x9c01 & 0x7f].push_back({"c_subw", MATCH_C_SUBW, MASK_C_SUBW, false});
    scalar_instructions[0x8c21 & 0x7f].push_back({"c_xor", MATCH_C_XOR, MASK_C_XOR, false});
}

// 初始化标量到向量指令映射表
void Vectorizer::init_scalar_to_vector_map() {
    // 算术指令映射
    scalar_to_vector_map["add"] = {"vadd_vx", MATCH_VADD_VX, MASK_VADD_VX};
    scalar_to_vector_map["addi"] = {"vaddi_v", MATCH_ADDI, MASK_ADDI};
    scalar_to_vector_map["addw"] = {"vadd_vx", MATCH_VADD_VX, MASK_VADD_VX};
    scalar_to_vector_map["addiw"] = {"vaddi_v", MATCH_ADDI, MASK_ADDI};
    scalar_to_vector_map["sub"] = {"vsub_vx", MATCH_VSUB_VX, MASK_VSUB_VX};
    scalar_to_vector_map["subw"] = {"vsub_vx", MATCH_VSUB_VX, MASK_VSUB_VX};
    
    // 逻辑指令映射
    scalar_to_vector_map["and"] = {"vand_vx", MATCH_VAND_VX, MASK_VAND_VX};
    scalar_to_vector_map["andi"] = {"vandi_v", MATCH_ANDI, MASK_ANDI};
    scalar_to_vector_map["or"] = {"vor_vx", MATCH_VOR_VX, MASK_VOR_VX};
    scalar_to_vector_map["ori"] = {"vori_v", MATCH_ORI, MASK_ORI};
    scalar_to_vector_map["xor"] = {"vxor_vx", MATCH_VXOR_VX, MASK_VXOR_VX};
    scalar_to_vector_map["xori"] = {"vxori_v", MATCH_XORI, MASK_XORI};
    
    // 移位指令映射
    scalar_to_vector_map["sll"] = {"vsll_vx", MATCH_VSLL_VX, MASK_VSLL_VX};
    scalar_to_vector_map["slli"] = {"vsll_vx", MATCH_VSLL_VX, MASK_VSLL_VX};
    scalar_to_vector_map["sllw"] = {"vsll_vx", MATCH_VSLL_VX, MASK_VSLL_VX};
    scalar_to_vector_map["slliw"] = {"vsll_vx", MATCH_VSLL_VX, MASK_VSLL_VX};
    scalar_to_vector_map["srl"] = {"vsrl_vx", MATCH_VSRL_VX, MASK_VSRL_VX};
    scalar_to_vector_map["srli"] = {"vsrl_vx", MATCH_VSRL_VX, MASK_VSRL_VX};
    scalar_to_vector_map["srlw"] = {"vsrl_vx", MATCH_VSRL_VX, MASK_VSRL_VX};
    scalar_to_vector_map["srliw"] = {"vsrl_vx", MATCH_VSRL_VX, MASK_VSRL_VX};
    scalar_to_vector_map["sra"] = {"vsra_vx", MATCH_VSRA_VX, MASK_VSRA_VX};
    scalar_to_vector_map["srai"] = {"vsra_vx", MATCH_VSRA_VX, MASK_VSRA_VX};
    scalar_to_vector_map["sraw"] = {"vsra_vx", MATCH_VSRA_VX, MASK_VSRA_VX};
    scalar_to_vector_map["sraiw"] = {"vsra_vx", MATCH_VSRA_VX, MASK_VSRA_VX};
    
    // 比较指令映射
    scalar_to_vector_map["slt"] = {"vmslt_vx", MATCH_VMSLT_VX, MASK_VMSLT_VX};
    scalar_to_vector_map["slti"] = {"vmslt_vx", MATCH_VMSLT_VX, MASK_VMSLT_VX};
    scalar_to_vector_map["sltu"] = {"vmsltu_vx", MATCH_VMSLTU_VX, MASK_VMSLTU_VX};
    scalar_to_vector_map["sltiu"] = {"vmsltu_vx", MATCH_VMSLTU_VX, MASK_VMSLTU_VX};
    
    // 乘除指令映射
    scalar_to_vector_map["mul"] = {"vmul_vx", MATCH_VMUL_VX, MASK_VMUL_VX};
    scalar_to_vector_map["mulw"] = {"vmul_vx", MATCH_VMUL_VX, MASK_VMUL_VX};
    scalar_to_vector_map["mulh"] = {"vmulh_vx", MATCH_VMULH_VX, MASK_VMULH_VX};
    scalar_to_vector_map["mulhsu"] = {"vmulhsu_vx", MATCH_VMULHSU_VX, MASK_VMULHSU_VX};
    scalar_to_vector_map["mulhu"] = {"vmulhu_vx", MATCH_VMULHU_VX, MASK_VMULHU_VX};
    scalar_to_vector_map["div"] = {"vdiv_vx", MATCH_VDIV_VX, MASK_VDIV_VX};
    scalar_to_vector_map["divw"] = {"vdiv_vx", MATCH_VDIV_VX, MASK_VDIV_VX};
    scalar_to_vector_map["divu"] = {"vdivu_vx", MATCH_VDIVU_VX, MASK_VDIVU_VX};
    scalar_to_vector_map["divuw"] = {"vdivu_vx", MATCH_VDIVU_VX, MASK_VDIVU_VX};
    scalar_to_vector_map["rem"] = {"vrem_vx", MATCH_VREM_VX, MASK_VREM_VX};
    scalar_to_vector_map["remw"] = {"vrem_vx", MATCH_VREM_VX, MASK_VREM_VX};
    scalar_to_vector_map["remu"] = {"vremu_vx", MATCH_VREMU_VX, MASK_VREMU_VX};
    scalar_to_vector_map["remuw"] = {"vremu_vx", MATCH_VREMU_VX, MASK_VREMU_VX};
    
    // 压缩指令映射
    scalar_to_vector_map["c_add"] = {"vadd_vx", MATCH_VADD_VX, MASK_VADD_VX};
    scalar_to_vector_map["c_addi"] = {"vadd_vx", MATCH_VADD_VX, MASK_VADD_VX};
    scalar_to_vector_map["c_addiw"] = {"vadd_vx", MATCH_VADD_VX, MASK_VADD_VX};
    scalar_to_vector_map["c_addw"] = {"vadd_vx", MATCH_VADD_VX, MASK_VADD_VX};
    scalar_to_vector_map["c_and"] = {"vand_vx", MATCH_VAND_VX, MASK_VAND_VX};
    scalar_to_vector_map["c_andi"] = {"vand_vx", MATCH_VAND_VX, MASK_VAND_VX};
    scalar_to_vector_map["c_mv"] = {"vadd_vx", MATCH_VADD_VX, MASK_VADD_VX};
    scalar_to_vector_map["c_or"] = {"vor_vx", MATCH_VOR_VX, MASK_VOR_VX};
    scalar_to_vector_map["c_slli"] = {"vsll_vx", MATCH_VSLL_VX, MASK_VSLL_VX};
    scalar_to_vector_map["c_srai"] = {"vsra_vx", MATCH_VSRA_VX, MASK_VSRA_VX};
    scalar_to_vector_map["c_srli"] = {"vsrl_vx", MATCH_VSRL_VX, MASK_VSRL_VX};
    scalar_to_vector_map["c_sub"] = {"vsub_vx", MATCH_VSUB_VX, MASK_VSUB_VX};
    scalar_to_vector_map["c_subw"] = {"vsub_vx", MATCH_VSUB_VX, MASK_VSUB_VX};
    scalar_to_vector_map["c_xor"] = {"vxor_vx", MATCH_VXOR_VX, MASK_VXOR_VX};
}


// 识别指令
Vectorizer::ScalarInstruction  Vectorizer::identify_instruction(ExtMachInst instr) {
    auto it = scalar_instructions.find(instr.opcode);
    if (it == scalar_instructions.end()) return {"unknown",0,0,false};
    
    for (const auto& entry : it->second) {
        if ((instr & entry.mask) == entry.match) {
            return entry;
        }
    }
    return {"unknown",0,0,false};
}

// 获取对应的向量指令
std::pair<bool ,Vectorizer::VectorMapping> Vectorizer::get_vector_mapping(ExtMachInst instr) {

    ScalarInstruction  scalar = identify_instruction(instr);
    auto it = scalar_to_vector_map.find(scalar.name);
    if (it != scalar_to_vector_map.end()) {
        DPRINTF(Decode, "Trans inst[%s] to vect inst[%s]\n",
            scalar.name, it->second.vector_name);
        return std::make_pair(scalar.is_imm,it->second);
    }
    return std::make_pair(false,VectorMapping{"unknown", 0, 0});
}

// TODO:暂时只把算数指令的处理放在这里
bool Vectorizer::get_vector(ExtMachInst &instr) {

    ExtMachInst vemi;
    
    auto vector = get_vector_mapping(instr);
    auto [name,match,mask] = vector.second;

    if (name != "unknown") {
        vemi.all = (vemi.all & ~mask) | match; // 全局替换
        instr = vemi;
        return false;
    }
    return vector.first;

}





}
}