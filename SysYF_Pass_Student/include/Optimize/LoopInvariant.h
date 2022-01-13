#ifndef MHSJ_LOOPINVARIANT_H
#define MHSJ_LOOPINVARIANT_H

#include "Module.h"
#include "Pass.h"
#include <vector>
#include <map>
#include <stack>
#include <set>
#include <memory>

struct LoopRecord {//自然循环的结构
    BasicBlock* BrBlock;
    BasicBlock* ToBlock;
    std::set<BasicBlock*>Loop;
};

class LoopInvariant : public Pass
{
public:
    explicit LoopInvariant(Module* module): Pass(module){}
    void execute() final;
    const std::string get_name() const override {return name;}
    void FindLoops();
    void FindLoopInvariantIns(LoopRecord* loop);//based on NaturalLoop
    void Hoist();
private:
    std::string name = "LoopInvariant";
    Function* CurrentFunction;
    BasicBlock* EntryBlock;
    std::set<LoopRecord*>AllLoops;
    std::set<Value*>Invariants;
};




#endif