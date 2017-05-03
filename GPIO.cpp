
#include <GPIO.h>

GPIO::GPIO() {

}

void GPIO::initiate(Pin p, bool input){
	Chip_IOCON_PinMuxSet(LPC_IOCON, p.port, p.pin, IOCON_DIGMODE_EN | IOCON_MODE_PULLUP);

	if(input){
		Chip_GPIO_SetPinDIRInput(LPC_GPIO, p.port, p.pin);
	}else{
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, p.port, p.pin);
	}

}

void GPIO::setPin(Pin p, bool state){
	Chip_GPIO_SetPinState(LPC_GPIO, p.port, p.pin, state);
}

bool GPIO::readPin(Pin p){
	return !Chip_GPIO_GetPinState(LPC_GPIO, p.port, p.pin);
}

