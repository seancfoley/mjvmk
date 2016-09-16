#include "thread.h"

/* the current thread register, altered each time a thread switch occurs */

THREAD pGlobalCurrentThread;

/* the idle thread identifier, always remains the same */

THREAD pIdleThread;


