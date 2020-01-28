.global main
.data
bad_input: .string "bad input"

.text
main:
  enter $0, $0

  mov $bad_input, %rdi
  call puts

  leave
  ret

