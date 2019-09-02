#ifndef _UAPI_LINUX_ELF_H
#define _UAPI_LINUX_ELF_H

#include <linux/types.h>
#include <linux/elf-em.h>

/* 32-bit ELF base types. */
typedef __u32	Elf32_Addr;
typedef __u16	Elf32_Half;
typedef __u32	Elf32_Off;
typedef __s32	Elf32_Sword;
typedef __u32	Elf32_Word;

/* 64-bit ELF base types. */
typedef __u64	Elf64_Addr;
typedef __u16	Elf64_Half;
typedef __s16	Elf64_SHalf;
typedef __u64	Elf64_Off;
typedef __s32	Elf64_Sword;
typedef __u32	Elf64_Word;
typedef __u64	Elf64_Xword;
typedef __s64	Elf64_Sxword;

/* These constants are for the segment types stored in the image headers
 * segment类型，也就是program类型 */
#define PT_NULL    0
#define PT_LOAD    1                // 标识这是个可加载的segment，一个runtime程序中通常存在多个LOAD segment
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6                // 标识这个segment存放的是program头列表
#define PT_TLS     7               /* Thread local storage segment */
#define PT_LOOS    0x60000000      /* OS-specific */
#define PT_HIOS    0x6fffffff      /* OS-specific */
#define PT_LOPROC  0x70000000
#define PT_HIPROC  0x7fffffff
#define PT_GNU_EH_FRAME		0x6474e550

#define PT_GNU_STACK	(PT_LOOS + 0x474e551)

/*
 * Extended Numbering
 *
 * If the real number of program header table entries is larger than
 * or equal to PN_XNUM(0xffff), it is set to sh_info field of the
 * section header at index 0, and PN_XNUM is set to e_phnum
 * field. Otherwise, the section header at index 0 is zero
 * initialized, if it exists.
 *
 * Specifications are available in:
 *
 * - Oracle: Linker and Libraries.
 *   Part No: 817–1984–19, August 2011.
 *   http://docs.oracle.com/cd/E18752_01/pdf/817-1984.pdf
 *
 * - System V ABI AMD64 Architecture Processor Supplement
 *   Draft Version 0.99.4,
 *   January 13, 2010.
 *   http://www.cs.washington.edu/education/courses/cse351/12wi/supp-docs/abi.pdf
 */
#define PN_XNUM 0xffff

/* These constants define the different elf file types 
 * ELF文件类型，e_type
 * */
#define ET_NONE   0
#define ET_REL    1         // 可重定位文件(.o, .a)
#define ET_EXEC   2         // 可执行文件
#define ET_DYN    3         // 共享目标文件(.so), 开启PIE后的可执行文件也属于这种类型
#define ET_CORE   4         // coredump文件
#define ET_LOPROC 0xff00
#define ET_HIPROC 0xffff

/* This is the info that is needed to parse the dynamic section of the file */
#define DT_NULL		0
#define DT_NEEDED	1
#define DT_PLTRELSZ	2
#define DT_PLTGOT	3
#define DT_HASH		4
#define DT_STRTAB	5
#define DT_SYMTAB	6
#define DT_RELA		7
#define DT_RELASZ	8
#define DT_RELAENT	9
#define DT_STRSZ	10
#define DT_SYMENT	11
#define DT_INIT		12
#define DT_FINI		13
#define DT_SONAME	14
#define DT_RPATH 	15
#define DT_SYMBOLIC	16
#define DT_REL	        17
#define DT_RELSZ	18
#define DT_RELENT	19
#define DT_PLTREL	20
#define DT_DEBUG	21
#define DT_TEXTREL	22
#define DT_JMPREL	23
#define DT_ENCODING	32
#define OLD_DT_LOOS	0x60000000
#define DT_LOOS		0x6000000d
#define DT_HIOS		0x6ffff000
#define DT_VALRNGLO	0x6ffffd00
#define DT_VALRNGHI	0x6ffffdff
#define DT_ADDRRNGLO	0x6ffffe00
#define DT_ADDRRNGHI	0x6ffffeff
#define DT_VERSYM	0x6ffffff0
#define DT_RELACOUNT	0x6ffffff9
#define DT_RELCOUNT	0x6ffffffa
#define DT_FLAGS_1	0x6ffffffb
#define DT_VERDEF	0x6ffffffc
#define	DT_VERDEFNUM	0x6ffffffd
#define DT_VERNEED	0x6ffffffe
#define	DT_VERNEEDNUM	0x6fffffff
#define OLD_DT_HIOS     0x6fffffff
#define DT_LOPROC	0x70000000
#define DT_HIPROC	0x7fffffff

/* This info is needed when parsing the symbol table */
// 符号绑定信息, st_info高28位 
#define STB_LOCAL  0    // 局部符号，外部不可见
#define STB_GLOBAL 1    // 全局符号，外部不可见
#define STB_WEAK   2    // 弱引用

// 符号类型, st_info低4位
#define STT_NOTYPE  0   // 未知类型
#define STT_OBJECT  1   // 该符号是个数据对象，比如变量、数组等
#define STT_FUNC    2   // 该符号是个函数
#define STT_SECTION 3   // 该符号是个section名，这种符号只能是STB_LOCAL
#define STT_FILE    4   // 该符号是个文件名，这种符号只能是STB_LOCAL
#define STT_COMMON  5
#define STT_TLS     6

#define ELF_ST_BIND(x)		((x) >> 4)
#define ELF_ST_TYPE(x)		(((unsigned int) x) & 0xf)
#define ELF32_ST_BIND(x)	ELF_ST_BIND(x)
#define ELF32_ST_TYPE(x)	ELF_ST_TYPE(x)
#define ELF64_ST_BIND(x)	ELF_ST_BIND(x)
#define ELF64_ST_TYPE(x)	ELF_ST_TYPE(x)

typedef struct dynamic{
    Elf32_Sword d_tag;
    union{
        Elf32_Sword	d_val;
        Elf32_Addr	d_ptr;
    } d_un;
} Elf32_Dyn;

typedef struct {
    Elf64_Sxword d_tag;		/* entry tag value */
    union {
        Elf64_Xword d_val;
        Elf64_Addr d_ptr;
    } d_un;
} Elf64_Dyn;

/* The following are used with relocations */
#define ELF32_R_SYM(x) ((x) >> 8)
#define ELF32_R_TYPE(x) ((x) & 0xff)

// 64位环境上，高32位表示重定位入口的符号在符号表中的下标，低32位表示重定位入口的类型
#define ELF64_R_SYM(i)			((i) >> 32)
#define ELF64_R_TYPE(i)			((i) & 0xffffffff)

typedef struct elf32_rel {
    Elf32_Addr	r_offset;
    Elf32_Word	r_info;
} Elf32_Rel;

// 64位重定位入口结构，是组成重定位表的元素
typedef struct elf64_rel {
    Elf64_Addr r_offset;	/* Location at which to apply the action 
                               在可重定位文件中，表示该重定位入口相对于所在section起始位置的偏移量
                               在可执行文件和共享对象文件中，表示该重定位入口的虚拟地址 
                               开启PIE后，则表示一个相对动态基址的偏移量 */
    Elf64_Xword r_info;	/* index and type of relocation */
} Elf64_Rel;

typedef struct elf32_rela{
    Elf32_Addr	r_offset;
    Elf32_Word	r_info;
    Elf32_Sword	r_addend;
} Elf32_Rela;

typedef struct elf64_rela {
    Elf64_Addr r_offset;	/* Location at which to apply the action */
    Elf64_Xword r_info;	/* index and type of relocation */
    Elf64_Sxword r_addend;	/* Constant addend used to compute value */
} Elf64_Rela;

typedef struct elf32_sym{
    Elf32_Word	st_name;
    Elf32_Addr	st_value;
    Elf32_Word	st_size;
    unsigned char	st_info;
    unsigned char	st_other;
    Elf32_Half	st_shndx;
} Elf32_Sym;

// 64位符号结构，是组成符号表的元素
typedef struct elf64_sym {
    Elf64_Word st_name;		    /* Symbol name, index in string tbl
                                   该符号名序号 */
    unsigned char	st_info;	/* Type and binding attributes
                                   该符号类型[3-0]和绑定信息[31-4], 比如STT_SECTION | (STB_LOCAL << 4) */
    unsigned char	st_other;	/* No defined meaning, 0 */
    Elf64_Half st_shndx;		/* Associated section index
                                   该符号所在的section序号，有一些保留的section序号 */
    Elf64_Addr st_value;		/* Value of the symbol
                                   该符号的值，具体含义跟符号类型有关 */
    Elf64_Xword st_size;		/* Associated symbol size
                                   符号大小，具体含义也是跟符号类型有关 */
} Elf64_Sym;


#define EI_NIDENT	16

typedef struct elf32_hdr{
    unsigned char	e_ident[EI_NIDENT];
    Elf32_Half	e_type;
    Elf32_Half	e_machine;
    Elf32_Word	e_version;
    Elf32_Addr	e_entry;  /* Entry point */
    Elf32_Off	e_phoff;
    Elf32_Off	e_shoff;
    Elf32_Word	e_flags;
    Elf32_Half	e_ehsize;
    Elf32_Half	e_phentsize;
    Elf32_Half	e_phnum;
    Elf32_Half	e_shentsize;
    Elf32_Half	e_shnum;
    Elf32_Half	e_shstrndx;
} Elf32_Ehdr;

// 64位ELF文件头结构
typedef struct elf64_hdr {
    unsigned char	e_ident[EI_NIDENT];	/* ELF "magic number" 
                                            16个字节的标识符目前只用了前8个字节，
                                            依次为magic、clas、data、version、os/abi */
    Elf64_Half e_type;      // ELF文件类型, 比如ET_EXEC
    Elf64_Half e_machine;   // 机器类型，比如EM_386
    Elf64_Word e_version;
    Elf64_Addr e_entry;		/* Entry point virtual address 
                               程序入口点虚拟地址
                               开启PIE后，该地址只是一个相对动态基址的偏移量 */
    Elf64_Off e_phoff;		/* Program header table file offset 
                               program头列表起始偏移量 */
    Elf64_Off e_shoff;		/* Section header table file offset 
                               section头列表起始偏移量 */
    Elf64_Word e_flags;
    Elf64_Half e_ehsize;    // 该ELF头大小
    Elf64_Half e_phentsize; // program头大小
    Elf64_Half e_phnum;     // program头数量
    Elf64_Half e_shentsize; // section头大小
    Elf64_Half e_shnum;     // section头数量
    Elf64_Half e_shstrndx;  // 名为".shstrtab"的字符串表在section头列表中的序号
} Elf64_Ehdr;

/* These constants define the permissions on sections in the program
   header, p_flags. */
#define PF_R		0x4
#define PF_W		0x2
#define PF_X		0x1

typedef struct elf32_phdr{
    Elf32_Word	p_type;
    Elf32_Off	p_offset;
    Elf32_Addr	p_vaddr;
    Elf32_Addr	p_paddr;
    Elf32_Word	p_filesz;
    Elf32_Word	p_memsz;
    Elf32_Word	p_flags;
    Elf32_Word	p_align;
} Elf32_Phdr;

// 64位program头结构
typedef struct elf64_phdr {
    Elf64_Word p_type;      // 该program类型，比如PT_LOAD
    Elf64_Word p_flags;
    Elf64_Off p_offset;		/* Segment file offset 
                               该program在ELF文件中的偏移量 */
    Elf64_Addr p_vaddr;		/* Segment virtual address 
                               该program在内存中的起始地址
                               开启PIE后，该地址只是一个相对动态基址的偏移量 */
    Elf64_Addr p_paddr;		/* Segment physical address */
    Elf64_Xword p_filesz;		/* Segment size in file */
    Elf64_Xword p_memsz;		/* Segment size in memory */
    Elf64_Xword p_align;		/* Segment alignment, file & memory */
} Elf64_Phdr;

/* section类型 sh_type */
#define SHT_NULL	0       // 无效section，一般section头列表中第一个section都是无效段
#define SHT_PROGBITS	1   // 程序section，".text"、".data"、".rodata"等都是这种类型
#define SHT_SYMTAB	2       // 表示该section为符号表，通常名为".symtab"
#define SHT_STRTAB	3       /* 表示该section为字符串表
                               ".strtab"    - 用来保存普通字符串，通常就是符号名
                               ".shstrtab"  - 专门用来保存section名
                               ".dynstr"    - 专门用来保存动态链接需要用到的符号名 */
#define SHT_RELA	4       // 表示该section用来存放重定位信息
#define SHT_HASH	5       // 表示该section是指定符号表的hash表，通常名为".gnu.hash"
#define SHT_DYNAMIC	6       // 表示该section用来存放动态链接信息, 通常名为".dynamic"
#define SHT_NOTE	7       /* 表示该section用来存放提示性信息, 
                               ".note.gnu.build-id" 可以唯一标识该ELF文件，常用来匹配对应的外部debuginfo文件 */
#define SHT_NOBITS	8       // 表示该section在ELF文件中没有实际内容，比如".bss"
#define SHT_REL		9       /* 表示该section为重定位表，section名通常以".rel."作为前缀
                               每个需要重定位的代码段或数据段，都会有一个相应的重定位表
                               ".rel.dyn"、".rel.plt"、".rel.text"等都是这种类型 */
#define SHT_SHLIB	10      
#define SHT_DYNSYM	11      /* 表示该section为动态链接的符号表，通常名为".dynsym"
                               这些符号都是本ELF文件用到，但定义在其他ELF文件 */
#define SHT_NUM		12
#define SHT_LOPROC	0x70000000
#define SHT_HIPROC	0x7fffffff
#define SHT_LOUSER	0x80000000
#define SHT_HIUSER	0xffffffff

/* sh_flags 
 * section的标志位表示该section在进程虚拟地址空间中的属性 */
#define SHF_WRITE	0x1             // 标识该section在进程空间中可写
#define SHF_ALLOC	0x2             /* 标识该section在进程空间中需要分配空间
                                       像代码段、数据段和.bss段都会有这个标志，
                                       而有些包含指示或控制信息的section(如debug相关的sections)不需要在进程中分配空间，所以一般不会有这个标志 */
#define SHF_EXECINSTR	0x4         // 标识该section在进程空间中可被执行
#define SHF_MASKPROC	0xf0000000

/* special section indexes 
 * 几个特殊的section序号，st_shndx */
#define SHN_UNDEF	0           // 表示该符号未定义，通常表示该符号在本目标文件被引用，但定义在其他目标文件中
#define SHN_LORESERVE	0xff00
#define SHN_LOPROC	0xff00
#define SHN_HIPROC	0xff1f
#define SHN_ABS		0xfff1      // 表示该符号是一个绝对的值，STT_FILE通常就对应这种类型
#define SHN_COMMON	0xfff2      // 通常".bss"中的全局(非static)符号就对应这种类型?
#define SHN_HIRESERVE	0xffff

typedef struct elf32_shdr {
    Elf32_Word	sh_name;
    Elf32_Word	sh_type;
    Elf32_Word	sh_flags;
    Elf32_Addr	sh_addr;
    Elf32_Off	sh_offset;
    Elf32_Word	sh_size;
    Elf32_Word	sh_link;
    Elf32_Word	sh_info;
    Elf32_Word	sh_addralign;
    Elf32_Word	sh_entsize;
} Elf32_Shdr;

// section头结构
typedef struct elf64_shdr {
    Elf64_Word sh_name;		/* Section name, index in string tbl 
                               该section名字符串在名为".shstrtab"的section中的偏移量 */
    Elf64_Word sh_type;		/* Type of section 
                               该section类型 */
    Elf64_Xword sh_flags;	/* Miscellaneous section attributes 
                               该section标志位集合 */
    Elf64_Addr sh_addr;		/* Section virtual addr at execution 
                               如果该section可加载，则为该section在进程地址空间中的虚拟地址, 
                               当然开启PIE后，该地址只是一个相对动态基址的偏移量;
                               如果该section不可加载，则这里为0 */
    Elf64_Off sh_offset;	/* Section file offset 
                               如果该section存在与ELF文件中，则表示其在ELF文件中的偏移；
                               否则无意义(.bss) */
    Elf64_Xword sh_size;	/* Size of section in bytes 
                               该section的大小 */
    Elf64_Word sh_link;		/* Index of another section 
                               当该section为某些特定的类型时，该字段用来从section头列表中定位依赖的section:
                                    SHT_DYNAMIC - 用来索引依赖的字符串表
                                    SHT_HASH    - 用来索引依赖的符号表
                                    SHT_REL[A]  - 用来索引依赖的符号表
                                    SHT_SYMTAB  - 用来索引依赖的字符串表
                                    SHT_DYNSYM  - 用来索引依赖的字符串表
                               否则无意义 */
    Elf64_Word sh_info;		/* Additional section information
                               当该section为某些特定的类型时，该字段用来从section头列表中定位作用的section:
                                    SHT_REL[A]  - 用来索引该重定位表作用的section
                               否则无意义 */
    Elf64_Xword sh_addralign;	/* Section alignment 
                                   如果该section的地址有对齐要求，则该值表示对齐值(指数值)，比如该值为3表示8字节对齐;
                                   否则该值为0或1表示无意义 */
    Elf64_Xword sh_entsize;	/* Entry size if section holds table 
                               如果该section包含了一张固定大小元素的表，则该值表示表的元素大小；
                               否则该值为0表示无意义 */
} Elf64_Shdr;

#define	EI_MAG0		0		/* e_ident[] indexes */
#define	EI_MAG1		1
#define	EI_MAG2		2
#define	EI_MAG3		3
#define	EI_CLASS	4
#define	EI_DATA		5
#define	EI_VERSION	6
#define	EI_OSABI	7
#define	EI_PAD		8

#define	ELFMAG0		0x7f		/* EI_MAG */
#define	ELFMAG1		'E'
#define	ELFMAG2		'L'
#define	ELFMAG3		'F'
#define	ELFMAG		"\177ELF"
#define	SELFMAG		4

#define	ELFCLASSNONE	0		/* EI_CLASS */
#define	ELFCLASS32	1
#define	ELFCLASS64	2
#define	ELFCLASSNUM	3

#define ELFDATANONE	0		/* e_ident[EI_DATA] */
#define ELFDATA2LSB	1
#define ELFDATA2MSB	2

#define EV_NONE		0		/* e_version, EI_VERSION */
#define EV_CURRENT	1
#define EV_NUM		2

#define ELFOSABI_NONE	0
#define ELFOSABI_LINUX	3

#ifndef ELF_OSABI
#define ELF_OSABI ELFOSABI_NONE
#endif

/*
 * Notes used in ET_CORE. Architectures export some of the arch register sets
 * using the corresponding note types via the PTRACE_GETREGSET and
 * PTRACE_SETREGSET requests.
 */
#define NT_PRSTATUS	1
#define NT_PRFPREG	2
#define NT_PRPSINFO	3
#define NT_TASKSTRUCT	4
#define NT_AUXV		6
/*
 * Note to userspace developers: size of NT_SIGINFO note may increase
 * in the future to accomodate more fields, don't assume it is fixed!
 */
#define NT_SIGINFO      0x53494749
#define NT_FILE         0x46494c45
#define NT_PRXFPREG     0x46e62b7f      /* copied from gdb5.1/include/elf/common.h */
#define NT_PPC_VMX	0x100		/* PowerPC Altivec/VMX registers */
#define NT_PPC_SPE	0x101		/* PowerPC SPE/EVR registers */
#define NT_PPC_VSX	0x102		/* PowerPC VSX registers */
#define NT_386_TLS	0x200		/* i386 TLS slots (struct user_desc) */
#define NT_386_IOPERM	0x201		/* x86 io permission bitmap (1=deny) */
#define NT_X86_XSTATE	0x202		/* x86 extended state using xsave */
#define NT_S390_HIGH_GPRS	0x300	/* s390 upper register halves */
#define NT_S390_TIMER	0x301		/* s390 timer register */
#define NT_S390_TODCMP	0x302		/* s390 TOD clock comparator register */
#define NT_S390_TODPREG	0x303		/* s390 TOD programmable register */
#define NT_S390_CTRS	0x304		/* s390 control registers */
#define NT_S390_PREFIX	0x305		/* s390 prefix register */
#define NT_S390_LAST_BREAK	0x306	/* s390 breaking event address */
#define NT_S390_SYSTEM_CALL	0x307	/* s390 system call restart data */
#define NT_S390_TDB	0x308		/* s390 transaction diagnostic block */
#define NT_S390_VXRS_LOW	0x309	/* s390 vector registers 0-15 upper half */
#define NT_S390_VXRS_HIGH	0x30a	/* s390 vector registers 16-31 */
#define NT_ARM_VFP	0x400		/* ARM VFP/NEON registers */
#define NT_ARM_TLS	0x401		/* ARM TLS register */
#define NT_ARM_HW_BREAK	0x402		/* ARM hardware breakpoint registers */
#define NT_ARM_HW_WATCH	0x403		/* ARM hardware watchpoint registers */
#define NT_ARM_SYSTEM_CALL	0x404	/* ARM system call number */
#define NT_METAG_CBUF	0x500		/* Metag catch buffer registers */
#define NT_METAG_RPIPE	0x501		/* Metag read pipeline state */
#define NT_METAG_TLS	0x502		/* Metag TLS pointer */


/* Note header in a PT_NOTE section */
typedef struct elf32_note {
    Elf32_Word	n_namesz;	/* Name size */
    Elf32_Word	n_descsz;	/* Content size */
    Elf32_Word	n_type;		/* Content type */
} Elf32_Nhdr;

/* Note header in a PT_NOTE section */
typedef struct elf64_note {
    Elf64_Word n_namesz;	/* Name size */
    Elf64_Word n_descsz;	/* Content size */
    Elf64_Word n_type;	/* Content type */
} Elf64_Nhdr;

#endif /* _UAPI_LINUX_ELF_H */
