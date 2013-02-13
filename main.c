#include  <msp430g2553.h>
#include  "RTC.h"
#include  "main.h"

#define TRUE 0x01
#define FALSE 0x00
#define AM 0x00
#define PM 0x01
#define BUZZER_KILL_TIMER_START 600 // 10minutes in seconds / number of seconds we'll allow the buzzer to remain buzzing before we stop it
#define BUZZER_TOGGLE_INTERVAL 0x04 // number of seconds between sounding buzzer when alarm is going off
#define BTN_PIN BIT6
#define BUZZER_PIN BIT0

unsigned int ALARM_SNOOZE_HRS = 1; // used when buzzer is timer runs out and we want to set the next buzzer for a time in the nearer future
unsigned int START_HOUR = 0; // [INIT IN MAIN] the hour to initially trigger the alarm if the nebulizer hasn't be used today
unsigned int ALARM_INTERVAL = 0; // [INIT IN MAIN] in hours / how often the alarm will sound the buzzer
unsigned int LATEST_ALARM_SET_HOUR = 0; // [INIT IN MAIN] the latest we'll worry about setting alarms
unsigned int nextAlarmHr = 0; // [INIT IN MAIN]
int buzzerTimer = -1;
char buttonPressedBeforeStartHr = FALSE;
char buzzerToggleCounter = 0x0;
int testCounter = 0;
inline void setNextAlarmCustom(unsigned int nextAlarmOffset) {
	char crntHr = get24Hour();
	nextAlarmOffset = _bcd_add_short(0, nextAlarmOffset); // convert nextAlarmOffset to BCD
	unsigned int bcdNextAlarmHour = _bcd_add_short(crntHr, nextAlarmOffset);

	if ( bcdNextAlarmHour > LATEST_ALARM_SET_HOUR || ( bcdNextAlarmHour < START_HOUR && !buttonPressedBeforeStartHr ) ) {
		// if the next alarm would be too late or too early, just set it to the START_HOUR
		nextAlarmHr = START_HOUR;
	    buttonPressedBeforeStartHr = FALSE; // just in case the buzzer was pressed before the start hour, we'll reset buttonPressedBeforeStartHr to false
	} else {
		nextAlarmHr = bcdNextAlarmHour;
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
        #ifndef TEST_MODE
	// tick the RTC one second forward
	incrementSeconds();
        #endif
	#ifdef TEST_MODE
	if ( testCounter == 150 ) {
		incrementHours();
		testCounter = 0;
	}
	testCounter++;
	#endif

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
	    setNextAlarmCustom(ALARM_SNOOZE_HRS);
	} else if ( get24Hour() == nextAlarmHr ) {
	    // time for the buzzer to buzz
	    turnBuzzerOn();
	}
}

// ON button pressed interrupt
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  P1IFG &= ~BTN_PIN; // kill buzzer interrupt flag cleared
  // NOTE: Not sure I need to check buttonPressedBeforeStartHr on the next line
  if ( buzzerTimer >= 0 || (buttonPressedBeforeStartHr == FALSE && get24Hour() < START_HOUR)) { // only worry about doing something if the buzzer is sounding or if the START_HOUR alarm hasn't gone off yet today
    if (buzzerTimer < 0) { // another way of saying: if (get24Hour() < START_HOUR) ...
      buttonPressedBeforeStartHr = TRUE;
    }
    turnBuzzerOff();
    setNextAlarm();
  }
}

inline unsigned int convertHourToBinary(unsigned int n) {
        unsigned int result = 0;
        result = _bcd_add_short(result, (n & 0x0007));
        if(n & 0x0008)
                result = _bcd_add_short(result, 0x0008);
        if(n & 0x0010)
                result = _bcd_add_short(result, 0x0016);
        if(n & 0x0020)
                result = _bcd_add_short(result, 0x0032);
        if(n & 0x0040)
                result = _bcd_add_short(result, 0x0064);
        if(n & 0x0080)
                result = _bcd_add_short(result, 0x0128);
        if(n & 0x0100)
                result = _bcd_add_short(result, 0x0256);
        if(n & 0x0200)
                result = _bcd_add_short(result, 0x0512);
        return result;
}

void main ( void )
{
	WDTCTL = WDTPW + WDTHOLD;   // Stop watchdog timer
	setTime(0x09,0x0,0x0,AM);  // initialize time to 9am
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

  START_HOUR = convertHourToBinary(11); // the hour to initially trigger the alarm if the nebulizer hasn't be used today
  ALARM_INTERVAL = convertHourToBinary(4); // in hours / how often the alarm will sound the buzzer
  LATEST_ALARM_SET_HOUR  = convertHourToBinary(19); // the latest we'll worry about setting alarms
  ALARM_SNOOZE_HRS = convertHourToBinary(1);
  nextAlarmHr = START_HOUR;

  if ( get24Hour() >= nextAlarmHr ) {
    // if the time was initialized to a time in the past, set the next alarm hour immediately.
    setNextAlarm();
  }
  turnBuzzerOff(); //initialize buzzer state
  __bis_SR_register(GIE + LPM3_bits); // Enable all interrupts and go into LPM3

  while(1); // The while loop prevents the cpu from stepping into instructions outside the scope of this program
}

