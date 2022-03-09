#pragma once

#include <cstdint>
#include <string>
#include <map>
#include <vector>

class Memory {
    std::map<std::string, uint32_t> label_map;
    std::vector<uint8_t> memory;
    uint8_t read_byte(size_t addr) const;
    void write_byte(size_t addr, uint8_t byte);
    public:
        template<class T>
            T read_type(size_t address) const {
                T res;
                for(size_t i = 0; i < sizeof(T); i++) {
                    ((uint8_t *)&res)[i] = read_byte(address + i);
                }
                return res;
            }
        template<class T>
            void write_type(size_t address, T val) {
                for(size_t i = 0; i < sizeof(T); i++) {
                    write_byte(address + i, ((uint8_t *)&val)[i]);
                }
            }
        uint32_t resolve_label(std::string label) const;
        void add_label(std::string label, uint32_t value);
        std::vector<uint8_t> get_memory();
};
