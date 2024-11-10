#include "stm32f4xx_hal.h"
#include <stdbool.h>

#define SHPR2 *(uint32_t*)0xE000ED1C //for setting SVC priority, bits 31-24
#define SHPR3 *(uint32_t*)0xE000ED20 // PendSV is bits 23-16
#define _ICSR *(uint32_t*)0xE000ED04 //This lets us trigger PendSV

typedef void (*ThreadFunction)(void*);
typedef struct k_thread{
	uint32_t* sp;
	uint32_t thread_runtime;
	uint32_t deadline;
	ThreadFunction thread_function;
}thread;

#define RUN_FIRST_THREAD 0x3
#define THREAD_MAX_RUN_TIME 50
#define STACK_SIZE 0x200
#define MAX_SIZE 31
#define TOTAL_STACK_SIZE 0x4000
#define YIELD 0x8

extern void runFirstThread (void);
int __io_putchar(int ch);
void setupThreadStack(thread* newThread, void* param);
void SVC_Handler_Main( unsigned int *svc_args );
uint32_t* allocateStack(void);
bool osCreateThread(ThreadFunction func, void* param);
void osKernelInitialize(void);
void osKernelStart(void);
void osSched(void);
void osYield(void);

