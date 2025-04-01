#ifndef PARSE_LOG_H
#define PARSE_LOG_H

#include <iostream>
#include <regex>
#include <string>
#include <optional>
#include "assert.h"

struct LogEntry {
    bool index;
    uint64_t pc;
    uint8_t writeReg;
    uint64_t writeValue;
    bool writeExtra;
    uint8_t readReg1;
    uint64_t readValue1;
    bool readExtra1;
    uint8_t readReg2;
    uint64_t readValue2;
    bool readExtra2;
    uint32_t inst;
    std::string asm_str;
};

std::optional<LogEntry> parseLogEntry(const std::string& log);

#endif // PARSE_LOG_H