#ifndef ILL_SDL_INPUT_ENUM_H__
#define ILL_SDL_INPUT_ENUM_H__

#include <cstdint>
#include "Input/serial/inputAxisEnum.h"

/**
Includes an enum of the keycodes
*/
#include <SDL_keycode.h>

namespace SdlPc {
    
/**
A button index on a mouse or joystick
*/
typedef uint8_t Button;

/**
The values that go into the InputBinding device type.

SDL isn't very flexible at detecting multiple mice and keyboards,
so these are the hardcoded enums for now.

Normally I'd try to detect multiple devices and let them be bound per player.
*/
enum InputType {
    PC_KEYBOARD,            ///<keys on the keyboard
    PC_MOUSE_BUTTON,        ///<buttons on the mouse
    PC_MOUSE_WHEEL,         ///<wheels on the mouse
    PC_MOUSE                ///<movement of the mouse
};

}

#endif
