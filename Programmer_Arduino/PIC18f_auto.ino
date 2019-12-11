// I wrote it according to the following datasheet:
// http://ww1.microchip.com/downloads/en/DeviceDoc/39592f.pdf
// and the code base from https://sites.google.com/site/thehighspark/arduino-pic18f

// To really understand what happens here carefully read the programing documentation of the PIC http://ww1.microchip.com/downloads/en/DeviceDoc/39592f.pdf

//pin out config
#define PGC 6
#define PGD 5

#define PGM 8
#define MCLR 7

#define P15 1
#define P12 1

// All delays are in microseconds and if possible the minimum.
#define DELAY_TSCLK     1       // Serial Clock (Program Clock, PGC) Period (MIN 100ns at 5V) /P2
#define DELAY_TSCLKL    1       // Serial Clock (PGC) Low Time (MIN 40ns at 5V) /P2A
#define DELAY_TSCLKH    1       // Serial Clock (PGC) High Time (MIN 40ns at 5V) /P2B
#define DELAY_TSET1     1       // Input Data Setup Time to Serial Clock fall (MIN 15ns) /P3
#define DELAY_THLD1     1       // Input Data Hold Time from SCK fall (MIN 15ns) /P4
#define DELAY_TDLY1     1       // Delay Between 4-Bit Command and Command Operand and vice versa (MIN 20ns) /P5
#define DELAY_TDLY2     1       // Delay Between Last PGC fall of Command Byte toFirst PGC rise of Read of Data Word (MIN 20ns) /P6
#define DELAY_TDLY5     1000    // SCK High Time (minimum programming time) /P9
#define DELAY_TDLY6     5       // SCK Low Time After Programming (high-voltage discharge time) /P10
#define DELAY_TDLY7     10000   // Delay to Allow Self-Timed Data Write or Bulk Erase to Occur /P11
#define DELAY_THDL2     2       // Input Data Hold Time from MCLR/V PP /RE3 rise /P12
#define DELAY_TSET2     1       // V DD riseSetup Time to MCLR/V PP /RE3 rise (MIN 100ns) /P13 
#define DELAY_TVALID    1       // Data Out Valid from SCK rise (MIN 10ns) /P14
#define DELAY_TSET3     2       // PGM riseSetup Time to MCLR/V PP /RE3 rise /P15

String inputString = "";
boolean stringComplete = false;

unsigned long temp;

byte buffer[32];
word buf[7];
byte address[3];

void setup(){
  Serial.begin(9600);
  pinMode(PGC,OUTPUT);
  pinMode(PGD,OUTPUT);
  pinMode(PGM,OUTPUT);
  pinMode(MCLR,OUTPUT);
  
 inputString.reserve(200);
 
}


void loop(){ 
  //turn on the chip (disable programming mode & diable reset)
  stop_programming();      
} 



//////THE MAIN CODE///////////
//////////////////////////////
//////////////////////////////
void serialEvent() {
   
  while (Serial.available()) {

    char inChar = (char)Serial.read(); 
    
    if (inChar == 'X') { //buffering till X
      stringComplete = true;
      break;
    } 
    
    inputString += inChar;
    
  }
  
  
  if (stringComplete && inputString.charAt(0) == 'W'){ //WRITE
    
    if(inputString.length() != 69) {
      Serial.println("Input is wrong, should be: W<address - 4 digit><32bytes>X"); 
      nullString();
      goto endofthisif;
      }
    
    address[2] = 0;
    address[1] = char2byte(inputString.charAt(2),inputString.charAt(1));
    address[0] = char2byte(inputString.charAt(4),inputString.charAt(3));
    
    nullBuffer(); // set everything FF
    
    for(int i=0;i<32;i++){
      buffer[i] = char2byte(inputString.charAt(2*i+6),inputString.charAt(2*i+5));
      } 
    
    nullString();
    
    
    ///Write
    
    start_programming();
    

    programBuffer(address[2],address[1],address[0]);
    
    stop_programming();
    
    Serial.println("Programming complete");    
    }
  
  endofthisif:;
  
 if (stringComplete && inputString.charAt(0) == 'I'){ //WRITE
    
    if(inputString.length() != 69) {
      Serial.println("Input is wrong, should be: W<address - 4 digit><32bytes>X"); 
      nullString();
      goto endofthisif;
      }
    
    address[2] = 0;
    address[1] = char2byte(inputString.charAt(2),inputString.charAt(1));
    address[0] = char2byte(inputString.charAt(4),inputString.charAt(3));
    
    nullBuffer(); // set everything FF
    
    for(int i=0;i<32;i++){
      buffer[i] = char2byte(inputString.charAt(2*i+6),inputString.charAt(2*i+5));
      } 
    
    nullString();
    
    ///Write
    
    start_programming();

    programIDBuffer(address[2],address[1],address[0]);
    
    stop_programming();
    
    Serial.println("Programming complete");    
    }
  
  
  if (stringComplete && inputString.charAt(0) == 'E'){ //Erase all
    start_programming();
     
     ////erase 
     erase_all();
     
    stop_programming();
     
     Serial.println("Erase complety");
     
     nullString();
    }
    
    
    
    
    
    
    if (stringComplete && inputString.charAt(0) == 'R'){ //READ
      
      
      
         address[2] = char2byte(inputString.charAt(2),inputString.charAt(1));
         address[1] = char2byte(inputString.charAt(4),inputString.charAt(3));
         address[0] = char2byte(inputString.charAt(6),inputString.charAt(5));
         
         //read
         
    
    temp=0;

    temp = ((long)address[2])<<(16); //doesn't work with out (long)
    temp |= ((long)address[1])<<(8);
    temp |= (long)address[0];
    
    Serial.print(temp,HEX); Serial.print(":");  
    
    start_programming();
    
    for(int i=0;i<32;i++){
        address[2] = byte( (temp&0xFF0000)>>16 );
        address[1] = byte( (temp&0xFF00)>>8 );
        address[0] = byte( temp&0xFF );
        Serial.print(readFlash(address[2],address[1],address[0]),HEX);
        Serial.print(" ");
        temp++;
       }
    
    Serial.println("");
    
    stop_programming();
    
    nullString();
     
    }
    
    
    if (stringComplete && inputString.charAt(0) == 'C'){ //config
    
    start_programming();

    for(byte i = 0; i < 7; i++){
      buf[i] =(((short)char2byte(inputString.charAt(4*i+4),inputString.charAt(4*i+3)) <<8) | char2byte(inputString.charAt(4*i+2),inputString.charAt(4*i+1)));
    }
    
    configWrite( 0,0);
    
    stop_programming();
    nullString();
    
    }
    
    if (stringComplete && inputString.charAt(0) == 'D'){ //Device ID
    
    if( checkIf_pic18f1320() ){
      delay(100); Serial.print("T");
    } else {
      delay(100); Serial.print("F");
    }
    
    
    stop_programming();
    nullString();
    
    }
    
    //clear string incase the first CHAR isn't E,R,W
    if(inputString.charAt(0) != 'E' && inputString.charAt(0) != 'R' && inputString.charAt(0) != 'W' && inputString.charAt(0) != 'C' && inputString.charAt(0) != 'D') nullString();
  
}




byte readFlash(byte usb,byte msb,byte lsb){
  
  byte value=0;
  set_address(usb, msb, lsb);
  send4bitcommand(B1001); //
  //send16bit(0x0000); 
  digitalWrite(PGD,LOW);
  
  for(byte i=0;i<8;i++){ //read
    digitalWrite(PGD,LOW);
    delayMicroseconds(DELAY_TSCLKH);
     digitalWrite(PGC,HIGH);
     delayMicroseconds(DELAY_TSCLKH);
     digitalWrite(PGC,LOW);
     delayMicroseconds(DELAY_TSCLKL);
    }
  delayMicroseconds(DELAY_TDLY2);
  pinMode(PGD,INPUT);
    
  for(byte i=0;i<8;i++){ //shift out
     digitalWrite(PGC,HIGH); 
     delayMicroseconds(DELAY_TSCLKH);
     if(digitalRead(PGD) == HIGH) value += 1<<i; //sample PGD
     delayMicroseconds(DELAY_TSCLKH);
     digitalWrite(PGC,LOW);
     delayMicroseconds(DELAY_TSCLKL);
    } 
    pinMode(PGD,OUTPUT);
    delayMicroseconds(DELAY_TDLY1);
  return value;
 
  }

  
byte readFlashsomebyte(){
  
  byte value=0;
  
  digitalWrite(PGD,LOW);
  send4bitcommand(B1001); //
  digitalWrite(PGD,LOW);
  
  for(byte i=0;i<8;i++){ //read
    digitalWrite(PGD,LOW);
    delayMicroseconds(DELAY_TSCLKH);
     digitalWrite(PGC,HIGH);
     delayMicroseconds(DELAY_TSCLKH);
     digitalWrite(PGC,LOW);
     delayMicroseconds(DELAY_TSCLKL);
    }
  delayMicroseconds(DELAY_TDLY2);
  pinMode(PGD,INPUT);
    
  for(byte i=0;i<8;i++){ //shift out
     digitalWrite(PGC,HIGH); 
     delayMicroseconds(DELAY_TSCLKH);
     if(digitalRead(PGD) == HIGH) value += 1<<i; //sample PGD
     delayMicroseconds(DELAY_TSCLKH);
     digitalWrite(PGC,LOW);
     delayMicroseconds(DELAY_TSCLKL);
    } 
    pinMode(PGD,OUTPUT);
    delayMicroseconds(DELAY_TDLY1);
  return value;
 
  }

void send_n_bit(unsigned int data, unsigned int n){
  pinMode(PGD,OUTPUT);
  digitalWrite(PGC,LOW);
  digitalWrite(PGD,LOW);
  for(byte i=0;i<n;i++){
    if( (1<<i) & data ) digitalWrite(PGD,HIGH); else digitalWrite(PGD,LOW); 
    n_clock_cycles(1);
    }
    delayMicroseconds(DELAY_TDLY1);
  
}

void n_clock_cycles(unsigned int n){
  for(byte i=0;i<n;i++){
    digitalWrite(PGC,HIGH);
    delayMicroseconds(DELAY_TSCLKH);
    digitalWrite(PGC,LOW);
    delayMicroseconds(DELAY_TSCLKL);
    }
}

void send4bitcommand(unsigned int data){
  send_n_bit(data,4);
}

void send16bit(unsigned int data){
  send_n_bit(data,16);
}

void send_command_and_data(unsigned int command, unsigned int data){
  send4bitcommand(command);
  send16bit(data);
  digitalWrite(PGD,LOW);
}

void start_programming(){
  digitalWrite(PGM,HIGH);
  delayMicroseconds(DELAY_TSET3);
  digitalWrite(MCLR,HIGH);
  delayMicroseconds(DELAY_THDL2);
  digitalWrite(PGD,HIGH);
}

void stop_programming(){
  digitalWrite(PGD,LOW);
  delayMicroseconds(DELAY_THDL2);
  digitalWrite(MCLR,LOW);
  delayMicroseconds(DELAY_TSET3);
  digitalWrite(PGM,LOW);
}

void set_address(byte usb,byte msb,byte lsb){
  send_command_and_data(B0000, 0x0e00 | usb);
  send_command_and_data(B0000, 0x6ef8);
  send_command_and_data(B0000, 0x0e00 | msb);
  send_command_and_data(B0000, 0x6ef7);
  send_command_and_data(B0000, 0x0e00 | lsb);
  send_command_and_data(B0000, 0x6ef6);
}

void erase_all(){
  
  set_address(0x003c,0x0000,0x0004);
  send_command_and_data(B1100, 0x0080);
  send_command_and_data(B0000, 0x0000);
  send_command_and_data(B0000, 0x0000);  
  delay(10);
  delayMicroseconds(5);  
}


void configWrite(byte address,short data){
  
  send_command_and_data(B0000, 0x8ea6);
  send_command_and_data(B0000, 0x8ca6);
  send_command_and_data(B0000, 0xef00);
  send_command_and_data(B0000, 0xf800);
  set_address(0x0030, 0x0000, 0x0000);
  for(byte i = 0; i < 7; i++){
    send_command_and_data(B1111, buf[i]); // if even, MSB ignored, LSB write
    program();
    send_command_and_data(B0000, 0x2af6);
    send_command_and_data(B1111, buf[i]); // if odd, MSB read, LSB ignored
    program();
    send_command_and_data(B0000, 0x2af6);
    delayMicroseconds(DELAY_TDLY6);
  }
}

void program()
{
    n_clock_cycles(3);
    digitalWrite(PGC,HIGH);
    delayMicroseconds(DELAY_TDLY5);
    digitalWrite(PGC,LOW);
    delayMicroseconds(100);

    send16bit(0x0000);
}
void programBuffer(byte usb,byte msb,byte lsb){ 
  
  //step 1
  send_command_and_data(B0000, 0x8ea6);
  send_command_and_data(B0000, 0x9ca6);
  //step 2
  set_address(usb, msb, lsb);
  for(byte i=0;i<4;i++){
    //step 3
    byte j=0;
    for(j=0; j<3;j++){    
      send_command_and_data(B1101, buffer[((i*8+2*j))+1]<<8 | buffer[((i*8+2*j))]); 
    }    
    //step 4
    send_command_and_data(B1111,  buffer[((i*8+6))+1]<<8 | buffer[((i*8+6))]); 
    //nop
    program();
    digitalWrite(PGD,LOW);
    //lsb += 8;
    readFlashsomebyte();
    readFlashsomebyte();
  }
  //done
  
}

void programIDBuffer(byte usb,byte msb,byte lsb){ 
  
  //step 1
  send_command_and_data(B0000, 0x8ea6);
  send_command_and_data(B0000, 0x9ca6);
  //step 2
  set_address(usb, msb, lsb);
  //step 3
  byte j=0;
  for(j=0; j<3;j++){    
    send_command_and_data(B1101, buffer[2*j+1]<<8 | buffer[j*2]); 
  }    
  //step 4
  send_command_and_data(B1111, buffer[7]<<8 | buffer[6]);  
  //nop
  n_clock_cycles(4);
  
  send16bit(0x0000); 
  digitalWrite(PGD,LOW);
  
  //done
  
}


/////////////////////////////
//nothing special underhere:

void nullString(){
  inputString = "";
  stringComplete = false; 
  }
  
void nullBuffer(){
  for(byte i=0;i<32;i++) buffer[i] = 0xFF;
  }
  
byte char2byte(char lsb,char msb){
  
  byte result=0;
  
  switch(lsb){
    case '0': result = 0; break;
    case '1': result = 1; break;
    case '2': result = 2; break;
    case '3': result = 3; break;
    case '4': result = 4; break;
    case '5': result = 5; break;
    case '6': result = 6; break;
    case '7': result = 7; break;
    case '8': result = 8; break;
    case '9': result = 9; break;
    case 'A': result = 0xA; break;
    case 'B': result = 0xB; break;
    case 'C': result = 0xC; break;
    case 'D': result = 0xD; break;
    case 'E': result = 0xE; break;
    case 'F': result = 0xF; break;
  }
  
switch(msb){
    case '0': result |= 0<<4; break;
    case '1': result |= 1<<4; break;
    case '2': result |= 2<<4; break;
    case '3': result |= 3<<4; break;
    case '4': result |= 4<<4; break;
    case '5': result |= 5<<4; break;
    case '6': result |= 6<<4; break;
    case '7': result |= 7<<4; break;
    case '8': result |= 8<<4; break;
    case '9': result |= 9<<4; break;
    case 'A': result |= 0xA<<4; break;
    case 'B': result |= 0xB<<4; break;
    case 'C': result |= 0xC<<4; break;
    case 'D': result |= 0xD<<4; break;
    case 'E': result |= 0xE<<4; break;
    case 'F': result |= 0xF<<4; break;
  }
  
  return result;
  
}

int checkIf_pic18f1320(){
  
  digitalWrite(PGM,HIGH);
  digitalWrite(MCLR,HIGH);
  delay(1);
  
  if( readFlash(0x3f,0xff,0xff) == 0x07 )
    return 1;
  else
    return 0;
}
