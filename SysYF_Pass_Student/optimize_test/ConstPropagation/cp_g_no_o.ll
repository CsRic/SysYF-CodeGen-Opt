@a = global i32 zeroinitializer
@b = global i32 zeroinitializer
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
  store i32 10, i32* @a
  store i32 5, i32* @b
  %op2 = load i32, i32* @a
  %op3 = mul i32 %op2, 2
  %op4 = load i32, i32* @b
  %op5 = sitofp i32 %op4 to float
  %op6 = fmul float %op5, 0x3ff19999a0000000
  %op7 = sitofp i32 %op3 to float
  %op8 = fadd float %op7, %op6
  %op9 = fadd float %op8, 0x400cccccc0000000
  %op10 = fptosi float %op9 to i32
  br label %label_ret
label_ret:                                                ; preds = %label_entry
  ret i32 %op10
}
