#pragma once

#include <printv.h>

#define CONFIG_DEBUG

#ifdef CONFIG_DEBUG
#define LOGI(...) printv("[" $(__FILE__) ":" $(__LINE__) ":" $((char *)__func__) "] "__VA_ARGS__, "\n")
#define LOGD(...) printv("[" $(__FILE__) ":" $(__LINE__) ":" $((char *)__func__) "] \e[32m"__VA_ARGS__, "\e[0m\n")
#define LOGW(...) printv("[" $(__FILE__) ":" $(__LINE__) ":" $((char *)__func__) "] \e[33m"__VA_ARGS__, "\e[0m\n")
#define LOGE(...) printv("[" $(__FILE__) ":" $(__LINE__) ":" $((char *)__func__) "] \e[31m"__VA_ARGS__, "\e[0m\n")
#define LOGF(...) printv("[" $(__FILE__) ":" $(__LINE__) ":" $((char *)__func__) "] \e[35m"__VA_ARGS__, "\e[0m\n")
#define PANIC(...) ({LOGF("[PANIC] ",__VA_ARGS__); while(1);})
#else
#define LOGI(...)
#define LOGD(...)
#define LOGW(...) printv("[" $((char *)__func__) ":" $(__LINE__) "] "__VA_ARGS__, "\n")
#define LOGE(...) printv("[" $((char *)__func__) ":" $(__LINE__) "] "__VA_ARGS__, "\n")
#define LOGF(...) printv("[" $((char *)__func__) ":" $(__LINE__) "] "__VA_ARGS__, "\n")
#define PANIC(...) ({LOGF("[PANIC] ",__VA_ARGS__); while(1);})
#endif