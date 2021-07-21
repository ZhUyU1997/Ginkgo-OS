#include <ipc.h>
#include <syscall.h>
#include <print.h>
#include <string.h>

ipc_msg_t *ipc_create_msg(ipc_struct_t *icb, u64_t data_len, u64_t cap_slot_number)
{
    ipc_msg_t *ipc_msg;
    int i;

    ipc_msg = (ipc_msg_t *)icb->shared_buf;
    ipc_msg->data_len = data_len;
    ipc_msg->cap_slot_number = cap_slot_number;

    ipc_msg->data_offset = sizeof(*ipc_msg);
    ipc_msg->cap_slots_offset = ipc_msg->data_offset + data_len;
    memset(ipc_get_msg_data(ipc_msg), 0, data_len);
    for (i = 0; i < cap_slot_number; i++)
        ipc_set_msg_cap(ipc_msg, i, -1);

    return ipc_msg;
}

char *ipc_get_msg_data(ipc_msg_t *ipc_msg)
{
    return (char *)ipc_msg + ipc_msg->data_offset;
}

int ipc_set_msg_data(ipc_msg_t *ipc_msg, char *data, u64_t offset, u64_t len)
{
    if (offset + len < offset || offset + len > ipc_msg->data_len)
        return -1;

    memcpy(ipc_get_msg_data(ipc_msg) + offset, data, len);
    return 0;
}

static u64_t *ipc_get_msg_cap_ptr(ipc_msg_t *ipc_msg, u64_t cap_id)
{
    return (u64_t *)((char *)ipc_msg + ipc_msg->cap_slots_offset) + cap_id;
}

u64_t ipc_get_msg_cap(ipc_msg_t *ipc_msg, u64_t cap_slot_index)
{
    if (cap_slot_index >= ipc_msg->cap_slot_number)
        return -1;
    return *ipc_get_msg_cap_ptr(ipc_msg, cap_slot_index);
}

int ipc_set_msg_cap(ipc_msg_t *ipc_msg, u64_t cap_slot_index, u32_t cap)
{
    if (cap_slot_index >= ipc_msg->cap_slot_number)
        return -1;
    *ipc_get_msg_cap_ptr(ipc_msg, cap_slot_index) = cap;
    return 0;
}

/* FIXME: currently ipc_msg is not dynamically allocated so that no need to free */
int ipc_destroy_msg(ipc_msg_t *ipc_msg)
{
    return 0;
}

#define SERVER_STACK_BASE 0x30000000UL
#define SERVER_STACK_SIZE 0x1000UL
#define SERVER_BUF_BASE 0x30400000UL
#define SERVER_BUF_SIZE 0x1000UL
#define CLIENT_BUF_BASE 0x30800000UL
#define CLIENT_BUF_SIZE 0x1000UL
#define MAX_CLIENT 16

int ipc_register_server(server_handler server_handler)
{
    struct ipc_vm_config vm_config = {
        .stack_base_addr = SERVER_STACK_BASE,
        .stack_size = SERVER_STACK_SIZE,
        .buf_base_addr = SERVER_BUF_BASE,
        .buf_size = SERVER_BUF_SIZE,
    };
    return usys_register_server((u64_t)server_handler, MAX_CLIENT, (u64_t)&vm_config);
}

int ipc_register_named_server(const char *name, server_handler server_handler)
{
    struct ipc_vm_config vm_config = {
        .stack_base_addr = SERVER_STACK_BASE,
        .stack_size = SERVER_STACK_SIZE,
        .buf_base_addr = SERVER_BUF_BASE,
        .buf_size = SERVER_BUF_SIZE,
    };
    return usys_register_named_server(name, (u64_t)server_handler, MAX_CLIENT, (u64_t)&vm_config);
}

int ipc_register_client(int server_thread_cap, ipc_struct_t *ipc_struct)
{
    int conn_cap;

    struct ipc_vm_config vm_config = {
        .buf_base_addr = CLIENT_BUF_BASE,
        .buf_size = CLIENT_BUF_SIZE,
    };

    conn_cap = usys_register_client((u32_t)server_thread_cap, (u64_t)&vm_config);

    if (conn_cap < 0)
        return -1;
    ipc_struct->shared_buf = vm_config.buf_base_addr;
    ipc_struct->shared_buf_len = vm_config.buf_size;
    ipc_struct->conn_cap = conn_cap;

    return 0;
}

int ipc_register_client_by_name(const char *name, ipc_struct_t *ipc_struct)
{
    int conn_cap;

    struct ipc_vm_config vm_config = {
        .buf_base_addr = CLIENT_BUF_BASE,
        .buf_size = CLIENT_BUF_SIZE,
    };

    conn_cap = usys_register_client_by_name(name, (u64_t)&vm_config);

    if (conn_cap < 0)
        return -1;
    ipc_struct->shared_buf = vm_config.buf_base_addr;
    ipc_struct->shared_buf_len = vm_config.buf_size;
    ipc_struct->conn_cap = conn_cap;

    return 0;
}

int ipc_call(ipc_struct_t *icb, ipc_msg_t *ipc_msg)
{
    u64_t ret = 0;
    ret = usys_ipc_call(icb->conn_cap, (u64_t)ipc_msg);

    return ret;
}

void ipc_return(int ret)
{
    usys_ipc_return((u64_t)ret);
}
