#include "register.h"

Registers::Registers() {
    for(auto &reg: regs) {
        reg = 0;
    }
}

uint32_t Registers::get(uint8_t num) const {
    if (num >= RegisterCount) {
        throw std::runtime_error("No such register.");
    }
    return regs[num];
}

void Registers::set(uint8_t num, uint32_t value){
    if (num >= RegisterCount) {
        throw std::runtime_error("No such register.");
    }
    regs[num] = value;
}
