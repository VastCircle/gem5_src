
#include "TaintTracker.hh"
#include "base/logging.hh"
#include "cpu/o3/dyn_inst.hh"
#include "cpu/o3/limits.hh"
#include "debug/Fetch.hh"
#include "debug/ROB.hh"
#include "debug/TTK.hh"
#include "params/BaseO3CPU.hh"

namespace gem5 {
namespace o3 {


TaintTracker::TaintTracker(CPU *_cpu, const BaseO3CPUParams &params) :   // Initialize all registers as clean
    cpu(_cpu),
    regTable(params.numPhysIntRegs, false),
    final_load_pc(0) {}



std::tuple<bool,bool,bool> TaintTracker::process(DynInstPtr &inst) {

    const StaticInstPtr &staticInst = inst->staticInst;

    // 检查所有源寄存器
    int num_src_regs = staticInst->numSrcRegs();


    bool * isTainted  = nullptr;
    bool  isTaintedAll = false;
    try {
        isTainted = new bool[num_src_regs];
    }catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
    }

    for (int i = 0; i < inst->numSrcs(); i++) {
        // 架构寄存器
        // const RegId &src_reg = staticInst->srcRegIdx(i);
        // isTainted[i] = src_reg.isRenameable() && isRegTainted(src_reg.index());
        isTainted[i] = isRegTainted(inst->renamedSrcIdx(i)->index());
        isTaintedAll = isTainted[i] | isTaintedAll;
    }

    if (isTaintedAll) {
        TaintReg(inst);
        if (inst->isLoad()) {
            final_load_pc = inst->pcState().instAddr();
        }
    } else {
        // If both source registers are clean, reset the destination register
        UnTaintReg(inst);
    }

    return std::make_tuple(false,false,false);
}


void TaintTracker::printRegTable() const {
    for (int i = 0; i < 32; ++i) {
        std::cout << "Register " << i << ": " << (regTable[i] ? "Tainted" : "Clean") << std::endl;
    }
}

void TaintTracker::TaintReg(RegIndex reg) {
    regTable[reg] = true;
}

void TaintTracker::UnTaintReg(RegIndex reg) {
    regTable[reg] = false;
}

void TaintTracker::TaintReg(DynInstPtr &inst) {
    for (int i = 0 ; i < inst->numDestRegs();i++) {
        TaintReg(inst->renamedDestIdx(i)->index()); // 污染目标寄存器
    }
    DPRINTF(TTK, "PC %s, TaintReg[%d]\n", inst->pcState()
           );
}

void TaintTracker::UnTaintReg(DynInstPtr &inst) {
    for (int i = 0 ; i < inst->numDestRegs();i++) {
        UnTaintReg(inst->renamedDestIdx(i)->index()); // 污染目标寄存器
    }
    DPRINTF(TTK, "PC %s, UnTaintReg[%d]\n", inst->pcState()
           );
}

bool TaintTracker::isRegTainted(RegIndex reg) const {
    return regTable[reg];
}

void TaintTracker::clearVTTIfSeen() {
        // If seen == 1, clear all the taints in VTT
        std::fill(regTable.begin(), regTable.end(), false);
        // std::cout << "VTT cleared due to seen flag being 1." << std::endl;
}
}
}