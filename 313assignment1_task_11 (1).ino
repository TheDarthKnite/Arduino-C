#include <util/delay.h>
#include <avr/io.h>

//Timer starts off with no interupts having happened
 volatile uint8_t TimerCount=0;
//increases 
ISR(TIMER1_COMPA_vect){
  TimerCount++;
}
int main(void){
  //setup
  //disables interupts
  	cli();
  
  //sets all necessary port b DDRs for the LEDs and oscilloscope
	DDRB |= (1<<DDB0);
	DDRB |= (1<<DDB1);
 	DDRB |= (1<<DDB2);
	DDRB |= (1<<DDB3);
  
  //sets input for switch
	DDRD &= ~(1<<DDD2);
  
  
  //sets counter and control registers for timer/counter1
  	TCCR1A = 0;
  	TCCR1B = 0;
  	TCNT1 = 0;
  
  //sets value for timer 1 to compare to 1s
  	OCR1A = 15625 ;
  
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
  	//turns off Green LED and turns Red LED on for 1 second
    if(TimerCount<1){
        PORTB &= ~(1<<PORTB2);
      	PORTB |= (1<<PORTB3);
    }
      
    //turns off Red LED and turns Green LED on for 1 second
    else if (TimerCount<2){
         PORTB &= ~(1<<PORTB3);
      	 PORTB |= (1<<PORTB1);
    }
    //turns off Green LED and turns Yellow LED on for for 1 second
     else if (TimerCount<3){
         PORTB &= ~(1<<PORTB1);
         PORTB |= (1<<PORTB2);
     }
     else{
     //set timer values to 0
     TCNT1 = 0;
     TimerCount = 0; 
     }
  }
}
