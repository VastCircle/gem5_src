#include "TaintTracker.h"


namespace gem5 {
namespace o3 {


std::tuple<bool,bool,bool> TaintTracker::processLogEntry(const LogEntry& entry,bool is_load) {
    bool isTainted1 = isRegTainted(entry.readReg1) && entry.readExtra1; // Check if first source register is tainted
    bool isTainted2 = isRegTainted(entry.readReg2) && entry.readExtra2; // Check if second source register is tainted

    if ((isTainted1 || isTainted2)) {
        // If any source register is tainted, the destination register will also be tainted
        regTable[entry.writeReg] = entry.writeExtra;
        if (is_load) {
            final_load_pc = entry.pc;
        }
        return std::make_tuple(entry.writeExtra,isTainted1,isTainted2);
    } else {
        // If both source registers are clean, reset the destination register
        regTable[entry.writeReg] = false;
        return std::make_tuple(false,false,false);
    }
}


void TaintTracker::printRegTable() const {
    for (int i = 0; i < 32; ++i) {
        std::cout << "Register " << i << ": " << (regTable[i] ? "Tainted" : "Clean") << std::endl;
    }
}

void TaintTracker::TaintReg(uint8_t reg) {
    // std::cout << "Tainting register " << static_cast<int>(reg) << std::endl;
    regTable[reg] = true;
}

bool TaintTracker::isRegTainted(uint8_t reg) const {
    return regTable[reg];
}

void TaintTracker::clearVTTIfSeen() {
        // If seen == 1, clear all the taints in VTT
        std::fill(regTable.begin(), regTable.end(), false);
        // std::cout << "VTT cleared due to seen flag being 1." << std::endl;
}
}
}