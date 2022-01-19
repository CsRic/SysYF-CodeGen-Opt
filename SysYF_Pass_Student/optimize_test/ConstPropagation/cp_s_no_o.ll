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
  %op9 = mul i32 10, 5
  %op11 = fadd float 0x3ff4000000000000, 0x3ff3333340000000
  %op14 = sitofp i32 %op9 to float
  %op15 = fmul float %op11, %op14
  %op17 = sitofp i32 10 to float
  %op18 = fadd float %op15, %op17
  %op19 = fptosi float %op18 to i32
  %op21 = srem i32 %op9, 3
  %op24 = sitofp i32 5 to float
  %op25 = fmul float %op24, 0x3ff3333340000000
  %op26 = sitofp i32 %op21 to float
  %op27 = fadd float %op26, %op25
  %op30 = fdiv float %op11, %op15
  %op32 = sitofp i32 %op19 to float
  %op33 = fmul float %op30, %op32
  %op34 = fadd float %op27, %op33
  %op35 = fptosi float %op34 to i32
  br label %label_ret
label_ret:                                                ; preds = %label_entry
  ret i32 %op35
}
