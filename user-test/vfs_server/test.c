#include <stddef.h>
#include <vfs.h>
#include "syscall.h"
#include "types.h"
#include "print.h"
#include "ipc.h"

static s64_t fs_dispatch(struct fs_request *fr)
{
    switch (fr->req)
    {
    case VFS_OPEN:
    {
        GET_FS_REQUEST(param, VFS_OPEN, fr);
        return vfs_open(param->path, param->flags, param->mode);
    }
    case VFS_CLOSE:
    {
        GET_FS_REQUEST(param, VFS_CLOSE, fr);
        return vfs_close(param->fd);
    }
    case VFS_READ:
    {
        GET_FS_REQUEST(param, VFS_READ, fr);
        return vfs_read(param->fd, param->buf, param->len);
    }
    case VFS_WRITE:
    {
        GET_FS_REQUEST(param, VFS_WRITE, fr);
        return vfs_write(param->fd, param->buf, param->len);
    }
    case VFS_LSEEK:
    {
        GET_FS_REQUEST(param, VFS_LSEEK, fr);
        return vfs_lseek(param->fd, param->off, param->whence);
    }
    case VFS_FSYNC:
    {
        GET_FS_REQUEST(param, VFS_FSYNC, fr);
        return vfs_fsync(param->fd);
    }
    case VFS_FCHMOD:
    {
        GET_FS_REQUEST(param, VFS_FCHMOD, fr);
        return vfs_fchmod(param->fd, param->mode);
    }
    case VFS_FSTAT:
    {
        GET_FS_REQUEST(param, VFS_FSTAT, fr);
        return vfs_fstat(param->fd, &param->st);
    }
    case VFS_OPENDIR:
    {
        GET_FS_REQUEST(param, VFS_OPENDIR, fr);
        return vfs_opendir(param->name);
    }
    case VFS_CLOSEDIR:
    {
        GET_FS_REQUEST(param, VFS_CLOSEDIR, fr);
        return vfs_closedir(param->fd);
    }
    case VFS_READDIR:
    {
        GET_FS_REQUEST(param, VFS_READDIR, fr);
        return vfs_readdir(param->fd, &param->dir);
    }
    case VFS_REWINDDIR:
    {
        GET_FS_REQUEST(param, VFS_REWINDDIR, fr);
        return vfs_rewinddir(param->fd);
    }
    case VFS_MKDIR:
    {
        GET_FS_REQUEST(param, VFS_MKDIR, fr);
        return vfs_mkdir(param->path, param->mode);
    }
    case VFS_RMDIR:
    {
        GET_FS_REQUEST(param, VFS_RMDIR, fr);
        return vfs_rmdir(param->path);
    }
    case VFS_RENAME:
    {
        GET_FS_REQUEST(param, VFS_RENAME, fr);
        return vfs_rename(param->src, param->dst);
    }
    case VFS_UNLINK:
    {
        GET_FS_REQUEST(param, VFS_UNLINK, fr);
        return vfs_unlink(param->path);
    }
    case VFS_ACCESS:
    {
        GET_FS_REQUEST(param, VFS_ACCESS, fr);
        return vfs_access(param->path, param->mode);
    }
    case VFS_CHMOD:
    {
        GET_FS_REQUEST(param, VFS_CHMOD, fr);
        return vfs_chmod(param->path, param->mode);
    }
    case VFS_STAT:
    {
        GET_FS_REQUEST(param, VFS_STAT, fr);
        return vfs_stat(param->path, &param->st);
    }
    default:
        break;
    }
    printf("Exception req:%d\n", fr->req);
    return -1;
}

static void ipc_handler(ipc_msg_t *ipc_msg)
{
    printf("ipc_msg %x\n", ipc_msg);

    struct fs_request *fr = ipc_get_msg_data(ipc_msg);
    s64_t ret = fs_dispatch(fr);
    ipc_return(ret);
}

u64_t block_read(struct block_t *blk, u8_t *buf, u64_t offset, u64_t count)
{
    return usys_block_read(buf, offset, count);
}
u64_t block_write(struct block_t *blk, u8_t *buf, u64_t offset, u64_t count)
{
    return usys_block_write(buf, offset, count);
}

void block_sync(struct block_t *blk)
{
}

extern struct filesystem_t filesystem_cpio;

struct block_t block_dev = {
    .name = "virtio",
};

static int exit_flag = 0;
int main(int argc, char **argv)
{
    printf("vfs_server\n");
    ipc_register_named_server("vfs", ipc_handler);
    do_vfs_init();
    block_dev.blksz = usys_block_size();
    block_dev.blkcnt = usys_block_count();

    vfs_mount(&block_dev, &filesystem_cpio, "/", MOUNT_RO);

    while (exit_flag != 1)
        usys_yield();

    return 0;
}