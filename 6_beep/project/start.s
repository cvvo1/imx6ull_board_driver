/*
选择按位与（1111）的方式，取出数据A的某几位 
选择按位或（先清零）的方式，将数据赋值给A的某几位
*/

/*汇编文件，用于设置处理器模式、sp指针*/

.global _start
.global _bss_start
_bss_start:
    .word __bss_start

.global _bss_end
_bss_end:
    .word __bss_end

_start: 
    /* 
    设置处理器进入SVC模式
    需要采用处理器内部数据传输指令：读取特殊寄存器和写特殊寄存器只能使用MRS和MSR指令
        MRS指令（读）-将特殊寄存器（如CPSR和SPSR）中的数据传递给通用寄存器
        MSR指令（写）-将普通寄存器的数据传递给特殊寄存器*/
    mrs r0,cpsr
     /*设置CPSR程序状态寄存器后四位M[4:0]=10011,进入SVS模式，再*/
    bic r0,r0,#0x1f    /*前将CPSR寄存器后5位清零，指令：BIC Rd, Rn, #immed——位清除*/
    orr r0,r0,#0x13    /*将10011按位或赋值，指令：ORR Rd, Rn, #immed——按位或*/
    msr cpsr,r0
    
    /*清除BSS段*/
    ldr r0, _bss_start   /*使用LDR伪指令将数据加载到寄存器时要在数据前面添加“=”*/
    ldr r1, _bss_end     /*使用MOV指令将数据加载到寄存器时要在前面添加“#” */ 
    mov r2, #0           /*将数据从一个寄存器拷贝到另外一个寄存器，或者将一个立即数传递到寄存器里面 */
bss_loop:
    stmia r0!, {r2}
    cmp r0, r1           /*比较r0和r1里面的值 */
    ble bss_loop         /*如果r0地址小于等于r1，继续清除bss段*/ 
    
    /*设置sp指针，指向DDR，DDR内存512MB的范围0x80000000~0x9FFFFFFF，设置栈大小，0x200000=2MB，栈向下增长*/
    ldr sp, =0x80200000
    b main    /*跳转到c语言main函数 */
