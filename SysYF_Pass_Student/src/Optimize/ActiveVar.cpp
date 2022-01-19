#include "ActiveVar.h"
#include <algorithm>

using namespace std;

void ActiveVar::execute() {
    for (auto &func : this->module->get_functions()) {
        if (func->get_basic_blocks().empty()) {
            continue;
        } else {
            func_ = func;

            // 避免受上一个函数的影响
            bb_def_val.clear();
            bb_use_val.clear();
            bb_pre_not_act_val.clear();
            bb_in.clear();
            bb_out.clear();

            //先 生成Use和Def的集合
            UseDefValueGen();
            //再 生成In和Out的集合
            InOutValueGen();
            //最后 展示结果
            // ShowResult();
            // cout << endl;
        }
    }
    return ;
}

std::set<Value *>* ActiveVar::DefValueGet(BasicBlock * block){
    std::set<Value *>* def_value_set = new std::set<Value *>();
    auto bb_inst_list = block->get_instructions();
    for(auto inst : bb_inst_list){
        if(!inst->is_void())//is_void是表示没有左操作数的那些，应该舍弃
            def_value_set->insert(inst);
    }
    return def_value_set;
}

//SSA形式，因此引用势必在定值后！
std::set<Value *>* ActiveVar::UseValueGet(BasicBlock * block, std::set<Value *>* def_value_set){
    std::set<Value *>* use_value_set = new std::set<Value *>();
    auto bb_inst_list = block->get_instructions();

    std::set<Value *> not_phi_but_use_set = {};

    for(auto inst : bb_inst_list){
        //bb_pre_not_act_val保存每一个当前块bb对应的phi字典，phi字典中键为bb的phi中出现的前驱块label，值为该label对应的变量集合
        if(inst->is_phi()){//注意!!!phi后面不一定只有两对数!
            for (unsigned int i = 0; i < inst->get_num_operand(); i += 2){
                auto phi_val = inst->get_operand(i);
                if (phi_val != 0)
                    if (!(dynamic_cast<ConstantFloat*>(phi_val) || dynamic_cast<ConstantInt*>(phi_val) || dynamic_cast<Function*>(phi_val) || dynamic_cast<BasicBlock*>(phi_val)))
                        bb_pre_not_act_val[block][dynamic_cast<BasicBlock*>(inst->get_operand(i + 1))].insert(phi_val);
            }
        }

        for (auto use_val : inst->get_operands()) {
            //不是phi指令时,就把变量加入"不是phi但使用指令"集合.
            if(!inst->is_phi()){
                if (!(dynamic_cast<ConstantFloat*>(use_val) || dynamic_cast<ConstantInt*>(use_val) || dynamic_cast<Function*>(use_val) || dynamic_cast<BasicBlock*>(use_val)))
                    not_phi_but_use_set.insert(use_val);
            }

            //！！！有特殊情况！！！右值是常数和函数时不应该被看做是使用变量！phi会把label也当一种操作数！
            //即操作数是：1、常整型，2、常浮点，3、函数类型，4、标签类型时不应该被考虑
            //全局变量是应该被考虑的
            if (dynamic_cast<ConstantFloat*>(use_val) || dynamic_cast<ConstantInt*>(use_val) || dynamic_cast<Function*>(use_val) || dynamic_cast<BasicBlock*>(use_val))
                continue;
            //不是常数和函数和标签时，判断在不在def里面，若在，则跳过
            if (def_value_set->find(use_val) != def_value_set->end())
                continue;
            //否则加入
            use_value_set->insert(use_val);
            //cout << "var:" << use_val->get_name() << endl;
        }
    }

    // 删除还在phi外面出现过的变量
    std::map<BasicBlock *, std::set<Value *>>::reverse_iterator   iter;
    for(iter = bb_pre_not_act_val[block].rbegin(); iter != bb_pre_not_act_val[block].rend(); iter++){
        std::set<Value *> wait_del = {};
        
        for(auto val : iter->second){
            if(not_phi_but_use_set.find(val) != not_phi_but_use_set.end())
                wait_del.insert(val);
        }
        
        set<Value *>::iterator pos;
        for(pos = wait_del.begin(); pos != wait_del.end();pos++){
		    iter->second.erase(*pos);
	    }
    }

    // 输出phi中搜集的变量
    // std::map<BasicBlock *, std::set<Value *>>::reverse_iterator   print_iter;
    // for(print_iter = bb_pre_not_act_val[block].rbegin(); print_iter != bb_pre_not_act_val[block].rend(); print_iter++){
    //     cout << print_iter->first->get_name() << ":" <<endl;
    //     for(auto val : print_iter->second){
    //         cout << val->get_name() << " ";
    //     }
    //     cout << endl;
    // }

    return use_value_set;
}

void ActiveVar::UseDefValueGen(){

    for(auto bb : func_->get_basic_blocks()){
        //测试可知，没有左操作数，指令本身就是一个左值
        //首先算Def，算出来的值保存在类的私有变量中,call有三个参数，第一个是函数名应该舍弃掉
        auto def_value_set = DefValueGet(bb);
        //然后算Use，算出来的值也保存在类的私有变量中
        
        bb_pre_not_act_val.insert({bb, {}});
        auto use_value_set = UseValueGet(bb, def_value_set);
        //添加到类的私有变量中
        bb_def_val.insert({bb, def_value_set});
        bb_use_val.insert({bb, use_value_set});
    }

    return ;
}

void ActiveVar::InOutValueGen(){

    //课本P285
    //1、先初始化
    for (auto bb : func_->get_basic_blocks()){
        bb_in.insert({bb, {}});     //按要求是空集
        bb_out.insert({bb, {}});
    }

    //2、迭代
    bool any_in_change = true;      //任何一个IN发生变化
    while(any_in_change){
        any_in_change = false; //假定都不变
        for(auto bb : func_->get_basic_blocks()){
            //求OUT
            //对每个后继
            for(auto bb_back : bb->get_succ_basic_blocks()){
                // 得到后继S的IN[S]
                std::set<Value *> true_in_set = {};
                set_union(bb_in[bb_back].begin(), bb_in[bb_back].end(), bb_in[bb_back].begin(), bb_in[bb_back].end(),inserter(true_in_set, true_in_set.end()));

                // 对于不是由当前块bb引起的在phi中的活跃变量，就把它剔除
                std::map<BasicBlock *, std::set<Value *>>::reverse_iterator  phi_iter;
                for(phi_iter = bb_pre_not_act_val[bb_back].rbegin(); phi_iter != bb_pre_not_act_val[bb_back].rend(); phi_iter++){
                    if(phi_iter->first != bb){
                        std::set<Value *> temp_true_in_set = {};
                        set_difference(true_in_set.begin(),true_in_set.end(),phi_iter->second.begin(),phi_iter->second.end(),inserter(temp_true_in_set, temp_true_in_set.end()));
                        true_in_set = temp_true_in_set;
                    }
                }

                std::set<Value *> temp_out_set = {};
                std::set_union(bb_out[bb].begin(), bb_out[bb].end(), true_in_set.begin(), true_in_set.end(), std::inserter(temp_out_set, temp_out_set.begin()));
                bb_out[bb] = temp_out_set;
            }

            //求IN
            std::set<Value *> temp_outNdef_set = {};
            set_difference(bb_out[bb].begin(), bb_out[bb].end(), bb_def_val[bb]->begin(), bb_def_val[bb]->end(), std::inserter(temp_outNdef_set, temp_outNdef_set.begin()));
            std::set<Value *> temp_in_set = {};
            std::set_union(bb_use_val[bb]->begin(), bb_use_val[bb]->end(), temp_outNdef_set.begin(), temp_outNdef_set.end(), std::inserter(temp_in_set, temp_in_set.begin()));
            //现在已经得到新的IN了，来看看有没有变化，将两个求并集，看看大小有没有变，要是没变，就可以证明无变化了。
            std::set<Value *> diff_in_set = {};
            std::set_union(bb_in[bb].begin(), bb_in[bb].end(), temp_in_set.begin(), temp_in_set.end(), std::inserter(diff_in_set, diff_in_set.begin()));
            
            if(bb_in[bb].size() != diff_in_set.size())
                any_in_change = true;

            bb_in[bb] = temp_in_set;
        }
    }

    for (auto bb : func_->get_basic_blocks()){
        bb->set_live_in(bb_in[bb]);
        bb->set_live_out(bb_out[bb]);
    }

    return ;
}

void ActiveVar::ShowResult(){
    cout << func_->get_name() << ":" << endl;

    for (auto bb : func_->get_basic_blocks()){

        cout << bb->get_name() << ":" << endl;

        set<Value *>::iterator pos;

        cout << "USE: ";
        for(pos = bb_use_val[bb]->begin(); pos != bb_use_val[bb]->end();pos++){
		    cout << (*pos)->get_name() << " ";
	    }
        cout << endl;

        cout << "DEF: ";
        for(pos = bb_def_val[bb]->begin(); pos != bb_def_val[bb]->end();pos++){
		    cout << (*pos)->get_name() << " ";
	    }
        cout << endl;

        cout << "IN: ";
        for(pos = bb_in[bb].begin(); pos != bb_in[bb].end();pos++){
		    cout << (*pos)->get_name() << " ";
	    }
        cout << endl;

        cout << "OUT: ";
        for(pos = bb_out[bb].begin(); pos != bb_out[bb].end();pos++){
		    cout << (*pos)->get_name() << " ";
	    }
        cout << endl;

        cout << "BACK: ";
        for(auto bb_back : bb->get_succ_basic_blocks()){
            cout << bb_back->get_name() << " ";
        }
        cout << endl;
    }

    return ;
}