#pragma once

#include <types.h>

typedef struct ipc_struct {
	u64_t conn_cap;
	u64_t shared_buf;
	u64_t shared_buf_len;
} ipc_struct_t;

typedef struct ipc_msg {
	u64_t server_conn_cap;
	u64_t data_len;
	u64_t cap_slot_number;
	u64_t data_offset;
	u64_t cap_slots_offset;
} ipc_msg_t;

struct ipc_vm_config {
	u64_t stack_base_addr;
	u64_t stack_size;
	u64_t buf_base_addr;
	u64_t buf_size;
};

int ipc_register_client(int server_thread_cap, ipc_struct_t * ipc_struct);
int ipc_register_client_by_name(const char *name, ipc_struct_t * ipc_struct);
ipc_msg_t *ipc_create_msg(ipc_struct_t * icb, u64_t data_len,u64_t cap_slot_number);
char *ipc_get_msg_data(ipc_msg_t * ipc_msg);
u64_t ipc_get_msg_cap(ipc_msg_t * ipc_msg, u64_t cap_id);
int ipc_set_msg_data(ipc_msg_t * ipc_msg, char *data, u64_t offset, u64_t len);
int ipc_set_msg_cap(ipc_msg_t * ipc_msg, u64_t cap_slot_index, u32_t cap);
int ipc_destroy_msg(ipc_msg_t * ipc_msg);

int ipc_call(ipc_struct_t * icb, ipc_msg_t * ipc_msg);
int ipc_reg_call(ipc_struct_t * icb, u64_t arg);
void ipc_return(int ret);

typedef void (*server_handler) (ipc_msg_t * ipc_msg);
int ipc_register_server(server_handler server_handler);
int ipc_register_named_server(const char *name, server_handler server_handler);

#define INFO_PAGE_VADDR ((void *)0x100000ll)
