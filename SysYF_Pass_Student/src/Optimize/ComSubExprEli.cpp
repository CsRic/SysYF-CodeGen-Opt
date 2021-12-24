#include "Pass.h"
#include "ComSubExprEli.h"
#include <set>
#include <algorithm>
#define DEBUG std::cout << "qwq" << std::endl;

void ComSubExprEli::execute() {
    module->set_print_name();
    for(auto fun:module->get_functions()){
        
        func = fun;
        if(func->get_num_basic_blocks()==0)
            continue;
        std::cout << "func"<<func->get_num_basic_blocks()<<std::endl;
        U.clear();
        bb_gen.clear();
        bb_in.clear();
        bb_out.clear();
        ComputeGen();
        
        std::cout << bb_gen[func->get_entry_block()].size()<<std::endl;
        std::cout << "Usize:" << U.size() << std::endl;
        for(auto inst :U){
            std::cout << inst->get_operand(0)<<inst->get_operands().size()<<inst->get_operand(1)->get_name()<<std::endl;
        }
        ComputeInOut();
        //DEBUG
        SubExprEli();
    }
    /*you need to finish this function*/
}

bool ComSubExprEli::is_valid_expr(Instruction *inst) {
    return !(inst->is_void()||inst->is_call()||inst->is_phi()||inst->is_alloca()||inst->is_load()||inst->is_cmp()||inst->is_zext());//TODO:CHECK VALID INST
}

void ComSubExprEli::ComputeGen(){
    for(auto bb:func->get_basic_blocks()){
    
        bb_gen.insert({bb, {}});
        for(auto inst:bb->get_instructions()){
            if(!is_valid_expr(inst))
                continue;
            //std::cout << inst->get_operand(0)<<std::endl;
            bb_gen[bb].insert(inst);
            U.insert(inst);
        }
        //std::cout << bb_gen[bb].size()<<std::endl;
    }
}

void ComSubExprEli::ComputeInOut(){
    bb_out.insert({func->get_entry_block(), bb_gen[func->get_entry_block()]});
    for(auto bb:func->get_basic_blocks()){
        if(bb!=func->get_entry_block()){
            bb_out.insert({bb, U});
            bb_in.insert({bb, {}});
        }
    }
    bool flag = true;//用于标记此次迭代有没有改变
    while (flag)
    {   
        // DEBUG
        flag = false;
        for(auto bb:func->get_basic_blocks()){
            if(bb==func->get_entry_block())
                continue;
            auto oldsize = bb_out[bb].size();
            bool first = true;
            
            for(auto prebb:bb->get_pre_basic_blocks()){
                if(first){
                    //std::cout <<" bbout:"<<bb_out[prebb].size()<<std::endl;
                    bb_in[bb] = bb_out[prebb];
                    first = false;
                }else{
                    std::set<Instruction *, Compare> tmp = {};
                    std::set_intersection(bb_in[bb].begin(), bb_in[bb].end(), bb_out[prebb].begin(), bb_out[prebb].end(), inserter(tmp, tmp.begin()));
                    bb_in[bb] = tmp;
                }
            }
            //std::cout << "bbin:"<<bb_in[bb].size()<<" bbout:"<<bb_out[bb].size()<<std::endl;
            std::set<Instruction *, Compare> tmp = {};
            std::set_union(bb_in[bb].begin(),bb_in[bb].end(),bb_gen[bb].begin(),bb_gen[bb].end(),inserter(tmp, tmp.begin()));
            bb_out[bb] = tmp;
            auto newsize = bb_out[bb].size();
            if(newsize!=oldsize){
                flag = true;
            }
        }
    } 
}




bool cmp(Instruction* a,Instruction*b){
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
                if (dynamic_cast<ConstantInt *>(*begin))
                {
                    if(dynamic_cast<ConstantInt *>(aoprand)->get_value()==dynamic_cast<ConstantInt *>(*begin)->get_value()){
                        bcopy.erase(begin);
                        break;
                    }
                }
            }
            else if(dynamic_cast<ConstantFloat *>(aoprand)){
                if(dynamic_cast<ConstantFloat *>(*begin)){
                    if(dynamic_cast<ConstantFloat *>(aoprand)->get_value()==dynamic_cast<ConstantFloat *>(*begin)->get_value()){
                        bcopy.erase(begin);
                        break;
                    }
                }
            }
            else if(aoprand->get_name()==(*begin)->get_name()){
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

void ComSubExprEli::SubExprEli(){
    
    std::map<Instruction*,Instruction*> delete_list = {};
    for(auto bb:func->get_basic_blocks()){
        
        auto bb_valid = bb_in[bb];
        
        for (auto inst : bb->get_instructions())
        {
            if(!is_valid_expr(inst))
                continue;
            // std::cout << inst->get_operand(0)<<inst->get_operands().size()<<inst->get_operand(1)->get_name()<<std::endl;
            // if(bb_valid.find(inst)!=bb_valid.end()){
            //     std::cout<<"qwq"<<std::endl;
            //     delete_list.insert(inst);
            // }else{
            //     bb_valid.insert(inst);
            //     std::cout<<bb_valid.size()<<std::endl;
            // }
            
            bool flag = false;
            for(auto iter:bb_valid){
                if(!cmp(iter,inst)){
                    delete_list.insert({inst,iter});
                    //inst->replace_all_use_with(iter);
                    flag =true;
                    break;
                }
            }
            if(flag ==false){
                bb_valid.insert(inst);
            }
        }
    }
    for(auto pair:delete_list){
        pair.first->replace_all_use_with(pair.second);
        pair.first->get_parent()->delete_instr(pair.first);
    }
}

