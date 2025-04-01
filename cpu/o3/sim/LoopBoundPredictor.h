#ifndef LOOP_BOUNDARY_PREDICTOR_H
#define LOOP_BOUNDARY_PREDICTOR_H

#include "common.h"
#include <unordered_map>

/**
 * @brief  循环边界预测器，通过ewma 和 loopboundarydetector 竞争输出结果
 *  在观察模式结束的同时，获取循环次数，发起向量化，并且进入等待模式
 *  在内循环的第一次迭代，此时数据都是不准确的，如果通过再跑一轮来获取，就会浪费时间（这种也是已经执行了一波外循环了的情况）
 *  因此可以在stride_load_pc , addr地址不是连续的时候，读取寄存器值 ，直接计算循环次数
 *  不止在观察模式下进行训练，而是持续训练
 *  
 */
class LoopBoundaryPredictor {
    private:
        struct LastCompare {
            uint64_t pc;
            uint8_t rs1;
            uint64_t src1;
            uint8_t rs2;
            uint64_t src2;
        };
        /**
         * @brief EWMABasePredictor
         * 
         */
        class EWMABasePredictor {
            private:
                uint16_t ewma;
                uint16_t counter;
                uint16_t ewma_counter_max; // TODO: 记录最大值的 , 实际表项中肯定不需要
            public : 
                EWMABasePredictor(uint16_t ewma_counter_max):ewma_counter_max(ewma_counter_max),ewma(0),counter(1){}
                void UpdateCounter(bool inc,bool reset);
                uint16_t IfCounterMax() {return counter == ewma_counter_max;}
                void UpdateEWMA();
                uint16_t GetEWMA() {return ewma;}
                uint16_t GetEWMACounter() {return counter;}

        };
        /**
         * @brief LoopBoundaryDetector
         * 
         */
        class LoopBoundaryDetector{
            private:
                int16_t loop_increment;
                LastCompare lc;
                uint16_t loop_bound;  // TODO : 方便起见，把循环边界放在这里
                uint16_t loop_number;
                uint16_t counter;
            public:
                LoopBoundaryDetector(LastCompare lc): 
                lc(lc),loop_increment(0),counter(0),loop_number(0){}
                bool ComparePC(uint64_t pc);
                void UpdateLoopIncrement(uint64_t src1,uint64_t src2);
                void UpdateLastCompare(LastCompare lc);
                void UpdateDetectorCounter(uint16_t counter,bool reset) ;
                uint16_t GetDetectorCounter() {return counter;}
                uint16_t GetLoop() {return loop_number;}
        };

        /**
         * @brief 循环边界预测器的一个表项
         * 
         */
        class LoopBundaryPredictorComponents: public LoopBoundaryDetector,public EWMABasePredictor { 
            private:
                uint64_t tag;
                uint8_t  confidence;// TODO:暂时没有什么用
                uint8_t  tournament; // 饱和计数器
            public:
                LoopBundaryPredictorComponents(uint64_t tag,LastCompare lc,uint16_t ewma_counter_max):
                tag(tag),confidence(2),EWMABasePredictor(ewma_counter_max),LoopBoundaryDetector(lc){}
                LoopBundaryPredictorComponents():
                tag(0),confidence(2),EWMABasePredictor(0),LoopBoundaryDetector({0,0,0,0,0}){}
                bool CompareTag(uint64_t tag);
                uint8_t UpdateConfidence(bool inc,bool reset);
                uint8_t UpdateTournament();
                bool GetLoopNumber(uint16_t n , uint16_t &LoopNumber);
        };
                
        uint8_t nEntry;  // 表项数
        uint16_t ewma_counter_max; // ewma计数器最大值
        uint8_t n;    // 向量化最大值
        LastCompare lc;
        std::unordered_map<uint8_t, LoopBundaryPredictorComponents> lbps;
    public:
        LoopBoundaryPredictor(uint8_t n , uint8_t nEntry,uint16_t ewma_counter_max):n(n),nEntry(nEntry),ewma_counter_max(ewma_counter_max) {}
        void UpdateLastCompare(uint64_t pc , uint8_t rs1, uint8_t rs2, uint64_t src1, uint64_t src2);
        bool UpdateLoopBoundaryDetector(uint64_t stride_load_pc,uint64_t branchPC, int64_t imm, uint8_t rs1,uint64_t src1, uint8_t rs2, uint64_t src2);
        bool AllocateEntry(uint64_t tag);
        bool UpdateEWMABasePredictor(uint64_t load_pc,bool is_stride) ;
        bool GetLoopNumber(uint64_t stride_load_pc, uint16_t &LoopNumber);
};






#endif // LOOP_BOUNDARY_DETECTOR_H
