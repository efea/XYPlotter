#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>
#include <string>
#include "string.h"
#include <cstdlib>
#include "ITM_write.h"
#include <cr_section_macros.h>
#include "RIT.h"
#include "GCode.h"

#include "StepperMotor.h"
#include "Pin.h"

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

xSemaphoreHandle sbRIT;
volatile uint32_t RIT_count;
int pps = 400;
bool direction;

Pin xSw1 = Pin(0, 9);
Pin xSw2 = Pin(0, 29);
Pin xDir = Pin(1, 0);
Pin xMove = Pin(0, 24);

StepperMotor xMotor = StepperMotor(xSw1, xSw2, xDir, xMove);

Pin ySw1 = Pin(0, 0);
Pin ySw2 = Pin(1, 3);
Pin yDir = Pin(0, 28);
Pin yMove = Pin(0, 27);

StepperMotor yMotor = StepperMotor(ySw1, ySw2, yDir, yMove);

Pin tempSw1;
Pin tempSw2;
Pin tempMove;

Gcode parser;

void RIT_start(bool dir, int count, int us, Pin tempx, Pin tempy, Pin tempM){
	uint64_t cmp_value;

	direction = dir;
	tempSw1 = tempx;
	tempSw2 = tempy;
	tempMove = tempM;

	// Determine approximate compare value based on clock rate and passed interval
	cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us / 1000000;

	// disable timer during configuration
	Chip_RIT_Disable(LPC_RITIMER);

	RIT_count = count;
	// enable automatic clear on when compare value==timer value
	// this makes interrupts trigger periodically
	Chip_RIT_EnableCompClear(LPC_RITIMER);
	// reset the counter
	Chip_RIT_SetCounter(LPC_RITIMER, 0);
	Chip_RIT_SetCompareValue(LPC_RITIMER, cmp_value);
	// start counting
	Chip_RIT_Enable(LPC_RITIMER);
	// Enable the interrupt signal in NVIC (the interrupt controller)
	NVIC_EnableIRQ(RITIMER_IRQn);

	// wait for ISR to tell that we're done
	if(xSemaphoreTake(sbRIT, portMAX_DELAY) == pdTRUE) {
		// Disable the interrupt signal in NVIC (the interrupt controller)
		NVIC_DisableIRQ(RITIMER_IRQn);
	}
	else
	{
		// unexpected error
	}
}

extern "C" {
void RIT_IRQHandler(void){
	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	// Tell timer that we have processed the interrupt.
	// Timer then removes the IRQ until next match occurs
	Chip_RIT_ClearIntStatus(LPC_RITIMER);

	// clear IRQ flag
	if(RIT_count > 0) {
		RIT_count--;

		if(direction == true){
			if(!GPIO::readPin(tempSw2)){
				GPIO::setPin(tempMove, true);
				GPIO::setPin(tempMove, false);
			}else{
				Chip_RIT_Disable(LPC_RITIMER);
				// disable timer
				// Give semaphore and set context switch flag if a higher priority task was woken up
				xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
			}
		}else{
			if(!GPIO::readPin(tempSw1)){
				GPIO::setPin(tempMove, true);
				GPIO::setPin(tempMove, false);
			}else{
				Chip_RIT_Disable(LPC_RITIMER);
				// disable timer
				// Give semaphore and set context switch flag if a higher priority task was woken up
				xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
			}
		}
	}
	else
	{
		Chip_RIT_Disable(LPC_RITIMER);
		// disable timer
		// Give semaphore and set context switch flag if a higher priority task was woken up
		xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
	}
	// End the ISR and (possibly) do a context switch
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}
}

static void mainTask(void *pvParameters) {


	printf("Ready \r\n");

//		if(xMotor.calibrate()){
//
//		}else{
//
//			while(1){
//
//			}
//		}
//
//		if(yMotor.calibrate()){
//
//		}else{
//
//			while(1){
//
//			}
//		}

	// Move away from the edge.
	bool dir = false;               	// start calibrating in negative direction

	while (!GPIO::readPin(xSw2)){
		xMotor.move(dir, 200, 1);
	}

	//Start in positive direction
	dir = (bool)!dir;                           	// Invert direction

	// Move away from the edge.
	while (GPIO::readPin(xSw2)){
		xMotor.move(dir, 200, 1);
	}

	// Move safe distance 200 steps
	xMotor.move(dir, 1000, 200);


	dir = false;               	// start calibrating in negative direction

	while (!GPIO::readPin(ySw2)){
		yMotor.move(dir, 200, 1);
	}

	//Start in positive direction
	dir = (bool)!dir;                           	// Invert direction

	// Move away from the edge.
	while (GPIO::readPin(ySw2)){
		yMotor.move(dir, 200, 1);
	}

	// Move safe distance 200 steps
	yMotor.move(dir, 1000, 200);

	while(1){
		parser.readfromUart();
	}

}

int main(void) {

#if defined (__USE_LPCOPEN)
	// Read clock settings and update SystemCoreClock variable
	SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
	// Set up and initialize all required blocks and
	// functions related to the board hardware
	Board_Init();
	// Set the LED to the state of "On"
#endif
#endif

	sbRIT = xSemaphoreCreateBinary();

	Chip_RIT_Init(LPC_RITIMER);
	NVIC_SetPriority(RITIMER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1 );

	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 17, IOCON_DIGMODE_EN | IOCON_MODE_PULLUP);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 17);

	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 11, IOCON_DIGMODE_EN | IOCON_MODE_PULLUP);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 1, 11);

	GPIO::initiate(xSw1, true);
	GPIO::initiate(xSw2, true);

	GPIO::initiate(xDir, false);
	GPIO::initiate(xMove, false);

	GPIO::initiate(ySw1, true);
	GPIO::initiate(ySw2, true);

	GPIO::initiate(yDir, false);
	GPIO::initiate(yMove, false);



	//yMotor = StepperMotor(ySw1, ySw2, yDir, yMove);

	//xMotor.calibrate();

	// Main task
	xTaskCreate(mainTask, "mainTask",
			configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	// Force the counter to be placed into memory
	volatile static int i = 0 ;
	// Enter an infinite loop, just incrementing a counter
	while(1) {
		i++ ;
	}
	return 0 ;
}


