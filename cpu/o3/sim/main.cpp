#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fstream>

#include "parse_log.h"
#include "stride_load.h"
#include "InstructionParser.h"
#include "TaintTracker.h"
#include "SmallBuffer.h"
#include "LoopBoundPredictor.h"
#include "common.h"


#include "cpu/o3/dyn_inst.hh"

namespace gem5 {
namespace o3 {



// // 当检测到stride_load时，进入DiscoverMode , 在DiscoverMode下，
class VectorRunahead : private ModeBase{
    private : 
        uint16_t TimeOut;
        uint64_t prev_addr; // 运行地址
        uint64_t pref_addr; // 预取地址的终点
        enum state_trans_e {KEEP,INIT2DISCOVER,DISCOVER2WAIT,WAIT2DISCOVER,WAIT2INIT,DISCOVER2INIT};
        state_trans_e state_trans;
        uint32_t count;
        uint16_t loopNumber; // 预取次数, 该说不说 , 如果要获取预取后的地址，loopNumber * 步进   
    public:
        VectorRunahead():ModeBase(),count(0),loopNumber(0),TimeOut(0){};
        
        void process(DynInstPtr &head_inst) {

           int lineNumber = 0;

           rptFiles rptFiles(128);
           TaintTracker  tk;
           SmallBuffer sb(20);
           InstructionParser::InstructionInit();
           LoopBoundaryPredictor lbp(30,8,512);


    // 逐行读取文件
        lineNumber++;
        // 调用解析函数
          std::cout << lineNumber << "Mode:" << mode << std::endl;
          bool is_load = head_inst->isLoad() && head_inst->effAddrValid();
          bool towait = false, vtt_clear = false, can_prf = false,stride_corr = false; 
          Mode last_mode = this->mode;
          // 如果是load 指令
          if (is_load) {
              // 更新rpt表项
              std::tie(towait,vtt_clear,can_prf,stride_corr) = rptFiles.processRequest(
                                                               (uint64_t) head_inst->pcState().instAddr(),
                                                               head_inst->physEffAddr); // 检测到是stride_load
          }
            // 状态转移
            // 如果是分支指令，训练循环边界预测器, TODO:对一些不是stride_load的指令也进行了训练
            // TODO:会出现WAIT刚退出进入INIT,需要等到下一个stride_load才进入discover,导致中间的beq没有被判断(解决)
          switch (this->mode) {
              case INIT: {
                   if (can_prf) {
                        tk.TaintReg(head_inst->renamedDestIdx(0)); // 污染目标寄存器
                        rptFiles.stride_load_pc = entry->pc;
                        SmallBuffer::InstructionInfo inst_info = {*entry,entry->pc,line,entry->asm_str,true,0,false,false,true};
                        sb.pushInstruction(inst_info);
                        mode = DISCOVER;
                    }
                    break;
                }
                case DISCOVER: {
                    if (towait) {
                        mode = WAIT;
                        count++;
                        // outfile << lineNumber << " cout: " << count << std::endl;
                        lbp.GetLoopNumber(rptFiles.stride_load_pc,loopNumber); // 
                        rptFiles.UpdatePrefAddr(loopNumber); // 在即将进入等待模式的时候更新预取地址的边界
                        // outfile << " Loop Number: " << loopNumber << std::endl;
                        // outfile << "-----------------------------------------------------" << std::endl;
                        sb.printBuffer(outfile,tk.GetFinalLoadPC(),loopNumber); // buffer是可以保留的
                        tk.ClearFinalLoadPC(); // 好像也可以保留
                        tk.clearVTTIfSeen(); // VTT清空
                        TimeOut = 0;
                        break;
                    }
                    TimeOut++;
                    if (TimeOut >= 256) {  // 超时检测
                        TimeOut = 0;
                        mode = INIT;
                        tk.clearVTTIfSeen();
                        sb.ClearBuffer();
                        tk.ClearFinalLoadPC();
                        break;
                    }
                    // 如果发现是stride_load, 两种可能，一种是并行的stride load ， seen = 0 ,可以一起处理， 一种是包含的stride load(vtt_clear) 需要
                    // 找到了更内侧的循环
                    if (is_load && can_prf) { // 如果是stride_load
                        if (vtt_clear) {
                            tk.clearVTTIfSeen();
                            tk.ClearFinalLoadPC();
                            sb.ClearBuffer();
                        }
                        tk.TaintReg(entry->writeReg); // 污染目标寄存器
                        SmallBuffer::InstructionInfo inst_info = {*entry,entry->pc,line,entry->asm_str,true,0,false,false,true};
                        sb.pushInstruction(inst_info);
                        break;
                    }
                    // 如果不是stride_load
                    auto [isTaint,isTainted1,isTainted2] = tk.processLogEntry(*entry,is_load);
                    if (isTaint) {
                        SmallBuffer::InstructionInfo inst_info = {*entry,entry->pc,line,entry->asm_str,false,0,isTainted1,isTainted2, true};
                        sb.pushInstruction(inst_info);
                    }
                    // 循环边界判断
                    break;
                }
                case WAIT: {
                // 退出WAIT状态的条件： 本次预取结束了 , 1.通过地址判断 , 可以通过设置下界来避免出现下面的情况
                // 会不会有已经到尽头了，下次不会再出现这个地址的情况(超时判断),单纯超时检测不够
                // 在wait 模式的时候，seen 的检测还是开着，
                    if (entry->pc == rptFiles.stride_load_pc    &&
                        !(entry->readValue1 > rptFiles.prev_addr &&  // TODO:是否能用stride_corr和loop_number计数代替
                        entry->readValue1 <= rptFiles.pref_addr)) {
                            mode = DISCOVER;
                            TimeOut = 0;
                            tk.clearVTTIfSeen();
                            tk.ClearFinalLoadPC();
                            sb.ClearBuffer();
                            tk.TaintReg(entry->writeReg); // 污染目标寄存器
                            rptFiles.stride_load_pc = entry->pc;
                            SmallBuffer::InstructionInfo inst_info = {*entry,entry->pc,line,entry->asm_str,true,0,false,false,true};  // stride_load
                            sb.pushInstruction(inst_info);
                    }
                    break;
                }
            }

            if (entry->pc == rptFiles.stride_load_pc) {
                rptFiles.prev_addr = entry->readValue1;
            }

            if (can_prf && entry->pc == rptFiles.stride_load_pc) {  // stride_load 索引 更新 ewma
                    lbp.AllocateEntry(rptFiles.stride_load_pc); // 申请stride_load_pc对应的表项
                    lbp.UpdateEWMABasePredictor(entry->pc,stride_corr);
            }

            if (InstructionParser::isBranchInstruction(asm_instruction) && mode != INIT) {
                // 记录分支指令
                lbp.UpdateLastCompare(entry->pc,entry->readReg1,entry->readReg2,entry->readValue1,entry->readValue2);
                lbp.UpdateLoopBoundaryDetector(
                rptFiles.stride_load_pc,
                entry->pc,
                InstructionParser::instruction.immediateValue,
                entry->readReg1,entry->readValue1,entry->readReg2,entry->readValue2);
        }


    }
};

}
}