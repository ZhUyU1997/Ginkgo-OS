#pragma once

struct thread_t;
typedef struct thread_t thread_t;

#include <types.h>
#include <core/class.h>
#include <core/kobject.h>

/*
 * Used in both server and client register.
 * Stack setting is invalid in client register.
 */
struct ipc_vm_config
{
    u64_t stack_base_addr;
    u64_t stack_size;
    u64_t buf_base_addr;
    u64_t buf_size;
};

#define IPC_MAX_CONN_PER_SERVER 32
struct server_ipc_config
{
    // I dont know how to specify the maximum callback number
    u64_t callback;
    u64_t max_client;
    /* bitmap for shared buffer and stack allocation */
    unsigned long conn_bmp;
    struct ipc_vm_config vm_config;
};

struct shared_buf
{
    u64_t client_user_addr;
    u64_t server_user_addr;
    u64_t size;
};

class(ipc_connection_t, kobject_t)
{
    /* Source Thread */
    thread_t *source;
    /* Target Thread */
    thread_t *target;
    /* Conn cap in server */
    u64_t server_conn_cap;
    /* Target function */
    u64_t callback;

    /* Shadow server stack top */
    u64_t server_stack_top;
    u64_t server_stack_size;

    /* Shared buffer */
    struct shared_buf buf;
};

typedef struct ipc_msg
{
    u64_t server_conn_cap;
    u64_t data_len;
    u64_t cap_slot_number;
    u64_t data_offset;
    u64_t cap_slots_offset;
} ipc_msg_t;

/* syscall related to IPC */
u64_t sys_register_server(u64_t callback, u64_t max_client, u64_t vm_config_ptr);
u32_t sys_register_client(u32_t server_cap, u64_t vm_config_ptr);
u64_t sys_ipc_call(u32_t conn_cap, ipc_msg_t *ipc_msg);
u64_t sys_ipc_reg_call(u32_t conn_cap, u64_t arg);
void sys_ipc_return(u64_t ret);

thread_t *search_named_server(const char *name);