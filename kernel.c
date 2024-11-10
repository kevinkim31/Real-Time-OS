#include "kernel.h"
#include<stdio.h>


uint32_t* MSP_INIT_VAL;
uint32_t currentstackint;
uint32_t* currentstack;
thread threadArray[MAX_SIZE];
uint32_t threadnumber = 0;
uint32_t currentThread = 0;
uint32_t kernelStarted = 0;

void setupThreadStack(thread* newThread, void* param) {

	  *(--newThread->sp) = 1<<24;
	  *(--newThread->sp) = (uint32_t)newThread->thread_function;

	  *(--newThread->sp) = 0xA;
	  *(--newThread->sp) = 0xA;
	  *(--newThread->sp) = 0xA;
	  *(--newThread->sp) = 0xA;
	  *(--newThread->sp) = 0xA;
	  *(--newThread->sp) = (uint32_t)param;
	  *(--newThread->sp) = 0xA;
	  *(--newThread->sp) = 0xA;
	  *(--newThread->sp) = 0xA;
	  *(--newThread->sp) = 0xA;
	  *(--newThread->sp) = 0xA;
	  *(--newThread->sp) = 0xA;
	  *(--newThread->sp) = 0xA;
	  *(--newThread->sp) = 0xA;
}

void SVC_Handler_Main( unsigned int *svc_args )
{
	unsigned int svc_number;

	svc_number = ( ( char * )svc_args[ 6 ] )[ -2 ] ;
	switch( svc_number )
	{
		case RUN_FIRST_THREAD:
			__set_PSP(threadArray[currentThread].sp);
			runFirstThread();
			break;
		case YIELD:
			_ICSR |= 1<<28;
			__asm("isb");
			break;
		default: /* unknown SVC */
			break;
	}
}

uint32_t* allocateStack(void) {
	currentstackint = currentstackint - STACK_SIZE; // get integer address of next thread stack

	if (((uint32_t)MSP_INIT_VAL - currentstackint) <= (TOTAL_STACK_SIZE - STACK_SIZE)) {
		currentstack = (uint32_t*)currentstackint; // if there is enough space then allocate and return pointer to next thread stack address
		return currentstack;
	}

	return NULL;
}

// OS chooses thread runtime (with THREAD_MAX_RUN_TIME)
bool osCreateThread(ThreadFunction func, void* param) {

	uint32_t* tempStack = allocateStack(); // pointer to new thread stack

	if (tempStack != NULL) { // if stack address exists
		threadArray[threadnumber].thread_function = func; // thread_function points to thread function (TCB)
		threadArray[threadnumber].thread_runtime = THREAD_MAX_RUN_TIME; // thread max runtime in ms
		threadArray[threadnumber].deadline = THREAD_MAX_RUN_TIME; // thread max runtime in ms
		threadArray[threadnumber].sp = tempStack; // sp points to new thread stack (TCB)
		setupThreadStack(&threadArray[threadnumber], param); // push register values into thread stack
		threadnumber ++; // increment threadnumber so that new thread gets stored in new index of TCB array
		return true;
	}
	return false;
}

// allows user to set thread runtime
bool osCreateThreadWithDeadline(ThreadFunction func, void* param, uint32_t thread_runtime) {

	uint32_t* tempStack = allocateStack(); // pointer to new thread stack

	if (tempStack != NULL) { // if stack address exists
		threadArray[threadnumber].thread_function = func; // thread_function points to thread function (TCB)
		threadArray[threadnumber].thread_runtime = thread_runtime; // thread max runtime in ms
		threadArray[threadnumber].deadline = thread_runtime; // thread max runtime in ms
		threadArray[threadnumber].sp = tempStack; // sp points to new thread stack (TCB)
		setupThreadStack(&threadArray[threadnumber], param); // push register values into thread stack
		threadnumber ++; // increment threadnumber so that new thread gets stored in new index of TCB array
		return true;
	}
	return false;
}



void osKernelInitialize(void) {
	//set the priority of PendSV to almost the weakest
	SHPR3 |= 0xFE << 16; //set priority of PendSV to almost the weakest
	SHPR3 |= 0xFFU << 24; // set priority of SysTick to be weakest
	SHPR2 |= 0xFdU << 24; //Set the priority of SVC highest of the three



	MSP_INIT_VAL = *(uint32_t**)0x0;
	currentstackint = (uint32_t)MSP_INIT_VAL; // integer address of main stack
	kernelStarted = 1; // indicates that kernel has started
}

void osKernelStart(void) {
	__set_PSP(threadArray[0].sp); // set PSP to sp of first thread
	__asm("SVC #3");
}

void osSched(void) {
	threadArray[currentThread].sp = (uint32_t*)(__get_PSP() - 8*4);
	currentThread = (currentThread + 1) % threadnumber;
	__set_PSP(threadArray[currentThread].sp);

}

void osYield(void) {
	__asm("SVC #8");
}
