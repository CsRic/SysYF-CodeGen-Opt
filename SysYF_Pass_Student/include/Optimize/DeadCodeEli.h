#ifndef SYSYF_DEADCODEELI_H
#define SYSYF_DEADCODEELI_H

#include "Pass.h"
#include "Module.h"

class DeadCodeEli : public Pass
{
public:
    DeadCodeEli(Module *module) : Pass(module) {}
    void execute() final;
    const std::string get_name() const override {return name;}
    static bool is_valid_expr(Instruction *inst);
    static bool is_critical_expr(Instruction *inst);
    void clean_inst(Instruction *inst);
    void initialize_mark(Function* fun);
    void mark(Function *fun);
    void sweep(Function *fun);
    void find_nearest_marked_pdombb(BasicBlock *bb);
    std::map<Instruction *, bool> inst_mark;
    std::map<BasicBlock *, bool> bb_mark;

private:
    Function *func_;
    const std::string name = "DeadCodeElimination";
};

#endif  // SYSYF_DEADCODEELI_H
