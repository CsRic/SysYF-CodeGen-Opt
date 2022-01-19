#include "LoopInvariant.h"
#include "DominateTree.h"
#include <algorithm>

void LoopInvariant::execute() {
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
    if (bb == CurrentFunction->get_entry_block())
      continue;
    auto dominator = bb; //只需考察bb是否有到其支配链上的回边
    do {
      dominator = dominator->get_idom();
      for (auto succ : bb->get_succ_basic_blocks()) {
        if (succ == dominator) { //找到一条回边, 开始记录一个自然循环
          LoopRecord *loop_record = new LoopRecord;
          loop_record->Loop.clear();
          loop_record->BrBlock = bb;
          loop_record->ToBlock = dominator;
          loop_record->Loop.insert(bb);
          loop_record->Loop.insert(dominator);
          std::vector<BasicBlock *> stack;
          stack.push_back(bb);
          while (!stack.empty()) {
            auto m = stack.back();
            stack.pop_back();
            for (auto pred : m->get_pre_basic_blocks()) {
              if (loop_record->Loop.find(pred) ==
                  loop_record->Loop.end()) { //没找到
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

void LoopInvariant::FindLoopInvariantIns(LoopRecord *loop) {
  Invariants.clear();
  std::set<Value *> define_list; //用以判断指令使用的参数是否定义在循环之外
  for (auto bb : loop->Loop) {
    for (auto ins : bb->get_instructions()) {
      if (ins->is_store()) {
        define_list.insert(ins->get_operand(1));
      } else {
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
        if (ins->is_alloca() || ins->is_ret() || ins->is_br() ||
            ins->is_cmp() || ins->is_phi() || ins->is_call() ||
            ins->is_load()) {
          continue;
        }
        if (define_list.find(ins) == define_list.end()) {
          continue;
        }
        if (Invariants.find(ins) != Invariants.end()) {
          continue;
        }

        for (auto operand : ins->get_operands()) {
          if (dynamic_cast<ConstantFloat *>(operand) ||
              dynamic_cast<ConstantInt *>(operand) ||
              dynamic_cast<ConstantArray *>(operand) ||
              dynamic_cast<ConstantZero *>(operand)) { //常数
            continue;
          }
          if (define_list.find(operand) == define_list.end()) { //定义在循环外
            continue;
          }
          if (Invariants.find(operand) != Invariants.end()) { //已被标记为inv
            continue;
          }
          is_inv = false;
          break;
        }
        if (is_inv) {
          Invariants.insert(ins);
          // std::cout <<"Find invariant " << ins->get_name() << "  in block "<<
          // bb->get_name() <<std::endl;
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
  std::set<LoopRecord *> well_done_loop; //已经保证不需要再重复外提的loop
  for (auto loop : AllLoops) {

    if (well_done_loop.find(loop) != well_done_loop.end())
      continue;
    std::vector<LoopRecord *> stack;
    stack.clear();
    stack.push_back(loop);
    while (!stack.empty()) {

      auto loop1 = stack.back();
      stack.pop_back();
      FindLoopInvariantIns(loop1);
      bool empty = true;
      if (!Invariants.empty()) {
        empty = false;
        std::vector<BasicBlock *> ToLoopBlocks; //所有循环外指向ToBlock的bb
        for (auto bb : loop1->ToBlock->get_pre_basic_blocks()) {
          if (loop1->Loop.find(bb) == loop1->Loop.end()) {
            ToLoopBlocks.push_back(bb);
          }
        }
        //应当是只有一个的，必须只有一个外部的先辈节点……这是不可置疑的原理，嗯！
        auto preBlock = ToLoopBlocks.back();
        auto terminator = preBlock->get_terminator();
        preBlock->get_instructions().pop_back();

        //删除块中的不变量，转移到preBlock中
        for (auto inv : Invariants) {
          for (auto bb : loop->Loop) {
            std::vector<Instruction *> hoist_list;
            for (auto ins : bb->get_instructions()) {
              if (ins == inv) {
                hoist_list.push_back(ins);
              }
            }
            for (auto ins : hoist_list) {
              bb->get_instructions().remove(ins);
              preBlock->add_use(ins);
              preBlock->add_instruction(ins);
              ins->set_parent(preBlock);
            }
          }
        }
        preBlock->add_instruction(terminator);
      }
      for (auto outerloop : AllLoops) {
        if (outerloop == loop1)
          continue;
        if (outerloop->ToBlock != loop1->ToBlock &&
            outerloop->Loop.find(loop1->ToBlock) != outerloop->Loop.end()) {
          //找到外层循环,入栈
          stack.push_back(outerloop);
          if (well_done_loop.find(outerloop) != well_done_loop.end()) {
            well_done_loop.insert(outerloop);
          }
        }
      }
    }
  }
}