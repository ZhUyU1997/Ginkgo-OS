#include <syscall.h>
#include <string.h>

#include <types.h>
#include <stdio.h>
#include <ipc.h>

int main(int argc, char **argv)
{
    printf("test vfs\n");
    ipc_struct_t client_ipc_struct;
    ipc_register_client_by_name("vfs", &client_ipc_struct);
	ipc_msg_t *ipc_msg = ipc_create_msg(&client_ipc_struct, 100, 0);
	ipc_set_msg_data(ipc_msg, "Hello IPC", 0, strlen("Hello IPC") + 1);

    int ret = ipc_call(&client_ipc_struct, NULL);
    printf("ipc_call ret:%d\n", ret);
	ipc_destroy_msg(ipc_msg);

    printf("test vfs end\n");
    while(1);
    return 0;
}