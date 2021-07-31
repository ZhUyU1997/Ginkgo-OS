#include "syscall.h"
#include "types.h"
#include "print.h"
#include "ipc.h"

static int exit_flag = 0;
static void fs_dispatch(ipc_msg_t *ipc_msg)
{
    const char *s = ipc_get_msg_data(ipc_msg);
    printf("ipc_msg:%s\n", s);

    static int i = 0;
    ipc_return(i++);
}

int main(int argc, char **argv)
{
    printf("vfs_server\n");
    ipc_register_named_server("vfs", fs_dispatch);
    printf("vfs_server end\n");
    while (exit_flag != 1) {
		usys_yield();
	}

    return 0;
}