#include "LIR.h"

void LIR::execute() {
    for (auto func : module->get_functions()){
        if (func->get_num_basic_blocks() > 0) {
            for (auto bb : func->get_basic_blocks()) {
                split_srem(bb);
            }
            for (auto bb : func->get_basic_blocks()) {
                merge_cmp_br(bb);
            }
        }
    }
}

void LIR::merge_cmp_br(BasicBlock* bb) {
    auto terminator = bb->get_terminator();
    if (terminator->is_br()) {
        auto br = dynamic_cast<BranchInst *>(terminator);
        if (br->is_cond_br()){
            auto inst = dynamic_cast<Instruction *>(br->get_operand(0));
            if (inst && inst->is_cmp()) {
                auto br_operands = br->get_operands();
                auto inst_cmp = dynamic_cast<CmpInst *>(inst);
                auto cmp_ops = inst_cmp->get_operands();
                auto cmp_op = inst_cmp->get_cmp_op();
                auto true_bb = dynamic_cast<BasicBlock* >(br_operands[1]);
                auto false_bb = dynamic_cast<BasicBlock* >(br_operands[2]);
                true_bb->remove_pre_basic_block(bb);
                false_bb->remove_pre_basic_block(bb);
                bb->remove_succ_basic_block(true_bb);
                bb->remove_succ_basic_block(false_bb);
                auto cmp_br = CmpBrInst::create_cmpbr(cmp_op,cmp_ops[0],cmp_ops[1],
                                                    true_bb,false_bb,
                                                    bb,module);
                bb->delete_instr(br);
            }
        }
    }
}

void LIR::split_srem(BasicBlock* bb) {
    auto &instructions = bb->get_instructions();
    for (auto iter = instructions.begin();iter != instructions.end();iter++) {
        auto inst_rem = *iter;
        if (inst_rem->is_rem()) {
            auto op1 = inst_rem->get_operand(0);
            auto op2 = inst_rem->get_operand(1);
            if (op1 == op2) continue;
            if (dynamic_cast<ConstantInt*>(op2) && dynamic_cast<ConstantInt*>(op2)->get_value() == 1) continue;
            auto inst_div = BinaryInst::create_sdiv(op1,op2,bb,module);
            instructions.pop_back();
            auto inst_mul = BinaryInst::create_mul(inst_div,op2,bb,module);
            instructions.pop_back();
            auto inst_sub = BinaryInst::create_sub(op1,inst_mul,bb,module);
            instructions.pop_back();
            bb->add_instruction(iter,inst_div);
            bb->add_instruction(iter,inst_mul);
            bb->add_instruction(iter,inst_sub);
            inst_rem->replace_all_use_with(inst_sub);
            iter--;
            bb->delete_instr(inst_rem);
        }
    }
}
