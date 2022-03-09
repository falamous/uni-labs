#include "opcode.h"

size_t IntArg::len() {
    return 4;
}

size_t IntArg::parse_raw(const Memory& m, size_t addr) {
    value = m.read_type<int32_t>(addr);
    return sizeof(uint32_t);
}

size_t IntArg::write_raw(Memory &m, size_t addr) {
    m.write_type<uint32_t>(addr, resolve(m));
    return 4;
}

void IntArg::parse_asm(std::string asm_string) { 
    auto int_parse_res = string_to_int(asm_string);
    if (int_parse_res.second) {
        value = int_parse_res.first;
    } else {
        is_label = true;
        label = asm_string;
    }
}

std::string IntArg::write_asm() {
    if (is_label) {
        return label;
    }
    return std::to_string(value);
}

uint32_t IntArg::resolve(const Memory &m) const {
    if (!is_label) {
        return value;
    }
    return m.resolve_label(label);
}

uint32_t IntArg::get_value(const Registers &r, const Memory& m) const {
    return resolve(m);
}

void IntArg::set_value(Registers &r, Memory& m, uint32_t value, uint8_t value_length) {
    throw std::logic_error("Trying to set consant value.");
}

uint32_t AddressArg::get_value(const Registers &r, const Memory& m) const {
    return m.read_type<uint32_t>(resolve(m));
}

void AddressArg::set_value(Registers &r, Memory& m, uint32_t value, uint8_t value_length) {
    if (value_length > 4) {
        throw std::logic_error("Can't write more than 4 bytes at a time.");
    }
    if (value_length == 3) {
        throw std::logic_error("Can't write 3 bytes.");
    }
    if (value_length == 0) {
        throw std::logic_error("Can't write 0 bytes.");
    }
    if (value_length == 1) {
        m.write_type<uint8_t>(resolve(m), value);
    }
    if (value_length == 2) {
        m.write_type<uint16_t>(resolve(m), value);
    }
    if (value_length == 4) {
        m.write_type<uint32_t>(resolve(m), value);
    }
}

size_t RegArg::parse_raw(const Memory& m, size_t addr) {
    register_number = m.read_type<int8_t>(addr);
    return sizeof(uint8_t);
}

size_t RegArg::write_raw(Memory &m, size_t addr) {
    m.write_type<uint8_t>(addr, register_number);
    return 1;
}

void RegArg::parse_asm(std::string asm_string) { 
    asm_string = to_lower(asm_string);
    if (asm_string == "rip") {
        register_number = RIP;
        return;
    }
    if (asm_string == "rsp") {
        register_number = RSP;
        return;
    }

    if (asm_string[0] != 'r') {
        throw AsmException("Invalid register '%s'.", asm_string.c_str());
    }
    std::string number_string = asm_string.substr(1);

    std::pair<uint32_t, bool> number_conversion_res = string_to_int(number_string);
    if (!number_conversion_res.second) {
        throw AsmException("Could not parse number '%s'.", number_string.c_str());
    }
    register_number = number_conversion_res.first;
}

std::string RegArg::write_asm() {
    return "r" + std::to_string(register_number);
}

uint32_t RegArg::get_value(const Registers &r, const Memory& m) const {
    return r.get(register_number);
}
void RegArg::set_value(Registers &r, Memory& m, uint32_t value, uint8_t value_length) {
    if (value_length > 4) {
        throw std::logic_error("Can't write more than 4 bytes at a time.");
    }
    if (value_length == 3) {
        throw std::logic_error("Can't write 3 bytes.");
    }
    if (value_length == 0) {
        throw std::logic_error("Can't write 0 bytes.");
    }
    r.set(register_number, value & ((uint32_t)((uint64_t)1 << (value_length * 8)) - 1));
}

size_t RegArg::len() {
    return 1;
}

size_t Opcode::len() {
    opcode_length = 0;
    for(auto arg: args_) {
        opcode_length += arg->len();
    }
    return opcode_length;
}

std::string Opcode::get_name() {
    return name_;
}

Opcode::Opcode(std::string name, std::vector<std::shared_ptr<OpcodeArg>> args)
    : args_ {args}
    , name_ {name}
{}

size_t Opcode::write_raw(Memory &m, size_t addr) {
    opcode_length = 0;
    for(auto arg: args_) {
        opcode_length += arg->write_raw(m, addr + opcode_length);
    }
    return opcode_length;
}

size_t Opcode::parse_raw(const Memory& m, size_t addr) {
    opcode_length = 0;
    for(auto arg: args_) {
        opcode_length += arg->parse_raw(m, addr + opcode_length);
    }
    return opcode_length;
}

std::string Opcode::write_asm() {
    std::string asm_string = get_name();

    std::string args_string = "";
    for(auto arg: args_) {
        if (args_string.length()) {
            args_string += ", ";
        }
        args_string += arg->write_asm();
    }

    if (args_string.length()) {
        asm_string = asm_string + " " + args_string;
    }
    return asm_string;
}

void Opcode::parse_asm(std::vector<std::string> asm_strings) {
    if (asm_strings.size() != args_.size()) {
        throw AsmException("Wrong name of arguments %zu to %s.", asm_strings.size(), get_name().c_str());
    }
    for(size_t i = 0; i < args_.size(); i++) {
        args_[i]->parse_asm(asm_strings[i]);
    }
}

BinaryOperationOpcode::BinaryOperationOpcode(std::string name, 
        std::shared_ptr<OpcodeArg> arg1,
        std::shared_ptr<OpcodeArg> arg2,
        std::function<uint32_t (uint32_t, uint32_t)>op, uint8_t value_length)
    : value_length_ {value_length}
    , op_ {op}
, Opcode(name,  std::vector<std::shared_ptr<OpcodeArg>>{arg1, arg2})
{
    if (value_length > 4) {
        throw std::logic_error("Can't write more than 4 bytes at a time.");
    }
    if (value_length == 3) {
        throw std::logic_error("Can't write 3 bytes.");
    }
    if (value_length == 0) {
        throw std::logic_error("Can't write 0 bytes.");
    }
}

bool BinaryOperationOpcode::execute(Registers &r, Memory &m, Memory &stack)  {

    uint32_t a = args_[0]->get_value(r, m) & ((uint32_t)((uint64_t)1 << ((value_length_) * 8)) - 1);
    uint32_t b = args_[1]->get_value(r, m) & ((uint32_t)((uint64_t)1 << ((value_length_) * 8)) - 1);
    args_[0]->set_value(r, m, op_(a,  b), value_length_);
    return false;
}

UnaryOperationOpcode::UnaryOperationOpcode(
        std::string name, std::shared_ptr<OpcodeArg> arg,
        std::function<uint32_t (uint32_t)> op, uint8_t value_length
        ) 
: op_ {op}
, value_length_ {value_length}
, Opcode(name,  std::vector<std::shared_ptr<OpcodeArg>>{arg})
{}

bool UnaryOperationOpcode::execute(Registers &r, Memory &m, Memory &stack) {
    uint32_t a = args_[0]->get_value(r, m) & ((uint32_t)((uint64_t)1 << ((value_length_) * 8)) - 1);
    args_[0]->set_value(r, m, op_(a), value_length_);
    return false;
}

JumpOpcode::JumpOpcode(std::string name, std::vector<std::shared_ptr<OpcodeArg>> args,
        std::function<bool (std::vector<uint32_t>)> holds
        )
: holds_ {holds}
, Opcode(name, args)
{ }

bool JumpOpcode::execute(Registers &r, Memory &m, Memory &stack)  {
    uint32_t jump_addr = args_[0]->get_value(r, m);

    std::vector<uint32_t> values;
    for(auto arg: args_) {
        values.push_back(arg->get_value(r, m));
    }

    if (holds_(values)) {
        r.set(RIP, jump_addr);
    }
    return false;
}

ExitOpcode::ExitOpcode(std::string name)
    : Opcode(name, {})
{}

bool ExitOpcode::execute(Registers &r, Memory &m, Memory &stack) {
    return true;
}

PushOpcode::PushOpcode(std::string name, std::shared_ptr<OpcodeArg> arg, uint8_t value_length) 
    : Opcode(name, {arg})
    , value_length_ {value_length}
{}

bool PushOpcode::execute(Registers &r, Memory &m, Memory &stack) {
    uint32_t rsp = r.get(RSP);
    uint32_t value = args_[1]->get_value(r, m);
    if (value_length_ > 4) {
        throw std::logic_error("Can't write more than 4 bytes at a time.");
    }
    if (value_length_ == 3) {
        throw std::logic_error("Can't write 3 bytes.");
    }
    if (value_length_ == 0) {
        throw std::logic_error("Can't write 0 bytes.");
    }
    if (value_length_ == 1) {
        stack.write_type<uint8_t>(rsp, value);
    }
    if (value_length_ == 2) {
        stack.write_type<uint16_t>(rsp, value);
    }
    if (value_length_ == 4) {
        stack.write_type<uint32_t>(rsp, value);
    }
    rsp += value_length_;
    r.set(RSP, rsp);
    return false;
}

PopOpcode::PopOpcode(std::string name, std::shared_ptr<OpcodeArg> arg, uint8_t value_length) 
    : Opcode(name, {arg})
    , value_length_ {value_length}
{}

bool PopOpcode::execute(Registers &r, Memory &m, Memory &stack) {
    uint32_t rsp = r.get(RSP);
    if (rsp < value_length_) {
        throw std::runtime_error("Callstack analysis failed, positive sp found.");
    }
    args_[1]->set_value(r, m, stack.read_type<uint32_t>(rsp - value_length_), value_length_);
    rsp -= value_length_;
    return false;
}

CallOpcode::CallOpcode(std::string name)
    : Opcode(name, {std::shared_ptr<OpcodeArg>(new IntArg())})
{}

bool CallOpcode::execute(Registers &r, Memory &m, Memory &stack) {
    uint32_t rsp = r.get(RSP);
    uint32_t rip = r.get(RIP);
    stack.write_type<uint32_t>(rsp, rip);
    rsp += 4;
    r.set(RSP, rsp);
    return false;
}

RetOpcode::RetOpcode(std::string name)
    : Opcode(name, {})
{}

bool RetOpcode::execute(Registers &r, Memory &m, Memory &stack) {
    uint32_t rsp = r.get(RSP);
    if (rsp < 4) {
        throw std::runtime_error("Callstack analysis failed, positive sp found.");
    }

    uint32_t rip = stack.read_type<uint32_t>(rsp - 4);
    rsp -= 4;

    r.set(RSP, rsp);
    r.set(RIP, rip);
    return false;
}

std::shared_ptr<OpcodeArg> IntArg::clone() {
    return std::shared_ptr<OpcodeArg>(new IntArg(*this));
}
std::shared_ptr<OpcodeArg> AddressArg::clone() {
    return std::shared_ptr<OpcodeArg>(new AddressArg(*this));
}
std::shared_ptr<OpcodeArg> RegArg::clone() {
    return std::shared_ptr<OpcodeArg>(new RegArg(*this));
}
std::shared_ptr<Opcode> BinaryOperationOpcode::clone() {
    return std::shared_ptr<Opcode>(new BinaryOperationOpcode(name_, args_[0]->clone(), args_[1]->clone(), op_, value_length_));
}
std::shared_ptr<Opcode> UnaryOperationOpcode::clone() {
    return std::shared_ptr<Opcode>(new UnaryOperationOpcode(name_, args_[0]->clone(), op_, value_length_));
}
std::shared_ptr<Opcode> JumpOpcode::clone() {
    auto args = args_;
    for(auto &arg: args) {
        arg = arg->clone();
    }
    return std::shared_ptr<Opcode>(new JumpOpcode(name_, args, holds_));
}
std::shared_ptr<Opcode> CallOpcode::clone() {
    return std::shared_ptr<Opcode>(new CallOpcode(name_));
}
std::shared_ptr<Opcode> RetOpcode::clone() {
    return std::shared_ptr<Opcode>(new RetOpcode(name_));
}
std::shared_ptr<Opcode> PushOpcode::clone() {
    return std::shared_ptr<Opcode>(new PushOpcode(name_, args_[0]->clone(), value_length_));
}
std::shared_ptr<Opcode> PopOpcode::clone() {
    return std::shared_ptr<Opcode>(new PopOpcode(name_, args_[0]->clone(), value_length_));
}
std::shared_ptr<Opcode> ExitOpcode::clone() {
    return std::shared_ptr<Opcode>(new ExitOpcode(name_));
}
