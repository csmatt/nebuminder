#include  <msp430g2553.h>
#include  "RTC.h"

#define TRUE 0x01
#define FALSE 0x00
#define ALARM_INTERVAL 0x04 // in hours / how often the alarm will sound the buzzer
#define BUZZER_KILL_TIMER_START 600 // 10minutes in seconds / number of seconds we'll allow the buzzer to remain buzzing before we stop it
#define START_HOUR 0x11 // the hour to initially trigger the alarm if the nebulizer hasn't be used today
#define START_PM 0x00
#define BUZZER_TOGGLE_INTERVAL 0x04 // number of seconds between sounding buzzer when alarm is going off
#define AM 0x00
#define PM 0x01
#define ALARM_OFFSET_HRS 0x01 // used when buzzer is timer runs out and we want to set the next buzzer for a time in the nearer future
#define BTN_PIN BIT6
#define BUZZER_PIN BIT0
#define LATEST_ALARM_SET_PM 0x01

const char LATEST_ALARM_SET_HOUR  = 0x11 - ALARM_INTERVAL; // the latest we'll worry about setting alarms
const char bcdHourLookupTable[] = {0x0,0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23};

int buzzerTimer = -1;
char nextAlarmHr = START_HOUR;
char nextAlarmPM = START_PM;
char buttonPressedBeforeStartHr = FALSE;
char buzzerToggleCounter = 0x0;

inline void setNextAlarmCustom(char nextAlarmOffset) {
  char crntHr = TI_hour;
  char crntPM = TI_PM;
  char resetToStartHour = FALSE;
  nextAlarmPM = crntPM;

  // This block sets resetToStartHour just to make things a little easier to understand (as opposed to having one big if-statement)
  if (crntHr == 0x12 || crntHr == 0x00) { // special case to handle 12am and 12pm
	  // NOTE: Don't combine these two if-statements. They're like this so that 0x12 gets swallowed
    if (crntHr == 0) { // if we're at 12am, reset to start hour (otherwise we're at 12pm and don't need to reset)
      resetToStartHour = TRUE;
    }
  } else if ((crntPM == PM && crntHr > LATEST_ALARM_SET_HOUR) || (buttonPressedBeforeStartHr == FALSE && crntPM == START_PM && crntHr < START_HOUR)) {
    resetToStartHour = TRUE;
  }
  if (buttonPressedBeforeStartHr == TRUE && crntPM == START_PM && crntHr >= START_HOUR) {
	  buttonPressedBeforeStartHr = FALSE;
  }
  if (resetToStartHour) {
    nextAlarmHr = START_HOUR;
    nextAlarmPM = START_PM;
  } else {
    // we'll deal with waking hour alarm setting here
    nextAlarmHr = crntHr + ALARM_INTERVAL;
    if (nextAlarmHr >= 12) { // since nextAlarmHr was calculated by merely adding ALARM_INTERVAL to crntHr, we know that anything >= 12 should be set to PM
      nextAlarmPM = PM;
      if (nextAlarmHr != 0x12) { // if we're > 12, subtract 12 to get the 12hr version of the nextAlarmHr
    	  nextAlarmHr -= 0x12;
      }
    }
    nextAlarmHr = bcdHourLookupTable[nextAlarmHr];
  }
}

inline void setNextAlarm() {
  setNextAlarmCustom(ALARM_INTERVAL);
}

inline void turnBuzzerOn() {
  P1OUT |= BUZZER_PIN;
  buzzerTimer = BUZZER_KILL_TIMER_START;
}

inline void turnBuzzerOff() {
  P1OUT &= ~BUZZER_PIN;
  buzzerTimer = -1;
}

// Timer0 A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
  // tick the RTC one second forward
  incrementSeconds();
  if ( buzzerTimer > 0 ) {
    // if the buzzer is currently buzzing
    buzzerTimer -= 1; // subtract a second from the buzzerTimer
    // toggle buzzer pin to create beeping effect
    if (buzzerToggleCounter == BUZZER_TOGGLE_INTERVAL) {
      P1OUT |= BUZZER_PIN;
      buzzerToggleCounter = 0x0;
    } else {
      buzzerToggleCounter++;
      P1OUT &= ~BUZZER_PIN;
    }
  } else if ( buzzerTimer == 0 ) {
    // buzzerTimer has run out, we need to turn it off
    turnBuzzerOff();
    // we'll set it for a time in the near future;
    setNextAlarmCustom(ALARM_OFFSET_HRS);
  } else if ( TI_hour == nextAlarmHr && TI_PM == nextAlarmPM ) {
    // time for the buzzer to buzz
    turnBuzzerOn();
  }
}

// ON button pressed interrupt
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  P1IFG &= ~BTN_PIN; // kill buzzer interrupt flag cleared

  if ( buzzerTimer >= 0 || (buttonPressedBeforeStartHr == FALSE && TI_PM == START_PM && TI_hour < START_HOUR)) { // only worry about doing something if the buzzer is sounding or if the START_HOUR alarm hasn't gone off yet today
    if (buzzerTimer < 0) { // another way of saying: if (TI_PM == START_PM && TI_HOUR < START_HOUR) ...
      buttonPressedBeforeStartHr = TRUE;
    }
    turnBuzzerOff();
    setNextAlarm();
  }
}

void main ( void )
{
  WDTCTL = WDTPW + WDTHOLD;   // Stop watchdog timer
  setTime(0x09,0x0,0x0,0);  // initialize time to 9am
  TACCR0 = 32768-1;
  TACTL = TASSEL_1+MC_1;  	// ACLK, upmode
  TACCTL0 |= CCIE;        	// enable TA0CCRO interrupt

  // set up buzzer pin on 1.0
  // set up interrupt for ON button press via BTN_PIN
  P1DIR |= BUZZER_PIN; // Set BUZZER_PIN to output and BTN_PIN to input direction
  P1OUT &= ~BUZZER_PIN; // set BUZZER_PIN to Off
  //P1REN |= BTN_PIN; // enable internal pull-up resistor
  P1IFG &= ~BTN_PIN; // BTN_PIN interrupt flag cleared
  P1IE |= BTN_PIN; // BTN_PIN interrupt enabled

  // turn off all unused ports
  P2DIR = 0xFF;
  P2OUT = 0x00;
  P3DIR = 0xFF;
  P3OUT = 0x00;

  if ( TI_hour >= nextAlarmHr && TI_PM == nextAlarmPM ) {
    // if the time was initialized to a time in the past, set the next alarm hour immediately.
    setNextAlarm();
  }
  turnBuzzerOff(); //initialize buzzer state
  __bis_SR_register(GIE + LPM3_bits); // Enable all interrupts and go into LPM3

  while(1); // The while loop prevents the cpu from stepping into instructions outside the scope of this program
}
