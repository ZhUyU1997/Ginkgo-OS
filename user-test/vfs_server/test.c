#include "syscall.h"
#include "types.h"
#include "print.h"
#include "ipc.h"

static int exit_flag = 0;
static void fs_dispatch(ipc_msg_t *ipc_msg)
{
    printf("fs_dispatch\n");
    ipc_return(0);
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