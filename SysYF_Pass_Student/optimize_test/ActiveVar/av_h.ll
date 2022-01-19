declare i32 @get_int()

declare float @get_float()

declare i32 @get_char()

declare i32 @get_int_array(i32*)

declare i32 @get_float_array(float*)

declare void @put_int(i32)

declare void @put_float(float)

declare void @put_char(i32)

declare void @put_int_array(i32, i32*)

declare void @put_float_array(i32, float*)

define i32 @deepWhileBr(i32 %arg0, i32 %arg1) {
label_entry:
  %op8 = add i32 %arg0, %arg1
  br label %label12
label_ret:                                                ; preds = %label22
  ret i32 %op46
label12:                                                ; preds = %label_entry, %label32
  %op44 = phi i32 [ %op47, %label32 ], [ undef, %label_entry ]
  %op45 = phi i32 [ 42, %label32 ], [ undef, %label_entry ]
  %op46 = phi i32 [ %op8, %label_entry ], [ %op48, %label32 ]
  %op14 = icmp slt i32 %op46, 75
  %op15 = zext i1 %op14 to i32
  %op16 = icmp ne i32 %op15, 0
  br i1 %op16, label %label17, label %label22
label17:                                                ; preds = %label12
  %op19 = icmp slt i32 %op46, 100
  %op20 = zext i1 %op19 to i32
  %op21 = icmp ne i32 %op20, 0
  br i1 %op21, label %label24, label %label32
label22:                                                ; preds = %label12
  br label %label_ret
label24:                                                ; preds = %label17
  %op27 = add i32 %op46, 42
  %op29 = icmp sgt i32 %op27, 99
  %op30 = zext i1 %op29 to i32
  %op31 = icmp ne i32 %op30, 0
  br i1 %op31, label %label33, label %label39
label32:                                                ; preds = %label17, %label39
  %op47 = phi i32 [ %op44, %label17 ], [ %op49, %label39 ]
  %op48 = phi i32 [ %op46, %label17 ], [ %op50, %label39 ]
  br label %label12
label33:                                                ; preds = %label24
  %op35 = mul i32 42, 2
  %op36 = icmp eq i32 1, 1
  %op37 = zext i1 %op36 to i32
  %op38 = icmp ne i32 %op37, 0
  br i1 %op38, label %label40, label %label43
label39:                                                ; preds = %label24, %label43
  %op49 = phi i32 [ %op44, %label24 ], [ %op35, %label43 ]
  %op50 = phi i32 [ %op27, %label24 ], [ %op51, %label43 ]
  br label %label32
label40:                                                ; preds = %label33
  %op42 = mul i32 %op35, 2
  br label %label43
label43:                                                ; preds = %label33, %label40
  %op51 = phi i32 [ %op27, %label33 ], [ %op42, %label40 ]
  br label %label39
}
define i32 @main() {
label_entry:
  %op1 = call i32 @deepWhileBr(i32 2, i32 2)
  br label %label_ret
label_ret:                                                ; preds = %label_entry
  ret i32 %op1
}
