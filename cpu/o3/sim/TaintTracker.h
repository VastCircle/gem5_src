#ifndef TAINT_TRACKER_H
#define TAINT_TRACKER_H

#include <vector>
#include <optional>
#include <iostream>
#include <cstdint>
#include "dyn_inst.hh"
#include "parse_log.h"

namespace gem5 {
namespace o3 {

class TaintTracker {
private:
    std::vector<bool> regTable;  // 32-bit register table, each entry is polluted (true) or clean (false)
    uint64_t final_load_pc ;

public:
    TaintTracker() : regTable(32, false) ,final_load_pc(0) {}  // Initialize all registers as clean

    std::tuple<bool,bool,bool> processLogEntry(DynInstPtr &head_inst);
    void printRegTable() const;
    bool isRegTainted(uint8_t reg) const;
    uint64_t GetFinalLoadPC() const { return final_load_pc; }
    void ClearFinalLoadPC() { final_load_pc = 0; }
    void TaintReg(uint8_t reg) ;
    void clearVTTIfSeen() ;
};
}
}

#endif // TAINT_TRACKER_H