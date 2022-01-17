#ifndef SYSYF_ACTIVEVAR_H
#define SYSYF_ACTIVEVAR_H

#include "Pass.h"
#include "Module.h"

class ActiveVar : public Pass
{
public:
    ActiveVar(Module *module) : Pass(module) {}
    void execute() final;
    std::set<Value *>* DefValueGet(BasicBlock * block);
    std::set<Value *>* UseValueGet(BasicBlock * block, std::set<Value *>* def_value_set);
    void UseDefValueGen();
    void InOutValueGen();
    void ShowResult();
    const std::string get_name() const override {return name;}
private:
    Function *func_;
    const std::string name = "ActiveVar";
    std::map<BasicBlock *, std::set<Value *>*> bb_def_val;//实际上我们是把每个指令看作一个变量，但是Instruction类是继承Value的，为了和后面统一，也用Value
    std::map<BasicBlock *, std::set<Value *>*> bb_use_val;
    std::map<BasicBlock *, std::map<BasicBlock *, std::set<Value *>>> bb_pre_not_act_val;
    std::map<BasicBlock *, std::set<Value *>> bb_in;
    std::map<BasicBlock *, std::set<Value *>> bb_out;
};

#endif  // SYSYF_ACTIVEVAR_H
