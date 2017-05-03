#include "board_api.h"
#include <stdlib.h>

struct FloatPoint{
	float x;
	float y;
};

class Gcode
{
public:
	Gcode();
	void readfromUart();
	void sendOk();
	void sendM10();
	FloatPoint returng1();
	FloatPoint returnhome();
	bool stopstatus();

private:
	char command[];
	FloatPoint fp;
	FloatPoint home;
	int absolutePosition = true;
	int m1param;
	int m4param;
	int i;
	int k;
	int sizeofcommand;
	bool endofcommand = false;
	char ch;
	bool stopstat = false;

};
