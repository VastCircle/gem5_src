#ifndef STRIDE_LOAD_H
#define STRIDE_LOAD_H

#include <utility>
#include <unordered_map>
#include "common.hh"

#include "base/statistics.hh"
#include "base/types.hh"
#include "cpu/inst_seq.hh"
#include "cpu/o3/dyn_inst_ptr.hh"
#include "cpu/o3/limits.hh"
#include "cpu/reg_class.hh"

namespace gem5 {

struct BaseO3CPUParams;

namespace o3{

class CPU;

class rptFiles  {
    private:
        /** Pointer to the CPU. */
        CPU *cpu;
        class StrideMetadata {
            public:
                enum State { INIT, STEADY, TRANS, NOPRED };
                StrideMetadata();
                bool isValid() const;
                bool seen;  // 观察位
                State GetState() const { return state; }
                bool GetCanPrf() const { return can_prf; }
                std::pair<bool, State> StateTrans(bool corr);
            private:
                State state;
                bool can_prf;
        };
        int nSets;
        class RPTEntry:public StrideMetadata {
            private:

            public:
                uint64_t tag;
                uint64_t prev_addr;
                int64_t stride;
                RPTEntry():StrideMetadata(),tag(0),prev_addr(0),stride(0){}
                RPTEntry(uint64_t tag):StrideMetadata(),tag(tag),prev_addr(0),stride(0){}
                void UpdateStride(bool corr,int64_t new_stride);
        };


        std::unordered_map<uint64_t, RPTEntry> rpt;

        void clearAllSeen();  // 清空所有表项的seen


    public:
        rptFiles(CPU *_cpu,const BaseO3CPUParams &params);
        std::tuple<bool,bool,bool,bool> processRequest(DynInstPtr &inst); ;
        bool CreateNewEntry(DynInstPtr &inst);
        bool GetRptStats(DynInstPtr &inst);
        std::tuple<bool,bool> UpdateEntrySeen(DynInstPtr &inst);
        bool UpdateEntry(DynInstPtr &inst);
        uint64_t stride_load_pc; 
        Addr pref_addr; // TODO:直接复制出来的话会不会更方便一些
        Addr prev_addr;
        int16_t  stride;
        void UpdatePrefAddr(uint16_t loopNumber) { pref_addr = prev_addr + loopNumber * stride; }
};
}
}

#endif // 