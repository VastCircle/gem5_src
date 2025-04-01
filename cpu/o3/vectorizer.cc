#include "cpu/o3/vectorizer.hh"

#include <list>

#include "base/logging.hh"
#include "cpu/o3/dyn_inst.hh"
#include "cpu/o3/limits.hh"
#include "debug/Fetch.hh"
#include "debug/ROB.hh"
#include "debug/RPT.hh"
#include "params/BaseO3CPU.hh"

namespace gem5
{


namespace o3
{

Vectorizer::Vectorizer(CPU *_cpu, const BaseO3CPUParams &params) 
    : cpu(_cpu)
{ }


} // namespace o3
} // namespace gem5