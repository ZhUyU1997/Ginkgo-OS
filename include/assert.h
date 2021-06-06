#pragma once

#include <log.h>
#define assert(cond) do {if(!(cond))PANIC(#cond);}while(0)
#define static_assert _Static_assert
