#include "DeadCodeEli.h"

bool DeadCodeEli::is_valid_expr(Instruction* inst){
    return !(inst->is_void());
}

void DeadCodeEli::execute() {
    // Start from here!
    std::list<Instruction *> delete_list = {};
    for(auto fun:module->get_functions()){
        //找到所有引用列表为空的指令，插入待删除的列表中
        for(auto bb:fun->get_basic_blocks()){
            for(auto inst:bb->get_instructions()){
                if(inst->get_use_list().empty()){
                    if(!is_valid_expr(inst))
                        continue;
                    delete_list.push_back(inst);
                }
            }
        }
        for(auto inst:delete_list){
            clean_inst(inst);
        }
    }
}


void DeadCodeEli::clean_inst(Instruction* inst){
    //将指令inst从其操作数的use列表中移除并删除inst
    for(auto operand:inst->get_operands()){
        if(dynamic_cast<ConstantInt *>(operand)){
            continue;//常数不用管
        }
        if(dynamic_cast<Instruction*>(operand)){
            auto curinst = dynamic_cast<Instruction *>(operand);//操作数对应的指令
            curinst->remove_use(dynamic_cast<Value *>(inst));
            if(curinst->get_use_list().empty()){
                clean_inst(curinst);
            }
        }
    }
    inst->get_parent()->delete_instr(inst);
}