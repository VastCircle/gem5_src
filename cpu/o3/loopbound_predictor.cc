#include "cpu/o3/loopbound_predictor.hh"
#include "cpu/o3/rob.hh"

#include <list>

#include "base/logging.hh"
#include "cpu/o3/dyn_inst.hh"
#include "cpu/o3/limits.hh"
#include "debug/Fetch.hh"
#include "debug/ROB.hh"
#include "params/BaseO3CPU.hh"

namespace gem5
{

namespace o3
{

LoopBoundaryPredictor::LoopBoundaryPredictor(CPU *_cpu, const BaseO3CPUParams &params) 
    :cpu(_cpu),
    n(params.MaxVectorSize),
    nEntry(params.numLoopBoundPredictorEntries),
    ewma_counter_max(0),
    lc(LastCompare{0,0,0,0,0}) {}

/**
 * @brief 更新lc 
 * 
 */
// void LoopBoundaryPredictor::UpdateLastCompare(uint64_t pc , uint8_t rs1, uint8_t rs2, uint64_t src1, uint64_t src2) {
//     this->lc = {pc, rs1, src1, rs2, src2};
// }
void LoopBoundaryPredictor::UpdateLastCompare(DynInstPtr &inst) {
    this->lc = {inst->pcState().instAddr(), 
                // inst->renamedSrcIdx(0),
                0,
                0,
                // inst->renamedSrcIdx(1),
                0,
                0,
                };
}

// TODO: 在执行阶段更新lc可行吗，此时执行是乱序的, 能否保证只找到一个往前跳的,
void LoopBoundaryPredictor::UpdateLastCompareSrc(DynInstPtr &inst) {
    if (this->lc.pc == inst->pcState().instAddr()) {
        // TODO: 重新读了一次寄存器，其实不太好,有什么更优的方法吗
        this->lc.src1 = inst->getRegOperand(&*inst->staticInst,0);
        this->lc.src2 = inst->getRegOperand(&*inst->staticInst,0);
    }
}

/**
 * @brief 在检测到stride_load的时候申请表项
 * 
 * @param tag 
 * @return true 
 * @return false 
 */
bool LoopBoundaryPredictor::AllocateEntry(uint64_t stride_load_pc) {
    uint8_t idx = stride_load_pc % nEntry;
    uint64_t tag = stride_load_pc / nEntry;
    auto it = lbps.find(idx);
    if (it == lbps.end()) {
        LoopBundaryPredictorComponents lbp(tag,(LastCompare){0,0,0,0,0},ewma_counter_max);
        lbps[idx] = lbp;
        return true;
    }
    return false;
}

/**
 * @brief 分支跳转的时候更新循环边界预测器
 * 
 * @param stride_load_pc stride_load_pc
 * @param branchPC       分支跳转的pc
 * @param is_front       往前跳
 * @param rs1            
 * @param src1           
 * @param rs2            
 * @param src2 
 * @return true 
 * @return false 
 */

bool LoopBoundaryPredictor::UpdateLoopBoundaryDetector(uint64_t stride_load_pc,DynInstPtr &inst) {
    // 每一次找到分支都会进行查找
    Addr npc = inst->branchTarget()->instAddr() ;
    Addr pc  = inst->pcState().instAddr();

    if (npc > stride_load_pc || npc > pc) return false;

    UpdateLastCompare(inst);
    UpdateLastCompareSrc(inst);

    //更新lc
    //UpdateLastCompare(branchPC,rs1,rs2,src1,src2);
    uint8_t idx = stride_load_pc % nEntry;
    uint64_t tag = stride_load_pc / nEntry;
    auto it = lbps.find(idx);
    // assert(it != lbps.end());  // 表项不应该不存在
    bool hit = it != lbps.end() && it->second.CompareTag(tag);
    if (!hit) { // 如果没有命中
        LoopBundaryPredictorComponents lbp(tag,this->lc,ewma_counter_max);
        lbps[idx] = lbp;
        return false;
    }
    else { // 表项存在
        LoopBundaryPredictorComponents  &lbp = it->second;
        if (!lbp.ComparePC(pc)) { // TODO: 如果pc不符合 ,直接换,不考虑置信度会出什么问题嘛
            // uint8_t confidence = lbp.UpdateConfidence(false,false);  // 减少置信度
            // if (confidence == 0) { // 如果置信度降到0，替换lc表项
                lbp.UpdateLastCompare(this->lc);
                // lbp.UpdateConfidence(true,true); // reset
                return false;
            // }
            return false;
        }
        else { // 表项pc匹配
            // lbp.UpdateConfidence(true,false); // 增加置信度
            lbp.UpdateLoopIncrement(this->lc);
            return true;
        }
    }
    return false;
}

/**
 * @brief 
 * 
 * @param load_pc 
 * @param is_stride 
 * @return true 
 * @return false 
 */
bool LoopBoundaryPredictor::UpdateEWMABasePredictor(uint64_t load_pc,bool is_stride) {
    uint8_t idx = load_pc % nEntry;
    uint64_t tag = load_pc / nEntry;
    auto it = lbps.find(idx);
    assert(it != lbps.end());  // 表项不应该不存在
    bool hit =  it->second.CompareTag(tag);
    if (!hit) { // 如果没有命中
        LoopBundaryPredictorComponents lbp(tag,(LastCompare){0,0,0,0,0},ewma_counter_max);
        lbps[idx] = lbp;
        return false;
    }
    else {
        LoopBundaryPredictorComponents  &lbp = it->second;
        if (is_stride && !lbp.IfCounterMax()) { 
            lbp.UpdateCounter(true,false);
            return true;
        }
        else {  // 出现不连续的地址，或者达到了最大值
            lbp.UpdateTournament();
            lbp.UpdateEWMA();  // 更新ewma
            lbp.UpdateCounter(false,true);
            lbp.UpdateDetectorCounter(0,true);
            return false;
        }
    }
}


/**
 * @brief Get the Loop Number object
 * 
 * @param stride_load_pc 
 * @return uint16_t 
 */
bool LoopBoundaryPredictor::GetLoopNumber(uint64_t stride_load_pc, uint16_t &LoopNumber) {
    uint8_t idx = stride_load_pc % nEntry;
    uint64_t tag = stride_load_pc / nEntry;
    auto it = lbps.find(idx);
    if (it == lbps.end()) {
        LoopNumber = n;
        return false;
    }
    if (it != lbps.end() && it->second.CompareTag(tag)) {
        it->second.GetLoopNumber(n,LoopNumber);
        return true;
    }
    else  {
        return false;
    }
}

/**
 * @brief 比较BranchPC
 * 
 * @param pc 
 */
bool LoopBoundaryPredictor::LoopBoundaryDetector::ComparePC(uint64_t pc) {
    return this->lc.pc == pc;
}

/**
 * @brief 复制lc到lbd表项中去
 * 
 * @param lc 
 */
void LoopBoundaryPredictor::LoopBoundaryDetector::UpdateLastCompare(LastCompare lc) {
    this->lc = lc;
}

/**
 * @brief 计算循环增量,及循环次数
 * 到底是在进入discover_mode 那一个周期训练，还是一直不停的训练
 * @param src1 
 * @param src2 
 * @return uint32_t 
 */
void LoopBoundaryPredictor::LoopBoundaryDetector::UpdateLoopIncrement(LastCompare lc) {
    if (this->lc.src1 == lc.src1 && this->lc.src2 != lc.src2) {
        this->loop_increment = (int64_t)(lc.src2 -  this->lc.src2);  // 步进值 , 预取值小于循环边界，最后一次时
        this->loop_number = (uint64_t)((int64_t) (lc.src1 - lc.src2)/this->loop_increment);
        this->lc.src2 = lc.src2;  // 不更新的话步进值会越来越大,但是更新的话，预取值会越來越小的
    }
    else if (this->lc.src2 == lc.src2 && this->lc.src1 != lc.src1) {
        this->loop_increment = (int64_t) (lc.src1 - this->lc.src1);  // 步进值
        this->loop_number = (uint64_t)((int64_t)(lc.src2 - lc.src1)/this->loop_increment);
        this->lc.src1 = lc.src1;
    }
    else if (this->lc.src2 != lc.src2 && this->lc.src2 != lc.src2) {
        this->lc.src1 = lc.src1;
        this->lc.src2 = lc.src2;
        int64_t temp = (int64_t)(lc.src2 - lc.src1)/this->loop_increment;
        this->loop_number =  temp < 0 ? -temp : temp;
    }
}

void LoopBoundaryPredictor::LoopBoundaryDetector::UpdateDetectorCounter(uint16_t counter,bool reset)  {
    if (reset) {
        this->counter = 0;
    }
    else {
        this->counter += counter;
    }
}


/**
 * @brief 在stride_load 的时候更新counter
 * 
 */
void LoopBoundaryPredictor::EWMABasePredictor::UpdateCounter(bool inc,bool reset) {
    if (reset) {
        counter = 0;
    }
    else if (inc) {
        counter = counter == ewma_counter_max - 1 ? 0 : counter + 1;
    }
}

/**
 * @brief 更新ewma
 * 
 */
void LoopBoundaryPredictor::EWMABasePredictor::UpdateEWMA() {
    if (ewma == 0) {  // 初始时直接赋值，不用计算，计算的话增速太慢了
        ewma = counter + 1; // 
    }
    else {
        this->ewma = (7* this->ewma + this->counter) / 8;
    }
}


/**
 * @brief 更新置信度
 * 
 * @param inc 
 * @param reset 
 * @return uint8_t 
 */
uint8_t LoopBoundaryPredictor::LoopBundaryPredictorComponents::UpdateConfidence(bool inc, bool reset) {
    if (reset) {
        confidence = 2;
    } else if (inc) {
        confidence = confidence == 3 ? 3 : confidence + 1;
    } else {
        confidence = confidence == 0 ? 0 : confidence - 1;
    }
    return confidence;
}

/**
 * @brief 根据实际迭代次数更新饱和计数器
 *  counter 是这次迭代的真实值 ， 
 * @param inc 
 * @param reset 
 * @return uint8_t 
 */
uint8_t LoopBoundaryPredictor::LoopBundaryPredictorComponents::UpdateTournament() {
        uint16_t dwma  = GetEWMA() > GetEWMACounter() ? (GetEWMA() - GetEWMACounter()) : (GetEWMACounter() - GetEWMA()); 
        uint16_t dloop = GetDetectorCounter() > GetEWMACounter() ? (GetDetectorCounter() - GetEWMACounter()) : (GetEWMACounter() - GetDetectorCounter());
        if (dloop > dwma) {
            tournament = tournament == 0 ? 0 : tournament - 1;
        }
        else {
            tournament = tournament == 3 ? 3 : tournament + 1;
        }
    return tournament;
}

/**
 * @brief  比较tag 
 * 
 * @param tag 
 */
bool LoopBoundaryPredictor::LoopBundaryPredictorComponents::CompareTag(uint64_t tag) {
    return this->tag == tag;
}

/**
 * @brief 仲裁获取预测的循环次数
 * 
 * @param stride_load_pc 
 * @param LoopNumber 
 * @return true 
 * @return false 
 */
bool LoopBoundaryPredictor::LoopBundaryPredictorComponents::GetLoopNumber(uint16_t n , uint16_t &LoopNumber) {
    // if (this->tournament >= 2) {  
        LoopNumber = n < GetLoop() ? n : GetLoop();
        // UpdateDetectorCounter(LoopNumber,false);
    // }
    // else {
    // uint16_t ewma_number = GetEWMA() > GetEWMACounter() ? GetEWMA() - GetEWMACounter(): GetEWMA(); // 
    // LoopNumber = n < ewma_number ? n : ewma_number;
    // }
    return true;
}
}
}