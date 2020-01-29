.global main
.data
bad_input: .string "bad input"
usage_msg:   .string "usage: ./sum_array N"
scan_fmt:    .string "%ld"
sum_fmt:  .string "number is: %ld\n"

.text
main:
  enter $0, $0

  mov $scan_fmt, %rdi
  lea (%r13, %r14, 8), %rsi
  mov $0, %al
  call scanf

  mov $sum_fmt, %rdi
  mov %rax, %rsi
  mov $0, %al
  call printf

  /*
  mov $bad_input, %rdi
  call puts
  */

  leave
  ret

