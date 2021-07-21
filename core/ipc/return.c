#include <task.h>
#include <core/ipc.h>
#include <core/sched.h>

/**
 * Lab4
 * Helper function
 * Server thread calls this function and then return to client thread
 * This function should never return
 *
 * Replace the place_holder to correct value!
 */
static int thread_migrate_to_client(ipc_connection_t *conn, u64_t ret_value)
{
    thread_t *source = conn->source;
    thread_self()->active_conn = NULL;
    source->thread_info.regs->a0 = ret_value;
    schedule_to(source);
    return 0;
}

/**
 * The thread of ipc_connection calls sys_ipc_return
 * you should migrate to the client now.
 * This function should never return!
 */
void sys_ipc_return(u64_t ret)
{
    ipc_connection_t *conn = thread_self()->active_conn;
    thread_migrate_to_client(conn, ret);
    return;
}
