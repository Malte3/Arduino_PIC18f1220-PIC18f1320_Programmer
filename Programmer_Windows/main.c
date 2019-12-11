/*
Original work Copyright (C) 2012  kirill Kulakov
Modified work Copyright (C) 2019  Malte Rethwisch

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "rs232.h"
#include "HEX.h"

#include <stdio.h>
#include <stdlib.h>


#define DEVICE_DELAY 150

int _main();

int main(int argc, char* argv[]){

	printf("Arduino as A PIC18f1320 programmer\n");

	if(_main()){
		printf("Error! can not continue\n");
		printf("Make sure you have uploaded the sketch to the Arduino,\nconnected the chip correctly and using a valid HEX file\n");
	}
	else {
		printf("The chip was programmed successfully!\n");
		
	}
	
	system("pause");

	return 0;
}


int _main(){

	HEX thehex;
	char buf[4096],key, buf1[4096];
	int n;
	unsigned int address=0,i,j;
	int comPort=-1;

	//reading the hex file
	printf("Enter location of hex file: ");
	scanf("%4096[^\n]",buf1);
	if( (thehex = readHex(buf1)) == NULL ){
		printf("Can not open the file\n");
		return 1;
	}

	printf("---------------------------\n");
	printf("Connecting to the Arduino...");

	for(i=0;i<16;i++){
		if(!OpenComport(i, 9600)){
			for(j=0;j<10;j++){
				SendBuf(i, (unsigned char *)"DX",strlen("DX"));
				n = PollComport(i, (unsigned char *)buf, 4095);
				Sleep(DEVICE_DELAY);
				if(*buf == 'T' || *buf == 'F' ){
					comPort = i;
					break;
				}
				
			}
			CloseComport(i);
			if( comPort != -1 )
				break;		
		}
	}

	if( comPort == -1 ){
		printf("Cannot connect to the programmer");
		return 1;
	}

	OpenComport(comPort, 9600);

	printf("Success!\nConnecting to the chip...");

	//wait for PIC18f1320

	do{
	if( SendBuf(comPort,(unsigned char *)"DX",strlen("DX")) == -1 ) return 1;
	n = PollComport(comPort, (unsigned char *)buf, 4095);
	Sleep(DEVICE_DELAY);
	}while( n == 0);
	
	printf("Success!\n");

	printf("Erasing the chip...");

	//chip reset
	if( SendBuf(comPort,(unsigned char *)"EX",strlen("EX")) == -1 ) return 1;

	printf("Success!\n");

	Sleep(DEVICE_DELAY);

	
	
	printf("Programming ID memory...");

	//ID write

	for (i = 0; i<0x8; i++) {
		if (getIDData(thehex, i) != 0xff) {
			sprintf(buf, "I");
			sprintf(buf + strlen(buf), "%04X", 0x200000 + i);
			for (i = 0; i < 0x8; i++)
				sprintf(buf + strlen(buf), "%02X", getIDData(thehex, i));
			sprintf(buf + strlen(buf), "X\n");
			SendBuf(comPort, (unsigned char *)buf, strlen(buf));
			Sleep(DEVICE_DELAY);
		}
	}

	printf("Success!\n");

	printf("Programming flash memory...");

	//memory write

	while( address < 0x2000 ){

		for(i=0;i<0x20;i++){
			if( getData(thehex,address+i) != 0xff ){
				sprintf(buf,"W");
				sprintf(buf+strlen(buf),"%04X",address);
				for(i=0;i<0x20;i++)
					sprintf(buf+strlen(buf),"%02X",getData(thehex,address+i));
				sprintf(buf+strlen(buf),"X\n");
				SendBuf(comPort,(unsigned char *)buf,strlen(buf));
				Sleep(DEVICE_DELAY);
				break;
			}
		}

		address += 0x20;

	}

	printf("Success!\n");
  
	printf("Programming fuse bits...");


	sprintf(buf, "C");
	for (i = 0; i<0x7; i += 1)
	{
		sprintf(buf + strlen(buf), "%04X", getfuse(thehex, i));
	}
	sprintf(buf + strlen(buf), "X\n");
	SendBuf(comPort, (unsigned char *)buf, strlen(buf));
	Sleep(DEVICE_DELAY);
		

	printf("Success!\n");

	CloseComport(comPort);

	printf("---------------------------\n");

	return 0;

}
