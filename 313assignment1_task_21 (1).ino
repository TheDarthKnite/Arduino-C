#include <util/delay.h>
#include <avr/io.h>

//sets initial state(not in configuration mode), initial mode(1), and timerOverflowCounter
  	volatile uint8_t State = 1;
  	volatile uint8_t Mode = 1;
	volatile uint8_t TimerCount = 0;
	int adcValue = 0;
	//volatile uint16_t topValue = 15625 / (2*Mode);
	volatile uint8_t Red = 0;
	volatile uint8_t LoopCount = 0;
//interupt service routine for when first button is pressed
ISR(INT0_vect) {
  if(State == 0) {
    //exit config mode
    State = 1;
  } else {  
    //enter config mode
  	State = 0;
  }
}
//sets ADC value
ISR(ADC_vect) {
  adcValue = ADC;
}
//runs an interupt logging
ISR(TIMER1_COMPA_vect){
  TimerCount += 1;
}

int main(void){
  //setup
  //disables interupts
  	cli();
  //ADC setup
    ADMUX = 0x0;
    ADMUX |= (1<<REFS0); //set adc reference voltage to the 5V input
    ADCSRA = 0x00;
    ADCSRA |= (1<<ADEN) | (1<<ADIE); //enable adc and completion interrupt flag
  
  //sets all necessary port b DDRs for the LEDs
	DDRB |= (1<<DDB0);
	DDRB |= (1<<DDB1);
 	DDRB |= (1<<DDB2);
	DDRB |= (1<<DDB3);
  
  //sets input for switch
	DDRD &= ~(1<<DDD2);
  
  //set ADC input from potentiometer
  	DDRC &= ~(1<<DDC0);
  
  //sets EICRA and EIMSK
  	EICRA |= (1<<ISC01)|(1<<ISC00); //detect rising edge
 	EIMSK |= (1<<INT0); //enable external interrupts on pin INT0
  
  //sets counter and control registers for timer/counter1
  	TCCR1A = 0;
  	TCCR1B = 0;
  	TCNT1 = 0;
  
  //sets value for timer 1 to compare to
  	OCR1A = 15625 / (2*Mode);
  
  //prescale to 1024 + enable CTC mode
  	TCCR1B |=(1<<CS12)|(1<<CS10)| (1<<WGM12);
  
  //enable time compare interrupt 
  	TIMSK1 |= (1 << OCIE1A);
  
  //enables interupts
  	sei();
  
  //set timer values to 0
    TCNT1 = 0;
    TimerCount = 0;
  
  //starts loop
  while(1){
    _delay_ms(1);
    //config mode should be activated on the next red light
    //or anytime within the red light.
    if (Red == 0 || State == 1) {
    //turn on White LED
    	PORTB |= (1<<PORTB0);
  	//turns off Green LED and turns Red LED on for Mode amount of seconds
      	if (TimerCount<2*Mode*Mode){
        	PORTB &= ~(1<<PORTB2);
      	 	PORTB |= (1<<PORTB3);
          	//red light is on 
            Red = 1;
        }
      
    //turns off Red LED and turns Green LED on for Mode amount of seconds
      	else if (TimerCount<4*Mode*Mode){
          	PORTB &= ~(1<<PORTB3);
          //Red Light is off
            Red = 0;
          
      		PORTB |= (1<<PORTB1);  
        }
    //turns off Green LED and turns Yellow LED on for Mode amount of seconds
      	else if (TimerCount<6*Mode*Mode){
          PORTB &= ~(1<<PORTB1);
          PORTB |= (1<<PORTB2);
        }
      	else{
      	//set timer values to 0
    	  TCNT1 = 0;
    	  TimerCount = 0; 
      	}
    }
  	//configuration mode
    else{
      ConfigLights();
    }
  }
}


void ConfigLights(){
  //check potentiometer value by polling to determine config mode 
  ADCSRA |= (1<<ADSC); //start adc conversion
  
  //sets mode according to the ADC input
  Mode = (adcValue/256) + 1;
  
  //sets value for timer 1 to compare to
  OCR1A = 15625 / (2*Mode);
  
  //turns light on and off for 500ms/Mode each
  
  if(LoopCount<Mode){
    //alternates between on and off every 500ms/Mode
    if (TimerCount == 0){
      PORTB |= (1<<PORTB0);
    }else if(TimerCount==1){
      PORTB &= ~(1<<PORTB0);
    }else{
      //counts amount of times it has looped, and sets timer Count to 0
      LoopCount++;
      TimerCount=0;
    }
  } else{
    //keeps light off for 4 seconds, 
    if(TimerCount<=(4*Mode)){
    PORTB &= ~(1<<PORTB0);
    }else{
      //when the 2 second are over reset timer count and Loop count
      //to start blinking again
       LoopCount=0;
       TimerCount=0;
    }
  }
}