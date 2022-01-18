# 编译原理H · PW7

## 一、小组成员及分工

彭怡腾：活跃变量分析 · 常量传播

朱一鸣：公共子表达式 · 死代码删除

陈思睿：支配树 · 循环不变式外提

## 二、必做部分报告

### 2、 必做-Part2：活跃变量分析

#### 1、算法介绍：

​		按照我们实现的流程，我们先介绍基础的活跃变量分析，即不考虑phi指令时的活跃变量分析算法和实现，然后在此基础上，我们介绍考虑SSA的phi指令时修改后的活跃变量分析算法及其实现。

**不考虑phi的基础活跃变量分析**

​		不考虑phi指令时的活跃变量分析较为简单，按照书本算法即可：

```
use_B：块中有引用且在引用前无定值的变量集
def_B：块中有定值的变量集
IN[B] = use_B ∪ (OUT[B] - def_B)
OUT[B] = ∪IN[S] (S是B的后继)
```

然后一直迭代到IN和OUT收敛即可。在具体实现时，由于我们是基于SSA形式的中间代码进行的优化，因此可以确保不会出现形如`i = i + 1`这种形式的代码，即对于一个变量的定值和引用不会出现在同一个语句中，且对于SSA来说，每一次被赋值的变量均不相同，因此我们绝不会看到一个变量的引用在定值之前。

​		于是基于以上认识，在实现的时候我们可以首先遍历整个块，获取定值的变量集合，然后再次遍历这个块，获取引用的变量集合，如果某一变量也出现在了定值集合中，则不将其加入引用集合。

​		拥有每个块的use集合和def集合后，我们便可以计算每个块的IN和OUT集合。首先计算OUT集合，将它的后继的IN集合全部利用set类的union方法求并集即可。然后通过已有的OUT，use，def，我们计算每个块的IN集合，使用set类的union方法可求并集，使用set_difference方法可求差集。然后我们将得到的新的IN集合和原来的IN集合做对比（两者求并集后和原来的IN集合比较元素个数），如果发生了变化，就代表还没收敛，于是进行下一次的IN集合和OUT集合的计算，直到所有的IN集合都不再变化。

​		此时得到的IN集合和OUT集合即是每个块入口和出口处的活跃变量集合。最后通过每个块的`set_live_in`和`set_live_out`方法分别设置它们的IN集合和OUT集合即可。

**在基础活跃变量分析的基础上引入phi的考虑**：

​		下面我们开始考虑phi指令，加入phi指令后，活跃变量将发生如下变化：例如对bb3块中的语句：`%0 = phi [%op1, %bb1], [%op2, %bb2]`来说，如果它在别处没有用到op1，op2，且在此块之后的任何代码都没有用到op1，op2。那么对于bb1来说，它不应该看到op2的活跃性，而对bb2来说，它不应该看到op1的活跃性。

​		因此我们所做的修改应该是当OUT[B]汇聚它后继的IN时，不考虑仅由该块其他前驱引起的活跃变量，即前文的OUT的汇聚算法将变为

```
OUT[B] = ∪(IN[S] / {S中仅由非B前驱而导致活跃的变量}) (S是B的后继)
```

​		基于此改动，我们对原来的实现过程做出如下改动：首先，我们在建立每个块的use集合时同时还建立not_phi_but_use_set（除phi以外的指令引用变量集合）以及bb_pre_not_act_val，该变量是一个字典，键为当前的bb块，值为另一个字典，字典的键为当前bb块中phi语句所使用的前驱label，字典的值为该label导致的引用变量集合。在遍历完该bb块的全部指令后，如前所述，我们将除了被phi引用，还被其他语句引用的变量从bb_pre_not_act_val中剔除。

​		下面我们依托于前两个新增的变量修改求OUT的实现：在遍历当前bb块的每一个后继时，我们不再是简单的将它们的IN集合全部并在一起，而是遍历每个后继块S的bb_pre_not_act_val[S]字典，对于键不为当前块bb的键值对，从IN[S]中将其值对应的所有变量剔除，即只保留下当前bb块会激活的变量在IN中。再将这些IN汇聚即可得到OUT。此时的OUT，即为我们前面所考虑的，增加了phi指令后所应得到的OUT。

#### 2、测试情况

下面给出一组测试情况：

对于如下函数：

```c
int deepWhileBr(int a, int b) {
  int c;
  c = a + b;
  while (c < 75) {
    int d;
    d = 42;
    if (c < 100) {
      c = c + d;
      if (c > 99) {
        int e;
        e = d * 2;
        if (1 == 1) {
          c = e * 2;
        }
      }
    }
  }
  return (c);
}
```

流图如下：

![](./img/必做2_流图.svg)

计算得到的各基本块use，def，IN，OUT集合如下：

```
deepWhileBr:
label_entry:
USE: arg0 arg1 
DEF: op8 
IN: arg0 arg1 
OUT: op8 
label_ret:
USE: op46 
DEF: 
IN: op46 
OUT: 
label12:
USE: op8 op48 op47 
DEF: op14 op15 op16 op46 op45 op44 
IN: op8 op48 op47 
OUT: op46 op44 
label17:
USE: op46 
DEF: op19 op20 op21 
IN: op46 op44 
OUT: op46 op44 
label22:
USE: 
DEF: 
IN: op46 
OUT: op46 
label24:
USE: op46 
DEF: op27 op29 op30 op31 
IN: op46 op44 
OUT: op27 op44 
label32:
USE: op46 op50 op49 op44 
DEF: op48 op47 
IN: op46 op50 op49 op44 
OUT: op48 op47 
label33:
USE: 
DEF: op35 op36 op37 op38 
IN: op27 
OUT: op27 op35 
label39:
USE: op27 op35 op51 op44 
DEF: op50 op49 
IN: op27 op35 op51 op44 
OUT: op50 op49 
label40:
USE: op35 
DEF: op42 
IN: op35 
OUT: op35 op42 
label43:
USE: op27 op42 
DEF: op51 
IN: op27 op35 op42 
OUT: op35 op51 
```

经过手动验算，结果准确。

## 三、选做部分报告

### 2、 选做-Part2：常量传播

#### 1、算法介绍

​		按照我们实现的流程，我们首先介绍基于SSA的过程内常量传播算法及其实现，然后介绍删除无用分支的实现。

**基础的常量传播**：

​		由于我们的优化基于SSA进行，因此一个变量在被引用前一定先被定值（除了phi语句相关的可能有undef的情况出现），于是我们可以顺次遍历每一个基本块，并且建立常量表，即我们判断每一条语句是否能进行常量折叠，若可以，则进行常量折叠后将此语句所欲赋值的变量加入常量表中，并在之后再遇到常量表中的变量时，用常量表所对应的常量进行替换。

​		这一基础的常量传播结束后，会将所有的常量算术型语句和常量条件判断进行折叠，但会遗留下一些恒定的条件跳转语句，按照实验要求，应将这些语句转化为硬跳转，方便后面死代码删除时进行清理。

**删除无用分支**：

​		对于这一工作，我们的思路是，首先找到所有条件是常量的跳转语句，然后将其变为硬跳转，对于不再能跳转到的那个块：remove_bb，我们把它从当前块bb的后继删除，并把当前块从它的前驱删除，最后我们再处理remove_bb中所有的phi语句，把所有涉及到bb的phi语句中bb的部分进行删除，如果遗留下的phi操作数为0，则直接将其删除，如果遗留下的phi操作数为2，则将所有使用其左值的地方换成其右边的第一个操作数，并把该phi语句删除（相当于进行了一次复写传播）。

​		具体实现基本如上所述，唯一一个细节是需要先将bb从其所有后继块的前驱中删除，然后再插入硬跳转语句，否则会有冗余的标签出现。

#### 2、测试情况

测试样例如下所示：

```
int deepWhileBr(int a, int b) {
  int c;
  c = 10 + 80;
  if (c < 75) {
    int d;
    d = 42;
    if (c < 100) {
      c = c + d;
      if (c > 99) {
        int e;
        e = d * 2;
        if (1 == 1) {
          c = e * 2;
        }
      }
    }
  }
  return (c);
}
```

不进行常量传播时，生成的中间代码如下所示：

```
define i32 @deepWhileBr(i32 %arg0, i32 %arg1) {
label_entry:
  %op7 = icmp slt i32 90, 75
  %op8 = zext i1 %op7 to i32
  %op9 = icmp ne i32 %op8, 0
  br i1 %op9, label %label13, label %label18
label_ret:                                                ; preds = %label18
  ret i32 %op42
label13:                                                ; preds = %label_entry
  %op15 = icmp slt i32 90, 100
  %op16 = zext i1 %op15 to i32
  %op17 = icmp ne i32 %op16, 0
  br i1 %op17, label %label20, label %label28
label18:                                                ; preds = %label_entry, %label28
  %op40 = phi i32 [ %op43, %label28 ], [ undef, %label_entry ]
  %op41 = phi i32 [ 42, %label28 ], [ undef, %label_entry ]
  %op42 = phi i32 [ 90, %label_entry ], [ %op44, %label28 ]
  br label %label_ret
label20:                                                ; preds = %label13
  %op23 = add i32 90, 42
  %op25 = icmp sgt i32 %op23, 99
  %op26 = zext i1 %op25 to i32
  %op27 = icmp ne i32 %op26, 0
  br i1 %op27, label %label29, label %label35
label28:                                                ; preds = %label13, %label35
  %op43 = phi i32 [ %op45, %label35 ], [ undef, %label13 ]
  %op44 = phi i32 [ 90, %label13 ], [ %op46, %label35 ]
  br label %label18
label29:                                                ; preds = %label20
  %op31 = mul i32 42, 2
  %op32 = icmp eq i32 1, 1
  %op33 = zext i1 %op32 to i32
  %op34 = icmp ne i32 %op33, 0
  br i1 %op34, label %label36, label %label39
label35:                                                ; preds = %label20, %label39
  %op45 = phi i32 [ %op31, %label39 ], [ undef, %label20 ]
  %op46 = phi i32 [ %op23, %label20 ], [ %op47, %label39 ]
  br label %label28
label36:                                                ; preds = %label29
  %op38 = mul i32 %op31, 2
  br label %label39
label39:                                                ; preds = %label29, %label36
  %op47 = phi i32 [ %op23, %label29 ], [ %op38, %label36 ]
  br label %label35
}
```

进行了常量传播后的中间代码如下所示：

```
define i32 @deepWhileBr(i32 %arg0, i32 %arg1) {
label_entry:
  br label %label18
label_ret:                                                ; preds = %label18
  ret i32 %op42
label13:
  br label %label20
label18:                                                ; preds = %label28, %label_entry
  %op40 = phi i32 [ 84, %label28 ], [ undef, %label_entry ]
  %op41 = phi i32 [ 42, %label28 ], [ undef, %label_entry ]
  %op42 = phi i32 [ 90, %label_entry ], [ 168, %label28 ]
  br label %label_ret
label20:                                                ; preds = %label13
  br label %label29
label28:                                                ; preds = %label35
  br label %label18
label29:                                                ; preds = %label20
  br label %label36
label35:                                                ; preds = %label39
  br label %label28
label36:                                                ; preds = %label29
  br label %label39
label39:                                                ; preds = %label36
  br label %label35
}
```

可以看到，我们的常量传播起到了一定的效果。
