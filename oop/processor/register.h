#pragma once
#include <bits/stdc++.h>

const size_t RegisterCount = 32;
const uint8_t RIP = 31;
const uint8_t RSP = 30;
class Registers {
    uint32_t regs[RegisterCount];
    public:
    Registers();
    uint32_t get(uint8_t number) const;
    void set(uint8_t number, uint32_t value);
};
