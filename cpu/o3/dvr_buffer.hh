// SmallBuffer.h
#ifndef SMALLBUFFER_H
#define SMALLBUFFER_H


#include "queue"
#include <string>
#include <utility>
#include <vector>

#include "base/statistics.hh"
#include "base/types.hh"
#include "cpu/inst_seq.hh"
#include "cpu/o3/dyn_inst_ptr.hh"
#include "cpu/o3/limits.hh"
#include "cpu/reg_class.hh"
#include "enums/SMTQueuePolicy.hh"

namespace gem5 {

struct BaseO3CPUParams;

namespace o3 {

class CPU;

class DvrBuffer {
    public:
        CPU *cpu;
        DvrBuffer(CPU *_cpu, const BaseO3CPUParams &params);
        void ClearBuffer();
        void pushInstruction(DynInstPtr &inst);
        // void updateStrideLoadPC(uint64_t new_pc);
        // void printBuffer(std::ofstream &outfile, uint64_t FinalLoadPC,uint16_t loopNumber);
    private:
        std::queue<DynInstPtr> fifo;
        size_t max_size;
        uint64_t stride_load_pc;
};
}
}

#endif

