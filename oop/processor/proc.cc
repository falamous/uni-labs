#include "proc.h"
#include "memory.h"
#include "opcode.h"
#include "error.h"
#include <string>
 
/** 
 * Processor constructor
*/
Processor::Processor() {
    init_opcodes();
}

/** 
 * Interactivly allow the user to view, change and assemble parts of the memory and stack.
 * @param[in] opcode - the provided opcode will be displayed allong with other info.
*/
bool Processor::debug_interact(std::shared_ptr<Opcode> opcode) {
    std::cerr << "registers:" << std::endl;
    for(size_t i = 0; i < RegisterCount; i++) {
        std::string register_name = "r" + std::to_string(i);
        if (i == RIP) {
            register_name += " (rip)";
        }
        if (i == RSP) {
            register_name += " (rsp)";
        }
        std::cerr << register_name << " " << regs.get(i) << std::endl;
    }
    std::cerr << "Instruction:" << std::endl;
    std::cerr << opcode->write_asm() << std::endl;
    std::map<std::string, std::tuple<std::string, std::function<int (std::vector<uint32_t>)>, int> > commands = {
        {"s", {"s (step)", [](std::vector<uint32_t> args) -> int { return 1; }, 0}},
        {"h", {"h halt", [](std::vector<uint32_t> args) -> int { return 2; }, 0}},
        {"pm", {"pm <addr> (print uint32_t at address in memory)",
                   [&](std::vector<uint32_t> args) -> int {
                       std::cerr << mem.read_type<uint32_t>(args[0]) << std::endl;
                       return 0; 
                   }, 1}},
        {"ps", {"ps <addr> (print uint32_t at address in stack)",
                   [&](std::vector<uint32_t> args) -> int {
                       std::cerr << stack.read_type<uint32_t>(args[0]) << std::endl;
                       return 0; 
                   }, 1}},
        {"wm", {"wm <addr> <value> (write uint32_t at address in memory)",
                   [&](std::vector<uint32_t> args) -> int {
                       mem.write_type<uint32_t>(args[0], args[1]);
                       return 0; 
                   }, 2}},
        {"ws", {"ws <addr> <value> (write uint32_t at address in stack)",
                   [&](std::vector<uint32_t> args) -> int {
                       stack.write_type<uint32_t>(args[0], args[1]);
                       return 0; 
                   }, 2}},
        {"wr", {"wr <n> <value> (write value to register)",
                   [&](std::vector<uint32_t> args) -> int {
                       regs.set(args[0], args[1]);
                       return 0; 
                   }, 2}},

        {"am", {"am <addr> (assemble at address in memory)",
                   [&](std::vector<uint32_t> args) -> int {
                       for(;;) {
                           std::cout << "assemble: ";
                           std::cout.flush();
                           std::string instr_str;
                           std::getline(std::cin, instr_str);
                           try {
                               auto opcode = opcode_from_string(instr_str);
                           } catch (const AsmException &e) {
                               std::cerr << e.what() << std::endl;
                               continue;
                           }
                           if (opcode == NULL) {
                               continue;
                           }

                           size_t address = args[0];
                           try {
                               opcode->write_raw(mem, address + 1);
                           } catch (const AsmException &e) {
                               std::cerr << e.what() << std::endl;
                               continue;
                           } catch (const std::runtime_error &e) {
                               std::cerr << e.what() << std::endl;
                               continue;
                           } 
                           mem.write_type<uint8_t>(address, opcodes_by_name[opcode->get_name()]);
                       }
                       return 0; 
                   }, 1}},

    };
    int res;
    for(;;) {
        for(auto entry: commands) {
            std::cerr << std::get<0>(entry.second) << std::endl;
        }
        std::string cmd;
        std::cout << "> ";
        std::cout.flush();
        std::cin >> cmd;
        if (commands.find(cmd) == commands.end()) {
            continue;
        }
        std::vector<uint32_t> args;
        std::string arg_line;
        std::getline(std::cin, arg_line);
        std::stringstream ss(arg_line);
        for(size_t i = 0; i < std::get<2>(commands[cmd]); i++) {
            uint32_t arg;
            ss >> arg;
            args.push_back(arg);
        }
        if ((res = std::get<1>(commands[cmd])(args))) {
            break;
        }
    }
    if (res == 2) {
        return true;
    }
    return false;
}

/**
 * Parse assembly string into and Opcode object.
 * @param[in] instr_str
 * @param[out] opcode
 */
std::shared_ptr<Opcode> Processor::opcode_from_string(std::string instr_str) {
            size_t first_whitespace_index = instr_str.find(" ");
            std::string opcode_str = filterwhitespace(instr_str.substr(0, first_whitespace_index));
            if (opcode_str == "") {
                return NULL;
            }
            std::vector<std::string> args;
            if (first_whitespace_index != std::string::npos) {
                args = string_split(instr_str.substr(first_whitespace_index), ",");
            }
            for(auto &arg: args) {
                arg = filterwhitespace(arg);
            }
            if (opcodes_by_name.find(opcode_str) == opcodes_by_name.end()) {
                throw AsmException("No such opcode %s.", opcode_str.c_str());
            }
            uint8_t opcode_no = opcodes_by_name[opcode_str];
            std::shared_ptr<Opcode> opcode = opcodes[opcode_no];
            opcode->parse_asm(args);
            return opcode;

}

/**
 * Compile a list of assembly string instructions into a binary blob.
 * @param[in] instructions
 * @param[out] mem.get_memory()
 */
std::vector<uint8_t> Processor::compile(std::vector<std::string> instructions) {
    size_t address = 0;

    std::vector<std::tuple<
        size_t,
        uint8_t,
        std::shared_ptr<Opcode>
            >> assembled;
    for(auto instr_str: instructions) {
        instr_str = strip(instr_str);
        if (instr_str == "") {
            continue;
        }

        if (instr_str[instr_str.size() - 1] == ':') {
            /* label */
            auto label_parts = string_split(instr_str, ":");
            if (label_parts.size() > 3) {
                throw AsmException("Ill formated label %s.", instr_str.c_str());
            }

            std::string label_name = strip(label_parts[0]);
            size_t label_length = 0;
            if (label_parts.size() == 3) {
                auto length_parse_res = string_to_int(label_parts[1]);
                if (!length_parse_res.second) {
                    throw AsmException("Couldn't parse number %s.", label_parts[1].c_str());
                }
                label_length = length_parse_res.first;
            }
            if (label_name != "") { 
                mem.add_label(label_name, address);
            }
            address += label_length;
        } else {
            /* instruction */
            auto opcode = opcode_from_string(instr_str);
            if (opcode == NULL) {
                continue;
            }
            assembled.emplace_back(
                    address,
                    opcodes_by_name[opcode->get_name()],
                    opcode->clone()
                    );
            address += 1;
            address += opcode->len();
        }
    }
    for(auto op: assembled) {
        address = std::get<0>(op);
        mem.write_type<uint8_t>(address, std::get<1>(op));
        address += 1;
        address += std::get<2>(op)->write_raw(mem, address);
    }
    return mem.get_memory();
}

/** 
 * Run the vm, potentially in debug mode on prepared ram and registers.
 * @param[in] debug
*/
void Processor::run(bool debug) {
    regs.set(0, 3);
    while (true) {
        uint32_t rip = regs.get(RIP);
        uint8_t opcode_no = mem.read_type<uint8_t>(rip);
        rip += 1;
        if (opcodes.find(opcode_no) == opcodes.end()) {
            throw std::runtime_error("No such opcode " + std::to_string(opcode_no) + ".");
        }
        std::shared_ptr<Opcode> opcode = opcodes[opcode_no];
        rip += opcode->parse_raw(mem, rip);
        regs.set(RIP, rip);

        if (debug) {
            if (debug_interact(opcode)) {
                break;
            }
        }

        if (opcode->execute(regs, mem, stack)) {
            break;
        }
    }
}

/** 
 * Initiallizes the lexer's opcodes and opcodes_by_name.
*/
void Processor::init_opcodes() {

    std::vector<std::shared_ptr<Opcode>> opcode_list = {
        /* arithmetic opcodes */
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "add",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a + b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "sub",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a + b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "mov",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "xor",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a ^ b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "and",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a & b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "or",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a | b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "lshift",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a << b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "rshift",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a >> b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "mul",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a * b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "div",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a / b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "mod",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a % b; },
                    4
                    )),


        /* arithmetic opcodes with consants */
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "addi",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new IntArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a + b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "subi",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new IntArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a + b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "movi",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new IntArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "xori",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new IntArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a ^ b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "andi",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new IntArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a & b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "ori",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new IntArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a | b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "lshifti",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new IntArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a << b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "rshifti",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new IntArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a >> b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "muli",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new IntArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a * b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "divi",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new IntArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a / b; },
                    4
                    )),
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "modi",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new IntArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return a % b; },
                    4
                    )),

        /* bit magic opcodes */
        std::shared_ptr<Opcode>(new UnaryOperationOpcode(
                    "bitflip",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    [](uint32_t a) -> uint32_t { return a ^ 0xffffffff; },
                    4
                    )),
        std::shared_ptr<Opcode>(new UnaryOperationOpcode(
                    "neg",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    [](uint32_t a) -> uint32_t { return -a; },
                    4
                    )),

        /* io opcodes */
        std::shared_ptr<Opcode>(new UnaryOperationOpcode(
                    "printc",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    [](uint32_t a) -> uint32_t { std::cout << (char)a; return a; },
                    4
                    )),
        std::shared_ptr<Opcode>(new UnaryOperationOpcode(
                    "print",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    [](uint32_t a) -> uint32_t { std::cout << a; return a; },
                    4
                    )),
        std::shared_ptr<Opcode>(new UnaryOperationOpcode(
                    "readc",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    [](uint32_t a) -> uint32_t { char c; std::cin >> c; return c; },
                    4
                    )),

        std::shared_ptr<Opcode>(new UnaryOperationOpcode(
                    "read",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    [](uint32_t a) -> uint32_t { std::cin >> a; return a; },
                    4
                    )),

        /* jumps */
        std::shared_ptr<Opcode>(new JumpOpcode(
                    "jz",
                    std::vector<std::shared_ptr<OpcodeArg>>({
                        std::shared_ptr<OpcodeArg>(new IntArg()),
                        std::shared_ptr<OpcodeArg>(new RegArg()),
                        }),
                    [](std::vector<uint32_t> values) -> bool { return values[1] == 0; }
                    )),
        std::shared_ptr<Opcode>(new JumpOpcode(
                    "jnz",
                    std::vector<std::shared_ptr<OpcodeArg>>({
                        std::shared_ptr<OpcodeArg>(new IntArg()),
                        std::shared_ptr<OpcodeArg>(new RegArg()),
                        }),
                    [](std::vector<uint32_t> values) -> bool { return values[1] != 0; }
                    )),
        std::shared_ptr<Opcode>(new JumpOpcode(
                    "jmp",
                    std::vector<std::shared_ptr<OpcodeArg>>({
                        std::shared_ptr<OpcodeArg>(new IntArg()),
                        }),
                    [](std::vector<uint32_t> values) -> bool { return true; }
                    )),


        /* memory opcodes */
        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "ldr",
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    std::shared_ptr<OpcodeArg>(new AddressArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return b; },
                    4
                    )),

        std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                    "str",
                    std::shared_ptr<OpcodeArg>(new AddressArg()),
                    std::shared_ptr<OpcodeArg>(new RegArg()),
                    [](uint32_t a, uint32_t b) -> uint32_t { return b; },
                    4
                    )),
    };
    for(size_t i = 0; i < opcode_list.size(); i++) {
        opcodes[i] = opcode_list[i];
    }


    opcodes[255] = std::shared_ptr<Opcode>(new ExitOpcode("exit"));
    for(auto opcode: opcodes) {
        opcodes_by_name[opcode.second->get_name()] = opcode.first;
    }
}

void Processor::dump_regs(FILE *f) {
    for(size_t i = 0; i < RegisterCount; i++) {
        uint32_t reg = regs.get(i);
        fwrite(&reg, sizeof(reg), 1, f);
    }
}
void Processor::load_regs(FILE *f) {
    for(size_t i = 0; i < RegisterCount; i++) {
        uint32_t reg;
        fread(&reg, sizeof(reg), 1, f);
        regs.set(i, reg);
    }
}

void Processor::dump_mem(FILE *f) {
    for(auto c: mem.get_memory()) {
        fputc(c, f);
    }
}
void Processor::load_mem(FILE *f) {
    size_t address = 0;
    int c;
    while((c = fgetc(f)) != EOF) {
        mem.write_type<uint8_t>(address, c);
        address += 1;
    }
}
