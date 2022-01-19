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

define i32 @main() {
label_entry:
  %op3 = icmp slt i32 90, 75
  %op4 = zext i1 %op3 to i32
  %op5 = icmp ne i32 %op4, 0
  br i1 %op5, label %label9, label %label14
label_ret:                                                ; preds = %label14
  ret i32 %op38
label9:                                                ; preds = %label_entry
  %op11 = icmp slt i32 90, 100
  %op12 = zext i1 %op11 to i32
  %op13 = icmp ne i32 %op12, 0
  br i1 %op13, label %label16, label %label24
label14:                                                ; preds = %label_entry, %label24
  %op36 = phi i32 [ %op39, %label24 ], [ undef, %label_entry ]
  %op37 = phi i32 [ 42, %label24 ], [ undef, %label_entry ]
  %op38 = phi i32 [ 90, %label_entry ], [ %op40, %label24 ]
  br label %label_ret
label16:                                                ; preds = %label9
  %op19 = add i32 90, 42
  %op21 = icmp sgt i32 %op19, 99
  %op22 = zext i1 %op21 to i32
  %op23 = icmp ne i32 %op22, 0
  br i1 %op23, label %label25, label %label31
label24:                                                ; preds = %label9, %label31
  %op39 = phi i32 [ %op41, %label31 ], [ undef, %label9 ]
  %op40 = phi i32 [ 90, %label9 ], [ %op42, %label31 ]
  br label %label14
label25:                                                ; preds = %label16
  %op27 = mul i32 42, 2
  %op28 = icmp eq i32 1, 1
  %op29 = zext i1 %op28 to i32
  %op30 = icmp ne i32 %op29, 0
  br i1 %op30, label %label32, label %label35
label31:                                                ; preds = %label16, %label35
  %op41 = phi i32 [ %op27, %label35 ], [ undef, %label16 ]
  %op42 = phi i32 [ %op19, %label16 ], [ %op43, %label35 ]
  br label %label24
label32:                                                ; preds = %label25
  %op34 = mul i32 %op27, 2
  br label %label35
label35:                                                ; preds = %label25, %label32
  %op43 = phi i32 [ %op19, %label25 ], [ %op34, %label32 ]
  br label %label31
}
