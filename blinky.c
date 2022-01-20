/*
 * blinky.c -- working with Serial I/O and GPIO
 *
 * Assumes the LED's are connected to AXI_GPIO_0, on channel 1
 *
 * Terminal Settings:
 *  -Baud: 115200
 *  -Data bits: 8
 *  -Parity: no
 *  -Stop bits: 1
 */
#include <stdio.h>							/* printf(), getchar() */
#include "xil_types.h"					/* u32, u16 etc */
#include "platform.h"						/* ZYBOboard interface */
#include <xgpio.h>							/* Xilinx GPIO functions */
#include "xparameters.h"				/* constants used by the hardware */
#include <string.h>
#include <stdlib.h>
#include "led.h"
#include "gic.h"		/* interrupt controller interface */
#include "io.h"
#include "xttcps.h"


#define OUTPUT 0x0							/* setting GPIO direction to output */
#define CHANNEL1 1							/* channel 1 of the GPIO port */

/* led states */
#define LED_ON true
#define LED_OFF false

static XTtcPs ttcPs;

void getLine (char *str);
void callback(u32 led_num){
	led_toggle(led_num);
}

void TTC_callback(void *devicep){
	//printf("Inside Handler\n\r");
	XTtcPs *dev = (XTtcPs*)devicep;
	led_toggle(4);
	XTtcPs_ClearInterruptStatus(dev, XTTCPS_IXR_INTERVAL_MASK);
}

int main() {
	led_init();
	s32 answer = gic_init(); /* initialize the gic (c.f. gic.h) */

	if (answer == 0){
		io_btn_init(callback);
		io_sw_init(callback);
	}

	XTtcPs_Config * ConfigPtr;
	ConfigPtr = XTtcPs_LookupConfig(XPAR_XTTCPS_0_DEVICE_ID);
	u32 outcome = XTtcPs_CfgInitialize(&ttcPs, ConfigPtr, ConfigPtr->BaseAddress);
	if (outcome == 0){
		u32 freq = 1;
		XInterval interval;
		u8 prescaler;
		printf("Before calcinterval\n\r");
		XTtcPs_CalcIntervalFromFreq(&ttcPs, freq, &interval, &prescaler);
		printf("After calcinterval\n\r");
		XTtcPs_SetPrescaler(&ttcPs, prescaler);
		XTtcPs_SetInterval(&ttcPs, interval);
		s32 outcome1 = XTtcPs_SetOptions(&ttcPs, XTTCPS_OPTION_INTERVAL_MODE);
		if (outcome1 == 0){
			XTtcPs_DisableInterrupts(&ttcPs, XTTCPS_IXR_INTERVAL_MASK);
			s32 outcome2 = gic_connect(XPAR_XTTCPS_0_INTR, TTC_callback, &ttcPs);
			if (outcome2 == 0){
				XTtcPs_EnableInterrupts(&ttcPs, XTTCPS_IXR_INTERVAL_MASK);
				XTtcPs_Start(&ttcPs);
			}

		}

	}



   int buff = 80;
   char str[buff];

	 /*
		* set stdin unbuffered, forcing getchar to return immediately when
		* a character is typed.
		*/
	 setvbuf(stdin,NULL,_IONBF,0);

	 printf("[Hello]\n");
	 led_set(4, LED_ON, 0);

	 led_set(0, LED_ON, 0);


	 int value = 0;
	 bool ans;

	 do {
		 if (strcmp(str, "0") == 0 || strcmp(str, "1") == 0 || strcmp(str, "2") == 0 || strcmp(str, "3") == 0){
			 //printf("[%s]\n\r", str);
			 if (strcmp(str, "0") == 0){
				 led_toggle(0);
				 ans = led_get(0);
			 } else if (strcmp(str, "1") == 0){
				 led_toggle(1);
				 ans = led_get(1);
			 } else if (strcmp(str, "2") == 0){
				 led_toggle(2);
				 ans = led_get(2);
			 } else if (strcmp(str, "3") == 0){
				 led_toggle(3);
				 ans = led_get(3);
			 }

			 if (ans == 1){
				 printf("[%s on]\n\r", str);
			 } else printf("[%s off]\n\r", str);

			 fflush(stdout);

		 }

		 if (strcmp(str, "r") == 0 || strcmp(str, "b") == 0 || strcmp(str, "g") == 0 || strcmp(str, "y") == 0){
			 if (strcmp(str, "r") == 0){
				 led_set(6, LED_OFF, 4);
			 } else if (strcmp(str, "b") == 0){
				 led_set(6, LED_OFF, 1);
			 } else if (strcmp(str, "g") == 0){
				 led_set(6, LED_OFF, 2);
			 } else if (strcmp(str, "y") == 0){
				 led_set(6, LED_OFF, 6);
			 }
		 }
		 printf(">");
		 fflush(stdout);
		 getLine(str);
		 printf("\n");
		 fflush(stdout);
		 value = strcmp(str, "q");
	 } while (value != 0);

	 led_set(0, LED_OFF, 0);
	 led_set(4, LED_OFF, 0);
	 led_set(6, LED_OFF, 0);

	  io_btn_close();
	  io_sw_close();
	  XTtcPs_Stop(&ttcPs);
	  gic_close();

   cleanup_platform();					/* cleanup the hardware platform */
   return 0;
}

void getLine(char * str){
	char c;
	int i = 0;

	c = getchar();
	printf("%c", c);
	fflush(stdout);

	while ( c != 13 ){
		str[i] = c;
		i++;
		c = getchar();
		printf("%c", c);
		fflush(stdout);
	}
	str[i] = '\0';
}
