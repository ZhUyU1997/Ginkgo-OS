#include <task.h>
#include <core/ipc.h>
#include <core/sched.h>
#include <uaccess.h>
#include <malloc.h>
#include <log.h>

/**
 * A helper function to transfer all the ipc_msg's capbilities of client's
 * process to server's process
 */
int ipc_send_cap(ipc_connection_t *conn, ipc_msg_t *ipc_msg)
{
    u64_t cap_slot_number;
    u64_t cap_slots_offset;

    copy_from_user((char *)&cap_slot_number, (char *)&ipc_msg->cap_slot_number, sizeof(cap_slot_number));
    copy_from_user((char *)&cap_slots_offset, (char *)&ipc_msg->cap_slots_offset, sizeof(cap_slots_offset));

    if (cap_slot_number == 0)
        return 0;

    u64_t *cap_buf = malloc(cap_slot_number * sizeof(*cap_buf));
    copy_from_user((char *)cap_buf, (char *)ipc_msg + cap_slots_offset, sizeof(*cap_buf) * cap_slot_number);

    for (int i = 0; i < cap_slot_number; i++)
    {
        u64_t dest_cap = slot_copy(process_self(), conn->target->process, cap_buf[i]);
        cap_buf[i] = dest_cap;
    }

    copy_to_user((char *)ipc_msg + cap_slots_offset, (char *)cap_buf, sizeof(*cap_buf) * cap_slot_number);

    free(cap_buf);
    return 0;
}

/**
 * Client thread calls this function and then return to server thread
 * This function should never return
 */
static u64_t thread_migrate_to_server(ipc_connection_t *conn, u64_t arg)
{
    thread_t *target = conn->target;
    conn->source = thread_self();
    target->active_conn = conn;

    struct pt_regs *regs = (struct pt_regs *)(target->thread_info.kernel_sp - sizeof(struct pt_regs));

    regs->sp = (uintptr_t)conn->server_stack_top;
    regs->a0 = (uintptr_t)arg;
    regs->sepc = (uintptr_t)conn->target->server_ipc_config->callback;
    schedule_to(target);
    return 0;
}

/**
 * Lab4
 * The client thread calls sys_ipc_call to migrate to the server thread.
 * When you transfer the ipc_msg (which is the virtual address in the client
 * vmspace), do not forget to change the virtual address to server's vmspace.
 * This function should never return!
 */
u64_t sys_ipc_call(u32_t conn_cap, ipc_msg_t *ipc_msg)
{
    ipc_connection_t *conn = dynamic_cast(ipc_connection_t)(slot_get(process_self(), conn_cap));

    if (ipc_msg != NULL)
    {
        // transfer all the capbiliies of client thread to
        copy_to_user((char *)&ipc_msg->server_conn_cap, (char *)&conn->server_conn_cap, sizeof(u64_t));
        ipc_send_cap(conn, ipc_msg);
    }

    u64_t arg = conn->buf.server_user_addr;
    thread_migrate_to_server(conn, arg);
    return 0;
}

u64_t sys_ipc_reg_call(u32_t conn_cap, u64_t arg0)
{
    ipc_connection_t *conn = dynamic_cast(ipc_connection_t)(slot_get(process_self(), conn_cap));
    thread_migrate_to_server(conn, arg0);
    return 0;
}
