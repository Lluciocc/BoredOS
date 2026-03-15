// Copyright (c) 2023-2026 Chris (boreddevnl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#ifndef DISK_H
#define DISK_H

#include <stdint.h>
#include <stdbool.h>

#define SECTOR_SIZE 512

typedef enum {
    DISK_TYPE_RAM,
    DISK_TYPE_IDE,
    DISK_TYPE_SATA,
    DISK_TYPE_USB
} DiskType;

typedef struct Disk {
    char letter;            
    DiskType type;
    bool is_fat32;
    char name[32];
    uint32_t partition_lba_offset;  // LBA offset of FAT32 partition (0 for raw)
    
    // Function pointers for driver operations
    int (*read_sector)(struct Disk *disk, uint32_t sector, uint8_t *buffer);
    int (*write_sector)(struct Disk *disk, uint32_t sector, const uint8_t *buffer);
    
    // Private driver data
    void *driver_data; 
} Disk;

void disk_manager_init(void);
void disk_manager_scan(void); // Scans for new disks
Disk* disk_get_by_letter(char letter);
char disk_get_next_free_letter(void);
void disk_register(Disk *disk);
int disk_get_count(void);
Disk* disk_get_by_index(int index);

#endif
