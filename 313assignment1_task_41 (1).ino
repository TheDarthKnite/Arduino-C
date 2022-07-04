#include <util/delay.h>
#include <avr/io.h>

//Timer starts off with no interupts having happened
	volatile uint8_t TimerCount = 0;
//Red light is not initially considered on
	volatile uint8_t Red=0;
//Red light has not been breached yet
	volatile uint8_t Breach=0;
//used to make sure the white light only flashes 2 times 
//set to 3 so white light doesn't start flashing when program is launched
	volatile uint8_t LoopCount=3;
//counts how many sets of 125ms have passed since 
//the white light started flashing.
	volatile uint8_t WhiteTimeCount=0;
//stores timer counter1 value fo when the red light is breached
	volatile uint8_t WhiteTCNT=0;
//stores increments of Timer2 in 10ms
	volatile uint8_t Timer2Count=0;
//stores increments of Timer2 in 2ms to convert to Timer2count
	volatile uint8_t Count2ms=0;

//interupt service routine for when first button is pressed
ISR(INT0_vect) {
  if(Red) {
    //records breach and sets loop count to 0 for white light to start flashing
    Breach++;
    LoopCount=0;
    //sets Timercount of 125ms increments since white light starts turning on.
    WhiteTimeCount=0;
    //stores TCNT1 value so that light can flash in exact 125ms increments
    WhiteTCNT=TCNT1;
  }
}

//increases 
ISR(TIMER1_COMPA_vect){
  TimerCount++;
  WhiteTimeCount++;
}
ISR(TIMER2_COMPA_vect){
  TCNT2=0;
  Count2ms++;
  if(Count2ms==5){
    Count2ms=0;
  	Timer2Count++;
  }
  if(Timer2Count==100){
    Timer2Count=0;
  }
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
  	DDRB |= (1<<DDB5);
  
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
  
  //sets value for timer 1 to compare to to 125ms
  	OCR1A = 31250;
  
  //prescale to 64 + enable CTC mode
  	TCCR1B |=(1<<CS11)|(1<<CS10)| (1<<WGM12);
  
  //enable time compare interrupt 
  	TIMSK1 |= (1 << OCIE1A);
  
  //sets counter and control registers for timer/counter2
    TCCR2A = 0;
  	TCCR2B = 0;
  	TCNT2 = 0;
  
  //sets value for timer 2 to compare to 2ms
  	OCR2A = 125;
  
  //prescale to 256 + enable CTC mode
  	TCCR2A |=(1<<WGM21);
  	TCCR2B |=(1<<CS21)|(1<<CS22);
   
  
  //enable time compare interrupt 
  	TIMSK2 |= (1 << OCIE2A);
  //enables interupts
  	sei();
  
  //set timer values to 0
    TCNT1 = 0;
    TimerCount = 0;
  	TCNT2  = 0;
  	Timer2Count =0;
  //starts loop
  while(1){
    //prevents slowdown
    _delay_ms(1);
    
    //white light sequence
    
    //will keep white light on until reaching the initial TCNT1 value on the next Timer count for 2 loops
    if (LoopCount<2&&((WhiteTimeCount==0&&WhiteTCNT<TCNT1)||(WhiteTimeCount==1&&WhiteTCNT>TCNT1))){
      PORTB |= (1<<PORTB0);
    //will keep white light off until reaching the initial TCNT1 value on the next Timer count for 2 loops
    }else if (LoopCount<2&&((WhiteTimeCount==1&&WhiteTCNT<TCNT1)||(WhiteTimeCount==2&&WhiteTCNT>TCNT1))){
      PORTB &= ~(1<<PORTB0);
    //increments to the next loop and resets timer count for the white light  
    }else if (LoopCount<2){
      LoopCount++;
      WhiteTimeCount=0;
    }
    
    //normal traffic light sequence
    
  	//turns off Green LED and turns Red LED on for 1 second
    if(TimerCount<8){
        PORTB &= ~(1<<PORTB2);
      	PORTB |= (1<<PORTB3);
        //red light is on 
        Red=1;
    }
      
    //turns off Red LED and turns Green LED on for 1 second
    else if (TimerCount<16){
         PORTB &= ~(1<<PORTB3);
      	 PORTB |= (1<<PORTB1);
          //Red Light is off
         Red=0;
    }
    //turns off Green LED and turns Yellow LED on for for 1 second
    else if (TimerCount<24){
         PORTB &= ~(1<<PORTB1);
         PORTB |= (1<<PORTB2);
    }
    else{
    //set timer values to 0
    TCNT1 = 0;
    TimerCount = 0; 
    }
    //while the breach count is greater than the amount of 10ms
    //saved in Timer2Count
    if(Timer2Count<Breach){
      PORTB |= (1<<PORTB5);
    }
    else{
      PORTB &= ~(1<<PORTB5);
    }
  }
}
