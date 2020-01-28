.global main
.text

main:
    enter $0, $0

    /*
    if(argc != 4 || (strncmp(argv[2], "+", 8) && strncmp(argv[2], "-", 8) && strncmp(argv[2], "*", 8) && strncmp(argv[2], "/", 8)) ){
        puts("Usage:\n./ccalc N op N");
        return 1;
    }
    */
    /*
    argc= %rdi
    argv= %rsi
    xx= %r13
    argv[2]= %r14
    yy= %r15
    */

    cmp $4, %rdi
    jne error

    mov %rsi, %r14

    //int xx = atoi(argv[1]);
    mov 8(%r14), %rdi
    call atoi
    mov %rax, %r13

    //int yy = atoi(argv[3]);
    mov 24(%r14), %rdi
    call atoi
    mov %rax, %r15

    mov 16(%r14), %rax
    mov (%rax), %al

    cmpb  $'+, %al
    je addition

    cmpb $'-, %al
    je subtraction

    cmpb $'*, %al
    je multiplication

    cmpb $'/, %al
    je division

error:
    mov $error_msg, %rdi
    call puts
    mov $1, %rax
    jmp main_end

addition:
    mov %r15, %rax
    add %r13, %rax
    mov %rax, %r8   
    jmp print_output

subtraction:
    mov %r13, %rax
    sub %r15, %rax
    mov %rax, %r8
    jmp print_output

multiplication:
    mov %r15, %rax
    imul %r13, %rax
    mov %rax, %r8
    jmp print_output

division:
    mov %r13, %rax
    cqo
    idiv %r15
    mov %rax, %r8
    jmp print_output

print_output:
    mov $result, %rdi
    mov %r13, %rsi
    mov 16(%r14), %rdx
    mov %r15, %rcx 
    mov $0, %al
    call printf

main_end:
    leave
    ret

.data
    error_msg: .string "Usage:\n./acalc N op N"
    result: .string "%d %s %d = %d\n"