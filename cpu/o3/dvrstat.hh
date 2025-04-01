#ifndef __CPU_O3_DVRSTAT_HH__
#define __CPU_O3_DVRSTAT_HH__

namespace gem5
{
namespace o3
{
class Mode {
    public : 
        enum ModeEnum {INIT , DISCOVER , WAIT , VECTOR};
        enum ModeTransEnum {KEEP, INIT2DISCOVER,DISCOVER2WAIT, WAIT2INIT,WAIT2DISCOVER};
    private : 
        ModeEnum mode;
        ModeTransEnum modeTrans;
    public:
        Mode() : mode(INIT),modeTrans(KEEP) {}

        ModeEnum getMode() { return mode; }
        void setMode(ModeEnum mode) { this->mode = mode; }

        ModeTransEnum getModeTrans() { return modeTrans; }
        void setModeTrans(ModeTransEnum modeTrans) { this->modeTrans = modeTrans; }
};
}
}

#endif // 