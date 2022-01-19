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
  br label %label14
label_ret:                                                ; preds = %label14
  ret i32 %op38
label9:
  br label %label16
label14:                                                ; preds = %label24, %label_entry
  %op36 = phi i32 [ 84, %label24 ], [ undef, %label_entry ]
  %op37 = phi i32 [ 42, %label24 ], [ undef, %label_entry ]
  %op38 = phi i32 [ 90, %label_entry ], [ 168, %label24 ]
  br label %label_ret
label16:                                                ; preds = %label9
  br label %label25
label24:                                                ; preds = %label31
  br label %label14
label25:                                                ; preds = %label16
  br label %label32
label31:                                                ; preds = %label35
  br label %label24
label32:                                                ; preds = %label25
  br label %label35
label35:                                                ; preds = %label32
  br label %label31
}
