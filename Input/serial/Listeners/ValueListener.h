#ifndef ILL_INPUT_LISTENER_VALUE_H_
#define ILL_INPUT_LISTENER_VALUE_H_

#include "Util/CopiedData.h"

namespace illInput {

/**
This may go way later in favor of things like gesture recognizing code, etc...

A value listener is for some very specific input types that don't really correspond to button or
joystick inputs.

This is good for complex things like mouse x,y coordinates on the screen, phone device orientation matrices,
gesture data, etc...  Mobile phone inputs usually have much more complex data associated with inputs.

At the moment it's up to the sending end and the receiving end to know what the type of data is,
and to just send it as a void* here in order to have inputs easily mappable.  So just typecast it to and from void *
on both ends and hope nothing went wrong along the way.

Try to keep this as platform independent as possible.  Try to avoid sending platform specific input data here.
For example if on Android you are receiving device orientation data in some format, convert that to a 3x3 orientation matrix.
That way if you are receiving orientation data on the iPhone and you also convert that to a 3x3 matrix the application is still portable
since the application always receives device orientaiton as a 3x3 matrix.
*/
struct ValueListener {
    virtual ~ValueListener() {}

    /**
    Receives some arbitrary value, such as mouse x,y coordinates on screen, or touch data.
    
    @param value The value.  Be careful with this, after the function returns, the data inside the CopiedValue
        struct is freed.  @see CopiedValue
    */
    virtual void onChange(CopiedData value) {}
};

}

#endif