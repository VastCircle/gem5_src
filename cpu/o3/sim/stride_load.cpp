#include "stride_load.h"


rptFiles::StrideMetadata::StrideMetadata():state(INIT), seen(false) {}

bool rptFiles::StrideMetadata::isValid() const {
    return state > INIT;
}

std::pair<bool, rptFiles::StrideMetadata::State> rptFiles::StrideMetadata::StateTrans(bool corr) {
    std::pair<bool,State> result(false, INIT); 
    switch (state) {  // TODO: 第一次见到是INIT状态，第二次见到是TRANS状态，即使不是stride_load ， 从INIT->TRANS也会触发一次预取，这样会导致错误的预取
        case INIT:    result =  corr ? std::make_pair(true, STEADY) : std::make_pair(false, TRANS); break;
        case STEADY:  result =  corr ? std::make_pair(true, STEADY) : std::make_pair(true, INIT); break;
        case TRANS:   result =  corr ? std::make_pair(true, STEADY) : std::make_pair(false, NOPRED); break;
        case NOPRED:  result =  corr ? std::make_pair(false, TRANS) : std::make_pair(false, NOPRED);break;
        default : result =  std::make_pair(false, INIT); break;
    }
    std::tie(this->can_prf, this->state) = result;
    return result;
}

void rptFiles::RPTEntry::UpdateStride(bool corr,int64_t new_stride) {
    if (GetState() == INIT && !corr) {
        stride = new_stride;
    }
    else if (GetState() == TRANS && !corr) {
        stride = new_stride;
    }
    else if (GetState() == NOPRED && !corr) {
        stride = new_stride;
    }
}


rptFiles::rptFiles(int sets) :ModeBase(),nSets(sets)  {}

std::tuple<bool,bool,bool,bool> rptFiles::processRequest(uint64_t pc, uint64_t address) {
    uint64_t idx = pc % nSets;
    uint64_t tag = pc / nSets;

    // std:: cout << "PC: " << pc << " Address: " << address << std::endl;
    auto it = rpt.find(idx);
    bool hit = (it != rpt.end() && it->second.tag == tag);

    RPTEntry entry;
    if (hit) {  
        entry = it->second;
    } else {// 刚命中的时候
        entry = RPTEntry(tag);
    }

    int64_t new_stride = address - entry.prev_addr;

    bool can_prf;
    StrideMetadata::State state;

    bool stride_corr = (new_stride == entry.stride) && new_stride != 0;
    if (hit) {
        std::tie(can_prf,state) = entry.StateTrans(stride_corr);  // 命中了进行状态转移
        entry.UpdateStride(stride_corr,new_stride); 
    }
    else { // 不命中的话就是初始状态
        can_prf = false;
        state = StrideMetadata::INIT;
    }


    bool clear_vtt = false;
    entry.prev_addr = address;

    // seen 的逻辑 
    // 在检测到一个跨步负载的时候，seem = 1 , 并且将stride_load_pc 设置为当前的pc
    // 在发现seen = 1并且stride_load_pc != pc 时， 清空VTT , 设置stride_load_pc
    // 在发现seen = 1并且stride_load_pc == pc 时， 清空所有的seem , 清空VTT ，
    // VTT得清，因为后续指令是可能影响前面的VTT的，每次发向量化必须是一个崭新的VTT
    bool towait = false;
    
    // TODO:要重新跑一次discover_mode吗 ,如果stride_load_pc一致的话，感觉是不需要重新跑的
    // 不过discover的时候也是发起向量化的时候，跑和不跑的效果貌似差不多

    // 在stride_load_pc 不变的情况下 ，discover_mode 得到的 final_load_pc , buffer 都可以保留的
    // stride_load_pc改变的情况 : 发现了更内层的循环，或者发现了另一个循环 ，表征的都是发现一个seen = 1 且 stride_load_pc != pc
    // 这个时候需要清空vtt, 清空 buffer  ,清空seen , seen 还是清了好, 比方说内循环刚结束，如果不清seen ， 还会把stride_load设置为外循环

    if (can_prf && entry.seen && stride_load_pc != pc) {  
        clearAllSeen();
        clear_vtt = true;
        stride_load_pc = pc;
    }

    else if (can_prf && entry.seen && stride_load_pc == pc) {  // 第二轮遇到同一个stride_load
        clearAllSeen();
        clear_vtt = true;
        towait = true;
    }
    else if (can_prf) { // 第一轮遇到stride_load
        entry.seen = true;
    }


    // 同步把stride_load中的表项提取出来
    if (stride_load_pc == pc) { // TODO: 如果非常巧合的在循环的最后一轮进入了discover_mode ， 就没办法退出discover_mode 了(已解决)
        //prev_addr = address;  // 这个还不能随时更新,要等到wait检测完了再更新
        stride = entry.stride;
    }

    rpt[idx] = entry; 
    return std::make_tuple(towait,clear_vtt,can_prf,stride_corr);
}


void rptFiles::clearAllSeen() {
    for (auto& entry : rpt) {
        entry.second.seen = false;   // 清空所有条目的 seen 标志位
    }
}
