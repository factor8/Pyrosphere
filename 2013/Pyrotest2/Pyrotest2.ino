#include <avr/pgmspace.h>
// #include <Shifter.h>

#define CHAIN_MODE 2

#define DATA_PIN1 7 //SER_IN
#define LATCH_PIN1 2 //L_CLOCK
#define CLOCK_PIN1 6 //CLOCK

#define DATA_PIN2 13 //SER_IN
#define LATCH_PIN2 8 //L_CLOCK
#define CLOCK_PIN2 12 //CLOCK

#define NUM_REGISTERS 12 //how many registers are in the chain
#define TOTAL_NODES 96
#define CHAIN1 47
#define CHAIN2 44

uint32_t sustain = 25;
uint32_t frequency = 40;

typedef struct _frame {
  int8_t frameChunk[NUM_REGISTERS];
} Frame;
Frame 			frameBuffer;

char				messageBuffer[8];
int 				readMode = 0;										//We're done reading. (until another.)
int					bufferIndex = 0;


//Some curious looking code without an explaination. Magic.
#define 		nodeOnMacro( FRAME, FLAMENUM )  ( (FRAME).frameChunk[((uint8_t)(FLAMENUM)) >> 3] |= _BV((FLAMENUM & 0x07 ) ) )
#define 		nodeOffMacro( FRAME, FLAMENUM ) ( (FRAME).frameChunk[((uint8_t)(FLAMENUM)) >> 3] &= ~_BV((FLAMENUM & 0x07 ) ) )
#define 		isFlameOn( FRAME, FLAMENUM ) ( (FRAME).frameChunk[((uint8_t)(FLAMENUM)) >> 3] & _BV((FLAMENUM & 0x07 ) ) )



// PROTOSPHERE
// const 			prog_int8_t mappingArray_P[TOTAL_NODES] PROGMEM = { 31, 74, 29, 90, 37, 33, 44, 34, 67, 76, 80, 30, 93, 75, 20, 35, 21, 65, 41, 42, 59, 69, 58, 85, 52, 53, 95, 89, 79, 39, 19, 87, 8, 22, 46, 32, 70, 66, 61, 18, 36, 86, 83, 77, 73, 84, 28, 94, 9, 11, 55, 68, 57, 63, 60, 26, 38, 43, 15, 12, 91, 72, 88, 16, 92, 64, 47, 51, 81, 71, 62, 50, 49, 27, 78, 10, 17, 14, 13, 82, 40, 45, 54, 56, 23, 48, 0, 1, 2, 3, 4, 5, 6, 7, 24, 25 };
// const 			prog_int8_t mappingArray_P[TOTAL_NODES] PROGMEM = { 31, -1, 74, 29, 90, 37, 33, 44, 34, 67, 76, 80, 30, 93, 75, 20, 35, 21, 65, 41, 42, 59, 69, 58, 85, 52, 53, 95, 89, 79, 39, 19, 87, 8, 22, 46, 32, 70, 66, 61, 18, 36, 86, 83, 77, 73, 84, 28, 94, 9, 11, 55, 68, 57, 63, 60, 26, 38, 43, 15, 12, 91, 72, 88, 16, 92, 64, 47, 51, 81, 71, 62, 50, 49, 27, 78, 10, 17, 14, 13, 82, 40, 45, 54, 56, 23, 48, 0, 1, 2, 3, 4, 5, 6, 7, 24};
// const 			prog_int8_t mappingArray_P[TOTAL_NODES] PROGMEM = { 25, 24, 7, 6, 5, 4, 3, 2, 1, 0, 48, 23, 56, 54, 45, 40, 82, 13, 14, 17, 10, 78, 27, 49, 50, 62, 71, 81, 51, 47, 64, 92, 16, 88, 72, 91, 12, 15, 43, 38, 26, 60, 63, 57, 68, 55, 11, 9, 94, 28, 84, 73, 77, 83, 86, 36, 18, 61, 66, 70, 32, 46, 22, 8, 87, 19, 39, 79, 89, 95, 53, 52, 85, 58, 69, 59, 42, 41, 65, 21, 35, 20, 75, 93, 30, 80, 76, 67, 34, 44, 33, 37, 90, 29, 74, 31 }; 

// PYROSPHERE REVERSED
const 			prog_int8_t mappingArray_P[TOTAL_NODES] PROGMEM = {0,-1,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94};	
// const 			prog_int8_t mappingArray_P[TOTAL_NODES] PROGMEM = {90,-1,89,88,87,86,85,84,83,82,81,80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,95,94,93,92,91};	
// const 			prog_int8_t mappingArray_P[TOTAL_NODES] PROGMEM = {95,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};	


//initaize shifter using the Shifter library
// Shifter shifter1(DATA_PIN1, LATCH_PIN1, CLOCK_PIN1, TOTAL_NODES);
// Shifter shifter2(DATA_PIN12, LATCH_PIN12, CLOCK_PIN12, TOTAL_NODES-CHAIN1); 

void setup(){
	// clear();
	// clear();
	
	// shifter1.write();
	// shifter2.write();

  Serial.begin(115200);

	// pepRally(); // Fire down and Fire up
 	delay(1000);
	Serial.println("pin setup...");
	pinMode(LATCH_PIN1, OUTPUT);
	pinMode(DATA_PIN1, OUTPUT);
	pinMode(CLOCK_PIN1, OUTPUT);
	pinMode(LATCH_PIN2, OUTPUT);
	pinMode(DATA_PIN2, OUTPUT);
	pinMode(CLOCK_PIN2, OUTPUT);

	// Serial.println("doin shit...");
	// digitalWrite(LATCH_PIN1, LOW);
	//   for(int i = 0; i < NUM_REGISTERS; i++){
	//     shiftOut(DATA_PIN1, CLOCK_PIN1, MSBFIRST, B11111111);
	//   }
	//   digitalWrite(LATCH_PIN1, HIGH);
	// delay(400);
	// digitalWrite(LATCH_PIN1, LOW);
	//   for(int i = 0; i < NUM_REGISTERS; i++){
	//     shiftOut(DATA_PIN1, CLOCK_PIN1, MSBFIRST, B00000000);
	//   }
	//   digitalWrite(LATCH_PIN1, HIGH);
	Serial.println("Done!");
	delay(100);
}

void loop(){
   // shifter.setAll(LOW); //Set all pins on the chain low		
		// pepRally();
		
//	prime(0);
//	ignite(100);

	// frequency = 150;
	// sustain = 250;
	// 
	// fire(0);
	// delay(150);
	// fire(1);
	// delay(150);
	// fire(2);


	// fireUp();
	// funMode();
	
		// fireAll();
		// fire(0,100);

		// fire(19);
		// delay(100);
		

		while(Serial.available() > 0) {
			    char x = Serial.read();
			
			//Flags, set read mode., begin
			/// Don't we want to prioritize this by most likely input? Probably +/- first.
		
			if 				( x == '!' ) 		{		readMode 	= 1;  	}					//Pattern
			else if 	( x == '@' ) 		{		readMode 	= 2;  	}	 				//Frame Duration
			else if  	( x == '#' ) 		{		readMode 	= 3; 		}					//Frame Interval
			else if   ( x == '+' ) 		{		readMode 	= 4;  	}					//Shift Register IDs, separated by comma (no whitespace)
			else if   ( x == '-' ) 		{		readMode 	= 5;  	}					//Shift Register IDs, separated by comma (no whitespace)
			else if   ( x == '~' ) 		{		readMode 	= 6;  	}					//System Mode 
			else if   ( x == '*' ) 		{		readMode 	= 7;  	}					//System Mode 	
			// else if  	( x == '/' ) 		{		getFiles(); 			}		
			// else if  	( x == '?' ) 		{		statusUpdate(); 	}			
			//Add custom flags here.
		
			//Finish up
			else if 	(x == '.') 		{ 	//...
		
				//This will update the global variables accordingly.
				//p
				switch(readMode){
					case 1: 			fireAll();   		break;
					case 2:  			fireUp();  		break;
					case 3:  			fireDown();  		break;
					case 4: 			fire(atoi(messageBuffer)); 			break;
					// case 5: 			setValveOff();			break;
					// case 6: 			setMode();					break;			
					// case 7: 			setActive();				break;						
					default:  												break;	
				}
		
					readMode = 0;										//We're done reading. (until another.)
		
					bufferIndex = 0;
		
					memset( messageBuffer, '\0', sizeof(messageBuffer) );		
		
			}
			else 										{ messageBuffer[bufferIndex++] = x; } 				//Magic.
		
		}


	 
	
}

void fireRange(uint16_t from,uint16_t to) {
for (int p=from;p<=to;p++) { /// may need adjusting.
		fire(p);						
		delay(frequency);
}
}

void funMode() {
	
	randomizeGlobals();
	uint8_t op = random(0,3);
	Serial.println(op);
	switch(op) {
		case 0:
			fireRandom();
			break;
		case 1:
			fireRandom(random(1,12),100);
			break;
		case 2:
			fireUp();
			fireDown();
			break;
		case 3:
			pulse(random(2,7));
			break;
	}	
}
void pepRally() {
	fireDown();
	fireUp();	
}
void randomizeGlobals() {
	// sust = random(20,150);
	// freq = random(20,250);
	// updateGlobals(sust,freq);
}
void updateGlobals(uint32_t sust, uint32_t freq) {
	// sustain = sust;
	// frequency = freq;
}

void pulse(uint16_t n) {
	
	uint32_t freq = random(50,250);
	for(size_t i = 0; i < n; ++i)
	{
		fireAll();
		delay(freq);
	}
}

void fireUp() {	
	Serial.print("Firing Through...");
	
	for (int p=0;p<(TOTAL_NODES);p++) {			
			fire(p);						
			delay(frequency);
 	}
}
void fireDown() {	
	Serial.print("Firing Through...");
	
	for (int p=TOTAL_NODES;p>=0;p--) {
			fire(p);						
			delay(frequency);
 	}
}
void fireRandom() {fireRandom(sustain);}
void fireRandom(uint32_t sust) {
	
	for (int p=0;p<40;p++) {		
		fire(random(0,TOTAL_NODES-1),sust);
		delay(frequency);
	}
}

void fireRandom(uint16_t n, uint32_t sust) {
	int p,s;	

	for (p=0;p<40;p++) {		
		for(s=0;s<n;s++) {	
			prime(random(0,TOTAL_NODES-1));						
		}
		ignite(sust);
		delay(frequency);			
	}

}


void fireAll() {
	fireAll(sustain);
}
void fireAll(uint32_t sust) {
	Serial.println("Firing All");
	
	for(int i = 0; i < TOTAL_NODES; i++){
		prime(i);
	}
	ignite(sust);	
	// shifter1.setAll(HIGH);
	//   shifter1.write(); //send changes to the chain and display them  
	//   delay(sust);
	//   clear(); //set all pins on the shift register chain to LOW
	//   shifter1.write(); //send changes to the chain and display them  
		
}

void fire(uint16_t pin) {
	fire(pin,sustain);	
}


void fire(uint16_t pin, uint32_t sust) {
	
	// Serial.print("Firing ");
	//   Serial.println(pin);
  
	// shifter.setPin(pin, HIGH);
	//   shifter.write(); //send changes to the chain and display them  
	//   delay(sust);
	// shifter.setPin(pin, LOW);
	//   	// shifter.clear(); //set all pins on the shift register chain to LOW
	//   shifter.write(); //send changes to the chain and display them  
	
	prime(pin);
	ignite(sust);
	
}

void prime(uint16_t n) {	
	// Serial.print("Priming ");
	//   Serial.println(n);

		// if (CHAIN_MODE == 2 && n >= CHAIN1) {
			
			// Serial.print("Firing Chain 2: ");
		  // Serial.println(n-CHAIN1);
			
			
			// shifter2.setPin(n-CHAIN1, HIGH);
		// } else {
			
			Serial.print("Firing: ");
		  Serial.println(n);			
			
			nodeOn(n);
		// }
	
}

// Send the frameBuffer (buffer) to the shift registers
void ignite(uint32_t sust){
	volatile int foo;
	static int bar = 0;
	
//	Serial.print("IGNITING ");
//	Serial.println(bar++);

  digitalWrite(LATCH_PIN1, LOW);
   for(int i = 0; i < NUM_REGISTERS; i++){
   			shiftOut(DATA_PIN1, CLOCK_PIN1, MSBFIRST, frameBuffer.frameChunk[i]);
   }
 //  	for (foo=0; foo <4000; foo++);
   	digitalWrite(LATCH_PIN1, HIGH);		
		delay(sust);	 
 //  	for (foo=0; foo <1000; foo++);
 /*  //  chain 2  	
   	
   	digitalWrite(LATCH_PIN2, LOW);
   	
//   	clear();
   	for(int i = 0; i < NUM_REGISTERS; i++){
   			shiftOut(DATA_PIN2, CLOCK_PIN2, MSBFIRST, frameBuffer.frameChunk[i]);
   	  }  	
 //  	for (foo=0; foo <4000; foo++);	
   	digitalWrite(LATCH_PIN2, HIGH);
//   	for (foo=0; foo <1000; foo++);
 */
		clear();
		
		digitalWrite(LATCH_PIN1, LOW);
		  for(int i = 0; i < NUM_REGISTERS; i++){
		    shiftOut(DATA_PIN1, CLOCK_PIN1, MSBFIRST, B00000000);
		  }
		  digitalWrite(LATCH_PIN1, HIGH);
/*
	  digitalWrite(LATCH_PIN1, LOW);
		digitalWrite(LATCH_PIN2, LOW);
	   for(int i = 0; i < NUM_REGISTERS; i++){
	   			shiftOut(DATA_PIN1, CLOCK_PIN1, MSBFIRST, frameBuffer.frameChunk[i]);
	   }
	   	clear();  // this will probably be bad when we actually use both channels   clear AFTER sending data
	   for(int i = 0; i < NUM_REGISTERS; i++){
	   			shiftOut(DATA_PIN2, CLOCK_PIN2, MSBFIRST, frameBuffer.frameChunk[i]);
	   }  	
	 //	for (foo=0; foo <1000; foo++);
		digitalWrite(LATCH_PIN1, HIGH);	
	 	digitalWrite(LATCH_PIN2, HIGH);
//	 	for (foo=0; foo <1000; foo++);
*/

/*
  digitalWrite(LATCH_PIN1, LOW);
	digitalWrite(LATCH_PIN2, LOW); 
   for(int i = 0; i < NUM_REGISTERS; i++){
 			shiftOut(DATA_PIN1, CLOCK_PIN1, MSBFIRST, frameBuffer.frameChunk[i]);
			shiftOut(DATA_PIN2, CLOCK_PIN2, MSBFIRST, frameBuffer.frameChunk[i]);
   }
 	for (foo=0; foo <4000; foo++);
 	digitalWrite(LATCH_PIN1, HIGH);
	digitalWrite(LATCH_PIN2, HIGH);
 	for (foo=0; foo <1000; foo++);
	clear(); 
*/

	/*
 	// delay(sust);	 
 	digitalWrite(LATCH_PIN12, LOW);
 	
 	clear();
 	for(int i = 0; i < NUM_REGISTERS; i++){
 			shiftOut(DATA_PIN12, CLOCK_PIN12, MSBFIRST, frameBuffer.frameChunk[i]);
 	  }  	
 	for (foo=0; foo <4000; foo++);	
 	digitalWrite(LATCH_PIN12, HIGH);
 	for (foo=0; foo <1000; foo++);
*/
  
	
	// digitalWrite(LATCH_PIN1, LOW);
	//   for(int i = 0; i < NUM_REGISTERS; i++){
	//     shiftOut(DATA_PIN1, CLOCK_PIN1, MSBFIRST, B11111111);
	//   }
	//   digitalWrite(LATCH_PIN1, HIGH);
	// delay(400);

	
}

// void ignite(uint32_t sust) { 		
// 										
// 	// Serial.println("Igniting! ");
// 		
// 	shifter1.write(); //send changes to the chain and display them  
// 	// shifter2.write(); //send changes to the chain and display them  	
//   delay(sust);
//   clear(); //send changes to the chain and display them  
//   // shifter2.clear(); //send changes to the chain and display them  
// 	shifter1.write();
// 	// shifter2.write(); //send changes to the chain and display them  	
// 	
// 	
// }
// void kill(uint16_t n) {
// 	/// FIX FOR 2
// 	shifter1.setPin(n, LOW);
// 	shifter1.write();
// }

void clear(){
  for(int i = 0; i < NUM_REGISTERS; i++){
    frameBuffer.frameChunk[i] = B00000000;
  }
}

void nodeOn(int8_t nodeNum){
	Serial.print("nodeOn: ");
	Serial.println(nodeNum);
	
  int8_t mappedValue = pgm_read_byte(&mappingArray_P[nodeNum]);
  // nodeTimeStamps[nodeNum] = millis();
  nodeOnMacro(frameBuffer, mappedValue);
}


void nodeOff(int8_t nodeNum){
  int8_t mappedValue = pgm_read_byte(&mappingArray_P[nodeNum]);
  // nodeTimeStamps[nodeNum] = -1;
  nodeOffMacro(frameBuffer, mappedValue);
}

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
	uint8_t i;
	volatile int foo;
	
	for (i = 0; i < 8; i++)  {
		if (bitOrder == LSBFIRST)
			digitalWrite(dataPin, !!(val & (1 << i)));
		else	
			digitalWrite(dataPin, !!(val & (1 << (7 - i))));
			
		for (foo=0; foo <300; foo++);
		
		digitalWrite(clockPin, HIGH);
		for (foo=0; foo <100; foo++);
		digitalWrite(clockPin, LOW);		
	}
}
