#include "Pass.h"
#include "ComSubExprEli.h"
#include <set>
#include <algorithm>
#define DEBUG std::cout << "qwq" << std::endl;

bool cmp(Instruction *a, Instruction *b);
Instruction *copyinst(Instruction *srcinst);
void ComSubExprEli::execute()
{
    module->set_print_name();
    for(auto fun:module->get_functions()){
        bool flag = false;
        while (!flag)
        {
            func = fun;
            if(func->get_num_basic_blocks()==0)
                break;
            // U.clear();
            // bb_gen.clear();
            // bb_in.clear();
            // bb_out.clear();
            // ComputeGen();
            ComputeInOut();
            flag = SubExprEli();
            flag = true;
        }
    }
    /*you need to finish this function*/
}

bool ComSubExprEli::is_valid_expr(Instruction *inst) {
    return !(inst->is_void()||inst->is_call()||inst->is_phi()||inst->is_alloca()||inst->is_load()||inst->is_cmp()||inst->is_zext());//TODO:CHECK VALID INST
}

void ComSubExprEli::ComputeGen(){
    for(auto bb:func->get_basic_blocks()){
    
        bb_gen.insert({bb, {}});
        //   std::cout << bb->get_instructions().size() << std::endl;
        for (auto inst : bb->get_instructions())
        {
            if(!is_valid_expr(inst))
                continue;
            bb_gen[bb].insert(inst);
            U.insert(inst);
        }
    }
}

void ComSubExprEli::ComputeInOut(){
    bool insert_or_not = true;//标记这次有没有插入新的语句
    while(insert_or_not){
        U.clear();
        bb_gen.clear();
        bb_in.clear();
        bb_out.clear();
        ComputeGen();
        bb_out.insert({func->get_entry_block(), bb_gen[func->get_entry_block()]});
        for (auto bb : func->get_basic_blocks())
        {
            if(bb!=func->get_entry_block()){
                bb_out.insert({bb, U});
                bb_in.insert({bb, {}});
            }
        }
        bool flag = true; //用于标记此次迭代有没有改变
        while (flag)
        {   
            // DEBUG
            flag = false;
            for(auto bb:func->get_basic_blocks()){
                if(bb==func->get_entry_block())
                    continue;
                auto oldsize = bb_out[bb].size();
                bool first = true;
                for (auto prebb : bb->get_pre_basic_blocks())
                {
                    if(first){
                        bb_in[bb] = bb_out[prebb];
                        first = false;
                    }else{
                        std::set<Instruction *, Compare> tmp = {};
                        //std::set_intersection(tmp1.begin(), tmp1.end(), bb_out[prebb].begin(), bb_out[prebb].end(), inserter(tmp2,tmp2.begin()));
                        //上面求交集总求错，下面是手动求交集
                        for(auto iter:bb_out[prebb]){
                            for(auto inst:bb_in[bb]){
                                if(!cmp(iter,inst)){
                                    tmp.insert(inst);
                                }
                            }
                        }
                        bb_in[bb] = tmp;
                    }
                }
                std::set<Instruction *, Compare> tmp = {};
                //std::set_union(bb_in[bb].begin(),bb_in[bb].end(),bb_gen[bb].begin(),bb_gen[bb].end(),inserter(tmp, tmp.begin()));
                //上面求并集总求错，下面是手动求并集
                for(auto in :bb_in[bb]){
                    tmp.insert(in);
                }
                for(auto gen:bb_gen[bb]){
                    tmp.insert(gen);
                }

                bb_out[bb] = tmp;
                // std::cout << "bbcount:" << bb->get_instructions().size()
                // << " bb_pre_count" << bb->get_pre_basic_blocks().size()
                // << " bbin:"<<bb_in[bb].size()<<" bbout:"<<bb_out[bb].size()<<std::endl;
                auto newsize = bb_out[bb].size();
                if(newsize!=oldsize){
                    flag = true;
                }
            }
        }
        insert_or_not = false;
        for(auto bb:func->get_basic_blocks()){
            auto bb_dom = bb->get_idom();
            for(auto bb_inst:bb_in[bb]){
                bool find = false;//用来指示有没有找到对应的指令
                for (auto bb_dom_inst : bb_out[bb_dom])
                {
                    if(!cmp(bb_inst,bb_dom_inst)){
                        find = true;
                    }
                }
                if(!find){
                    bb_inst->get_parent()->get_instructions().remove(bb_inst);
                    //std::cout << "bbdomcount:" << bb_dom->get_instructions().size();
                    bb_dom->add_instruction(bb_inst);
                    // std::cout << " bbdomcount:" << bb_dom->get_instructions().size() << std::endl;
                    insert_or_not = true;
                }
            }
        }
    }
}




bool cmp(Instruction* a,Instruction*b){
    //相等返回false ，不相等返回true
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

bool ComSubExprEli::SubExprEli(){
    //若删除了指令，则返回假，否则返回真
    std::map<Instruction*,Instruction*> delete_list = {};
    for(auto bb:func->get_basic_blocks()){
        
        auto bb_valid = bb_in[bb];
        
        for (auto inst : bb->get_instructions())
        {
            if(!is_valid_expr(inst))
                continue;
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
    return delete_list.empty();
}

