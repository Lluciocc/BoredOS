#include "vfs.h"
#include "../sys/process.h"
#include "../sys/syscall.h"
#include "../dev/disk.h"
#include "memory_manager.h"
#include "core/kutils.h"

typedef struct {
    uint32_t pid;
    char type[32]; 
    int offset;
    bool is_root;
} procfs_handle_t;

void* procfs_open(void *fs_private, const char *path, const char *mode) {
    if (path[0] == '/') path++;
    
    procfs_handle_t *h = (procfs_handle_t*)kmalloc(sizeof(procfs_handle_t));
    k_memset(h, 0, sizeof(procfs_handle_t));
    h->offset = 0;

    if (path[0] == '\0') {
        h->is_root = true;
        return h;
    }

    if (path[0] >= '0' && path[0] <= '9') {
        char pid_str[16];
        int i = 0;
        while (path[i] && path[i] != '/' && i < 15) {
            pid_str[i] = path[i];
            i++;
        }
        pid_str[i] = 0;
        h->pid = k_atoi(pid_str);

        if (path[i] == '/') {
            k_strcpy(h->type, path + i + 1);
        } else {
            h->type[0] = 0; 
        }
        return h;
    }

    h->pid = 0xFFFFFFFF;
    k_strcpy(h->type, path);
    return h;
}

void procfs_close(void *fs_private, void *handle) {
    if (handle) kfree(handle);
}

int procfs_read(void *fs_private, void *handle, void *buf, int size) {
    procfs_handle_t *h = (procfs_handle_t*)handle;
    if (!h) return -1;

    char out[1024];
    out[0] = 0;

    if (h->pid == 0xFFFFFFFF) {
        if (k_strcmp(h->type, "version") == 0) {
            extern void get_os_info(os_info_t *info);
            os_info_t info;
            get_os_info(&info);
            k_strcpy(out, info.os_name);
            k_strcpy(out + k_strlen(out), " [");
            k_strcpy(out + k_strlen(out), info.os_codename);
            k_strcpy(out + k_strlen(out), "] Version ");
            k_strcpy(out + k_strlen(out), info.os_version);
            k_strcpy(out + k_strlen(out), "\nKernel: ");
            k_strcpy(out + k_strlen(out), info.kernel_name);
            k_strcpy(out + k_strlen(out), " ");
            k_strcpy(out + k_strlen(out), info.kernel_version);
            k_strcpy(out + k_strlen(out), "\nBuild: ");
            k_strcpy(out + k_strlen(out), info.build_date);
            k_strcpy(out + k_strlen(out), " ");
            k_strcpy(out + k_strlen(out), info.build_time);
            k_strcpy(out + k_strlen(out), "\n");
        } else if (k_strcmp(h->type, "uptime") == 0) {
            extern uint32_t wm_get_ticks(void);
            uint32_t ticks = wm_get_ticks();
            k_itoa(ticks / 60, out);
            k_strcpy(out + k_strlen(out), " seconds\nRaw_Ticks:");
            char t_s[16]; k_itoa(ticks, t_s);
            k_strcpy(out + k_strlen(out), t_s);
            k_strcpy(out + k_strlen(out), "\n");
        } else if (k_strcmp(h->type, "cpuinfo") == 0) {
            extern uint32_t smp_cpu_count(void);
            extern void platform_get_cpu_model(char *model);
            char model[64];
            platform_get_cpu_model(model);

            k_strcpy(out, "Processor: ");
            k_strcpy(out + k_strlen(out), model);
            k_strcpy(out + k_strlen(out), "\nCores: ");
            char c_s[16]; k_itoa(smp_cpu_count(), c_s);
            k_strcpy(out + k_strlen(out), c_s);
            k_strcpy(out + k_strlen(out), "\nArchitecture: x86_64\n");
        } else if (k_strcmp(h->type, "meminfo") == 0) {
            extern MemStats memory_get_stats(void);
            MemStats stats = memory_get_stats();
            k_strcpy(out, "MemTotal: ");
            char m_s[32]; k_itoa(stats.total_memory / 1024, m_s);
            k_strcpy(out + k_strlen(out), m_s);
            k_strcpy(out + k_strlen(out), " kB\nMemFree: ");
            k_itoa(stats.available_memory / 1024, m_s);
            k_strcpy(out + k_strlen(out), m_s);
            k_strcpy(out + k_strlen(out), " kB\nMemUsed: ");
            k_itoa(stats.used_memory / 1024, m_s);
            k_strcpy(out + k_strlen(out), m_s);
            k_strcpy(out + k_strlen(out), " kB\nPeak: ");
            k_itoa(stats.peak_memory_used / 1024, m_s);
            k_strcpy(out + k_strlen(out), m_s);
            k_strcpy(out + k_strlen(out), " kB\nBlocks: ");
            k_itoa(stats.allocated_blocks, m_s);
            k_strcpy(out + k_strlen(out), m_s);
            k_strcpy(out + k_strlen(out), "\nFragmentation: ");
            k_itoa(stats.fragmentation_percent, m_s);
            k_strcpy(out + k_strlen(out), m_s);
            k_strcpy(out + k_strlen(out), "%\n");
        } else if (k_strcmp(h->type, "devices") == 0) {
            extern int disk_get_count(void);
            extern Disk* disk_get_by_index(int index);
            int dcount = disk_get_count();
            k_strcpy(out, "Block Devices:\n");
            for (int i = 0; i < dcount; i++) {
                Disk *d = disk_get_by_index(i);
                if (d) {
                    k_strcpy(out + k_strlen(out), "  - ");
                    k_strcpy(out + k_strlen(out), d->devname);
                    k_strcpy(out + k_strlen(out), "\n");
                }
            }
        }
    }
 else {
        process_t *proc = process_get_by_pid(h->pid);
        if (!proc) return -1;

        if (k_strcmp(h->type, "name") == 0 || k_strcmp(h->type, "cmdline") == 0) {
            k_strcpy(out, proc->name);
            k_strcpy(out + k_strlen(out), "\n");
        } else if (k_strcmp(h->type, "status") == 0) {
            k_strcpy(out, "Name: ");
            k_strcpy(out + k_strlen(out), proc->name);
            k_strcpy(out + k_strlen(out), "\nPID: ");
            char pid_s[16]; k_itoa(proc->pid, pid_s);
            k_strcpy(out + k_strlen(out), pid_s);
            k_strcpy(out + k_strlen(out), "\nState: RUNNING\nMemory: ");
            uint64_t mem_val = proc->used_memory;
            if (h->pid == 0) {
                extern MemStats memory_get_stats(void);
                mem_val = memory_get_stats().used_memory;
            }
            char mem_s[32]; k_itoa(mem_val / 1024, mem_s);
            k_strcpy(out + k_strlen(out), mem_s);
            k_strcpy(out + k_strlen(out), " KB\nTicks: ");
            char tick_s[32]; k_itoa(proc->ticks, tick_s);
            k_strcpy(out + k_strlen(out), tick_s);
            k_strcpy(out + k_strlen(out), "\nIdle: ");
            k_strcpy(out + k_strlen(out), proc->is_idle ? "1" : "0");
            k_strcpy(out + k_strlen(out), "\n");
        }
    }

    int len = k_strlen(out);
    if (h->offset >= len) return 0;

    int to_copy = len - h->offset;
    if (to_copy > size) to_copy = size;

    k_memcpy(buf, out + h->offset, to_copy);
    h->offset += to_copy;
    return to_copy;
}

int procfs_write(void *fs_private, void *handle, const void *buf, int size) {
    procfs_handle_t *h = (procfs_handle_t*)handle;
    if (!h || h->pid == 0xFFFFFFFF) return -1;

    if (k_strcmp(h->type, "signal") == 0) {
        char cmd[16];
        int to_copy = size < 15 ? size : 15;
        k_memcpy(cmd, buf, to_copy);
        cmd[to_copy] = 0;

        if (k_strcmp(cmd, "9") == 0 || k_strcmp(cmd, "kill") == 0) {
            process_t *proc = process_get_by_pid(h->pid);
            if (proc && proc->pid != 0) {
                process_terminate(proc);
                return size;
            }
        }
    }

    return -1;
}

int procfs_readdir(void *fs_private, const char *path, vfs_dirent_t *entries, int max) {
    if (path[0] == '/') path++;

    if (path[0] == '\0') {
        int out = 0;
        k_strcpy(entries[out++].name, "version");
        entries[out-1].is_directory = 0;
        k_strcpy(entries[out++].name, "uptime");
        entries[out-1].is_directory = 0;
        k_strcpy(entries[out++].name, "cpuinfo");
        entries[out-1].is_directory = 0;
        k_strcpy(entries[out++].name, "meminfo");
        entries[out-1].is_directory = 0;
        k_strcpy(entries[out++].name, "devices");
        entries[out-1].is_directory = 0;

        extern process_t processes[];
        for (int i = 0; i < 16 && out < max; i++) {
            if (processes[i].pid != 0xFFFFFFFF) {
                k_itoa(processes[i].pid, entries[out].name);
                entries[out].is_directory = 1;
                entries[out].size = 0;
                out++;
            }
        }
        return out;
    }

    if (path[0] >= '0' && path[0] <= '9') {
        int out = 0;
        k_strcpy(entries[out++].name, "name");
        k_strcpy(entries[out++].name, "status");
        k_strcpy(entries[out++].name, "cmdline");
        k_strcpy(entries[out++].name, "signal");
        for(int i=0; i<out; i++) entries[i].is_directory = 0;
        return out;
    }

    return 0;
}

bool procfs_exists(void *fs_private, const char *path) {
    if (path[0] == '/') path++;
    if (path[0] == '\0') return true;

    if (path[0] >= '0' && path[0] <= '9') {
        char pid_str[16];
        int i = 0;
        while (path[i] && path[i] != '/' && i < 15) {
            pid_str[i] = path[i];
            i++;
        }
        pid_str[i] = 0;
        uint32_t pid = k_atoi(pid_str);
        if (process_get_by_pid(pid)) return true;
    }

    if (k_strcmp(path, "version") == 0 || k_strcmp(path, "uptime") == 0) return true;
    if (k_strcmp(path, "cpuinfo") == 0 || k_strcmp(path, "meminfo") == 0) return true;
    if (k_strcmp(path, "devices") == 0) return true;

    return false;
}

bool procfs_is_dir(void *fs_private, const char *path) {
    if (path[0] == '/') path++;
    if (path[0] == '\0') return true;

    if (path[0] >= '0' && path[0] <= '9') {
        int i = 0;
        while (path[i] && path[i] != '/') i++;
        if (path[i] == '\0') return true; 
        return false; 
    }

    return false; 
}

vfs_fs_ops_t procfs_ops = {
    .open = procfs_open,
    .close = procfs_close,
    .read = procfs_read,
    .write = procfs_write,
    .readdir = procfs_readdir,
    .exists = procfs_exists,
    .is_dir = procfs_is_dir 
};

vfs_fs_ops_t* procfs_get_ops(void) {
    return &procfs_ops;
}
