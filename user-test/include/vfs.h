#pragma once

#include <types.h>
#include <stddef.h>
#include <list.h>
#include <mutex.h>

#define VFS_MAX_PATH (1024)
#define VFS_MAX_NAME (256)
#define VFS_MAX_FD (256)
#define VFS_NODE_HASH_SIZE (256)

#define O_RDONLY (1 << 0)
#define O_WRONLY (1 << 1)
#define O_RDWR (O_RDONLY | O_WRONLY)
#define O_ACCMODE (O_RDWR)

#define O_CREAT (1 << 8)
#define O_EXCL (1 << 9)
#define O_NOCTTY (1 << 10)
#define O_TRUNC (1 << 11)
#define O_APPEND (1 << 12)
#define O_DSYNC (1 << 13)
#define O_NONBLOCK (1 << 14)
#define O_SYNC (1 << 15)

#define S_IXOTH (1 << 0)
#define S_IWOTH (1 << 1)
#define S_IROTH (1 << 2)
#define S_IRWXO (S_IROTH | S_IWOTH | S_IXOTH)

#define S_IXGRP (1 << 3)
#define S_IWGRP (1 << 4)
#define S_IRGRP (1 << 5)
#define S_IRWXG (S_IRGRP | S_IWGRP | S_IXGRP)

#define S_IXUSR (1 << 6)
#define S_IWUSR (1 << 7)
#define S_IRUSR (1 << 8)
#define S_IRWXU (S_IRUSR | S_IWUSR | S_IXUSR)

#define S_IFDIR (1 << 16)
#define S_IFCHR (1 << 17)
#define S_IFBLK (1 << 18)
#define S_IFREG (1 << 19)
#define S_IFLNK (1 << 20)
#define S_IFIFO (1 << 21)
#define S_IFSOCK (1 << 22)
#define S_IFMT (S_IFDIR | S_IFCHR | S_IFBLK | S_IFREG | S_IFLNK | S_IFIFO | S_IFSOCK)

#define S_ISDIR(m) ((m)&S_IFDIR)
#define S_ISCHR(m) ((m)&S_IFCHR)
#define S_ISBLK(m) ((m)&S_IFBLK)
#define S_ISREG(m) ((m)&S_IFREG)
#define S_ISLNK(m) ((m)&S_IFLNK)
#define S_ISFIFO(m) ((m)&S_IFIFO)
#define S_ISSOCK(m) ((m)&S_IFSOCK)

#define R_OK (1 << 2)
#define W_OK (1 << 1)
#define X_OK (1 << 0)

#define VFS_SEEK_SET (0)
#define VFS_SEEK_CUR (1)
#define VFS_SEEK_END (2)

struct vfs_stat_t;
struct vfs_dirent_t;
struct vfs_node_t;
struct vfs_mount_t;
struct filesystem_t;

struct vfs_stat_t {
	u64_t st_ino;
	s64_t st_size;
	u32_t st_mode;
	u64_t st_dev;
	u32_t st_uid;
	u32_t st_gid;
	u64_t st_ctime;
	u64_t st_atime;
	u64_t st_mtime;
};

enum vfs_dirent_type_t {
	VDT_UNK,
	VDT_DIR,
	VDT_CHR,
	VDT_BLK,
	VDT_REG,
	VDT_LNK,
	VDT_FIFO,
	VDT_SOCK,
};

struct vfs_dirent_t {
	u64_t d_off;
	u32_t d_reclen;
	enum vfs_dirent_type_t d_type;
	char d_name[VFS_MAX_NAME];
};

enum vfs_node_flag_t {
	VNF_NONE,
	VNF_ROOT,
};

enum vfs_node_type_t {
	VNT_UNK,
	VNT_DIR,
	VNT_CHR,
	VNT_BLK,
	VNT_REG,
	VNT_LNK,
	VNT_FIFO,
	VNT_SOCK,
};

struct vfs_node_t {
	struct list_head v_link;
	struct vfs_mount_t * v_mount;
	atomic_t v_refcnt;
	char v_path[VFS_MAX_PATH];
	enum vfs_node_flag_t v_flags;
	enum vfs_node_type_t v_type;
	struct mutex_t v_lock;
	u64_t v_ctime;
	u64_t v_atime;
	u64_t v_mtime;
	u32_t v_mode;
	s64_t v_size;
	void * v_data;
};

enum {
	MOUNT_RW	= (0x0 << 0),
	MOUNT_RO	= (0x1 << 0),
	MOUNT_MASK	= (0x1 << 0),
};

struct vfs_mount_t {
	struct list_head m_link;
	struct filesystem_t * m_fs;
	struct block_t * m_dev;
	char m_path[VFS_MAX_PATH];
	u32_t m_flags;
	atomic_t m_refcnt;
	struct vfs_node_t * m_root;
	struct vfs_node_t * m_covered;
	struct mutex_t m_lock;
	void * m_data;
};

struct filesystem_t {
	struct list_head list;
	const char * name;

	int (*mount)(struct vfs_mount_t *, struct block_t *);
	int (*unmount)(struct vfs_mount_t *);
	int (*msync)(struct vfs_mount_t *);
	int (*vget)(struct vfs_mount_t *, struct vfs_node_t *);
	int (*vput)(struct vfs_mount_t *, struct vfs_node_t *);

	u64_t (*read)(struct vfs_node_t *, s64_t, void *, u64_t);
	u64_t (*write)(struct vfs_node_t *, s64_t, void *, u64_t);
	int (*truncate)(struct vfs_node_t *, s64_t);
	int (*sync)(struct vfs_node_t *);
	int (*readdir)(struct vfs_node_t *, s64_t, struct vfs_dirent_t *);
	int (*lookup)(struct vfs_node_t *, const char *, struct vfs_node_t *);
	int (*create)(struct vfs_node_t *, const char *, u32_t);
	int (*remove)(struct vfs_node_t *, struct vfs_node_t *, const char *);
	int (*rename)(struct vfs_node_t *, const char *, struct vfs_node_t *, struct vfs_node_t *, const char *);
	int (*mkdir)(struct vfs_node_t *, const char *, u32_t);
	int (*rmdir)(struct vfs_node_t *, struct vfs_node_t *, const char *);
	int (*chmod)(struct vfs_node_t *, u32_t);
};

extern struct list_head __filesystem_list;

struct filesystem_t * search_filesystem(const char * name);
bool_t register_filesystem(struct filesystem_t * fs);
bool_t unregister_filesystem(struct filesystem_t * fs);

void vfs_force_unmount(struct vfs_mount_t * m);
int vfs_mount(struct block_t *bdev, struct filesystem_t * fs, const char * dir, u32_t flags);
int vfs_unmount(const char * path);
int vfs_sync(void);
struct vfs_mount_t * vfs_mount_get(int index);
int vfs_mount_count(void);
int vfs_open(const char * path, u32_t flags, u32_t mode);
int vfs_close(int fd);
u64_t vfs_read(int fd, void * buf, u64_t len);
u64_t vfs_write(int fd, void * buf, u64_t len);
s64_t vfs_lseek(int fd, s64_t off, int whence);
int vfs_fsync(int fd);
int vfs_fchmod(int fd, u32_t mode);
int vfs_fstat(int fd, struct vfs_stat_t * st);
int vfs_opendir(const char * name);
int vfs_closedir(int fd);
int vfs_readdir(int fd, struct vfs_dirent_t * dir);
int vfs_rewinddir(int fd);
int vfs_mkdir(const char * path, u32_t mode);
int vfs_rmdir(const char * path);
int vfs_rename(const char * src, const char * dst);
int vfs_unlink(const char * path);
int vfs_access(const char * path, u32_t mode);
int vfs_chmod(const char * path, u32_t mode);
int vfs_stat(const char * path, struct vfs_stat_t * st);

void do_vfs_init(void);

struct block_t
{
	/* The block name */
	char * name;

	/* The size of block */
	u64_t blksz;

	/* The total count of block */
	u64_t blkcnt;

	/* Read block device, return the block counts of reading */
	u64_t (*read)(struct block_t * blk, u8_t * buf, u64_t blkno, u64_t blkcnt);

	/* Write block device, return the block counts of writing */
	u64_t (*write)(struct block_t * blk, u8_t * buf, u64_t blkno, u64_t blkcnt);

	/* Sync cache to block device */
	void (*sync)(struct block_t * blk);

	/* Private data */
	void * priv;
};

u64_t block_read(struct block_t *blk, u8_t *buf, u64_t offset, u64_t count);
u64_t block_write(struct block_t *blk, u8_t *buf, u64_t offset, u64_t count);
void block_sync(struct block_t *blk);

static inline u64_t block_capacity(struct block_t * blk)
{
	return (blk->blksz * blk->blkcnt);
}

enum VFS_REQ
{
    VFS_OPEN,
    VFS_CLOSE,
    VFS_READ,
    VFS_WRITE,
    VFS_LSEEK,
    VFS_FSYNC,
    VFS_FCHMOD,
    VFS_FSTAT,
    VFS_OPENDIR,
    VFS_CLOSEDIR,
    VFS_READDIR,
    VFS_REWINDDIR,
    VFS_MKDIR,
    VFS_RMDIR,
    VFS_RENAME,
    VFS_UNLINK,
    VFS_ACCESS,
    VFS_CHMOD,
    VFS_STAT,
};

struct fs_request
{
    enum VFS_REQ req;

    union
    {
        struct VFS_OPEN_T { char path[VFS_MAX_PATH]; u32_t flags; u32_t mode;} VFS_OPEN;
        struct VFS_CLOSE_T {int fd;} VFS_CLOSE;
        struct VFS_READ_T {int fd; u64_t len; char buf[0];} VFS_READ;
        struct VFS_WRITE_T {int fd; u64_t len; char buf[0];} VFS_WRITE;
        struct VFS_LSEEK_T {int fd; s64_t off; int whence;} VFS_LSEEK;
        struct VFS_FSYNC_T {int fd;} VFS_FSYNC;
        struct VFS_FCHMOD_T {int fd; u32_t mode;} VFS_FCHMOD;
        struct VFS_FSTAT_T {int fd; struct vfs_stat_t st;} VFS_FSTAT;
        struct VFS_OPENDIR_T { char name[VFS_MAX_PATH];} VFS_OPENDIR;
        struct VFS_CLOSEDIR_T {int fd;} VFS_CLOSEDIR;
        struct VFS_READDIR_T {int fd; struct vfs_dirent_t dir;} VFS_READDIR;
        struct VFS_REWINDDIR_T {int fd;} VFS_REWINDDIR;
        struct VFS_MKDIR_T { char path[VFS_MAX_PATH]; u32_t mode;} VFS_MKDIR;
        struct VFS_RMDIR_T { char path[VFS_MAX_PATH];} VFS_RMDIR;
        struct VFS_RENAME_T { char src[VFS_MAX_PATH];  char dst[VFS_MAX_PATH];} VFS_RENAME;
        struct VFS_UNLINK_T { char path[VFS_MAX_PATH];} VFS_UNLINK;
        struct VFS_ACCESS_T { char path[VFS_MAX_PATH]; u32_t mode;} VFS_ACCESS;
        struct VFS_CHMOD_T { char path[VFS_MAX_PATH]; u32_t mode;} VFS_CHMOD;
        struct VFS_STAT_T { char path[VFS_MAX_PATH]; struct vfs_stat_t st;} VFS_STAT;
    };
};

#define GET_FS_REQUEST_SIZE(tag) (offsetof(struct fs_request, tag) + sizeof(struct tag##_T))
#define GET_FS_REQUEST(name, tag, ptr) struct tag##_T *name = (&(((struct fs_request *)(ptr))->tag))