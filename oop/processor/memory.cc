#include <cstdint>
#include <string>
#include <bits/stdc++.h>
#include "memory.h"
#include "error.h"

uint32_t Memory::resolve_label(std::string label) const {
    auto res = label_map.find(label);
    if (res == label_map.end()) {
        throw AsmException("No such label %s.", label.c_str());
    }
    return res->second;
}

void Memory::add_label(std::string label, uint32_t value) {
    auto res = label_map.find(label);
    if (res != label_map.end()) {
        throw AsmException("Label %s already exists.", label.c_str());
    }
    label_map[label] = value;
}

uint8_t Memory::read_byte(size_t addr) const {
    if (addr >= memory.size()) {
        return 0;
    }
    return memory[addr];
}

void Memory::write_byte(size_t addr, uint8_t byte) {
    if (addr >= memory.size()) {
        memory.resize(addr + 1);
    }
    memory[addr] = byte;
}

std::vector<uint8_t> Memory::get_memory() {
    return memory;
}
