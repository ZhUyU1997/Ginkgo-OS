#pragma once

#include <types.h>

typedef enum
{
    VMO_ANONYM = 0,     /* lazy allocation */
    VMO_DATA = 1,       /* immediate allocation */
    VMO_FILE = 2,       /* file backed */
    VMO_SHM = 3,        /* shared memory */
    VMO_USER_PAGER = 4, /* support user pager */
    VMO_DEVICE = 5,     /* memory mapped device registers */
} vmo_type_t;

#define VM_READ  (1 << 0)
#define VM_WRITE (1 << 1)
#define VM_EXEC  (1 << 2)

void usys_putc(char c);
int usys_process_create();
void usys_process_exit(int);

int usys_vmo_create(u64_t size, u64_t type);
int usys_vmo_write(u64_t slot, u64_t offset, u64_t user_ptr, u64_t len);
int usys_vmo_read(u64_t slot, u64_t offset, u64_t user_ptr, u64_t len);
int usys_vmo_map(u64_t target_process_slot, u64_t slot, u64_t addr, u64_t prot, u64_t flags);
u64_t usys_register_server(u64_t callback, u64_t max_client, u64_t vm_config_ptr);
u64_t usys_register_named_server(const char *name, u64_t callback, u64_t max_client, u64_t vm_config_ptr);
u32_t usys_register_client(u32_t server_cap, u64_t vm_config_ptr);
u32_t usys_register_client_by_name(const char *name, u64_t vm_config_ptr);
u32_t usys_ipc_call(u64_t icb, u64_t ipc_msg);
void usys_ipc_return(u64_t ret);

void usys_yield();