.global main
.text

/*
long fib(long x){
    if(x == 0) 
        return 0;
    if(x == 1)  
        return 1;
    return fib(x - 1) + fib(x-2);
}

x= %r12
final result return = %r13 = %rax

*/

fib:
    enter $0, $0

    cmp $0, %rdi
    je fib0

    cmp $1, %rdi
    je fib1

    push %r12
    push %r13
    // r12= x, set r13=0
    mov %rdi, %r12
    mov $0, %r13

    // calculate fib(x-1)
    dec %r12
    mov %r12, %rdi
    call fib
    add %rax, %r13

    // calculate fib(x-2)
    //dec only once as (x-1)-1
    dec %r12
    mov %r12, %rdi
    call fib
    add %rax, %r13

    mov %r13, %rax

    pop %r13
    pop %r12

    jmp fib_end

fib0:
    mov $0, %rax
    jmp fib_end

fib1:
    mov $1, %rax
    jmp fib_end

fib_end:
    leave 
    ret

/*
int
main(int argc, char* argv[])
{
    if (argc != 2 || atol(argv[1]) < 0) {
        printf("Usage:\n  %s N, where N > 0\n", argv[0]);
        return 1;
    }

    printf("fib(%ld) = %ld", atol(argv[1]), fib(atol(argv[1])));
    return 0;
}
*/

main:
    enter $0, $0

    cmp $2, %rdi
    jne error

    //atol(argv[1])
    mov 8(%rsi), %rdi
    call atol
    cmp $0, %rax
    jl error
    
    mov %rax, %r12
    mov %r12, %rdi
    call fib

    mov $fib_result, %rdi
    mov %r12, %rsi
    mov %rax, %rdx
    mov $0, %al
    call printf
    mov $0, %rax
    jmp main_end

error:
    mov $error_msg, %rdi
    call puts
    mov $1, %rax
    jmp main_end

main_end:
    leave
    ret

.data
error_msg: .string "Usage:\n  %s N, where N > 0"
fib_result: .string "fib(%ld) = %ld\n"