.global main
.data
bad_input: .string "bad input"
int_fmt: .string "%ld"
new_line: .string "\n"
int_fmt_nl: .string "%ld\n"
int_fmt_space: .string "%ld "

.text
/*
void
read_int(long* ptr)
{
    if (scanf("%ld", ptr) != 1) {
        puts("bad input");
        exit(1);
    }
}
*/
/*
  Variable mapping
  ptr: %r15
*/
read_int:
  push %r15
  enter $8, $0

//scanf("%ld", ptr)
  mov %rdi, %rsi
  mov $int_fmt, %rdi
  mov $0, %al
  call scanf

// check for wrong input
  cmp $1, %rax
  jne read_int_error

read_int_done:
  leave
  pop %r15
  ret

read_int_error:
  //puts("bad input");
  mov $bad_input, %rdi
  call puts

  //exit(1)
  mov $60, %rax
  mov $1, %rdi
  syscall

/*
void
swap(long* xs, long ii, long jj)
{
    if (ii != jj) {
        long tmp = xs[ii];
        xs[ii] = xs[jj];
        xs[jj] = tmp;
    }
}
*/
/*
  Variable mapping
  xs: %rdi
  ii: %rsi
  jj: %rdx
*/
swap:
  enter $0, $0
  cmp %rsi, %rdx
  je swap_done

  //long tmp = xs[ii];
  mov (%rdi, %rsi, 8), %rcx
  mov (%rdi, %rdx, 8), %rax
  mov %rcx, (%rdi, %rdx, 8)
  mov %rax, (%rdi, %rsi, 8) 

swap_done:
  leave
  ret


/*
long
partition(long* xs, long lo, long hi)
{
    long pivot = xs[hi - 1];
    long ii = lo;
    long jj = lo;
    for (; jj < (hi - 1); ++jj) {
        if (xs[jj] < pivot) {
            swap(xs, ii, jj);
            ++ii;
        }
    }
    swap(xs, ii, hi - 1);
    return ii;
}
*/
/*
  Variable Mapping
  xs: %r12
  hi: %r13
  ii: %r14
  lo: %rsi
  pivot: %rbx
  jj: %r15
*/
partition:
  push %r12
  push %r13
  push %r14
  push %r15
  push %rbx
  enter $8, $0

  mov %rdi, %r12
  mov %rdx, %r13
  //(hi - 1)
  dec %r13
  //long pivot = xs[hi - 1];
  mov (%r12, %r13, 8),%rbx
  //long ii = lo;
  mov %rsi, %r14
  //long jj = lo;
  mov %rsi, %r15
  jmp partition_for_loop

partition_for_loop:

  //for (; jj < (hi - 1); ++jj)
  cmp %r15, %r13
  jle partition_done

/*
if (xs[jj] < pivot)
*/
  cmp (%r12, %r15, 8), %rbx
  jg  partition_swap
  jmp partition_inc

partition_swap:
//swap(xs, ii, jj);
//++ii;
mov %r12, %rdi
mov %r14, %rsi
mov %r15, %rdx
call swap
inc %r14
jmp partition_inc

partition_inc:
  inc %r15
  jmp partition_for_loop

partition_done:
  //swap(xs, ii, hi - 1);
  mov %r12, %rdi
  mov %r14, %rsi
  mov %r13, %rdx
  call swap
  //return ii;
  mov %r14, %rax
  leave
  pop %rbx
  pop %r15
  pop %r14
  pop %r13
  pop %r12
  ret

/*
void
quicksort(long* xs, long lo, long hi)
{
    if (hi - lo < 1) {
        return;
    }
    long pp = partition(xs, lo, hi);
    quicksort(xs, lo, pp);
    quicksort(xs, pp + 1, hi);
}
*/
/*
  Variable Mapping
  xs: %r12
  lo: %r13
  hi: %r14
  pp: %r15
*/
quicksort:
  push %r12
  push %r13
  push %r14
  push %r15
  enter $0, $0

  mov %rdi, %r12
  mov %rsi, %r13
  mov %rdx, %r14
  /*
  if ( 1 > hi - lo ) {
        return;
    }
  */
  mov %r14, %rcx
  sub %r13, %rcx
  cmp $1, %rcx
  jle quicksort_done

  //long pp = partition(xs, lo, hi);
  mov %r12, %rdi
  mov %r13, %rsi
  mov %r14, %rdx
  call partition
  mov %rax, %r15

  //quicksort(xs, lo, pp);
  mov %r12, %rdi
  mov %r13, %rsi
  mov %r15, %rdx
  call quicksort
  //quicksort(xs, pp + 1, hi);
  inc %r15
  mov %r12, %rdi
  mov %r15, %rsi
  mov %r14, %rdx
  call quicksort

quicksort_done:
  leave
  pop %r15
  pop %r14
  pop %r13
  pop %r12
  ret




/*
int
main(int _argc, char* _argv[])
{
    long nn;
    read_int(&nn);
    long* xs = malloc(nn * sizeof(long));
    for (long ii = 0; ii < nn; ++ii) {
        read_int(&(xs[ii]));
    }
    quicksort(xs, 0, nn);
    printf("%ld\n", nn);
    for (long ii = 0; ii < nn; ++ii) {
        printf("%ld ", xs[ii]);
    }
    printf("\n");
    free(xs);
    return 0;
}
*/
/*
  Variable Mapping
  argc: %rdi
  nn: 8(%rsp)
  *xs: %r12
  ii: %r15 
*/
main:
  push %r12
  push %r15
  //Initialise space on the stack;  long nn;
  enter $16, $0

  #read_int(&nn);
  lea 8(%rsp), %rdi
  call read_int

  #long* xs = malloc(nn * sizeof(long))
  mov 8(%rsp), %rax
  mov $8, %rdx
  imul %rdx
  
  mov %rax, %rdi
  call malloc
  mov %rax, %r12


  // r15 is the ii
  mov $0, %r15
for_loop_input:
/*
for (long ii = 0; ii < nn; ++ii) {
        read_int(&(xs[ii]));
    }
*/
  cmp 8(%rsp), %r15
  jge for_loop_input_done

  lea (%r12, %r15, 8), %rdi
  call read_int

  inc %r15
  jmp for_loop_input



for_loop_input_done:

//quicksort(xs, 0, nn);
mov %r12, %rdi
mov $0, %rsi
mov 8(%rsp), %rdx
call quicksort

#printf("%ld\n", nn);
  mov $int_fmt_nl, %rdi
  mov 8(%rsp), %rsi
  mov $0, %al
  call printf

  mov $0, %r15
  jmp for_loop_print

for_loop_print:
/*
for (long ii = 0; ii < nn; ++ii) {
        printf("%ld ", xs[ii]);
    }
*/
cmp  8(%rsp), %r15
jge for_loop_print_done

mov $int_fmt_space, %rdi
mov (%r12, %r15, 8), %rsi
mov $0, %al
call printf

inc %r15
jmp for_loop_print

for_loop_print_done:
jmp main_done

main_done:

  //printf("\n");
  mov $new_line, %rdi
  mov $0, %al
  call printf

  //free(xs);
  mov %r12, %rdi
  call free

  leave
  pop %r13
  pop %r12

  mov $0, %rax
  ret