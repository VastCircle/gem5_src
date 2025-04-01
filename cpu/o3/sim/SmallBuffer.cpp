#include "SmallBuffer.h"
#include "iomanip"

namespace gem5 {
namespace o3 {


SmallBuffer::SmallBuffer(size_t size) : max_size(size), stride_load_pc(0) {}

void SmallBuffer::pushInstruction(InstructionInfo &info) {
        if (fifo.size() < max_size) {
            // std::cout << "Pushing instruction: " << info.instruction << std::endl;
            fifo.push(info);
        }
}

void SmallBuffer::updateStrideLoadPC(uint64_t new_pc) {
    stride_load_pc = new_pc;
    while (!fifo.empty() && fifo.front().valid) {
        fifo.pop();
    }
}

void SmallBuffer::ClearBuffer() {
    while (!fifo.empty()) {
        fifo.pop();
    }
}

// bool SmallBuffer::shouldDiscard(const InstructionInfo& inst) {
//         return inst.reg_id == 0; // 例如：丢弃某些特定的寄存器
// }

void SmallBuffer::printBuffer(uint64_t FinalLoadPC,uint16_t loopNumber) {
    if (FinalLoadPC == 0) {
        ClearBuffer();
        return;
    }
    std::queue<InstructionInfo> temp = fifo;
    DPRINTF(Buffer, "=== Loop Analysis ===\n");
    DPRINTF(Buffer, "LoopNum: %d\n", loopNumber);

    DPRINTF(Buffer, "%-10s %-20s %-5s %-5s %-5s %-5s\n",
        "PC", "ASM", "SL", "Imm", "T1", "T2");


    // outfile << std::left << std::setw(10) << "PC" << " "
    //     << std::setw(20) << "ASM" << " "
    //     << std::setw(5)  << "SL" << " "
    //     << std::setw(5)  << "Imm" << " "
    //     << std::setw(5)  << "T1" << " "
    //     << std::setw(5)  << "T2" << std::endl;

    while (!temp.empty()) {
        auto inst = temp.front();
        temp.pop();
        // outfile << inst.instruction << "\n";
        bool need_imm = !inst.entry.readExtra2;
        bool need_trans1 = inst.entry.readExtra1 && !inst.isTainted1;
        bool need_trans2 = inst.entry.readExtra2 && !inst.isTainted2;


     // 使用 gem5 调试输出（假设已定义调试标志 "LoopAnalysis"）
        DPRINTF(Buffer, "%-10#x %-20s %-5x %-5x %-5x %-5x\n",
        inst.pc,         // 地址用十六进制带 0x 前缀
        inst.asm_str.c_str(), // 汇编字符串
        inst.is_stride_load,  // 布尔值转十六进制
        need_imm,        // 整数用十六进制
        need_trans1,     // 整数用十六进制
        need_trans2);    // 整数用十六进制

        // std::cout << inst.instruction << "\n";
        if (inst.pc == FinalLoadPC) {
            break;
        }
    }


    while(!temp.empty()) {
        temp.pop();
    }
}

}
}
