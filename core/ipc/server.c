#include <core/ipc.h>
#include <task.h>
#include <malloc.h>
#include <uaccess.h>
#include <log.h>
#include <hmap.h>

static struct hmap_t *map = NULL;

/**
 * The core function for registering the server
 */
static int register_server(thread_t *server, u64_t callback, u64_t max_client, u64_t vm_config_ptr)
{
    struct server_ipc_config *server_ipc_config = malloc(sizeof(struct server_ipc_config));
    server_ipc_config->callback = callback;
    server_ipc_config->max_client = max_client;
    server_ipc_config->conn_bmp = 0;

    server->server_ipc_config = server_ipc_config;
    struct ipc_vm_config *vm_config = &server_ipc_config->vm_config;
    copy_from_user((char *)vm_config, (char *)vm_config_ptr, sizeof(struct ipc_vm_config));
    return 0;
}

u64_t sys_register_server(u64_t callback, u64_t max_client, u64_t vm_config_ptr)
{
    return register_server(thread_self(), callback, max_client, vm_config_ptr);
}

u64_t sys_register_named_server(u64_t name, u64_t callback, u64_t max_client, u64_t vm_config_ptr)
{
    if (map == NULL)
    {
        map = hmap_alloc(0);
    }

    int ret = register_server(thread_self(), callback, max_client, vm_config_ptr);

    hmap_add(map, strdup(name), thread_self());
    return ret;
}

thread_t *search_named_server(const char *name)
{
    thread_t *ret = hmap_search(map, name);
    return ret;
}