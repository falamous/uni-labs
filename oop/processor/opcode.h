#pragma once
#include <bits/stdc++.h>
#include "register.h"
#include "memory.h"
#include "util.h"
#include "error.h"

class OpcodeArg {
    public:
    virtual size_t len() = 0;
    virtual std::shared_ptr<OpcodeArg> clone() = 0;
    virtual size_t parse_raw(const Memory& m, size_t address) = 0;
    virtual size_t write_raw(Memory &m, size_t address) = 0;
    virtual void parse_asm(std::string asm_string) = 0;
    virtual std::string write_asm() = 0;
    virtual uint32_t get_value(const Registers &r, const Memory& m) const = 0;
    virtual void set_value(Registers &r, Memory& m, uint32_t value, uint8_t value_length=4) = 0;
};

class IntArg : public OpcodeArg {

    protected:
    bool is_label = false;
    uint32_t value;
    std::string label;

    public:
    size_t parse_raw(const Memory& m, size_t addr);

    size_t len();
    std::shared_ptr<OpcodeArg> clone() ;
    size_t write_raw(Memory &m, size_t addr);
    void parse_asm(std::string asm_string);
    std::string write_asm();
    uint32_t resolve(const Memory &m) const;
    virtual uint32_t get_value(const Registers &r, const Memory& m) const;
    virtual void set_value(Registers &r, Memory& m, uint32_t value, uint8_t value_length=4);
};

class AddressArg : public IntArg {
    std::shared_ptr<OpcodeArg> clone() ;
    uint32_t get_value(const Registers &r, const Memory& m) const;
    void set_value(Registers &r, Memory& m, uint32_t value, uint8_t value_length=4);
};

class RegArg : public OpcodeArg {
    uint8_t register_number;
    public:
    std::shared_ptr<OpcodeArg> clone() ;
    size_t len();
    size_t parse_raw(const Memory& m, size_t addr);
    size_t write_raw(Memory &m, size_t addr);
    void parse_asm(std::string asm_string);
    std::string write_asm();
    uint32_t get_value(const Registers &r, const Memory& m) const;
    void set_value(Registers &r, Memory& m, uint32_t value, uint8_t value_length=4);

};

class Opcode {
    protected:
    std::vector<std::shared_ptr<OpcodeArg>> args_;
    std::string name_;
    size_t opcode_length = 0;

    public:
    virtual bool execute(Registers &r, Memory &m, Memory &stack) = 0;

    std::string get_name();
    Opcode(std::string name, std::vector<std::shared_ptr<OpcodeArg>> args);

    virtual std::shared_ptr<Opcode> clone() = 0;
    virtual size_t len();
    virtual size_t write_raw(Memory &m, size_t addr);
    virtual size_t parse_raw(const Memory& m, size_t addr);
    virtual std::string write_asm();
    virtual void parse_asm(std::vector<std::string> asm_strings);
    virtual ~Opcode() = default;
};

class UnaryOperationOpcode : public Opcode {
    uint8_t value_length_ = 0;
    std::function<uint32_t (uint32_t)> op_;

    public:
    UnaryOperationOpcode(
            std::string name, std::shared_ptr<OpcodeArg> arg,
            std::function<uint32_t (uint32_t)> op, uint8_t value_length
            );

    std::shared_ptr<Opcode> clone() ;
     bool execute(Registers &r, Memory &m, Memory &stack);
};

class BinaryOperationOpcode : public Opcode {
    uint8_t value_length_ = 0;
    std::function<uint32_t (uint32_t, uint32_t)> op_;

    public:
    BinaryOperationOpcode(
            std::string name, 
            std::shared_ptr<OpcodeArg> arg1,
            std::shared_ptr<OpcodeArg> arg2,
            std::function<uint32_t (uint32_t, uint32_t)> op, uint8_t value_length
            );

    std::shared_ptr<Opcode> clone() ;
     bool execute(Registers &r, Memory &m, Memory &stack);
};

class JumpOpcode : public Opcode {
    public:
    std::function<bool (std::vector<uint32_t>)> holds_;
    JumpOpcode(std::string name, std::vector<std::shared_ptr<OpcodeArg>> args,
            std::function<bool (std::vector<uint32_t>)> holds
            );
    std::shared_ptr<Opcode> clone() ;
    bool execute(Registers &r, Memory &m, Memory &stack);
};

class ExitOpcode : public Opcode {
    public:
    ExitOpcode(std::string name);
    std::shared_ptr<Opcode> clone() ;
    bool execute(Registers &r, Memory &m, Memory &stack);
};

class PushOpcode : public Opcode {
    uint8_t value_length_ = 0;

    public:
    PushOpcode(
            std::string name, 
            std::shared_ptr<OpcodeArg> arg,
            uint8_t value_length
            );
    std::shared_ptr<Opcode> clone() ;

     bool execute(Registers &r, Memory &m, Memory &stack);
};

class PopOpcode : public Opcode {
    uint8_t value_length_ = 0;

    public:
    PopOpcode(
            std::string name, 
            std::shared_ptr<OpcodeArg> arg,
            uint8_t value_length
            );

    std::shared_ptr<Opcode> clone() ;
     bool execute(Registers &r, Memory &m, Memory &stack);
};

class CallOpcode : public Opcode {
    public:
    CallOpcode(std::string name);

    std::shared_ptr<Opcode> clone() ;
    bool execute(Registers &r, Memory &m, Memory &stack);
};

class RetOpcode : public Opcode {
    public:
    RetOpcode(std::string name);

    std::shared_ptr<Opcode> clone() ;
    bool execute(Registers &r, Memory &m, Memory &stack);
};
