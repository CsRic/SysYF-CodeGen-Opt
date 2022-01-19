#include "ConstPropagation.h"
#include "DominateTree.h"

using namespace std;

// 给出了返回整型值的常数折叠实现，大家可以参考，在此基础上拓展
// 当然如果同学们有更好的方式，不强求使用下面这种方式

void ConstPropagation::execute() {
    for (auto &func : this->module->get_functions()) {
        if (func->get_basic_blocks().empty()) {
            continue;
        } else {
            func_ = func;

            //计算每个函数之前单独初始化
            DeleteSet.clear();
            
            for(auto bb : func_->get_basic_blocks()){
                DeleteSet.insert({bb, {}});
            }

            //先 计算每个块内的常量
            IntraBlockVarCompute();
            //再 删除恒定的跳转块块
            DeleteConstCondBr();
            //再 删除值得删除的语句
            DeleteConstInst();
        }
    }
    
    DominateTree dom_tree(module); 
    dom_tree.execute();
    return ;
}

//助教给的整数的常量折叠
ConstantInt *ConstFolder::compute(Instruction::OpID op, ConstantInt *value1, ConstantInt *value2) {
    int const_value1 = value1->get_value();
    int const_value2 = value2->get_value();
    switch (op) {
    case Instruction::add:
        return ConstantInt::get(const_value1 + const_value2, module_);
        break;
    case Instruction::sub:
        return ConstantInt::get(const_value1 - const_value2, module_);
        break;
    case Instruction::mul:
        return ConstantInt::get(const_value1 * const_value2, module_);
        break;
    case Instruction::sdiv:
        return ConstantInt::get((int)(const_value1 / const_value2), module_);
        break;
    case Instruction::srem:
        return ConstantInt::get(const_value1 % const_value2, module_);
        break;
    default:
        return nullptr;
        break;
    }
}

ConstantFloat *ConstFolder::compute_float(Instruction::OpID op, ConstantFloat *value1, ConstantFloat *value2) {
    float const_value1 = value1->get_value();
    float const_value2 = value2->get_value();
    switch (op) {
    case Instruction::fadd:
        return ConstantFloat::get(const_value1 + const_value2, module_);
        break;
    case Instruction::fsub:
        return ConstantFloat::get(const_value1 - const_value2, module_);
        break;
    case Instruction::fmul:
        return ConstantFloat::get(const_value1 * const_value2, module_);
        break;
    case Instruction::fdiv:
        return ConstantFloat::get((const_value1 / const_value2), module_);
        break;
    default:
        return nullptr;
        break;
    }
}

ConstantInt *ConstFolder::compute_zext(ConstantInt *value1) {
    int const_value1 = value1->get_value();
    return ConstantInt::get(const_value1, module_);
}

ConstantFloat *ConstFolder::compute_sitofp(ConstantInt *value1) {
    float const_value1 = value1->get_value();
    return ConstantFloat::get(const_value1, module_);
}

ConstantInt *ConstFolder::compute_fptosi(ConstantFloat *value1) {
    int const_value1 = value1->get_value();
    return ConstantInt::get(const_value1, module_);
}

//把条件语句也判断掉
//在外面判断是条件语句的时候才准进入, 然后直接返回值
ConstantInt *ConstFolder::compute_cmp(CmpInst::CmpOp op, ConstantInt *value1, ConstantInt *value2) {
    int const_value1 = value1->get_value();
    int const_value2 = value2->get_value();
    switch (op) {
    case CmpInst::EQ:
        return ConstantInt::get(const_value1 == const_value2, module_);
        break;
    case CmpInst::NE:
        return ConstantInt::get(const_value1 != const_value2, module_);
        break;
    case CmpInst::GT:
        return ConstantInt::get(const_value1 > const_value2, module_);
        break;
    case CmpInst::GE:
        return ConstantInt::get(const_value1 >= const_value2, module_);
        break;
    case CmpInst::LT:
        return ConstantInt::get(const_value1 < const_value2, module_);
        break;
    case CmpInst::LE:
        return ConstantInt::get(const_value1 <= const_value2, module_);
        break;
    default:
        return nullptr;
        break;
    }
}

ConstantInt *ConstFolder::compute_fcmp(FCmpInst::CmpOp op, ConstantFloat *value1, ConstantFloat *value2) {
    float const_value1 = value1->get_value();
    float const_value2 = value2->get_value();
    switch (op) {
    case FCmpInst::EQ:
        return ConstantInt::get(const_value1 == const_value2, module_);
        break;
    case FCmpInst::NE:
        return ConstantInt::get(const_value1 != const_value2, module_);
        break;
    case FCmpInst::GT:
        return ConstantInt::get(const_value1 > const_value2, module_);
        break;
    case FCmpInst::GE:
        return ConstantInt::get(const_value1 >= const_value2, module_);
        break;
    case FCmpInst::LT:
        return ConstantInt::get(const_value1 < const_value2, module_);
        break;
    case FCmpInst::LE:
        return ConstantInt::get(const_value1 <= const_value2, module_);
        break;
    default:
        return nullptr;
        break;
    }
}

// 用来判断value是否为ConstantInt，如果不是则会返回nullptr
ConstantInt *cast_to_const_int(Value *value) {
    auto const_int_ptr = dynamic_cast<ConstantInt *>(value);
    if (const_int_ptr) {
        return const_int_ptr;
    } else {
        return nullptr;
    }
}

ConstantFloat *cast_to_const_float(Value *value) {
    auto const_float_ptr = dynamic_cast<ConstantFloat *>(value);
    if (const_float_ptr) {
        return const_float_ptr;
    } else {
        return nullptr;
    }
}

void ConstPropagation::IntraBlockVarCompute() {
    for(auto bb : func_->get_basic_blocks()){
        //全局变量仅更换每个块内的写后读
        std::map<GlobalVariable *, Constant *> bb_global_const_map;

        auto bb_inst_list = bb->get_instructions();
        //对本块内所有指令作常量折叠
        for(auto inst : bb_inst_list){
            //判断语句是否能进行折叠,如果能,就折叠并且把代码加入待删除区
            if (inst->get_num_operand() == 1){
                switch(inst->get_instr_type()){
                    case Instruction::zext:{
                        auto u_op = cast_to_const_int(inst->get_operand(0));

                        if(u_op == nullptr)
                            break;

                        auto u_const = const_folder->compute_zext(u_op);
                        DeleteSet[bb].insert(inst);
                        inst->replace_all_use_with(u_const);
                        break;
                    }
                    case Instruction::fptosi:{
                        auto u_op = cast_to_const_float(inst->get_operand(0));

                        if(u_op == nullptr)
                            break;

                        auto u_const = const_folder->compute_fptosi(u_op);
                        DeleteSet[bb].insert(inst);
                        inst->replace_all_use_with(u_const);
                        break;
                    }
                    case Instruction::sitofp:{
                        auto u_op = cast_to_const_int(inst->get_operand(0));

                        if(u_op == nullptr)
                            break;

                        auto u_const = const_folder->compute_sitofp(u_op);
                        DeleteSet[bb].insert(inst);
                        inst->replace_all_use_with(u_const);
                        break;
                    }
                    case Instruction::load:{//load只考虑对全局变量的处理
                        auto global_var = dynamic_cast<GlobalVariable *>(inst->get_operand(0));
                        if (global_var != nullptr){
                            auto global_iter = bb_global_const_map.find(global_var);

                            if (global_iter != bb_global_const_map.end()){
                                inst->replace_all_use_with(global_iter->second);
                                DeleteSet[bb].insert(inst);
                            }
                        }
                    }
                    default:{
                        break;
                    }
                }
            }
            else if (inst->get_num_operand() == 2){

                bool binary_int_const = false;
                bool binary_float_const = false;
                bool store_global = false;
                
                auto int_op1 = cast_to_const_int(inst->get_operand(0));
                auto int_op2 = cast_to_const_int(inst->get_operand(1));
                if (int_op1 != nullptr && int_op2 != nullptr){
                    binary_int_const = true;
                }

                auto float_op1 = cast_to_const_float(inst->get_operand(0));
                auto float_op2 = cast_to_const_float(inst->get_operand(1));
                if (float_op1 != nullptr && float_op2 != nullptr){
                    binary_float_const = true;
                }

                auto global_int_op = cast_to_const_int(inst->get_operand(0));
                auto global_float_op = cast_to_const_float(inst->get_operand(0));
                auto global_op = dynamic_cast<GlobalVariable *>(inst->get_operand(1));
                if (inst->is_store() && global_op != nullptr){
                    store_global = true;
                }
                
                if (binary_int_const){
                    ConstantInt *const_int;

                    if (inst->is_cmp()){    //是不是条件判断
                        auto cmp_inst = dynamic_cast<CmpInst *>(inst);
                        const_int = const_folder->compute_cmp(cmp_inst->get_cmp_op(), int_op1, int_op2);
                    }
                    else{
                        const_int = const_folder->compute(inst->get_instr_type(), int_op1, int_op2);
                    }

                    if(const_int){
                        DeleteSet[bb].insert(inst);
                        inst->replace_all_use_with(const_int);
                    }
                }
                else if(binary_float_const){

                    if (inst->is_fcmp()){    //是不是浮点条件判断
                        auto fcmp_inst = dynamic_cast<FCmpInst *>(inst);
                        auto const_fcmp_bool = const_folder->compute_fcmp(fcmp_inst->get_cmp_op(), float_op1, float_op2);
                        if(const_fcmp_bool){
                            DeleteSet[bb].insert(inst);
                            inst->replace_all_use_with(const_fcmp_bool);
                        }
                    }
                    else{
                        auto const_float = const_folder->compute_float(inst->get_instr_type(), float_op1, float_op2);
                        if(const_float){
                            DeleteSet[bb].insert(inst);
                            inst->replace_all_use_with(const_float);
                        }
                    }
                    
                }
                else if(store_global){
                    if (global_int_op != nullptr)
                        bb_global_const_map[global_op] = global_int_op;
                    else if (global_float_op != nullptr)
                        bb_global_const_map[global_op] = global_float_op;
                    else    //此时的全局变量已经不是INT或者FLOAT了
                        bb_global_const_map.erase(global_op);
                    //不可以删store指令,这是会修改内存空间的.
                }
            }
        }
    }

    return ;
}

void ConstPropagation::DeleteConstCondBr(){
    for(auto bb : func_->get_basic_blocks()){
        auto inst_br = bb->get_terminator();
        
        //非跳转结尾或返回值结尾,不做处理
        if (inst_br == nullptr)
            continue;
        if (inst_br->is_ret())
            continue;

        //尝试转化成条件跳转,若不是条件跳转就也不处理
        auto inst_cond_br = dynamic_cast<BranchInst *>(inst_br);
        if (inst_cond_br == nullptr)
            continue;

        // 第一个参数是条件(Value类没有get_value,需要转化)
        auto cond = dynamic_cast<ConstantInt *>(inst_cond_br->get_operand(0));
        //如果不是常量,也不处理
        if (cond == nullptr)
            continue;
        auto cond_value = cond->get_value();

        BasicBlock * wait_remove_block;
        BasicBlock * not_remove_block;

        if (cond_value == 1){
            bb->delete_instr(inst_cond_br); //把条件跳转的删掉,换成硬跳!
            not_remove_block = dynamic_cast<BasicBlock *>(inst_cond_br->get_operand(1));
            wait_remove_block = dynamic_cast<BasicBlock *>(inst_cond_br->get_operand(2));
        }
        else{
            bb->delete_instr(inst_cond_br); //把条件跳转的删掉,换成硬跳!
            wait_remove_block = dynamic_cast<BasicBlock *>(inst_cond_br->get_operand(1));
            not_remove_block = dynamic_cast<BasicBlock *>(inst_cond_br->get_operand(2));
        }
    
        //处理后缀和后缀的前缀
        wait_remove_block->remove_pre_basic_block(bb);
        //!!!要先把bb的所有后继全删掉,最后再给一个br,不然的话,会有重复(br似乎会插一个点进去)
        //(其实应该也就是两个后缀,以防万一用个for循环)
        for (auto back_bb : bb->get_succ_basic_blocks()){
            back_bb->remove_pre_basic_block(bb);
        }
        BranchInst::create_br(not_remove_block, bb);
        bb->remove_succ_basic_block(wait_remove_block);

        // phi可以不止2对!!!!phi的对数是偶数,但是可以很多很多对!!!!
        auto pre_bb_list = wait_remove_block->get_pre_basic_blocks();
        for (auto back_inst_phi : wait_remove_block->get_instructions()) {
            if (!back_inst_phi->is_phi())
                continue;
        
            for (unsigned int i = 0; i < back_inst_phi->get_num_operand(); i += 2){//每次跳跃一个对
                // 如果已经不在前缀里面了, 就踢出去
                bool remove_op = true;
                for (auto cur_bb : pre_bb_list){
                    if (cur_bb == back_inst_phi->get_operand(i + 1)){
                        remove_op = false;
                    }
                }
                if (remove_op)
                    back_inst_phi->remove_operands(i, i + 1);
            }

            if (back_inst_phi->get_num_operand() == 0)//保险起见添加phi操作数为0的情况
                DeleteSet[wait_remove_block].insert(back_inst_phi);
            else if (back_inst_phi->get_num_operand() == 2){//phi操作数是2的时候说明只有一种可能了，全部替换即可
                back_inst_phi->replace_all_use_with(back_inst_phi->get_operand(0));
                DeleteSet[wait_remove_block].insert(back_inst_phi);
            }
        }
    }
    return ;
}

void ConstPropagation::DeleteConstInst() {
    for(auto bb : func_->get_basic_blocks()){
        for(auto inst : DeleteSet[bb]){
            bb->delete_instr(inst);
        }
    }
    return ;
}