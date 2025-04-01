#ifndef TAINT_TRACKER_H
#define TAINT_TRACKER_H

#include <vector>
#include <optional>
#include <iostream>
#include <cstdint>

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

class TaintTracker {
private:
    CPU *cpu;
    std::vector<bool> regTable;  // 32-bit register table, each entry is polluted (true) or clean (false)
    uint64_t final_load_pc ;

public:
    TaintTracker(CPU *_cpu, const BaseO3CPUParams &params);   // Initialize all registers as clean

    std::tuple<bool,bool,bool> process(DynInstPtr &head_inst);
    void printRegTable() const;

    bool isRegTainted(RegIndex reg) const;
    uint64_t GetFinalLoadPC() const { return final_load_pc; }
    void ClearFinalLoadPC() { final_load_pc = 0; }
    void TaintReg(DynInstPtr &inst);
    void TaintReg(RegIndex reg) ;
    void UnTaintReg(DynInstPtr &inst);
    void UnTaintReg(RegIndex reg) ;
    void clearVTTIfSeen() ;
};
}
}

#endif // TAINT_TRACKER_H