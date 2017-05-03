
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Pin.h"

#ifndef GPIO_H_
#define GPIO_H_

class GPIO {
public:
    GPIO();

    static void initiate(Pin p, bool input);
    static void setPin(Pin pn, bool state);
    static bool readPin(Pin p);

};

#endif /* GPIO_H_ */



