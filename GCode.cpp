#include "GCode.h"
#include <stdio.h>
#include <string.h>
#include <cstdlib>

Gcode::Gcode(){


}

void Gcode::readfromUart(){
	//record the command

	ch = Board_UARTGetChar();
	if(ch != 255){
		Board_UARTPutChar(ch);
		if(endofcommand != true){
			command[i] = ch;
			i++;
			if(ch == '\r' || ch == '\n' || ch == '\0'){
				//the command from the gcode has ended
				endofcommand = true;
				sizeofcommand = i;
				command[i-1] = '\0';
			}
		}
	}

	//g1 received
	if((endofcommand == true) && (command[0] == 'G' && command[1] == '1')){
		char tempx[sizeofcommand];
		char tempy[sizeofcommand];
		char templast[sizeofcommand];
		k = 4;
		i = 0;
		while(command[k] != ' '){
			tempx[i] = command[k];
			k++;
			i++;
		}
		fp.x = atof(tempx);

		k = k + 2;
		i = 0;
		while(command[k] != ' '){
			tempy[i] = command[k];
			k++;
			i++;
		}
		fp.y = atof(tempy);
		k = k + 1;
		i = 0;
		while(command[k] != '\0'){
			templast[i] = command[k];
			i++;
			k++;
		}
		if(strcmp(templast, "A0") == 0){
			absolutePosition = TRUE;
		}
		i = 0;
		k = 0;
		sendOk();
		//returng1();
	}

	//receiving g28
	else if((endofcommand == true) && (command[0] == 'G' && command[1] == '2' && command[2] == '8')){
		sendOk();
	}

	//receiving m10
	else if((endofcommand == true) && (command[0] == 'M' && command[1] == '1' && command[2] == '0')){
		sendM10();
		sendOk();
	}

	//receiving m1
	else if((endofcommand == true) && (command[0] == 'M' && command[1] == '1')){
		int t = 3;
		int z = 0;
		char tempm1[sizeofcommand];
		while(command[t] != '\0'){
			tempm1[z] = command[t];
			z++;
			t++;
		}
		m1param = atoi(tempm1);
		sendOk();
		stopstat = true;
		//stopMotors();

	}

	//receiving m4
	else if((endofcommand == true) && (command[0] == 'M' && command[1] == '1')){
		sendOk();

	}
	return ;
}

void Gcode::sendM10(){
	Board_UARTPutSTR("M10 XY 380 310 0.00 0.00 A0 B0 H0 S80 U160 D90");
	return;
}

void Gcode::sendOk(){
	Board_UARTPutSTR("OK");
	return;

}

FloatPoint Gcode::returng1(){
	return fp;
};

FloatPoint Gcode::returnhome(){
	return home;

};

bool Gcode::stopstatus(){

	return stopstat;

}
