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
  br label %label_ret
label_ret:                                                ; preds = %label_entry
  ret i32 29
}
