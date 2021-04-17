
#include <log.h>

long sys_putstring(const char *s)
{
    LOGI($(s));
    return 0;
}