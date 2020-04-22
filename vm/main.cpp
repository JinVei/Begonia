#include <stdio.h>
int main(void)
{
    char *buf = "test\n";
    ssize_t n;
    // asm volatile (
    //     "movl $0x00000002, %%edi\n"  /* first argument == stderr */
    //     "movl $0x00000006, %%edx\n"  /* third argument == number of bytes */
    //     "movl $1, %%eax\n"  /* syscall number == write on amd64 linux */
    //     "syscall\n"
    //     : "=A"(n)         /* %rax: return value */
    //     : "S"(buf));      /* %rsi: second argument == address of data to write */
    return n;
}