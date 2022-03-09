#include "opcode.h"
#include "proc.h"

std::map<int, std::shared_ptr<Opcode>> opcodes;
std::map<std::string, std::shared_ptr<Opcode>> opcodes_by_name;


void usage(char *argv[]) {
    fprintf(stderr, "usage: %s <compile|run|debug> <fname>", argv[0]);
    exit(1);
}

void compile(char *fname) {
    Processor p;

    std::fstream fs(fname);

    std::vector<std::string> instructions;
    std::string s;
    while (std::getline(fs, s)) {
        instructions.push_back(s);
    }

    p.compile(instructions);
    p.dump_regs(stdout);
    p.dump_mem(stdout);
}

void run(char *fname, bool debug=false) {
    Processor p;
    FILE *f;
    f = fopen(fname, "rb");
    if (f == NULL) {
        throw std::runtime_error("Could not open file.");
    }
    p.load_regs(f);
    p.load_mem(f);
    p.run(debug);
    fclose(f);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        usage(argv);
    }

    if (strcmp(argv[1], "compile") == 0) {
        compile(argv[2]);
        return 0;
    }
    if (strcmp(argv[1], "run") == 0) {
        run(argv[2]);
        return 0;
    }
    if (strcmp(argv[1], "debug") == 0) {
        run(argv[2], true);
        return 0;
    }
}
