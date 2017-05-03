#ifndef STEPPERMOTOR_H
#define STEPPERMOTOR_H

#include "GPIO.h"
#include "Pin.h"

class StepperMotor
{
public:
	StepperMotor(Pin &pos, Pin &neg, Pin &dir, Pin &move);
	bool calibrate();
	void receiveCommand();
	int move(bool dir, int speed, unsigned long distance);
	bool moveTo(int coordinate, int speed);

	Pin &posLs;
	Pin &negLs;

	Pin &dirPin;
	Pin &movePin;

private:
	unsigned long curPos, maxPos;

};

#endif // STEPPERMOTOR_H_INCLUDED


