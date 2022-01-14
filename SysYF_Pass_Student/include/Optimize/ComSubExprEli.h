#ifndef SYSYF_COMSUBEXPRELI_H
#define SYSYF_COMSUBEXPRELI_H

#include "Pass.h"
#include <map>
#include <set>
#include <algorithm>
#include "Instruction.h"

class Compare{
    public:
bool operator()(Instruction* a,Instruction*b)const{
    // if(a->get_operands().size()!=b->get_operands().size()){
    //     return true;
    // }
    // for(auto aoprand:a->get_operands()){
    //     if(find(b->get_operands().begin(),b->get_operands().end(),aoprand)==b->get_operands().end()){
    //         return true;
    //     }
    // }
    // return false;
    if(a->get_operands().size()!=b->get_operands().size()){
        return true;
    }
    if(a->get_instr_type()!=b->get_instr_type()){
        return true;
    }
    std::vector<Value *> bcopy;
    bcopy.assign(b->get_operands().begin(), b->get_operands().end());
    for(auto aoprand:a->get_operands()){
        auto begin = bcopy.begin();
        auto end = bcopy.end();
        while(begin!=end){
            if(dynamic_cast<ConstantInt *>(aoprand)){
                if(dynamic_cast<ConstantInt *>(*begin)){
                    if(dynamic_cast<ConstantInt *>(aoprand)->get_value()==dynamic_cast<ConstantInt *>(*begin)->get_value()){
                        bcopy.erase(begin);
                        break;
                    }
                }
            }
            if(dynamic_cast<ConstantFloat *>(aoprand)){
                if(dynamic_cast<ConstantFloat *>(*begin)){
                    if(dynamic_cast<ConstantFloat *>(aoprand)->get_value()==dynamic_cast<ConstantFloat *>(*begin)->get_value()){
                        bcopy.erase(begin);
                        break;
                    }
                }
            }
            if(aoprand->get_name()==(*begin)->get_name()){
                bcopy.erase(begin);
                break;
            }
            begin++;
            if(begin==end){
                return true;
            }
        }
    }
    return false;
}
};

/*****************************CommonSubExprElimination**************************************/
/***************************This class is based on SSA form*********************************/
/***************************you need to finish this class***********************************/
class ComSubExprEli : public Pass {
public:
    explicit ComSubExprEli(Module* module):Pass(module){}
    const std::string get_name() const override {return name;}
    void execute() override;
    static bool is_valid_expr(Instruction *inst);
    void ComputeGen();
    void ComputeInOut();
    bool SubExprEli();

private:
    const std::string name = "ComSubExprEli";
    Function *func;
    std::set<Instruction *,Compare> U;
    std::map<BasicBlock *, std::set<Instruction *,Compare>> bb_in;
    std::map<BasicBlock *, std::set<Instruction *,Compare>> bb_out;
    std::map<BasicBlock *, std::set<Instruction *,Compare>> bb_gen;
    // SSA中bb_kill都为空集
};

#endif // SYSYF_COMSUBEXPRELI_H
