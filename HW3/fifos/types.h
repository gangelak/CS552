#define FALSE 0
#define TRUE 1

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned long int uint32;
typedef unsigned long long int uint64;

typedef signed char sint8, s8;
typedef signed short int sint16, s16;
typedef signed long int sint32, s32;
typedef signed long long int sint64, s64;



#ifndef _SIZE_T
typedef int size_t;
#define _SIZE_T 1
#endif

typedef signed char bool;

typedef unsigned long uint;
typedef signed long sint;

#ifndef _STDINT_
#define _STDINT_
typedef uint8 uint8_t;
typedef uint16 uint16_t;
typedef uint32 uint32_t;
typedef uint64 uint64_t;
#endif

#define MAX_THREADS 2




/* PCB for a process/thread */
#ifndef _PCB
#define _PCB
struct context{
	uint16_t ds;
	uint16_t es;
	uint16_t fs;
	uint16_t gs;
	uint16_t flags;
	uint32_t edi;
	uint32_t esi;
	uint32_t ebx;
	uint32_t ebp;
	uint32_t eip;
};

struct proc_crtl_block{
	int tid;
	uint32_t bp;
	int (*entry)();
	int status;
	struct proc_crtl_block *next;
	struct proc_crtl_block *prev;
	uint32_t sp;
	struct context *ctx;
};


typedef struct proc_crtl_block pcb;

#endif
