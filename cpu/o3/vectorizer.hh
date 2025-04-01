#ifndef __CPU_O3_VECTORIZER_HH__
#define __CPU_O3_VECTORIZER_HH__

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

namespace gem5
{
struct BaseO3CPUParams;

namespace o3
{

class CPU;

class Vectorizer {
    private:
    CPU *cpu;

    public:
    Vectorizer(CPU *_cpu, const BaseO3CPUParams &params);
};

}
}


#endif