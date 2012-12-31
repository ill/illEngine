#ifndef ILL_CASTING_H__
#define ILL_CASTING_H__

#include <sstream>
#include "Logging/logging.h"

/**
Functions useful for typecasting in C++ that make use of dynamic_cast.
This is the proper way to typecast most things instead of using C style casts.
Use your best judgement...
*/

/**
Returns if object is of a type

example usage:

Vehicle * vehicle = new FighterJet();
bool vehicleIsFighterJet = is<FighterJet>(vehicle);

This returns true because vehicle is of type FighterJet.

Vehicle * vehicle = new Mech();
bool vehicleIsFighterJet = is<Vehicle, FighterJet>(vehicle);

This returns false because vehicle is of type Mech.
*/
template<typename ThisType, typename CastType>
inline bool is(ThisType * thisObject) {
    return dynamic_cast<CastType *>(thisObject) != NULL;
}

/**
Returns a pointer to an object of a type.
Use this if you already know a pointer is an object of a type otherwise the pointer will be null.

example usage:

Vehicle * vehicle = new FighterJet();
FighterJet * fighterJet = as<Vehicle, FighterJet>(vehicle);

Casting vehicle to a fighter jet on the fly to fire the miniguns on the fighter jet.

as<Vehicle, FighterJet>(vehicle)->fireMiniguns();
*/
template<typename ThisType, typename CastType>
inline CastType * as(ThisType * thisObject) {
    return dynamic_cast<CastType *>(thisObject);
}

/**
Convenient version of the as that returns a useful error if something can't be cast to another object.
Use this if you don't know for sure that something is of a type but is required to be that type.
This will basically help ensure the code is running smoothly.

Pass in the macros __FILE__ and __LINE__ most of the time for file and lineNumber to know the origin of the error if one happens.
*/
template<typename ThisType, typename CastType>
inline CastType * safeAs(ThisType * thisObject, const char * file, unsigned int lineNumber) {
    CastType * retVal;

    if(!(retVal = dynamic_cast<CastType *>(thisObject))) {
        LOG_FATAL_ERROR("Cannot cast");
    }

    return retVal;
}

#endif
