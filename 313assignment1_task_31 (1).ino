#include <avr/io.h>
#include <avr/interrupt.h>

//pre-allocate variables
volatile uint16_t timerInterruptCounter = 0;
volatile uint32_t ticksElapsed = 0;
volatile double secondsElapsed = 0;
volatile uint16_t clockFreq = 62500;
volatile uint8_t carSpeed = 0;
volatile bool validLb2Breach = false;

//Class to simplify time elapsed calculation
class RecordTime {
	private:
  	  volatile uint16_t currentTick;
  	  volatile uint8_t interruptNumber;
  	  volatile bool timeRecorded;
  	public:
      RecordTime() {
      	this->currentTick = 0;
        this->interruptNumber = 0;
      }
  	  void setParams() { 
        this-> currentTick = TCNT1;
        this-> interruptNumber = timerInterruptCounter; 
      }
  	  uint16_t getTick() { return this->currentTick; }
  	  uint8_t getIntrptNum() { return this->interruptNumber; }
 	  bool emptyCheck() { return this->timeRecorded; }
};

//Queue data structure setup
int8_t queueHead = -1;
int8_t queueTail = -1;
RecordTime lb1Times[8]; //preallocate array with 8 cells to simulate queue between LB1&2
RecordTime lb2Time;

//create one object of RecordTime to operate on in calculations
RecordTime *currentTime = new RecordTime();

//Light Barrier 1 Breach Interrupt Service Routine
ISR(INT0_vect) {
  
  currentTime->setParams();  //record LB1 breach time
  PORTB |= (1<<PORTB3);   //blink red light
  
  if ((queueTail + 1) == 8 && queueHead == -1) { //don't overwrite head of queue if it hasn't passed LB2 yet
  } else if ((queueTail + 1) % 8 != queueHead) {
    queueTail = (queueTail + 1) % 8; //increment tail index
    lb1Times[queueTail] = *currentTime; //save time of barrier breach    
  } 
}

void calculateSpeed() {
  
  //calculate ticks elapsed = endTime - startTime + timerInterruptCounterDIFF*OCR1A
  ticksElapsed = (int32_t) lb2Time.getTick() - lb1Times[queueHead].getTick()
    + (int32_t)(lb2Time.getIntrptNum() - lb1Times[queueHead].getIntrptNum()) * OCR1A;
  
  //calculate elapsed seconds in real time
  secondsElapsed = (double) ticksElapsed / clockFreq;
  
  //calculate carspeed in km/h
  carSpeed = round(72 / secondsElapsed);
  
  //cap PWM signal at 100% after 100 km/h
  if (carSpeed >= 100) {
  	OCR1B = OCR1A;
    
  } else { 	//calculate OCR1B for COMPB interrupt for PWM duty cycle
     OCR1B = (1 - (float)carSpeed/100) * OCR1A; 
  }
}

//Light Barrier 2 Breach Interrupt Service Routine
ISR(INT1_vect) {
  
  currentTime->setParams();  //record LB2 breach time
  PORTB |= (1<<PORTB1);	//blink green light

  if (queueHead == queueTail) { //clear the queue if head catches up to tail  
    queueHead = -1;
    queueTail = -1;  
  } else {
    queueHead = (queueHead + 1) % 8;  //increment head index
  	lb2Time = *currentTime; //save LB2 breach time
    validLb2Breach = true; 
  }
}

//Timer1 PWM interrupt A triggered when TCNT1 == OCR1A
ISR(TIMER1_COMPA_vect) { 
  PORTB &= ~(1<<PORTB5); //send digital low signal to oscilloscope
  timerInterruptCounter += 1; //keep track of number of times TCNT1 cleared to 0 to calculate time
}

////Timer1 PWM interrupt B triggered when TCNT1 == OCR1B
ISR(TIMER1_COMPB_vect) {
  PORTB |= (1<<PORTB5); //send digital high signal to oscilloscope
}

void setup(){   
  
  //set pins for LED's and oscilloscope as output
	DDRB |= (1<<DDB1);
	DDRB |= (1<<DDB3);
  	DDRB |= (1<<DDB5);
  
  //sets both switch pins as inputs
	DDRD &= ~(1<<DDD2);
  	DDRD &= ~(1<<DDD1);
  
   //enable external interrupts on pin INT0 and INT1
 	EIMSK |= (1<<INT0) | (1<<INT1); 
  //detect rising edge on both interrupt 0 and 1
  	EICRA |= (1<<ISC01) | (1<<ISC00) | (1<<ISC11) | (1<<ISC10);
  
  //sets counter and control registers for timer/counter1
  	TCCR1A = 0;
  	TCCR1B = 0;
  	TCNT1 = 0;
  
  //timer/counter setup
  	OCR1A = 62500;
  	TCCR1B |= (1<<CS12) | (1<<WGM12); //prescaler 256, CTC mode
  	TIMSK1 |= (1<<OCIE1A) | (1<<OCIE1B); //enable ctc interrupts A and B	
  	 
  //enables interupts
  	sei();
}

void loop() {
  while(1) {
    
    //only calculate speed if LB1 was breached before LB2
    if (validLb2Breach) {
      calculateSpeed();
      validLb2Breach = false;
    }
    
    //turn off red and green LED's
    PORTB &= ~(1<<PORTB3);
    PORTB &= ~(1<<PORTB1);
    _delay_ms(1);
  }
}