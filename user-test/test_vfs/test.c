#include <syscall.h>
#include <string.h>

#include "types.h"
#include "print.h"
#include "ipc.h"

int main(int argc, char **argv)
{
    printf("test vfs\n");
    ipc_struct_t client_ipc_struct;
    ipc_register_client_by_name("vfs", &client_ipc_struct);
    ipc_call(&client_ipc_struct, NULL);

    printf("test vfs end\n");
    return 0;
}