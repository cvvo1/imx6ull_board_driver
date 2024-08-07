/*
选择按位与（1111）的方式，取出数据A的某几位 
选择按位或（先清零）的方式，将数据赋值给A的某几位
*/

/*汇编文件，用于设置处理器模式、sp指针*/

.global _start

_start: 
    /*首先定义Cortex-A中断向量表*/
    ldr pc,=Reset_Handler      /*复位中断函数 */
    ldr pc,=Undefined_Handler  /*未定义指令中断服务函数 */
    ldr pc,=SVC_Handler        /*SVC */
    ldr pc,=PreAbort_Handler   /*预取终止 */
    ldr pc,=DataAbort_Handler  /*数据终止 */
    ldr pc,=NotUsed_Handler    /*未使用 */
    ldr pc,=IRQ_Handler        /*IRQ外部中断函数 */
    ldr pc,=FIR_Handler        /*FIQ快速中断函数 */

/*
复位中断服务函数
1、关闭I Cache（是指令高速缓冲存储器）、D Cache（数据高速缓冲存储器）和MMU（Memory Management Unit，内存管理单元）。
   按照读写速度由快到慢：CPU->寄存器->cache->内存->外存，icache和dcache的作用是提高系统性能
2、设置处理器9种工作模式下的SP指针。3、清除BSS段。4、跳到C函数（main）
*/
Reset_Handler:
    cpsid i     /*关闭IRQ中断 */
    @ cpsid f     /*关闭FIQ中断 */
    /*关闭I.D Cache和MMU，操作SCTLR(System Control Register)寄存器，属于CP15协处理器量，采用读-改-写方式
            bit0位打开或关闭MMU，bit1位控制对齐，bit2控制D Cache，Bit11用于控制分支预测，Bit12用于控制I Cache。
            其中，MRC就是读CP15寄存器，将CP15协处理器中的寄存器数据读到 ARM 寄存器中。
                 MCR就是写CP15寄存器，MCR指令格式如下：MCR{cond} p15, <opc1>, <Rt>, <CRn>, <CRm>, <opc2>  */
    MRC p15,0,r0,c1,c0,0        /*读SCTLR寄存器 */
    /*指令：BIC Rd, Rn, #immed——位清除*/
    bic r0,r0,#(0x1 << 12)    /*关闭I Cache*/
    bic r0,r0,#(0x1 << 2)     /*关闭D Cache*/
    bic r0,r0,#(0x1 << 1)     /*关闭控制对齐*/
    bic r0,r0,#(0x1 << 11)    /*关闭分支预测*/
    bic r0,r0,#(0x1 << 0)     /*关闭MMU*/
    MCR p15,0,r0,c1,c0,0        /*写SCTLR寄存器 */


    @ /*设置中断向量偏移，将新低中断向量表首地址写入CP15协处理器的VBAR寄存器（Vector Base Address Register）*/
    @ ldr r0, =0X87800000
    @ dsb           /*数据同步屏障是一种特殊类型的内存屏障。只有当此指令执行完毕后，才会执行程序中位于此指令后的指令。 */
    @ isb           /*指令同步屏障可刷新处理器中的管道，因此可确保在ISB指令完成后，才从高速缓存或内存中提取位于该指令后的其他所有指令。 */
    @ MCR p15,0,r0,c12,c0,0      /*写VBAR寄存器 */
    @ dsb
    @ isb

.global _bss_start
_bss_start:
    .word __bss_start

.global _bss_end
_bss_end:
    .word __bss_end

    /*清除BSS段*/
    ldr r0, _bss_start   /*使用LDR伪指令将数据加载到寄存器时要在数据前面添加“=”*/
    ldr r1, _bss_end     /*使用MOV指令将数据加载到寄存器时要在前面添加“#” */ 
    mov r2, #0           /*将数据从一个寄存器拷贝到另外一个寄存器，或者将一个立即数传递到寄存器里面 */
bss_loop:
    stmia r0!, {r2}
    cmp r0, r1           /*比较r0和r1里面的值 */
    ble bss_loop         /*如果r0地址小于等于r1，继续清除bss段*/ 

    /*设置处理器9种工作模式下低SP指针 */
    /*设置处理器进入IRQ模式 */
    mrs r0,cpsr
    /*设置CPSR程序状态寄存器后四位M[4:0]=10010,进入IRQ模式*/
    bic r0,r0,#0x1f    /*将CPSR寄存器后5位清零，指令：BIC Rd, Rn, #immed——位清除*/
    orr r0,r0,#0x12    /*将10010按位或赋值，指令：ORR Rd, Rn, #immed——按位或*/
    msr cpsr,r0
    ldr sp, =0x80600000  /*设置sp指针，指向DDR，DDR内存512MB的范围0x80000000~0x9FFFFFFF，设置栈大小，0x200000=2MB，栈向下增长*/

    /*设置处理器进入SYS模式 */
    mrs r0,cpsr
    /*设置CPSR程序状态寄存器后四位M[4:0]=11111进入IRQ模式*/
    bic r0,r0,#0x1f    /*将CPSR寄存器后5位清零，指令：BIC Rd, Rn, #immed——位清除*/
    orr r0,r0,#0x1f    /*将11111按位或赋值，指令：ORR Rd, Rn, #immed——按位或*/
    msr cpsr,r0
    ldr sp, =0x80400000  /*设置sp指针，指向DDR，DDR内存512MB的范围0x80000000~0x9FFFFFFF，设置栈大小，0x200000=2MB，栈向下增长*/

    /* 
    设置处理器进入SVC模式
    需要采用处理器内部数据传输指令：读取特殊寄存器和写特殊寄存器只能使用MRS和MSR指令
        MRS指令（读）-将特殊寄存器（如CPSR和SPSR）中的数据传递给通用寄存器
        MSR指令（写）-将普通寄存器的数据传递给特殊寄存器*/
    mrs r0,cpsr
    /*设置CPSR程序状态寄存器后四位M[4:0]=10011,进入SVS模式*/
    bic r0,r0,#0x1f    /*将CPSR寄存器后5位清零，指令：BIC Rd, Rn, #immed——位清除*/
    orr r0,r0,#0x13    /*将10011按位或赋值，指令：ORR Rd, Rn, #immed——按位或*/
    msr cpsr,r0
    ldr sp, =0x80200000  /*设置sp指针，指向DDR，DDR内存512MB的范围0x80000000~0x9FFFFFFF，设置栈大小，0x200000=2MB，栈向下增长*/

    cpsie i    /*使能IRQ中断 */
    @ cpsie f    /*使能FIQ中断 */

    b main    /*跳转到c语言main函数 */



/*未定义指令中断服务函数 */
Undefined_Handler:
    ldr r0,=Undefined_Handler
    bx r0

/*SVC */
SVC_Handler:
    ldr r0,=SVC_Handler
    bx r0

/*预取终止 */
PreAbort_Handler:
    ldr r0,=PreAbort_Handler
    bx r0   

/*数据终止 */
DataAbort_Handler:
    ldr r0,=DataAbort_Handler
    bx r0

/*未使用 */
NotUsed_Handler:
    ldr r0,=NotUsed_Handler 
    bx r0

/*IRQ外部中断函数 */
IRQ_Handler:
    push {lr}					/*保存lr地址，push指令用于将数据或指令推入栈（stack）中*/
	push {r0-r3, r12}			/*保存r0-r3，r12寄存器 */

	mrs r0, spsr				/*读取保存程序状态寄存器(SPSR)，当异常发生时用于保存CPSR(当前程序状态寄存器)的值，。 */
	push {r0}					/*保存spsr寄存器 */

	/* 读取CP15协处理器的CBAR(Configuration Base Address Register)寄存器值到R1寄存器中 */
    mrc p15, 4, r1, c15, c0, 0  /*CBAR寄存器-保存内存映射的GIC寄存器组的物理首地址*
                                /*GIC架构分为两个逻辑快：Distributor(分发器端)-0x1000~0x1fff和CPU Interface(CPU接口端)-0x2000~0x3fff*/

	add r1, r1,#0x2000			/* GIC基地址偏移0X2000为CPU接口端基地址 */
	ldr r0, [r1, #0XC]			/* GIC的CPU接口端基地址加0X0C就是GICC_IAR寄存器，
								 * GICC_IAR寄存器bit9~0位读取中断ID，根据这个中断号来绝对调用哪个中断服务函数*/
	
    push {r0, r1}				/* 保存r0,r1 */
	
	cps #0x13					/* 进入SVC模式，允许其他中断再次进去 */
	
	push {lr}					/* 保存SVC模式的lr寄存器(链接寄存器)-指向子程序的返回地址 */
	ldr r2, =system_irqhandler	/* 加载C语言中断处理函数(system_irqhandler)到r2寄存器中*/
	blx r2						/* 运行system_irqhandler函数，输入参数为GICC_IAR寄存器值，blx 处理函数小于等于3个参数，默认读取r0——r2参数传递*/

	pop {lr}					/* 执行完C语言中断服务函数，lr出栈，用于返回IRQ模式*/
	cps #0x12					/* 进入IRQ模式 */
	pop {r0, r1}				/*r0、r1出栈 */
	str r0, [r1, #0X10]			/* 中断执行完成，将r0值(GICC_IAR寄存器-将对应中断ID值)，写入GICC_EOIR寄存器 */

	pop {r0}						
	msr spsr_cxsf, r0			/* 恢复spsr */

	pop {r0-r3, r12}	/* r0-r3,r12出栈 */
	pop {lr}			/* lr出栈 */
	subs pc, lr, #4		/* 表示将两个操作数相减，并将结果存储到第一个操作数中，即lr-4赋给PC，执行中断后程序
                            PC代表程序计数器，ARM指令流水线使用三个阶段，因此指令分为三个阶段执行：
                            1.取指（从存储器装载一条指令）；2.译码（识别将要被执行的指令）；3.执行（处理指令并将结果写回寄存器），实际为3-2-1
                            以正在执行的指令作为参考点，也就是以第一条指令为参考点，那么 R15(PC)中存放的就是第三条指令，
                            对于32位的ARM处理器，每条指令是4字节，PC值=当前程序执行位置+8；
                            */

/*FIQ快速中断函数 */
FIR_Handler:
    ldr r0,=FIR_Handler
    bx r0 



   
    

