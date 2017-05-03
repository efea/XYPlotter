

#ifndef PIN_H_
#define PIN_H_

class Pin {
private:
public:
	Pin();
	Pin(int portno, int pinno);
	virtual ~Pin();
	int pin;
	int port;
};

#endif /* PIN_H_ */
