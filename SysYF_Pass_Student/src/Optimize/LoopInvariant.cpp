#include "LoopInvariant.h"
#include <algorithm>

void LoopInvariant::execute(){
    /*you need to finish this function*/
    module->set_print_name();

    for (auto func : module->get_functions()) {
        if (func->get_basic_blocks().empty())
            continue;
        EntryBlock = func->get_entry_block();
        CurrentFunction = func;
        FindLoops();
        Hoist();
    }
}
void LoopInvariant::FindLoops() {
    for (auto bb : CurrentFunction->get_basic_blocks()) {
        if (bb == CurrentFunction->get_entry_block())continue;
        auto dominator = bb;//只需考察bb是否有到其支配链上的回边
        do {
            dominator = dominator->get_idom();
            for (auto succ : bb->get_succ_basic_blocks()) {
                if (succ == dominator) {//找到一条回边, 开始记录一个自然循环
                    LoopRecord* loop_record = new LoopRecord;
                    loop_record->Loop.clear();
                    loop_record->BrBlock = bb;
                    loop_record->ToBlock = dominator;
                    loop_record->Loop.insert(bb);
                    loop_record->Loop.insert(dominator);
                    std::vector<BasicBlock*>stack;
                    stack.push_back(bb);
                    while (!stack.empty()) {
                        auto m = stack.back();
                        stack.pop_back();
                        for (auto pred : m->get_pre_basic_blocks()) {
                            if (loop_record->Loop.find(pred) == loop_record->Loop.end()) {//没找到
                                loop_record->Loop.insert(pred);
                                stack.push_back(pred);
                            }
                        }
                    }
                    //填入自然循环记录表
                    AllLoops.insert(loop_record);
                    break;
                }
                //没有回边
            }
        } while (dominator != EntryBlock);
    }
}

void LoopInvariant::FindLoopInvariantIns(LoopRecord* loop) {
    Invariants.clear();
    std::set<Value*>define_list;//用以判断指令使用的参数是否定义在循环之外
    for (auto bb : loop->Loop) {
        for (auto ins : bb->get_instructions()) {
            if (ins->is_store()) {
                define_list.insert(ins->get_operand(1));
            }
            else {
                define_list.insert(ins);
            }
        }
    }
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto bb : loop->Loop) {
            for (auto ins : bb->get_instructions()) {
                bool is_inv = true;
                if (ins->is_call() || ins->is_alloca() || ins->is_ret() || ins->is_br() || ins->is_cmp() || ins->is_phi() || ins->is_load()) {
                    continue;
                }
                if (define_list.find(ins) == define_list.end()) {
                    continue;
                }
                if (Invariants.find(ins) != Invariants.end()) {
                    continue;
                }
               
                for (auto operand : ins->get_operands()) {
                    if (dynamic_cast<ConstantFloat*>(operand) || dynamic_cast<ConstantInt*>(operand)) {//常数
                        continue;
                    }
                    if (define_list.find(operand) == define_list.end()) {//定义在循环外
                        continue;
                    }
                    if (Invariants.find(operand) != Invariants.end()) {//已被标记为inv
                        continue;
                    }
                    is_inv = false;
                    break;
                }
                if (is_inv) {
                    Invariants.insert(ins);
                    //std::cout <<"Find invariant " << ins->get_name() << "  in block "<< bb->get_name() <<std::endl;
                    changed = true;
                }
            }
        }
    }
}

void LoopInvariant::Hoist() {
    /* 
        要从内层循环开始外提，所以在每个循环开始时判断是否有内层循环，先执行内层循环的外提
        但是给循环排序太麻烦了
        直接判断一个循环是不是在某个外层循环中，外提后执行外层的循环外提。虽然外层循环可能执行了多遍外提，没所谓。。。
        如果一个循环的ToBlock在别的loop中且不是ToBlock，则可确定它是loop的内层循环
    */
    std::set<LoopRecord*>well_done_loop;//已经保证不需要再重复外提的loop
    for (auto loop : AllLoops) {
        
        if (well_done_loop.find(loop) != well_done_loop.end())continue;
        std::vector<LoopRecord*>stack;
        stack.clear();
        stack.push_back(loop);
        while (!stack.empty()) {
            
            auto loop1 = stack.back();
            stack.pop_back();
            FindLoopInvariantIns(loop1);
            BasicBlock* new_block;
            bool empty = true;
            if (!Invariants.empty()) {
                empty = false;
                std::vector<BasicBlock*> ToLoopBlocks;//所有循环外指向ToBlock的bb
                for (auto bb : loop1->ToBlock->get_pre_basic_blocks()) {
                    if (loop1->Loop.find(bb) == loop1->Loop.end()) {
                        ToLoopBlocks.push_back(bb);
                    }
                }
                //外提到将要插到ToBlock之前的new_block中
                new_block = BasicBlock::create(module, "lih_"+loop->ToBlock->get_name(), loop1->ToBlock->get_parent());
                //唯一问题是ToBlock中的phi指令，路径被更改了
                //把这些phi指令拆分，路径源于循环外的 作为新的phi指令放到new_block中，然后ToBlock中的phi指令只可能有两条来源：循环内部bb，new_block
                for (auto ins : loop1->ToBlock->get_instructions()) {
                    if (!ins->is_phi()) {
                        continue;
                    }
                    auto phi_ins = dynamic_cast<PhiInst*>(ins);
                    std::vector<std::pair<Value*, BasicBlock*>> source_outside;
                    std::vector<std::pair<Value*, BasicBlock*>> source_inside;
                    for (unsigned int i = 0; i < phi_ins->get_num_operand(); i += 2) {
                        auto val = ins->get_operand(i);
                        auto source = dynamic_cast<BasicBlock*>(phi_ins->get_operand(i + 1));
                        val->remove_use(ins);
                        source->remove_use(ins);

                        if (loop1->Loop.find(source) == loop1->Loop.end()) {
                            source_outside.push_back({ val,source });
                        }
                        else {
                            source_inside.push_back({ val,source });
                        }
                    }
                    if (source_outside.size() > 1) {
                        auto new_phi = PhiInst::create_phi(source_outside.begin()->first->get_type(), new_block);
                        new_block->add_instruction(new_phi);
                        for (auto pair : source_outside) {
                            new_phi->add_phi_pair_operand(pair.first, pair.second);
                        }
                        phi_ins->remove_operands(0, phi_ins->get_num_operand() - 1);
                        for (auto pair : source_inside) {
                            phi_ins->add_phi_pair_operand(pair.first, pair.second);
                        }
                        phi_ins->add_phi_pair_operand(new_phi, new_block);
                    }
                    else {
                        phi_ins->remove_operands(0, phi_ins->get_num_operand() - 1);
                        for (auto pair : source_inside) {
                            phi_ins->add_phi_pair_operand(pair.first, pair.second);
                        }
                        for (auto pair : source_outside) {
                            phi_ins->add_phi_pair_operand(pair.first, new_block);
                        }
                    }
                }
                
                for (auto inv : Invariants) {
                    for (auto bb : loop->Loop) {
                        std::vector<Instruction*>hoist_list;
                        for (auto ins : bb->get_instructions()) {
                            if (ins == inv) {
                                hoist_list.push_back(ins);
                            }
                        }
                        for (auto ins : hoist_list) {
                            bb->get_instructions().remove(ins);
                            new_block->add_instruction(ins);
                            ins->set_parent(new_block);
                        }
                    }
                }

                for (auto bb : ToLoopBlocks) {
                    auto terminator = bb->get_terminator();
                    if (terminator->get_num_operand() == 1) {
                        terminator->set_operand(0, new_block);
                    }
                    else{
                        if (dynamic_cast<BasicBlock*>(terminator->get_operand(1)) == loop->ToBlock) {
                            terminator->set_operand(1, new_block);
                        }
                        if (dynamic_cast<BasicBlock*>(terminator->get_operand(2)) == loop->ToBlock) {
                            terminator->set_operand(2, new_block);
                        }
                    }
                    bb->remove_succ_basic_block(loop->ToBlock);
                    bb->add_succ_basic_block(new_block);
                    new_block->add_pre_basic_block(bb);
                    loop->ToBlock->remove_use(terminator);
                    loop->ToBlock->remove_pre_basic_block(bb);
                }
                //std::cout << new_block->get_name() << "\n";
                new_block->add_succ_basic_block(loop->ToBlock);
                //添加new_block到ToBlock的无条件跳转
                BranchInst::create_br(loop->ToBlock, new_block);
            }
            for (auto outerloop : AllLoops) {
                if (outerloop == loop1)continue;
                if (outerloop->ToBlock != loop1->ToBlock && outerloop->Loop.find(loop1->ToBlock) != outerloop->Loop.end() ) {
                    //找到外层循环,入栈
                    stack.push_back(outerloop);
                    if (well_done_loop.find(outerloop) != well_done_loop.end()) {
                        well_done_loop.insert(outerloop);
                    }
                    //顺便把新加的new_block更新到外层循环中
                    if (!empty) {
                        outerloop->Loop.insert(new_block);
                    }
                }
            }
        }
    }
}