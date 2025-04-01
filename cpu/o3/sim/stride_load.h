#ifndef STRIDE_LOAD_H
#define STRADE_LOAD_H

#include <utility>
#include <unordered_map>
#include "common.h"

class rptFiles :private ModeBase {
    private:
        class StrideMetadata {
            public:
                enum State { INIT, STEADY, TRANS, NOPRED };
                StrideMetadata();
                bool isValid() const;
                bool seen;  // 观察位
                State GetState() const { return state; }
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
                int64_t stride;
                uint64_t prev_addr;
                RPTEntry():tag(0),prev_addr(0),stride(0),StrideMetadata(){}
                RPTEntry(uint64_t tag):tag(tag),prev_addr(0),stride(0),StrideMetadata(){}
                void UpdateStride(bool corr,int64_t new_stride);
        };

        std::unordered_map<uint64_t, RPTEntry> rpt;

        void clearAllSeen();  // 清空所有表项的seen


    public:
        rptFiles(int sets);
        std::tuple<bool,bool,bool,bool> processRequest(uint64_t pc, uint64_t address) ;
        uint64_t stride_load_pc; 
        uint64_t pref_addr; // TODO:直接复制出来的话会不会更方便一些
        uint64_t prev_addr;
        int16_t  stride;
        void UpdatePrefAddr(uint16_t loopNumber) { pref_addr = prev_addr + loopNumber * stride; }
};

#endif // 