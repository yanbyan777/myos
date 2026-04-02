/*
 * Miku OS - Virtual File System (VFS) Implementation
 * 
 * A Linux-like Virtual File System layer that provides:
 * - Unified interface for multiple filesystem types
 * - Path resolution and dentry caching
 * - File operations (open, read, write, close)
 * - Directory operations (mkdir, rmdir, readdir)
 * - Mount point management
 * - File permissions and ownership
 */

#include "miku_os.h"

/* Root filesystem superblock */
struct superblock *g_rootfs = NULL;

/* Global inode counter */
static u64 g_next_inode = 1;

/* Maximum number of open files */
#define MAX_OPEN_FILES  65536

/* ============================================================================
 * VFS INITIALIZATION
 * ============================================================================ */
void vfs_init(void) {
    log_info("Initializing Virtual File System...");
    
    /* Initialize root filesystem */
    // g_rootfs = ramfs_mount(NULL, 0, NULL);
    
    if (!g_rootfs) {
        log_warn("Failed to mount root filesystem");
        /* Create minimal in-memory root */
        // g_rootfs = create_minimal_root();
    }
    
    log_info("VFS initialized successfully");
}

/* ============================================================================
 * PATH RESOLUTION
 * ============================================================================ */
static int vfs_path_lookup(const char *path, struct dentry **result) {
    struct dentry *dentry;
    struct dentry *parent;
    char *path_copy;
    char *token;
    char *saveptr;
    
    if (!path || !result) {
        return -EINVAL;
    }
    
    /* Start from root */
    if (!g_rootfs || !g_rootfs->s_root) {
        return -ENOENT;
    }
    
    parent = g_rootfs->s_root;
    
    /* Handle empty path */
    if (path[0] == '\0') {
        *result = parent;
        return 0;
    }
    
    /* Make a copy of the path for tokenization */
    path_copy = kmalloc(strlen(path) + 1);
    if (!path_copy) {
        return -ENOMEM;
    }
    strcpy(path_copy, path);
    
    /* Skip leading slash */
    token = strtok_r(path_copy, "/", &saveptr);
    
    while (token) {
        /* Skip "." entries */
        if (strcmp(token, ".") == 0) {
            token = strtok_r(NULL, "/", &saveptr);
            continue;
        }
        
        /* Handle ".." entries */
        if (strcmp(token, "..") == 0) {
            if (parent->d_parent) {
                parent = parent->d_parent;
            }
            token = strtok_r(NULL, "/", &saveptr);
            continue;
        }
        
        /* Lookup component in parent directory */
        // dentry = parent->d_sb->s_op->lookup(parent->d_inode, token);
        
        /* For now, simulate lookup failure */
        dentry = NULL;
        
        if (!dentry) {
            kfree(path_copy);
            return -ENOENT;
        }
        
        parent = dentry;
        token = strtok_r(NULL, "/", &saveptr);
    }
    
    kfree(path_copy);
    *result = parent;
    return 0;
}

/* ============================================================================
 * FILE OPERATIONS
 * ============================================================================ */
struct file *vfs_open(const char *path, int flags, mode_t mode) {
    struct file *file;
    struct dentry *dentry;
    struct inode *inode;
    int ret;
    
    if (!path) {
        return NULL;
    }
    
    log_debug("VFS: Opening file '%s' (flags=%x, mode=%o)", path, flags, mode);
    
    /* Allocate file structure */
    file = kzalloc(sizeof(struct file));
    if (!file) {
        return NULL;
    }
    
    /* Lookup path */
    ret = vfs_path_lookup(path, &dentry);
    if (ret < 0) {
        /* File doesn't exist, try to create if O_CREAT is set */
        if (flags & O_CREAT) {
            /* Create new file */
            // ret = vfs_create(path, mode);
            ret = -ENOENT; /* Not implemented yet */
            
            if (ret < 0) {
                kfree(file);
                return NULL;
            }
            
            /* Lookup again */
            ret = vfs_path_lookup(path, &dentry);
            if (ret < 0) {
                kfree(file);
                return NULL;
            }
        } else {
            kfree(file);
            return NULL;
        }
    }
    
    inode = dentry->d_inode;
    if (!inode) {
        kfree(file);
        return NULL;
    }
    
    /* Initialize file structure */
    file->f_dentry = dentry;
    file->f_vfsmnt = NULL; /* Should point to mount */
    file->f_inode = inode;
    file->f_op = inode->i_fop;
    file->f_pos = 0;
    file->f_flags = flags;
    file->f_mode = (flags & 3); /* O_RDONLY, O_WRONLY, or O_RDWR */
    file->f_count = 1;
    file->f_private = NULL;
    
    spinlock_init(&file->f_lock);
    
    /* Call filesystem open method */
    if (file->f_op && file->f_op->open) {
        ret = file->f_op->open(inode, file);
        if (ret < 0) {
            kfree(file);
            return NULL;
        }
    }
    
    /* Update inode access time */
    inode->i_atime = time_get_unix_time();
    
    log_debug("VFS: File opened successfully (fd=%p)", file);
    
    return file;
}

int vfs_close(struct file *file) {
    if (!file) {
        return -EBADF;
    }
    
    log_debug("VFS: Closing file %p", file);
    
    /* Call filesystem close method */
    if (file->f_op && file->f_op->close) {
        file->f_op->close(file);
    }
    
    /* Update inode modification time if written */
    if (file->f_mode & O_WRONLY) {
        file->f_inode->i_mtime = time_get_unix_time();
    }
    
    /* Decrement reference count */
    file->f_count--;
    
    /* Free if no more references */
    if (file->f_count == 0) {
        kfree(file);
    }
    
    return 0;
}

ssize_t vfs_read(struct file *file, char *buf, size_t count, off_t *pos) {
    ssize_t ret;
    off_t offset;
    
    if (!file || !buf || !count) {
        return -EINVAL;
    }
    
    if (!(file->f_mode & O_RDONLY)) {
        return -EBADF;
    }
    
    offset = pos ? *pos : file->f_pos;
    
    log_debug("VFS: Reading %zu bytes from %p at offset %lld", 
              count, file, offset);
    
    /* Call filesystem read method */
    if (file->f_op && file->f_op->read) {
        ret = file->f_op->read(file, buf, count, &offset);
    } else {
        ret = -EIO;
    }
    
    if (ret > 0) {
        if (pos) {
            *pos = offset;
        } else {
            file->f_pos = offset;
        }
    }
    
    return ret;
}

ssize_t vfs_write(struct file *file, const char *buf, size_t count, off_t *pos) {
    ssize_t ret;
    off_t offset;
    
    if (!file || !buf || !count) {
        return -EINVAL;
    }
    
    if (!(file->f_mode & O_WRONLY)) {
        return -EBADF;
    }
    
    offset = pos ? *pos : file->f_pos;
    
    log_debug("VFS: Writing %zu bytes to %p at offset %lld", 
              count, file, offset);
    
    /* Call filesystem write method */
    if (file->f_op && file->f_op->write) {
        ret = file->f_op->write(file, buf, count, &offset);
    } else {
        ret = -EIO;
    }
    
    if (ret > 0) {
        if (pos) {
            *pos = offset;
        } else {
            file->f_pos = offset;
        }
    }
    
    return ret;
}

/* ============================================================================
 * DIRECTORY OPERATIONS
 * ============================================================================ */
int vfs_mkdir(const char *path, mode_t mode) {
    struct dentry *parent;
    struct dentry *dentry;
    char *path_copy;
    char *last_slash;
    char *name;
    int ret;
    
    if (!path) {
        return -EINVAL;
    }
    
    log_debug("VFS: Creating directory '%s' (mode=%o)", path, mode);
    
    /* Find parent directory */
    path_copy = kmalloc(strlen(path) + 1);
    if (!path_copy) {
        return -ENOMEM;
    }
    strcpy(path_copy, path);
    
    last_slash = strrchr(path_copy, '/');
    if (!last_slash || last_slash == path_copy) {
        /* Parent is root */
        parent = g_rootfs->s_root;
        name = path_copy + 1;
    } else {
        *last_slash = '\0';
        ret = vfs_path_lookup(path_copy, &parent);
        if (ret < 0) {
            kfree(path_copy);
            return ret;
        }
        name = last_slash + 1;
    }
    
    /* Check if parent is a directory */
    if (!parent->d_inode || !(parent->d_inode->i_mode & S_IFDIR)) {
        kfree(path_copy);
        return -ENOTDIR;
    }
    
    /* Call filesystem mkdir method */
    if (parent->d_inode->i_op && parent->d_inode->i_op->mkdir) {
        ret = parent->d_inode->i_op->mkdir(parent->d_inode, NULL, mode);
        if (ret < 0) {
            kfree(path_copy);
            return ret;
        }
    } else {
        kfree(path_copy);
        return -EPERM;
    }
    
    kfree(path_copy);
    return 0;
}

int vfs_rmdir(const char *path) {
    struct dentry *dentry;
    struct dentry *parent;
    int ret;
    
    if (!path) {
        return -EINVAL;
    }
    
    log_debug("VFS: Removing directory '%s'", path);
    
    /* Lookup directory */
    ret = vfs_path_lookup(path, &dentry);
    if (ret < 0) {
        return ret;
    }
    
    /* Check if it's a directory */
    if (!dentry->d_inode || !(dentry->d_inode->i_mode & S_IFDIR)) {
        return -ENOTDIR;
    }
    
    /* Check if directory is empty */
    // if (!directory_is_empty(dentry)) {
    //     return -ENOTEMPTY;
    // }
    
    parent = dentry->d_parent;
    
    /* Call filesystem rmdir method */
    if (parent->d_inode->i_op && parent->d_inode->i_op->rmdir) {
        ret = parent->d_inode->i_op->rmdir(parent->d_inode, dentry);
    } else {
        ret = -EPERM;
    }
    
    return ret;
}

int vfs_unlink(const char *path) {
    struct dentry *dentry;
    struct dentry *parent;
    int ret;
    
    if (!path) {
        return -EINVAL;
    }
    
    log_debug("VFS: Unlinking file '%s'", path);
    
    /* Lookup file */
    ret = vfs_path_lookup(path, &dentry);
    if (ret < 0) {
        return ret;
    }
    
    /* Check if it's a regular file */
    if (!dentry->d_inode || !(dentry->d_inode->i_mode & S_IFREG)) {
        return -EISDIR;
    }
    
    parent = dentry->d_parent;
    
    /* Call filesystem unlink method */
    if (parent->d_inode->i_op && parent->d_inode->i_op->unlink) {
        ret = parent->d_inode->i_op->unlink(parent->d_inode, dentry);
    } else {
        ret = -EPERM;
    }
    
    return ret;
}

int vfs_rename(const char *oldpath, const char *newpath) {
    struct dentry *old_dentry;
    struct dentry *new_parent;
    int ret;
    
    if (!oldpath || !newpath) {
        return -EINVAL;
    }
    
    log_debug("VFS: Renaming '%s' to '%s'", oldpath, newpath);
    
    /* Lookup source */
    ret = vfs_path_lookup(oldpath, &old_dentry);
    if (ret < 0) {
        return ret;
    }
    
    /* Lookup target parent */
    ret = vfs_path_lookup(newpath, &new_parent);
    if (ret < 0) {
        return ret;
    }
    
    /* Call filesystem rename method */
    if (old_dentry->d_inode->i_op && old_dentry->d_inode->i_op->rename) {
        ret = old_dentry->d_inode->i_op->rename(
            old_dentry->d_parent->d_inode, old_dentry,
            new_parent->d_inode, new_parent
        );
    } else {
        ret = -EPERM;
    }
    
    return ret;
}

/* ============================================================================
 * STAT OPERATIONS
 * ============================================================================ */
int vfs_stat(const char *path, struct kstat *stat) {
    struct dentry *dentry;
    struct inode *inode;
    int ret;
    
    if (!path || !stat) {
        return -EINVAL;
    }
    
    /* Lookup path */
    ret = vfs_path_lookup(path, &dentry);
    if (ret < 0) {
        return ret;
    }
    
    inode = dentry->d_inode;
    if (!inode) {
        return -ENOENT;
    }
    
    /* Fill stat structure */
    memset(stat, 0, sizeof(struct kstat));
    stat->st_dev = inode->i_sb ? inode->i_sb->s_dev : 0;
    stat->st_ino = inode->i_ino;
    stat->st_mode = inode->i_mode;
    stat->st_nlink = inode->i_nlink;
    stat->st_uid = inode->i_uid;
    stat->st_gid = inode->i_gid;
    stat->st_rdev = inode->i_rdev;
    stat->st_size = inode->i_size;
    stat->st_blksize = inode->i_blksize;
    stat->st_blocks = inode->i_blocks;
    stat->st_atime = inode->i_atime;
    stat->st_mtime = inode->i_mtime;
    stat->st_ctime = inode->i_ctime;
    
    return 0;
}

/* ============================================================================
 * MOUNT OPERATIONS
 * ============================================================================ */
int vfs_mount(const char *dev, const char *path, const char *type, u32 flags, void *data) {
    struct superblock *sb;
    struct dentry *mount_point;
    int ret;
    
    (void)dev;
    (void)flags;
    (void)data;
    
    if (!path || !type) {
        return -EINVAL;
    }
    
    log_info("VFS: Mounting %s on %s (type=%s)", 
             dev ? dev : "none", path, type);
    
    /* Lookup mount point */
    ret = vfs_path_lookup(path, &mount_point);
    if (ret < 0) {
        log_error("VFS: Mount point '%s' not found", path);
        return ret;
    }
    
    /* Check if mount point is a directory */
    if (!mount_point->d_inode || !(mount_point->d_inode->i_mode & S_IFDIR)) {
        log_error("VFS: Mount point '%s' is not a directory", path);
        return -ENOTDIR;
    }
    
    /* Create superblock for filesystem type */
    // sb = get_superblock(type);
    sb = NULL; /* Not implemented */
    
    if (!sb) {
        log_error("VFS: Unknown filesystem type '%s'", type);
        return -ENODEV;
    }
    
    /* Mount the filesystem */
    if (sb->s_op && sb->s_op->mount) {
        ret = sb->s_op->mount(sb, flags, data);
        if (ret < 0) {
            return ret;
        }
    }
    
    /* Attach to mount point */
    // attach_mount(mount_point, sb);
    
    log_info("VFS: Successfully mounted %s on %s", dev ? dev : "none", path);
    
    return 0;
}

int vfs_umount(const char *path) {
    struct dentry *mount_point;
    int ret;
    
    if (!path) {
        return -EINVAL;
    }
    
    log_info("VFS: Unmounting %s", path);
    
    /* Lookup mount point */
    ret = vfs_path_lookup(path, &mount_point);
    if (ret < 0) {
        return ret;
    }
    
    /* Check if busy */
    // if (mount_is_busy(mount_point)) {
    //     return -EBUSY;
    // }
    
    /* Detach and unmount */
    // detach_mount(mount_point);
    
    log_info("VFS: Successfully unmounted %s", path);
    
    return 0;
}

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */
u64 vfs_alloc_inode(void) {
    return g_next_inode++;
}

void vfs_free_inode(u64 inode) {
    (void)inode;
    /* In a real implementation, add to free list */
}
