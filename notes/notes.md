# 项目笔记

## 名词

### GDT
Global Descriptor Table，全局描述符表，这是操作系统在保护模式（Protected Mode）下用于管理内存分段的一种数据结构。它定义了内存段的起始地址、大小和访问权限（如特权级）。在内核启动和初始化时，操作系统会创建并加载GDT。该实验中2^13个描述符。

### LDT
Local Descriptor Table，局部描述符表，局部的。每个进程可以拥有自己的 LDT（如果操作系统支持），用来存放该进程私有的代码段、数据段。为了实现进程间隔离的一种硬件手段。

### CS
Code Segment Register，代码段寄存器，它存储着当前正在执行的代码所在的内存段的基础地址。在实模式下，它相当于一个段基址。由于早期CPU的地址线宽度有限，需要通过“段基址 + 偏移量”的方式来访问整个内存。CS就负责提供这个“段基址”。

### IP
Instruction Pointer Register，指令指针寄存器，它存储着当前将要执行的下一条指令在代码段中的 偏移地址（也叫偏移量）。

### DS
Data Segment Register，数据段寄存器，指向当前程序默认的数据段。

### ES
Extra Segment Register，附加段寄存器，这是一个“辅助”的数据段寄存器。它主要用于一些特定的、需要处理大量数据的字符串操作指令。

### SS
Stack Segment Register，栈段寄存器，它指向当前程序使用的栈所在的段。

### ELF
Executable and Linkable Format，可执行与可链接格式，在 Unix 和类 Unix 系统（如 Linux）中最常用的可执行文件、目标代码、共享库和核心转储（core dump）的文件格式。它是在现代 Linux 系统中事实上的标准二进制格式。在没有操作系统干预的情况下，CPU 只能执行连续的内存中的机器码。但一个程序通常由代码、数据、调试信息、动态链接信息等组成。ELF 的作用就是将这些分散的组件打包成一个单一文件，并为操作系统提供如何将它们加载到内存中的“地图”。

ELF文件 = ELF头 + ELF程序头表 + 其余部分（数据代码 + 节头条目）

ELF头：描述这个ELF

ELF程序头表：描述如何将文件加载到内存以创建进程，描述一个段的信息

### PCB
Proceed Control Block，进程控制块

### 80386
x86架构历史上第一款32位处理器

## Q&A

### 为什么32位段选择符的base和limit如此零散**：
1. 历史兼容性：Intel 必须保证 80286 的代码能在 80386 上正常运行，80286有24根地址线，80386有32根
2. 字段按需对齐：描述符中不仅有基址和限长，还有权限位和标志位。这些控制位对于 CPU 的保护机制至关重要，需要放在固定的、易于快速解码的位置。


### 为什么start.S中不能直接orl $1, %cr0

在 x86 汇编中，像 orl（逻辑或）、addl（加法）这类算术逻辑运算指令，它们的操作数有一些限制：
1. 可以：对内存单元 (orl $1, (mem)) 或通用寄存器 (orl $1, %eax) 进行操作。
2. 不可以：对控制寄存器（cr0, cr2, cr3, cr4）或调试寄存器（dr0-dr7）直接进行算术运算。

## 注意

### 汇编代码的单位
.byte = 1字节，.word = 2字节，.long = 4字节，1字节 = 8位，一个十六进制的数占4位

### 指针的加法
在load_kernal()函数中，我们使用memcpy()函数将elf文件复制进内存，其中程序头表的内存位置`(void *)(elf) + (ph->p_offset)`一开始写成了`(void *)(elf + ph->p_offset)`，错误写法中elf是一个`Elf32_Ehdr *`类型而不是`void *`。

后面同样涉及到了程序头表的位置，不过就变成了`phoff + i*sizeof(ph)`。原因在于phoff是一个`uint32_t`类型

### static __attribute__的作用
在proc.c中`static __attribute__((used)) int next_pid = 1;`，强制保留这个静态变量，防止编译器优化时把它删除，即使当前代码文件里没有直接引用它。

## 其它

### 段描述符结构
```
DESCRIPTORS USED FOR APPLICATIONS CODE AND DATA SEGMENTS

  31                23                15                7               0
 +-----------------+-+-+-+-+---------+-+-----+-+-----+-+-----------------+
 |                 | | | |A|         | |     | |     | |                 |
 |   BASE 31..24   |G|X|O|V| LIMIT   |P| DPL |S| TYPE|A|  BASE 23..16    | 4
 |                 | | | |L| 19..16  | |     | |     | |                 |
 |-----------------+-+-+-+-+---------+-+-----+-+-----+-+-----------------|
 |                                   |                                   |
 |        SEGMENT BASE 15..0         |       SEGMENT LIMIT 15..0         | 0
 |                                   |                                   |
 +-----------------------------------+-----------------------------------+

           A      - ACCESSED
           AVL    - AVAILABLE FOR USE BY SYSTEMS PROGRAMMERS
           DPL    - DESCRIPTOR PRIVILEGE LEVEL
           G      - GRANULARITY
           P      - SEGMENT PRESENT
```
每个段描述符为 8 个字节，共 64 位。
段基址为第 2，3，4，7 字节，共 32 位。我们模拟扁平模式，因此段基址均为 0。
段限长为第 0，1 字节及第 6 字节的低 4 位，共 20 位，表示该段的最大长度。我们模拟扁平模式，因此段限长为全 1。

G 代表粒度，说明段限长的单位是什么（4KB或者1B）。当属性 G 为 0 时，20 位段限长为实际段的最大长度（最大为 1MB）；当属性 G 为 1 时，该 20 位段限长左移 12 位后加上 0xFFF 即为实际段的最大长度（最大为 4GB）。我们模拟扁平模式，因此这一项设置为 1。
X 意为这一段使用操作数位数，为 1 时使用 32 位操作数，为 0 时使用 16 位操作数。这一项设置为 1。
AVL：Available and Reserved Bit，通常设为 0。
P：存在位，P 为 1 表示段在内存中。这一项设置为 1。
DPL：描述符特权级，取值 0-3 共 4 级；0 特权级最高，3 特权级最低，表示访问该段时 CPU 所处于的最低特权级，后续实验会详细讨论。本次实验中的段都是在最高特权级上的，因此这一项设为 0。
S：描述符类型标志，S 为 1 表示代码段或数据段，S 为 0 表示系统段(TSS，LDT)和门描述符。目前我们只需要代码段和数据段，因此设置为 1。

TYPE：当 S 为 1，TYPE 表示的代码段，数据段的各种属性如下表所示（注意下面的TYPE包括了A位作为最低位）

#### 段基址&段限长

**段基址**：段基址是一个内存段的起始地址。它告诉 CPU 这个段从内存的哪个具体位置开始。
**段限长**：段限长规定了该内存段的最大长度（或大小）。用于边界检查（保护）。

### ELF头
#### ELF文件结构简图
```
+-------------+-----------------------+--------------------------------------+----------------------+
| ELF Header  | Program Header Table  |        Rest of the ELF File          | Section Header Table |
+-------------+-----------------------+--------------------------------------+----------------------+
```
#### ELF头的结构
```
The ELF header is described by the type Elf32_Ehdr or Elf64_Ehdr:
#define EI_NIDENT 16

typedef struct {
    unsigned char e_ident[EI_NIDENT];
    uint16_t      e_type;
    uint16_t      e_machine;
    uint32_t      e_version;
    ElfN_Addr     e_entry;        // 程序的入口地址
    ElfN_Off      e_phoff;        // 程序头表在ELF文件中的偏移量
    ElfN_Off      e_shoff;
    uint32_t      e_flags;
    uint16_t      e_ehsize;
    uint16_t      e_phentsize;    // 程序头表中每个表项的大小
    uint16_t      e_phnum;        // 程序头表中包含表项的个数
    uint16_t      e_shentsize;
    uint16_t      e_shnum;
    uint16_t      e_shstrndx;
} ElfN_Ehdr;
```

### 为什么要把ELF文件的数据段和代码段装载到内存的不同位置？
CPU在执行指令时，**只会使用绝对地址**，而代码中的绝对地址是固定的，比如说int A = 1，A的地址是绝对位置，所以要将A写入vaddr中，再根据虚拟地址到物理地址的映射进行转变。