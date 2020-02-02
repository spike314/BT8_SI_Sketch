/*
 Name:		MySensorsNRF5setup.h
 Created:	2/5/2018 9:40:20 PM
 
 Routines for use with MySensors and NRF5 based controllers originally developed by NeverDie
 See sketch here:  https://www.openhardware.io/view/499/10-years-wireless-PIR-Sensor-on-just-one-set-of-3-AAs#tabs-source
 Also, background in the forum here: https://forum.mysensors.org/topic/6961/nrf5-action?_=1580654209821

*/

// If using MY_DEBUG for serial prints, declare #define MY_DEBUG before including this file
// in order to prevent sleeping the UART

#ifdef LED_BUILTIN
void blinkityBlink(uint8_t pulses, uint8_t repetitions) {
	for (int x = 0; x<repetitions; x++) {
		for (int i = 0; i<pulses; i++) {
			digitalWrite(LED_BUILTIN, HIGH);
			wait(20);
			digitalWrite(LED_BUILTIN, LOW);
			wait(100);
		}
		wait(500);
	}
}

void blinkityBlink(uint8_t repetitions) {
	for (int x = 0; x<repetitions; x++) {
		digitalWrite(LED_BUILTIN, HIGH);
		wait(20);
		digitalWrite(LED_BUILTIN, LOW);
		wait(100);
		digitalWrite(LED_BUILTIN, HIGH);
		wait(20);
		digitalWrite(LED_BUILTIN, LOW);
		if (x<(repetitions - 1)) {  //skip waiting at the end of the final repetition
			wait(500);
		}
	}
}
#endif


void disableNfc() {  //only applied to nRF52
#ifndef IS_NRF51
	//Make pins 9 and 10 usable as GPIO pins.
	NRF_NFCT->TASKS_DISABLE = 1;  //disable NFC
	NRF_NVMC->CONFIG = 1;  // Write enable the UICR
	NRF_UICR->NFCPINS = 0; //Make pins 9 and 10 usable as GPIO pins.
	NRF_NVMC->CONFIG = 0;  // Put the UICR back into read-only mode.
#endif
}

void turnOffRadio() {
	NRF_RADIO->TASKS_DISABLE = 1;
	while (!(NRF_RADIO->EVENTS_DISABLED)) {}  //until radio is confirmed disabled
}

void turnOffUarte0() {
#ifndef IS_NRF51  
	NRF_UARTE0->TASKS_STOPRX = 1;
	NRF_UARTE0->TASKS_STOPTX = 1;
//	NRF_UARTE0->TASKS_SUSPEND = 1;
	NRF_UARTE0->ENABLE = 0;  //disable UART0
	while (NRF_UARTE0->ENABLE != 0) {};  //wait until UART0 is confirmed disabled.
#endif

#ifdef IS_NRF51
	NRF_UART0->TASKS_STOPRX = 1;
	NRF_UART0->TASKS_STOPTX = 1;
	NRF_UART0->TASKS_SUSPEND = 1;
	NRF_UART0->ENABLE = 0;  //disable UART0
	while (NRF_UART0->ENABLE != 0) {};  //wait until UART0 is confirmed disabled.
#endif
}

void turnOffAdc() {
#ifndef IS_NRF51
	if (NRF_SAADC->ENABLE) { //if enabled, then disable the SAADC
		NRF_SAADC->TASKS_STOP = 1;
		while (NRF_SAADC->EVENTS_STOPPED) {} //wait until stopping of SAADC is confirmed
		NRF_SAADC->ENABLE = 0;  //disable the SAADC
		while (NRF_SAADC->ENABLE) {} //wait until the disable is confirmed
	}
#endif
}


void turnOffHighFrequencyClock() {
	NRF_CLOCK->TASKS_HFCLKSTOP = 1;
	while ((NRF_CLOCK->HFCLKSTAT) & 0x0100) {}  //wait as long as HF clock is still running.
}


void mySleepPrepare() {  //turn-off energy drains prior to sleep
	turnOffHighFrequencyClock();
	turnOffRadio();
#ifndef MY_DEBUG
	turnOffUarte0();
#endif
}


void mySleep(uint32_t ms) {
	mySleepPrepare();  //Take steps to reduce drains on battery current prior to sleeping
	sleep(ms);
}

void activateLpComp(uint8_t AIN_num) {
  NRF_LPCOMP->PSEL = AIN_num; // monitor AIN0 to AIN7 nRF52832 for interrupt).
  while (!(NRF_LPCOMP->PSEL == AIN_num)) {} //wait until confirmed
  NRF_LPCOMP->REFSEL=3;  // choose 1/2 VDD as the reference voltage
  while (!(NRF_LPCOMP->REFSEL==3)) {} //wait until confirmed
  NRF_LPCOMP->ANADETECT = AIN_num;  //detect CROSS events on desired interrupt pin
  while (NRF_LPCOMP->ANADETECT != AIN_num) {} //wait until confirmed
  NRF_LPCOMP->INTENSET=B1000;  //Enable interrupt for CROSS event
  while (!(((NRF_LPCOMP->INTENSET)&B1000)==B1000)) {} //wait until confirmed
  NRF_LPCOMP->ENABLE=1;  //Enable LPCOMP
  while (!(NRF_LPCOMP->ENABLE==1)) {} //wait until confirmed
  NRF_LPCOMP->TASKS_START=1;  //start the LPCOMP
  while (!(NRF_LPCOMP->EVENTS_READY)) {}  //wait until ready
  
  NVIC_SetPriority(LPCOMP_IRQn, 15);
  NVIC_ClearPendingIRQ(LPCOMP_IRQn);
  NVIC_EnableIRQ(LPCOMP_IRQn);
}

void suspendLpComp() { //suspend getting more interrupts from LPCOMP before the first interrupt can be handled
  if ((NRF_LPCOMP->ENABLE) && (NRF_LPCOMP->EVENTS_READY)) {  //if LPCOMP is enabled
    NRF_LPCOMP->INTENCLR=B0100;  //disable interrupt from LPCOMP
    while (((NRF_LPCOMP->INTENCLR)&B0100)==B0100) {} //wait until confirmed
  }
}

void resumeLpComp() { //suspend getting interrupts from LPCOMP
  NRF_LPCOMP->INTENSET=B0100;  //Enable interrupt for UP event
  while (((NRF_LPCOMP->INTENSET)&B1000)!=B0100) {} //wait until confirmed
}

// * Reset events and read back on nRF52
//* http://infocenter.nordicsemi.com/pdf/nRF52_Series_Migration_v1.0.pdf

#if __CORTEX_M == 0x04
#define NRF5_RESET_EVENT(event)                                                 \
        event = 0;                                                                   \
        (void)event
#else
#define NRF5_RESET_EVENT(event) event = 0
#endif


