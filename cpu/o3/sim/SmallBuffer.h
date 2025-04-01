// SmallBuffer.h
#ifndef SMALLBUFFER_H
#define SMALLBUFFER_H

#include "common.h"
#include <cstdint>
#include "queue"
#include <fstream>
#include "parse_log.h"


class SmallBuffer {
    public:
        struct InstructionInfo {
            LogEntry entry;
            uint64_t pc;
            std::string instruction;
            std::string asm_str;
            bool is_stride_load;
            // uint32_t stride;
            // uint32_t loop_number;
            uint64_t reg_value;
            bool isTainted1;
            bool isTainted2;
            bool valid;
        };
        SmallBuffer(size_t size);
        void ClearBuffer();
        void pushInstruction(InstructionInfo &info);
        void updateStrideLoadPC(uint64_t new_pc);
        // bool shouldDiscard(const InstructionInfo& inst);
        void printBuffer(std::ofstream &outfile, uint64_t FinalLoadPC,uint16_t loopNumber);
    private:
        std::queue<InstructionInfo> fifo;
        size_t max_size;
        uint64_t stride_load_pc;
};

#endif

