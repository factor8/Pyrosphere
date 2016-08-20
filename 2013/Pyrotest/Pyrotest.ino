#include <avr/pgmspace.h>
#include <Shifter.h>

#define CHAIN_MODE 1

#define SER_Pin 7 //SER_IN
#define RCLK_Pin 2 //L_CLOCK
#define SRCLK_Pin 6 //CLOCK

#define SER_Pin2 13 //SER_IN
#define RCLK_Pin2 8 //L_CLOCK
#define SRCLK_Pin2 12 //CLOCK

#define NUM_REGISTERS 12 //how many registers are in the chain
#define TOTAL_NODES 91
#define CHAIN1 47
#define CHAIN2 44

uint32_t sustain = 25;
uint32_t frequency = 40;

// const prog_int8_t mappingArray_P[TOTAL_NODES] PROGMEM = { 31, 74, 29, 90, 37, 33, 44, 34, 67, 76, 80, 30, 93, 75, 20, 35, 21, 65, 41, 42, 59, 69, 58, 85, 52, 53, 95, 89, 79, 39, 19, 87, 8, 22, 46, 32, 70, 66, 61, 18, 36, 86, 83, 77, 73, 84, 28, 94, 9, 11, 55, 68, 57, 63, 60, 26, 38, 43, 15, 12, 91, 72, 88, 16, 92, 64, 47, 51, 81, 71, 62, 50, 49, 27, 78, 10, 17, 14, 13, 82, 40, 45, 54, 56, 23, 48, 0, 1, 2, 3, 4, 5, 6, 7, 24, 25 };
// const uint8_t map[TOTAL_NODES] = [31, 74, 29, 90, 37, 33, 44, 34, 67, 76, 80, 30, 93, 75, 20, 35, 21, 65, 41, 42, 59, 69, 58, 85, 52, 53, 95, 89, 79, 39, 19, 87, 8, 22, 46, 32, 70, 66, 61, 18, 36, 86, 83, 77, 73, 84, 28, 94, 9, 11, 55, 68, 57, 63, 60, 26, 38, 43, 15, 12, 91, 72, 88, 16, 92, 64, 47, 51, 81, 71, 62, 50, 49, 27, 78, 10, 17, 14, 13, 82, 40, 45, 54, 56, 23, 48, 0, 1, 2, 3, 4, 5, 6, 7, 24, 25];

//initaize shifter using the Shifter library
// Shifter shifter1(SER_Pin, RCLK_Pin, SRCLK_Pin, TOTAL_NODES);
Shifter shifter1(SER_Pin, RCLK_Pin, SRCLK_Pin, TOTAL_NODES);
Shifter shifter2(SER_Pin2, RCLK_Pin2, SRCLK_Pin2, CHAIN2); 


void setup(){
	
	if (CHAIN_MODE == 2) {

	} else {
		
	}
	
	
	shifter1.clear();
	if (CHAIN_MODE == 2) shifter2.clear();
	
	shifter1.write();
	if (CHAIN_MODE == 2) shifter2.write();

  Serial.begin(115200);

	// pepRally(); // Fire down and Fire up
 	delay(1000);
}

void loop(){
   // shifter.setAll(LOW); //Set all pins on the chain low		
		// pepRally();
		// fireAll();

		// delay(100)
//65,69
		// fire(39);	
	// frequency = 20;
	// sustain = 20;
		/*

			 #####  ####### ####### #     # ######  
			#     # #          #    #     # #     # 
			#       #          #    #     # #     # 
			 #####  #####      #    #     # ######  
			      # #          #    #     # #       
			#     # #          #    #     # #       
			 #####  #######    #     #####  #

			REMEMBER TO CHANGE TOTAL NODES FROM 44 to 91!
			REMEMBER TO SWITCH PIN CONFIG from 257 to 267

		*/
			fireUp();
			// fire(5);

		// fireRange();
		// frequency = 25;
		// sustain = 30;
			// fire(0);
			// fireRange(40,85);
		// fireDown();
	// funMode();

		// delay(100);
		
		// fireRandom(random(0,86),50);
		
		// delay(700);
		// fireAll();
		// delay(300);		
		// fireAll();
		
		// kill(17);
		// kill(16);
		// kill(15);
		// kill(14);
		// kill(13);		

		// fireAll();
		// delay(10);

		// delay(200);
		// fire(17,500);
		// fire(16,500);
		// fire(15,500);
		// fire(14,500);
		// fire(13,500);
		// 
		  
 // delay(1000);  
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
	
	frequency = random(20,40);
	sustain = random(20,30);
		
	switch(op) {
		case 0:
			fireRandom();
			break;
		case 1:
			fireRandom(random(0,TOTAL_NODES-1),random(20,140));
			break;
		case 2:
			pepRally();	
			break;
		case 3:
			pulse(random(2,12));
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
	Serial.print("Firing All");
	
	shifter1.setAll(HIGH);
  shifter1.write(); //send changes to the chain and display them  
  delay(sust);
  shifter1.clear(); //set all pins on the shift register chain to LOW
  shifter1.write(); //send changes to the chain and display them  
		
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
		// 	
		// 	Serial.print("Firing Chain 2: ");
		//   Serial.println(n-CHAIN1);
		// 	
		// 	
		// 	shifter2.setPin(n-CHAIN1, HIGH);
		// } else {
			
			Serial.print("Firing Chain 1: ");
		  Serial.println(n);			
			
			shifter1.setPin(n, HIGH);
		// }
	
	
}

void ignite(uint32_t sust) { 		
										
	// Serial.println("Igniting! ");
		
	shifter1.write(); //send changes to the chain and display them  
	if (CHAIN_MODE == 2) shifter2.write(); //send changes to the chain and display them  	
  delay(sust);
  shifter1.clear(); //send changes to the chain and display them  
	if (CHAIN_MODE == 2) shifter2.clear(); //send changes to the chain and display them  
	shifter1.write();
	if (CHAIN_MODE == 2) shifter2.write(); //send changes to the chain and display them  	
	
	
}
void kill(uint16_t n) {
	/// FIX FOR 2
	shifter1.setPin(n, LOW);
	shifter1.write();
}