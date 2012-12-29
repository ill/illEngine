/*
 * debug.h
 *
 *  Created on: Dec 29, 2012
 *      Author: Ilya
 */

#ifndef DEBUG_H_
#define DEBUG_H_

//TODO: my own version of assert possibly?  I forget what it was missing that I wish it had.

/*
These are macros and not first class functions so that the debugger breaks
on the assertion line and not in some random guts of SDL, and so each
assert can have unique static variables associated with it.
*/

//This is lifted from SDL_assert.h so I can use these functions even if I'm not using SDL

#if defined(_MSC_VER) && !defined(_WIN32_WCE)
/* Don't include intrin.h here because it contains C++ code */
extern void __cdecl __debugbreak(void);
    #define TriggerBreakpoint() __debugbreak()
#elif (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
    #define TriggerBreakpoint() __asm__ __volatile__ ( "int $3\n\t" )
#elif defined(HAVE_SIGNAL_H)
    #include <signal.h>
    #define TriggerBreakpoint() raise(SIGTRAP)
#else
    /* How do we trigger breakpoints on this platform? */
    #define TriggerBreakpoint()
#endif


#endif /* DEBUG_H_ */
