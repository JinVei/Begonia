; source_filename = file.ll

module asm ".format:"
module asm "    .string \22%d, hello world %s\n\22"
module asm "    .text"
module asm "    .globl print"
;module asm "    .type print @function"
module asm "_print:"
module asm "        pushq   %rbp"
module asm "        movq    %rsp, %rbp"
module asm "        subq    $16, %rsp"
module asm "        movl    %edi, -4(%rbp)"
module asm "        movq    %rsi, -16(%rbp)"
module asm "        movq    -16(%rbp), %rdx"
module asm "        movl    -4(%rbp), %eax"
module asm "        movl    %eax, %esi"
module asm "        leaq    .format(%rip), %rdi"
module asm "        movl    $0, %eax"
module asm "        call    _printf"
;module asm "        call    printf@PLT"
module asm "        movq    %rbp, %rsp"
module asm "        popq    %rbp"
module asm "        ret"
module asm "exit:"
module asm "        call    _exit"
module asm "        ret"

@.str = global [16 x i8] c", how are you?\0A\00"

declare void @print(i64, i8*)
declare void @exit()

define i32 @main() {
start:
  %cast210 = getelementptr [16 x i8], [16 x i8]* @.str, i64 0, i64 0
  call void @exit()
  call void @print(i64 10, i8* %cast210)
  ret i32 0
}

;llc file.ll -filetype=obj -o file.o
;ld -o test ./file.o -dynamic-linker /lib64/ld-linux-x86-64.so.2 /usr/lib/x86_64-linux-gnu/crt1.o /usr/lib/x86_64-linux-gnu/crti.o /usr/lib/x86_64-linux-gnu/crtn.o -lc -melf_x86_64
;./test

;llc file.ll -filetype=obj -o file.o
;ld -o test ./file.o   -lSystem -macosx_version_min 10.14