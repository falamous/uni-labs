#include "opcode.h"
#include "memory.h"
#include <cstdio>

class Processor {
    std::map<uint8_t, std::shared_ptr<Opcode>> opcodes;
    std::map<std::string, uint8_t> opcodes_by_name;
    Memory mem;
    Memory stack;
    Registers regs;

    void init_opcodes();
    bool debug_interact(std::shared_ptr<Opcode>);
    std::shared_ptr<Opcode> opcode_from_string(std::string);
    public:
        void dump_regs(FILE *f);
        void dump_mem(FILE *f);
        void load_regs(FILE *f);
        void load_mem(FILE *f);
        Processor();
        std::vector<uint8_t> compile(std::vector<std::string> instructions);
        void run(bool debug=false);
};
