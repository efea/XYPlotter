#include "StepperMotor.h"
#include "GPIO.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>
#include <cstdio>
#include <string>
#include "string.h"
#include <cstdlib>
#include "ITM_write.h"
#include <cr_section_macros.h>
#include "RIT.h"

StepperMotor::StepperMotor(Pin &pos, Pin &neg, Pin &dir, Pin &move):posLs(pos), negLs(neg), dirPin(dir), movePin(move) {
	curPos = 0;
	maxPos = 26700;
}


bool StepperMotor::calibrate()
{

	int stepsFirst = 0;
	int stepsSecond = 0;


	bool dir = false;               	// start calibrating in negative direction

	while (!GPIO::readPin(negLs)){
		move(dir, 200, 1);
	}

	//Start in positive direction
	dir = (bool)!dir;                           	// Invert direction

	// Move away from the edge.
	while (GPIO::readPin(negLs)){
		move(dir, 200, 1);
	}

	// Move safe distance 200 steps
	move(dir, 1000, 200);

	// Continue moving, and calculating average
	while (!GPIO::readPin(posLs)){
		move(dir, 200, 1);
		stepsFirst++;
	}

	// Reached another edge, switch direction and move away from switch
	dir = (bool)!dir;
	while (GPIO::readPin(posLs)){
		move(dir, 200, 1);
	}

	// Move safe distance 50 steps
	move(dir, 1000, 200);
	stepsFirst = stepsFirst - 200; // Minus 50 steps

	// Return in negative direction
	while (!GPIO::readPin(negLs))
	{
		move(dir, 200, 1);
		stepsSecond++;
	}

	// Move away from edge.
	dir = (bool)!dir;
	while (GPIO::readPin(negLs))
	{
		move(dir, 200, 1);
	}

	// Move safe distance 50 steps
	move(dir, 1000, 200);
	stepsSecond = stepsSecond - 200; // Minus 50 steps

	if(abs((stepsSecond) - (stepsFirst)) > 100){

		//printf("FirstSteps: %d and second: %d \n", stepsFirst, stepsSecond);
		return false;
	}else{
		curPos = 0;
		maxPos = (stepsFirst + stepsSecond) / 2;

		//printf("Difference: %d \n", abs(stepsFirst-stepsSecond));
		return true;
	}
}



int StepperMotor::move(bool dir, int speed, unsigned long distance)
{

	GPIO::setPin(dirPin, dir);

	RIT_start(dir, distance, speed, this->negLs, this->posLs, this->movePin);

	return 0;
}

bool StepperMotor::moveTo(int coordinate, int speed)
{
	bool dir;
	if(coordinate <= maxPos && coordinate >= 0){
		if(coordinate < curPos){

			dir = false;
			GPIO::setPin(dirPin, dir);
			RIT_start(dir, (curPos-coordinate), speed, this->negLs, this->posLs, this->movePin);
			curPos = coordinate;
			return true;
		}else if(coordinate > curPos){

			dir = true;
			GPIO::setPin(dirPin, dir);
			RIT_start(dir, (coordinate-curPos), speed, this->negLs, this->posLs, this->movePin);
			curPos = coordinate;
			return true;
		}
	}else{
		// Tried to move somewhere beyond edge.
		return false;
	}

}
