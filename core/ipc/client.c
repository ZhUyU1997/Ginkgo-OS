#include <task.h>
#include <core/ipc.h>
#include <malloc.h>
#include <uaccess.h>
#include <ffs.h>
#include <log.h>

#define min(x, y) (         \
    {                       \
        typeof(x) _x = (x); \
        typeof(y) _y = (y); \
        (void)(&_x == &_y); \
        _x < _y ? _x : _y;  \
    })

class_impl(ipc_connection_t, kobject_t){};
/**
 * Helper function called when an ipc_connection is created
 */
static thread_t *create_server_thread(thread_t *target)
{
    thread_t *thread = thread_create(target->process, NULL, NULL, NULL);

    thread->server_ipc_config = calloc(1, sizeof(struct server_ipc_config));
    thread->server_ipc_config->callback = target->server_ipc_config->callback;
    thread->server_ipc_config->vm_config = target->server_ipc_config->vm_config;
    return thread;
}

/**
 * Helper function to create an ipc_connection by the client thread
 */
static int create_connection(thread_t *source, thread_t *target, struct ipc_vm_config *client_vm_config)
{
    // Get the ipc_connection
    // Get the server's ipc config
    struct server_ipc_config *server_ipc_config = target->server_ipc_config;
    struct ipc_vm_config *server_vm_config = &server_ipc_config->vm_config;
    //TODO:
    int conn_idx = ffs(server_ipc_config->conn_bmp);
    LOGI("idx:" $(conn_idx));
    server_ipc_config->conn_bmp |= 1UL << conn_idx;

    // Create the server thread's stack
    u64_t server_stack_base = server_vm_config->stack_base_addr + conn_idx * server_vm_config->stack_size;
    u64_t stack_size = server_vm_config->stack_size;

    vmobject_t *stack_vmo = vmo_create(stack_size, VMO_DATA);
    slot_alloc_install(target->process, stack_vmo);
    bool_t ret = vmspace_map_range_user(target->vmspace, server_stack_base, stack_size, PROT_WRITE | PROT_READ, 0, stack_vmo);
    LOGI("MAP migrate thread statck ["$(server_stack_base)"]["$(ret)"]");

    // Create and map the shared buffer for client and server
    u64_t server_buf_base = server_vm_config->buf_base_addr + conn_idx * server_vm_config->buf_size;
    u64_t client_buf_base = client_vm_config->buf_base_addr;
    u64_t buf_size = min(server_vm_config->buf_size, client_vm_config->buf_size);
    client_vm_config->buf_size = buf_size;

    vmobject_t *buf_vmo = vmo_create(buf_size, VMO_DATA);
    slot_alloc_install(source->process, buf_vmo);
    slot_alloc_install(target->process, buf_vmo);
    LOGI("buf_size:" $(buf_size));

    vmspace_map_range_user(source->vmspace, client_buf_base, buf_size, PROT_WRITE | PROT_READ, 0, buf_vmo);
    vmspace_map_range_user(target->vmspace, server_buf_base, buf_size, PROT_WRITE | PROT_READ, 0, buf_vmo);
    LOGI("MAP migrate thread buf ["$(server_buf_base)"]");

    ipc_connection_t *conn = new (ipc_connection_t);
    conn->target = create_server_thread(target);
    conn->server_stack_top = server_stack_base + stack_size;
    conn->buf.client_user_addr = client_buf_base;
    conn->buf.server_user_addr = server_buf_base;

    int conn_cap = slot_alloc_install(source->process, conn);
    int server_conn_cap = slot_copy(source->process, target->process, conn_cap);

    conn->server_conn_cap = server_conn_cap;
    return conn_cap;
}

u32_t sys_register_client(u32_t server_cap, u64_t vm_config_ptr)
{
    thread_t *client = thread_self();
    thread_t *server = dynamic_cast(thread_t)(slot_get(process_self(), server_cap));

    struct ipc_vm_config vm_config = {0};
    assign_struct_from_user(&vm_config, vm_config_ptr);

    int conn_cap = create_connection(client, server, &vm_config);
    assign_struct_to_user(vm_config_ptr, &vm_config);
    return conn_cap;
}

u32_t sys_register_client_by_name(const char *name, u64_t vm_config_ptr)
{
    thread_t *client = thread_self();
    thread_t *server = search_named_server(name);

    if (server == NULL)
    {
        LOGE("Unable to find named server(" $(name) ")");
        return 0;
    }

    struct ipc_vm_config vm_config = {0};
    assign_struct_from_user(&vm_config, vm_config_ptr);

    int conn_cap = create_connection(client, server, &vm_config);
    assign_struct_to_user(vm_config_ptr, &vm_config);
    return conn_cap;
}