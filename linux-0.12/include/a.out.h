#ifndef _A_OUT_H
#define _A_OUT_H

#define __GNU_EXEC_MACROS__

<<<<<<< HEAD
struct exec { /* 文件头结构 */
  unsigned long a_magic;	/* Use macros N_MAGIC, etc for access */  /* 执行文件魔数。使用N_MAGIC等宏访问 */
  unsigned a_text;		/* length of text, in bytes */  /* 代码长度，字节数 */
  unsigned a_data;		/* length of data, in bytes */  /* 数据长度，字节数 */
  unsigned a_bss;		/* length of uninitialized data area for file, in bytes */ /* 文件中的未初始化数据区的长度，字节数 */
  unsigned a_syms;		/* length of symbol table data in file, in bytes */ /* 文件中的符号表长度，字节数 */
  unsigned a_entry;		/* start address */ /* 执行开始地址 */
  unsigned a_trsize;		/* length of relocation info for text, in bytes */ /* 代码重定位信息长度，字节数 */
  unsigned a_drsize;		/* length of relocation info for data, in bytes */ /* 数据重定位信息长度，字节数 */
};

/* 魔数：很多类型的文件，其起始的几个字节的内容是固定的（或是有意填充，或是本就如此）
  。因此这几个字节的内容也被称为魔数 (magic number)，因为根据这几个字节的内容就可以
  确定文件类型。

*/

#ifndef N_MAGIC
#define N_MAGIC(exec) ((exec).a_magic)
#endif
/* OMAGIC 和 ZMAGIC 的主要区别在于它们对各个部分的存储分配方式上。虽然该结构的总长度只有32B，但是对于一个ZMAGIC类型的执行文件来说，
  其文件开始部分却需要专门留出1KB的空间给头结构使用。除被头结构占用的32B以外，其余部分均为0。从1024字节之后才开始放置程序的
  正文段和数据段等信息。而对于一个OMAGIC类型的.o模块文件来说，文件开始部分的32字节头结构后面紧接着就是代码区和数据区。 
*/
#ifndef OMAGIC
/* Code indicating object file or impure executable.  */ /* OMAGIC(Old Magic)指明文件是目标文件或者不纯的可执行文件 */
#define OMAGIC 0407
/* Code indicating pure executable.  */ /* 文件为纯粹的可执行文件  */ 
#define NMAGIC 0410
/* Code indicating demand-paged executable.  */ /* 文件为需求分页处理（demand-paging，即需求加载，load on demand）的可执行文件  */
=======
struct exec {
  unsigned long a_magic;	/* Use macros N_MAGIC, etc for access */
  unsigned a_text;		/* length of text, in bytes */
  unsigned a_data;		/* length of data, in bytes */
  unsigned a_bss;		/* length of uninitialized data area for file, in bytes */
  unsigned a_syms;		/* length of symbol table data in file, in bytes */
  unsigned a_entry;		/* start address */
  unsigned a_trsize;		/* length of relocation info for text, in bytes */
  unsigned a_drsize;		/* length of relocation info for data, in bytes */
};

#ifndef N_MAGIC
#define N_MAGIC(exec) ((exec).a_magic)
#endif

#ifndef OMAGIC
/* Code indicating object file or impure executable.  */
#define OMAGIC 0407
/* Code indicating pure executable.  */
#define NMAGIC 0410
/* Code indicating demand-paged executable.  */
>>>>>>> 0f46eaaa6f9bc5248db3f909e59d9b30af2578a4
#define ZMAGIC 0413
#endif /* not OMAGIC */

#ifndef N_BADMAG
#define N_BADMAG(x)					\
 (N_MAGIC(x) != OMAGIC && N_MAGIC(x) != NMAGIC		\
  && N_MAGIC(x) != ZMAGIC)
#endif

#define _N_BADMAG(x)					\
 (N_MAGIC(x) != OMAGIC && N_MAGIC(x) != NMAGIC		\
  && N_MAGIC(x) != ZMAGIC)

#define _N_HDROFF(x) (SEGMENT_SIZE - sizeof (struct exec))

#ifndef N_TXTOFF
#define N_TXTOFF(x) \
 (N_MAGIC(x) == ZMAGIC ? _N_HDROFF((x)) + sizeof (struct exec) : sizeof (struct exec))
#endif

#ifndef N_DATOFF
#define N_DATOFF(x) (N_TXTOFF(x) + (x).a_text)
#endif

#ifndef N_TRELOFF
#define N_TRELOFF(x) (N_DATOFF(x) + (x).a_data)
#endif

#ifndef N_DRELOFF
#define N_DRELOFF(x) (N_TRELOFF(x) + (x).a_trsize)
#endif

#ifndef N_SYMOFF
#define N_SYMOFF(x) (N_DRELOFF(x) + (x).a_drsize)
#endif

#ifndef N_STROFF
#define N_STROFF(x) (N_SYMOFF(x) + (x).a_syms)
#endif

/* Address of text segment in memory after it is loaded.  */
#ifndef N_TXTADDR
#define N_TXTADDR(x) 0
#endif

/* Address of data segment in memory after it is loaded.
   Note that it is up to you to define SEGMENT_SIZE
   on machines not listed here.  */
#if defined(vax) || defined(hp300) || defined(pyr)
#define SEGMENT_SIZE PAGE_SIZE
#endif
#ifdef	hp300
#define	PAGE_SIZE	4096
#endif
#ifdef	sony
#define	SEGMENT_SIZE	0x2000
#endif	/* Sony.  */
#ifdef is68k
#define SEGMENT_SIZE 0x20000
#endif
#if defined(m68k) && defined(PORTAR)
#define PAGE_SIZE 0x400
#define SEGMENT_SIZE PAGE_SIZE
#endif

#define PAGE_SIZE 4096
#define SEGMENT_SIZE 1024

#define _N_SEGMENT_ROUND(x) (((x) + SEGMENT_SIZE - 1) & ~(SEGMENT_SIZE - 1))

#define _N_TXTENDADDR(x) (N_TXTADDR(x)+(x).a_text)

#ifndef N_DATADDR
#define N_DATADDR(x) \
    (N_MAGIC(x)==OMAGIC? (_N_TXTENDADDR(x)) \
     : (_N_SEGMENT_ROUND (_N_TXTENDADDR(x))))
#endif

/* Address of bss segment in memory after it is loaded.  */
#ifndef N_BSSADDR
#define N_BSSADDR(x) (N_DATADDR(x) + (x).a_data)
#endif

#ifndef N_NLIST_DECLARED
struct nlist {
  union {
    char *n_name;
    struct nlist *n_next;
    long n_strx;
  } n_un;
  unsigned char n_type;
  char n_other;
  short n_desc;
  unsigned long n_value;
};
#endif

#ifndef N_UNDF
#define N_UNDF 0
#endif
#ifndef N_ABS
#define N_ABS 2
#endif
#ifndef N_TEXT
#define N_TEXT 4
#endif
#ifndef N_DATA
#define N_DATA 6
#endif
#ifndef N_BSS
#define N_BSS 8
#endif
#ifndef N_COMM
#define N_COMM 18
#endif
#ifndef N_FN
#define N_FN 15
#endif

#ifndef N_EXT
#define N_EXT 1
#endif
#ifndef N_TYPE
#define N_TYPE 036
#endif
#ifndef N_STAB
#define N_STAB 0340
#endif

/* The following type indicates the definition of a symbol as being
   an indirect reference to another symbol.  The other symbol
   appears as an undefined reference, immediately following this symbol.

   Indirection is asymmetrical.  The other symbol's value will be used
   to satisfy requests for the indirect symbol, but not vice versa.
   If the other symbol does not have a definition, libraries will
   be searched to find a definition.  */
#define N_INDR 0xa

/* The following symbols refer to set elements.
   All the N_SET[ATDB] symbols with the same name form one set.
   Space is allocated for the set in the text section, and each set
   element's value is stored into one word of the space.
   The first word of the space is the length of the set (number of elements).

   The address of the set is made into an N_SETV symbol
   whose name is the same as the name of the set.
   This symbol acts like a N_DATA global symbol
   in that it can satisfy undefined external references.  */

/* These appear as input to LD, in a .o file.  */
#define	N_SETA	0x14		/* Absolute set element symbol */
#define	N_SETT	0x16		/* Text set element symbol */
#define	N_SETD	0x18		/* Data set element symbol */
#define	N_SETB	0x1A		/* Bss set element symbol */

/* This is output from LD.  */
#define N_SETV	0x1C		/* Pointer to set vector in data area.  */

#ifndef N_RELOCATION_INFO_DECLARED

/* This structure describes a single relocation to be performed.
   The text-relocation section of the file is a vector of these structures,
   all of which apply to the text section.
   Likewise, the data-relocation section applies to the data section.  */

<<<<<<< HEAD
/* 重定位项的功能有两个。一是当代码段被重定位到一个不同的基地址处时，
   重定位项则用于指出需要修改的地方。二是在模块文件中存在对未定义符
   号引用时，当此未定义符号最终被定义时链接程序就可以使用相应重定位
   项对符号的值进行修正。由上面重定位记录项的结构可以看出，每个记录
   项含有模块文件代码区（代码段）和数据区（数据段）中需要重定位处长
   度为4B的地址以及规定如何具体进行重定位操作的信息。  */

struct relocation_info  /* 重定向信息部分 */
{
  /* Address (within segment) to be relocated.  */
  int r_address;  // 段内需要重定位的地址。
  /* The meaning of r_symbolnum depends on r_extern.  */
  unsigned int r_symbolnum:24;  // 含义与r_extern有关。指定符号表中一个符号或者一个段。
  /* Nonzero means value is a pc-relative offset
     and it should be relocated for changes in its own address
     as well as for changes in the symbol or section specified.  */
  unsigned int r_pcrel:1; // 1位。PC相关标志。即它作为一个相对地址被用于指令当中
  /* Length (as exponent of 2) of the field to be relocated.
     Thus, a value of 2 indicates 1<<2 bytes.  */
  unsigned int r_length:2;  // 2位。指定要被重定位字段长度（2的次方），0到3分别表示被重定位项的宽度是1B、2B、4B或8B
=======
struct relocation_info
{
  /* Address (within segment) to be relocated.  */
  int r_address;
  /* The meaning of r_symbolnum depends on r_extern.  */
  unsigned int r_symbolnum:24;
  /* Nonzero means value is a pc-relative offset
     and it should be relocated for changes in its own address
     as well as for changes in the symbol or section specified.  */
  unsigned int r_pcrel:1;
  /* Length (as exponent of 2) of the field to be relocated.
     Thus, a value of 2 indicates 1<<2 bytes.  */
  unsigned int r_length:2;
>>>>>>> 0f46eaaa6f9bc5248db3f909e59d9b30af2578a4
  /* 1 => relocate with value of symbol.
          r_symbolnum is the index of the symbol
	  in file's the symbol table.
     0 => relocate with the address of a segment.
          r_symbolnum is N_TEXT, N_DATA, N_BSS or N_ABS
	  (the N_EXT bit may be set also, but signifies nothing).  */
<<<<<<< HEAD
  unsigned int r_extern:1;  // 外部标志位。1 - 以符号的值重定位。0 - 以段的地址重定位。
  /* Four bits that aren't used, but when writing an object file
     it is desirable to clear them.  */
  unsigned int r_pad:4; // 没有使用的4个位，但最好将它们复位掉。
};

/* 外部标志位r_extern控制着r_symbolnum的含义，指明重定位项参考的是段
   还是一个符号。如果该标志值是0，那么该重定位项是一个普通的重定位项，
   此时r_symbolnum字段指定在哪个段中寻址定位。如果该标志是1，那么该重
   定位项是对一个外部符号的引用，此时r_symbolnum指定目标文件中符号表
   中的一个符号，需要使用符号的值进行重定位。 */

=======
  unsigned int r_extern:1;
  /* Four bits that aren't used, but when writing an object file
     it is desirable to clear them.  */
  unsigned int r_pad:4;
};
>>>>>>> 0f46eaaa6f9bc5248db3f909e59d9b30af2578a4
#endif /* no N_RELOCATION_INFO_DECLARED.  */


#endif /* __A_OUT_GNU_H__ */
