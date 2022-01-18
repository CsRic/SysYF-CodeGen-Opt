#ifndef SYSYF_CONSTPROPAGATION_H
#define SYSYF_CONSTPROPAGATION_H
#include "Pass.h"
#include "Constant.h"
#include "Instruction.h"
#include "Module.h"
#include "Value.h"
#include "IRStmtBuilder.h"

// 用来判断value是否为ConstantFloat/ConstantInt
ConstantInt *cast_to_const_int(Value *value);
ConstantFloat *cast_to_const_float(Value *value);

class ConstFolder
{
public:
    ConstFolder(Module *module) : module_(module) {}

    ConstantInt *compute(Instruction::OpID op, ConstantInt *value1, ConstantInt *value2);
    ConstantFloat *compute_float(Instruction::OpID op, ConstantFloat *value1, ConstantFloat *value2);
    ConstantInt *compute_cmp(CmpInst::CmpOp op, ConstantInt *value1, ConstantInt *value2);
    ConstantInt *compute_fcmp(FCmpInst::CmpOp op, ConstantFloat *value1, ConstantFloat *value2);

    ConstantInt *compute_fptosi(ConstantFloat *value1);
    ConstantFloat *compute_sitofp(ConstantInt *value1);
    ConstantInt *compute_zext(ConstantInt *value1);
private:
    Module *module_;
};

class ConstPropagation : public Pass
{
private:
    Function *func_;
    ConstFolder *const_folder;
    const std::string name = "ConstPropagation";
    std::map<BasicBlock *, std::set<Instruction *>> DeleteSet; //每个块中要被删除的指令
    std::map<Value *, Constant *> BlockConstSet; //每个块内的常量

public:
    ConstPropagation(Module *module) : Pass(module) {
        const_folder = new ConstFolder(module);
    }
    void execute() final;
    const std::string get_name() const override {return name;}
    void IntraBlockVarCompute();
    void InterBlockVarCompute();
    void DeleteConstCondBr();
    void DeleteConstInst();
};

#endif  // SYSYF_CONSTPROPAGATION_H