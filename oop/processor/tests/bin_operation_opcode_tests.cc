#include <cmath>
#include "gtest/gtest.h"
#include "../opcode.h"
#include "../memory"
#include "../register.h"


TEST(BinaryOperationOpcodeTestSuite, AsmRegs){
    std::string name = "kek";
    std::shared_ptr<Opcode> opcode = std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                name, 
                std::shared_ptr<OpcodeArg>(new RegArg()),
                std::shared_ptr<OpcodeArg>(new RegArg()),
                [](uint32_t a, uint32_t b) -> uint32_t {
                    return a ^ b + b;
                },
                4
                ));
    EXPECT_EQ(opcode->get_name(), name);

    Memory m;
    Registers r;
    Memory stack;

    uint32_t r0 = 3235;
    uint32_t r1 = 949989;
    r.set(0, r0);
    r.set(1, r1);
    opcode->parse_asm(std::vector<std::string>({"r0", "r1"}));
    opcode->execute(r, m, stack);
    EXPECT_EQ(r.get(0), r0 ^ r1 + r1);
}

TEST(BinaryOperationOpcodeTestSuite, MemI){
    std::string name = "kek";
    std::shared_ptr<Opcode> opcode = std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                name, 
                std::shared_ptr<OpcodeArg>(new AddressArg()),
                std::shared_ptr<OpcodeArg>(new IntArg()),
                [](uint32_t a, uint32_t b) -> uint32_t {
                    return a + (b ^ 777) * 8;
                },
                4
                ));
    Memory m;
    Registers r;
    Memory stack;

    uint32_t mem = 3235;
    uint32_t i = 949989;
    m.write_type<uint32_t>(0, mem);
    opcode->parse_asm(std::vector<std::string>({"0", std::to_string(i)}));
    opcode->execute(r, m, stack);
    EXPECT_EQ(m.read_type<uint32_t>(0), mem + (i ^ 777) * 8);
}

TEST(BinaryOperationOpcodeTestSuite, MemMem){
    std::string name = "kek";
    std::shared_ptr<Opcode> opcode = std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                name, 
                std::shared_ptr<OpcodeArg>(new AddressArg()),
                std::shared_ptr<OpcodeArg>(new AddressArg()),
                [](uint32_t a, uint32_t b) -> uint32_t {
                    return a + (b ^ 777) * 8;
                },
                4
                ));
    Memory m;
    Registers r;
    Memory stack;

    uint32_t mem = 31423;
    uint32_t mem2 = 778467;
    m.write_type<uint32_t>(0, mem);
    m.write_type<uint32_t>(10, mem2);
    opcode->parse_asm(std::vector<std::string>({"0", "10"}));
    opcode->execute(r, m, stack);
    EXPECT_EQ(m.read_type<uint32_t>(0), mem + (mem2 ^ 777) * 8);
}

TEST(BinaryOperationOpcodeTestSuite, RawRegI){
    std::string name = "kek";
    std::shared_ptr<Opcode> opcode = std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                name, 
                std::shared_ptr<OpcodeArg>(new RegArg()),
                std::shared_ptr<OpcodeArg>(new IntArg()),
                [](uint32_t a, uint32_t b) -> uint32_t {
                    return (a + b) * (a - b) ^ (a | b);
                },
                4
                ));
    Memory m;
    Registers r;
    Memory stack;

    uint32_t r0 = 220190;
    uint32_t i = 56081982;
    r.set(0, r0);
    m.write_type<uint8_t>(0, 0);
    m.write_type<uint32_t>(1, i);
    opcode->parse_raw(m, 0);
    opcode->execute(r, m, stack);
    EXPECT_EQ(r.get(0), (r0 + i) * (r0 - i) ^ (r0 | i));
}

TEST(BinaryOperationOpcodeTestSuite, RawMemReg){
    std::string name = "kek";
    std::shared_ptr<Opcode> opcode = std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                name, 
                std::shared_ptr<OpcodeArg>(new AddressArg()),
                std::shared_ptr<OpcodeArg>(new RegArg()),
                [](uint32_t a, uint32_t b) -> uint32_t {
                    return a * a * a - b * b & 0xffff132;
                },
                4
                ));
    Memory m;
    Registers r;
    Memory stack;

    uint32_t mem = 28567854;
    uint32_t r0 = 3467867;
    r.set(0, r0);
    m.write_type<uint32_t>(0, 10);
    m.write_type<uint8_t>(4, 0);

    m.write_type<uint32_t>(10, mem);
    opcode->parse_raw(m, 0);
    opcode->execute(r, m, stack);
    EXPECT_EQ(m.read_type<uint32_t>(10), mem * mem * mem - r0 * r0 & 0xffff132);
}

TEST(BinaryOperationOpcodeTestSuite, SaveRegMem){
    std::string name = "kek";
    std::shared_ptr<Opcode> opcode = std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                name, 
                std::shared_ptr<OpcodeArg>(new RegArg()),
                std::shared_ptr<OpcodeArg>(new AddressArg()),
                [](uint32_t a, uint32_t b) -> uint32_t {
                    return a ^ b + b;
                },
                4
                ));
    EXPECT_EQ(opcode->get_name(), name);

    Memory m;
    Registers r;
    Memory stack;

    uint32_t some_addr = 1353244;
    opcode->parse_asm(std::vector<std::string>({"r0", std::to_string(some_addr)}));
    opcode->write_raw(m, 0);

    EXPECT_EQ(m.read_type<uint8_t>(0), 0);
    EXPECT_EQ(m.read_type<uint32_t>(1), some_addr);
}
TEST(BinaryOperationOpcodeTestSuite, SaveMemI){
    std::string name = "kek";
    std::shared_ptr<Opcode> opcode = std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                name, 
                std::shared_ptr<OpcodeArg>(new AddressArg()),
                std::shared_ptr<OpcodeArg>(new IntArg()),
                [](uint32_t a, uint32_t b) -> uint32_t {
                    return a ^ b + b;
                },
                4
                ));
    EXPECT_EQ(opcode->get_name(), name);

    Memory m;
    Registers r;
    Memory stack;

    uint32_t some_addr = 16345634;
    uint32_t some_int = 63456435;
    opcode->parse_asm(std::vector<std::string>({std::to_string(some_addr), std::to_string(some_int)}));
    opcode->write_raw(m, 0);

    EXPECT_EQ(m.read_type<uint32_t>(0), some_addr);
    EXPECT_EQ(m.read_type<uint32_t>(4), some_int);
}

TEST(BinaryOperationOpcodeTestSuite, SaveAsmRegI){
    std::string name = "kek";
    std::shared_ptr<Opcode> opcode = std::shared_ptr<Opcode>(new BinaryOperationOpcode(
                name, 
                std::shared_ptr<OpcodeArg>(new RegArg()),
                std::shared_ptr<OpcodeArg>(new IntArg()),
                [](uint32_t a, uint32_t b) -> uint32_t {
                    return a ^ b + b;
                },
                4
                ));
    EXPECT_EQ(opcode->get_name(), name);

    Memory m;
    Registers r;
    Memory stack;

    uint32_t some_int = 63456435;
    m.write_type<uint8_t>(0, 13);
    m.write_type<uint32_t>(1, some_int);
    opcode->parse_raw(m, 0);
    EXPECT_EQ(opcode->write_asm(), name + " " + "r13, " + std::to_string(some_int));
}
