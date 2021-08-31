#include <vfs.h>
#include <ipc.h>
#include <string.h>
#include <stdio.h>

static ipc_struct_t *vfs_client_ipc = &(ipc_struct_t){};

static inline ipc_struct_t *get_vfs_ipc()
{
    return vfs_client_ipc;
}

int vfs_open(const char *path, u32_t flags, u32_t mode)
{
    ipc_msg_t *msg = ipc_create_msg(get_vfs_ipc(), GET_FS_REQUEST_SIZE(VFS_OPEN), 0);
    struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(msg);
    fr->req = VFS_OPEN;
    GET_FS_REQUEST(param, VFS_OPEN, fr);

    strncpy(param->path, path, VFS_MAX_PATH);
    param->flags = flags;
    param->mode = mode;

    int fd = ipc_call(get_vfs_ipc(), msg);
    ipc_destroy_msg(msg);
    return fd;
}

int vfs_close(int fd)
{
    ipc_msg_t *msg = ipc_create_msg(get_vfs_ipc(), GET_FS_REQUEST_SIZE(VFS_CLOSE), 0);
    struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(msg);
    fr->req = VFS_CLOSE;
    GET_FS_REQUEST(param, VFS_CLOSE, fr);
    param->fd = fd;
    int ret = ipc_call(get_vfs_ipc(), msg);
    ipc_destroy_msg(msg);
    return ret;
}

u64_t vfs_read(int fd, void *buf, u64_t len)
{
    ipc_msg_t *msg = ipc_create_msg(get_vfs_ipc(), GET_FS_REQUEST_SIZE(VFS_READ) + len, 0);
    struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(msg);
    fr->req = VFS_READ;
    GET_FS_REQUEST(param, VFS_READ, fr);

    param->fd = fd;
    param->len = len;

    u64_t ret = ipc_call(get_vfs_ipc(), msg);
    memcpy(buf, param->buf, len); //TODO: check ret

    ipc_destroy_msg(msg);
    return ret;
}

u64_t vfs_write(int fd, void *buf, u64_t len)
{
    ipc_msg_t *msg = ipc_create_msg(get_vfs_ipc(), GET_FS_REQUEST_SIZE(VFS_WRITE) + len, 0);
    struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(msg);
    fr->req = VFS_WRITE;
    GET_FS_REQUEST(param, VFS_WRITE, fr);

    param->fd = fd;
    param->len = len;
    memcpy(param->buf, buf, len);

    u64_t ret = ipc_call(get_vfs_ipc(), msg);
    ipc_destroy_msg(msg);
    return ret;
}

s64_t vfs_lseek(int fd, s64_t off, int whence)
{
    ipc_msg_t *msg = ipc_create_msg(get_vfs_ipc(), GET_FS_REQUEST_SIZE(VFS_LSEEK), 0);
    struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(msg);
    fr->req = VFS_LSEEK;
    GET_FS_REQUEST(param, VFS_LSEEK, fr);

    param->fd = fd;
    param->off = off;
    param->whence = whence;

    s64_t ret = ipc_call(get_vfs_ipc(), msg);
    ipc_destroy_msg(msg);
    return ret;
}

int vfs_fsync(int fd)
{
    ipc_msg_t *msg = ipc_create_msg(get_vfs_ipc(), GET_FS_REQUEST_SIZE(VFS_FSYNC), 0);
    struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(msg);
    fr->req = VFS_FSYNC;
    GET_FS_REQUEST(param, VFS_FSYNC, fr);

    param->fd = fd;

    int ret = ipc_call(get_vfs_ipc(), msg);
    ipc_destroy_msg(msg);
    return ret;
}

int vfs_fchmod(int fd, u32_t mode)
{
    ipc_msg_t *msg = ipc_create_msg(get_vfs_ipc(), GET_FS_REQUEST_SIZE(VFS_FCHMOD), 0);
    struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(msg);
    fr->req = VFS_FCHMOD;
    GET_FS_REQUEST(param, VFS_FCHMOD, fr);

    param->fd = fd;

    int ret = ipc_call(get_vfs_ipc(), msg);
    ipc_destroy_msg(msg);
    return ret;
}

int vfs_fstat(int fd, struct vfs_stat_t *st)
{
    ipc_msg_t *msg = ipc_create_msg(get_vfs_ipc(), GET_FS_REQUEST_SIZE(VFS_FSTAT), 0);
    struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(msg);
    fr->req = VFS_FSTAT;
    GET_FS_REQUEST(param, VFS_FSTAT, fr);

    param->fd = fd;

    int ret = ipc_call(get_vfs_ipc(), msg);
    ipc_destroy_msg(msg);
    return ret;
}

int vfs_opendir(const char *name)
{
    ipc_msg_t *msg = ipc_create_msg(get_vfs_ipc(), GET_FS_REQUEST_SIZE(VFS_OPENDIR), 0);
    struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(msg);
    fr->req = VFS_OPENDIR;
    GET_FS_REQUEST(param, VFS_OPENDIR, fr);

    strncpy(param->name, name, VFS_MAX_PATH);

    int ret = ipc_call(get_vfs_ipc(), msg);
    ipc_destroy_msg(msg);
    return ret;
}

int vfs_closedir(int fd)
{
    ipc_msg_t *msg = ipc_create_msg(get_vfs_ipc(), GET_FS_REQUEST_SIZE(VFS_CLOSEDIR), 0);
    struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(msg);
    fr->req = VFS_CLOSEDIR;
    GET_FS_REQUEST(param, VFS_CLOSEDIR, fr);

    param->fd = fd;

    int ret = ipc_call(get_vfs_ipc(), msg);
    ipc_destroy_msg(msg);
    return ret;
}

int vfs_readdir(int fd, struct vfs_dirent_t *dir)
{
    ipc_msg_t *msg = ipc_create_msg(get_vfs_ipc(), GET_FS_REQUEST_SIZE(VFS_READDIR), 0);
    struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(msg);
    fr->req = VFS_READDIR;
    GET_FS_REQUEST(param, VFS_READDIR, fr);

    param->fd = fd;

    int ret = ipc_call(get_vfs_ipc(), msg);

    *dir = param->dir;

    ipc_destroy_msg(msg);
    return ret;
}

int vfs_rewinddir(int fd)
{
    ipc_msg_t *msg = ipc_create_msg(get_vfs_ipc(), GET_FS_REQUEST_SIZE(VFS_REWINDDIR), 0);
    struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(msg);
    fr->req = VFS_REWINDDIR;
    GET_FS_REQUEST(param, VFS_REWINDDIR, fr);

    param->fd = fd;

    int ret = ipc_call(get_vfs_ipc(), msg);
    ipc_destroy_msg(msg);
    return ret;
}

int vfs_mkdir(const char *path, u32_t mode)
{
    ipc_msg_t *msg = ipc_create_msg(get_vfs_ipc(), GET_FS_REQUEST_SIZE(VFS_MKDIR), 0);
    struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(msg);
    fr->req = VFS_MKDIR;
    GET_FS_REQUEST(param, VFS_MKDIR, fr);

    strncpy(param->path, path, VFS_MAX_PATH);

    param->mode = mode;

    int ret = ipc_call(get_vfs_ipc(), msg);
    ipc_destroy_msg(msg);
    return ret;
}

int vfs_rmdir(const char *path)
{
    ipc_msg_t *msg = ipc_create_msg(get_vfs_ipc(), GET_FS_REQUEST_SIZE(VFS_RMDIR), 0);
    struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(msg);
    fr->req = VFS_RMDIR;
    GET_FS_REQUEST(param, VFS_RMDIR, fr);

    strncpy(param->path, path, VFS_MAX_PATH);

    int ret = ipc_call(get_vfs_ipc(), msg);
    ipc_destroy_msg(msg);
    return ret;
}

int vfs_rename(const char *src, const char *dst)
{
    ipc_msg_t *msg = ipc_create_msg(get_vfs_ipc(), GET_FS_REQUEST_SIZE(VFS_RENAME), 0);
    struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(msg);
    fr->req = VFS_RENAME;
    GET_FS_REQUEST(param, VFS_RENAME, fr);

    strncpy(param->src, src, VFS_MAX_PATH);

    strncpy(param->dst, dst, VFS_MAX_PATH);

    int ret = ipc_call(get_vfs_ipc(), msg);
    ipc_destroy_msg(msg);
    return ret;
}

int vfs_unlink(const char *path)
{
    ipc_msg_t *msg = ipc_create_msg(get_vfs_ipc(), GET_FS_REQUEST_SIZE(VFS_UNLINK), 0);
    struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(msg);
    fr->req = VFS_UNLINK;
    GET_FS_REQUEST(param, VFS_UNLINK, fr);

    strncpy(param->path, path, VFS_MAX_PATH);

    int ret = ipc_call(get_vfs_ipc(), msg);
    ipc_destroy_msg(msg);
    return ret;
}

int vfs_access(const char *path, u32_t mode)
{
    ipc_msg_t *msg = ipc_create_msg(get_vfs_ipc(), GET_FS_REQUEST_SIZE(VFS_ACCESS), 0);
    struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(msg);
    fr->req = VFS_ACCESS;
    GET_FS_REQUEST(param, VFS_ACCESS, fr);
    strncpy(param->path, path, VFS_MAX_PATH);
    param->mode = mode;

    int ret = ipc_call(get_vfs_ipc(), msg);
    ipc_destroy_msg(msg);
    return ret;
}

int vfs_chmod(const char *path, u32_t mode)
{
    ipc_msg_t *msg = ipc_create_msg(get_vfs_ipc(), GET_FS_REQUEST_SIZE(VFS_CHMOD), 0);
    struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(msg);
    fr->req = VFS_CHMOD;
    GET_FS_REQUEST(param, VFS_CHMOD, fr);

    strncpy(param->path, path, VFS_MAX_PATH);
    param->mode = mode;
    int ret = ipc_call(get_vfs_ipc(), msg);
    ipc_destroy_msg(msg);
    return ret;
}

int vfs_stat(const char *path, struct vfs_stat_t *st)
{
    ipc_msg_t *msg = ipc_create_msg(get_vfs_ipc(), GET_FS_REQUEST_SIZE(VFS_STAT), 0);
    struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(msg);
    fr->req = VFS_STAT;
    GET_FS_REQUEST(param, VFS_STAT, fr);

    strncpy(param->path, path, VFS_MAX_PATH);

    int ret = ipc_call(get_vfs_ipc(), msg);

    *st = param->st;
    ipc_destroy_msg(msg);
    return ret;
}

void do_vfs_init(void)
{
    printf("[vfs client]\n");
    ipc_register_client_by_name("vfs", vfs_client_ipc);
}