// Wire Slave RX
// Kevin Kuwata
// recieves byte from master, and activates relay
// Created 1/25/2018

#include <Wire.h>

#define RELAY_PIN 3

#define MAX_BYTES_RECEIVED 3 //we only are sending to turn ON, OFF, STATUS
// I think its 3 bytes because, you first send address
// then you send register you want to talk to
// then you send the value you want to send. 

#define REGISTER_MAP_SIZE   3// ADDRESS, STATUS, ON
#define SLAVE_ADDRESS   1 //whats a good way to choose?


//Address Map for COMMANDS
#define TURN_ON_REG			(0X01)
#define	STATUS_REG			(0X02)


//Control Flags
int update_register = 0;
int relay_state = 0; //default off;


byte registerMap[REGISTER_MAP_SIZE];
byte receievedCommands[MAX_BYTES_RECEIVED];


//IRS prototypes
void requestEvent(void);
void receiveEvent(void);

void setup() {
  Wire.begin(SLAVE_ADDRESS);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  //Wire.onRequest(requestEvent); // register interrupt requestEvent, when the master asks for STATUS
    Wire.onRequest(requestEvent);


  
  Serial.begin(9600);           // start serial for output
  Serial.println("Slave awake");

  //led set up led indirectly hooked up to pin 3 right now.
  // pin 3 high is led on
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  digitalWrite(RELAY_PIN, LOW);
  
  registerMap[1] = 0;
  
}

void loop() {
	
	//check the flags ... polling?
	if(update_register == 1){
		//update the stuff
		update(); // updates the relay state only. 
		//TODO: add ability to change slave address. 
		update_register = 0; //reset flag`
	}
	
	//Serial.print("registerMAP[1] =  ");
	//Serial.println(registerMap[1]);
	
	
	
	Serial.print("relay state  ");
	Serial.println(relay_state);
	
	
	//check here the state of the relay, in registger map
	// set relay accordingly. 
	if(registerMap[1] == 1){
		digitalWrite(RELAY_PIN, HIGH);
		Serial.println(" \n ON!! @74 \n");
		digitalWrite(13, HIGH);
	}
	if(registerMap[1] == 0){
		digitalWrite(RELAY_PIN, LOW);
				Serial.println(" OFF!! @79");
		digitalWrite(13, LOW);
	}
  delay(100);
}


void update(){
	//write to the memory register. status register?
	registerMap[1] = relay_state;
}



//When the slave receives data from the master
//we know we expect only the address and a single command.
// so we know we only are expecting 2 bytes. 
void receiveEvent(int bytesReceived) {
	Serial.println("yo!");
	digitalWrite(13, HIGH);
	
  for(int i = 0; i < bytesReceived; i++){
	  //loop through the data from the master
	  if(i < MAX_BYTES_RECEIVED){
		  
		  receievedCommands[i] = Wire.read();
	  }
	  else{
		Wire.read(); // let them come but don't collect
	  }
  }
	  
	  Serial.print("index 0: ");
	  Serial.println(receievedCommands[0]);
	  
	  Serial.print("index 1: ");
	  Serial.println(receievedCommands[1]);
//now we have collected info, now we need to parse it. 
// so we are really parsing the command here, and 
// use a switch statement to change based on the command
// this provides flexibility just in case
// we change the meaning of a given address.
//this is how we filter out a read vs a write register.

switch(receievedCommands[0]){

	//case TURN_ON_REG:
	case 0x01:
	//digitalWrite(13, HIGH);
	//digitalWrite(RELAY_PIN, HIGH);
	//next byte is the state, 1 is on, 0 is off. 
	relay_state = receievedCommands[1]; 
	update_register = 1;
		bytesReceived--; 
		if(bytesReceived == 1){
			return; // only expecting 2 bytes 
		}

	default:
	//trying to write to a READ-ONLY register.
	
	digitalWrite(13, LOW);
		return;// out of bounds
	}
	  

}// end of receive ISR

//When the slave receives data from the bus 
void requestEvent() {
	Wire.write(registerMap, REGISTER_MAP_SIZE);
	//we will send entire map, but we only need to 
	// send the status, so probably bit shift?
	
}// end of request ISR