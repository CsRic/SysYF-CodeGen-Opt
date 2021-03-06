#ifndef BAZINGA_COMPILER_FUNCTION_H
#define BAZINGA_COMPILER_FUNCTION_H

#include <iterator>
#include <list>
#include <cstddef>
#include <map>
#include <cassert>

#include "User.h"
#include "Module.h"
#include "BasicBlock.h"
#include "Type.h"

class Module;
class Argument;
class BasicBlock;
class Type;
class FunctionType;

class Function : public Value
{
public:
    Function(FunctionType *ty, const std::string &name, Module *parent);
    ~Function();
    static Function *create(FunctionType *ty, const std::string &name, Module *parent);

    FunctionType *get_function_type() const;

    Type *get_return_type() const;

    void add_basic_block(BasicBlock *bb);

    unsigned get_num_of_args() const;
    unsigned get_num_basic_blocks() const;
    
    Module *get_parent() const;

    std::list<Argument *>::iterator arg_begin() { return arguments_.begin(); }
    std::list<Argument *>::iterator arg_end() { return arguments_.end(); }
    
    void remove(BasicBlock* bb);
    BasicBlock *get_entry_block() { return *basic_blocks_.begin(); }

    std::list<BasicBlock *> &get_basic_blocks() { return basic_blocks_; }
    std::list<Argument *> &get_args() { return arguments_; }
    
    bool is_declaration() { return basic_blocks_.empty(); }
    
    void set_instr_name();
    std::string print();

private:
    void build_args();

private:
    std::list<BasicBlock *> basic_blocks_;
    std::list<Argument *> arguments_;
    Module *parent_;
    unsigned seq_cnt_;
};

class Argument : public Value
{
public:
    explicit Argument(Type *ty, const std::string &name = "", Function *f = nullptr,
                    unsigned arg_no = 0)
        : Value(ty, name), parent_(f), arg_no_(arg_no) {}
    ~Argument() {}

    inline const Function *get_parent() const { return parent_; }
    inline       Function *get_parent()       { return parent_; }
    Argument* deepcopy(){
        return new Argument(type_, name_, parent_, arg_no_);
    }
    unsigned get_arg_no() const {
        return arg_no_;
    }
    
    virtual std::string print() override ;
private:
    Function *parent_;
    unsigned arg_no_;
};

#endif // BAZINGA_COMPILER_FUNCTION_H
