#ifndef ILL_INPUT_ENUM_H__
#define ILL_INPUT_ENUM_H__

namespace Input {
/**
Enumerates the axis types in a joystick or mouse
*/
enum Axis {
    AX_X_VAL,   ///<Mouse X position on screen
    AX_Y_VAL,   ///<Mouse Y position on screen

    AX_X,       ///<X axis
    AX_Y,       ///<Y axis

    AX_X_POS,   ///<X axis in the positive direction
    AX_X_NEG,   ///<X axis in the negative direction
    AX_Y_POS,   ///<Y axis in the positive direction
    AX_Y_NEG,   ///<Y axis in the negative direction
};

}

#endif