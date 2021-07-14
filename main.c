#include <stdio.h>
#include <stdlib.h>
#include "diag/trace.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#define CCM_RAM __attribute__((section(".ccmram")))

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
QueueHandle_t myqueue=0;
TimerHandle_t senderTimer=0;
TimerHandle_t receiverTimer=0;
SemaphoreHandle_t senderSem=0;
SemaphoreHandle_t receiverSem=0;

int sent=0,blocked=0,received=0;
int TsenderArray[6]={100,140,180,220,260,300};
int TsenderCount=0;
int Tsender;
char message_sent[50];
void receiverCallback(TimerHandle_t receiverTimer){
	//printf("ana receiver timer\n");
	if(received==500)
		init();
    else xSemaphoreGive(receiverSem);
}
void senderTask(void *s){
	//int xyz=xTaskGetTickCount();

	//sprintf(message_sent,"Time is %d",xyz);
	while(1){
		int xyz=xTaskGetTickCount();

		sprintf(message_sent,"Time is %d",xyz);
		if(xSemaphoreTake(senderSem,portMAX_DELAY)){
		if(xQueueSend(myqueue,message_sent,50)==pdPASS){
			sent+=1;
			//printf("sent successfully \n\r");
		}
		else blocked+=1;
		}
	}
}
void senderCallback(TimerHandle_t senderTimer){
	//printf("ana sender timer\n");
	xSemaphoreGive(senderSem);

}

void receiverTask(void *r){
	char message_re[50];
	//printf("I took the semaphore before\n");
	while(1){
		if(xSemaphoreTake(receiverSem,portMAX_DELAY)){
			//printf("I took the semaphore after\n");
		if(xQueueReceive(myqueue,message_re,50)==pdPASS){
			received+=1;
			//printf("I received %d\n",received);
		}
		//else printf("failed to receive\n\r");
		}
	}
}

void init(){

		printf("total number of successfully sent messages: %d\n\r",sent);
		printf("total number of blocked messages: %d\n\r",blocked);
	    //printf("total number of received messages: ",received);
		sent=0;
        blocked=0;
        received=0;
        xQueueReset(myqueue);
        xTimerChangePeriod(senderTimer ,pdMS_TO_TICKS(TsenderArray[TsenderCount]) , 1);
        TsenderCount+=1;
        xTimerReset(senderTimer,0);
        xTimerReset(receiverTimer,0);
        //Tsender=TsenderArray[TsenderCount];
        if(TsenderCount==7){
        	    printf("Game Over.\n\r");

        		xTimerDelete(senderTimer,0);
        		xTimerDelete(receiverTimer,0);

        		vTaskEndScheduler();
        }

}
int main(void){

	myqueue = xQueueCreate(20,sizeof(message_sent));

	xTaskCreate(senderTask,"sender",1000,NULL,1,NULL);
		xTaskCreate(receiverTask,"receiver",1000,NULL,2,NULL);


	vSemaphoreCreateBinary(senderSem);
	vSemaphoreCreateBinary(receiverSem);

	senderTimer=xTimerCreate(  "senderTimer",pdMS_TO_TICKS(TsenderArray[TsenderCount]), pdTRUE,( void * ) 0, senderCallback);
	receiverTimer=xTimerCreate(  "receiverTimer",pdMS_TO_TICKS(200), pdTRUE,( void * ) 0, receiverCallback);
	xTimerStart(senderTimer , 0 );
	xTimerStart(receiverTimer , 0 );

init();
	vTaskStartScheduler();
	return 0;
}

void vApplicationMallocFailedHook( void )
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	volatile size_t xFreeStackSpace;

		/* This function is called on each cycle of the idle task.  In this case it
		does nothing useful, other than report the amout of FreeRTOS heap that
		remains unallocated. */
		xFreeStackSpace = xPortGetFreeHeapSize();

		if( xFreeStackSpace > 100 )
		{
			/* By now, the kernel has allocated everything it is going to, so
			if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}

void vApplicationTickHook(void) {
}

StaticTask_t xIdleTaskTCB CCM_RAM;
StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE] CCM_RAM;

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
  /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
  state will be stored. */
	  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

	  /* Pass out the array that will be used as the Idle task's stack. */
	  *ppxIdleTaskStackBuffer = uxIdleTaskStack;

	  /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
	  Note that, as the array is necessarily of type StackType_t,
	  configMINIMAL_STACK_SIZE is specified in words, not bytes. */
	  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	}

static StaticTask_t xTimerTaskTCB CCM_RAM;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH] CCM_RAM;

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
