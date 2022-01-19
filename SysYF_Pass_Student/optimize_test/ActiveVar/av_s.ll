@a = global i32 zeroinitializer
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
  %op1 = icmp sgt i32 2, 1
  %op2 = zext i1 %op1 to i32
  %op3 = icmp ne i32 %op2, 0
  br i1 %op3, label %label6, label %label7
label_ret:                                                ; preds = %label18
  ret i32 20
label6:                                                ; preds = %label_entry
  store i32 2, i32* @a
  br label %label8
label7:                                                ; preds = %label_entry
  store i32 1, i32* @a
  br label %label8
label8:                                                ; preds = %label6, %label7
  %op9 = load i32, i32* @a
  br label %label10
label10:                                                ; preds = %label8, %label15
  %op19 = phi i32 [ %op9, %label8 ], [ %op17, %label15 ]
  %op12 = icmp slt i32 %op19, 10
  %op13 = zext i1 %op12 to i32
  %op14 = icmp ne i32 %op13, 0
  br i1 %op14, label %label15, label %label18
label15:                                                ; preds = %label10
  %op17 = add i32 %op19, 1
  br label %label10
label18:                                                ; preds = %label10
  br label %label_ret
}
