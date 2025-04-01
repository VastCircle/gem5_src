#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include "string.h"
#include <cstdint>

class ModeBase {
    public : 
        enum Mode { INIT , DISCOVER, WAIT};
        Mode mode;
        ModeBase () : mode(INIT) {}

};


#endif

