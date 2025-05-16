/* filesystem.h - Simple File System for PhiOS */
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>

/* File system limits */
#define MAX_FILENAME_LENGTH 16
#define MAX_FILES 32
#define MAX_FILE_SIZE 4096  // 4KB per file maximum
#define SECTOR_SIZE 512
#define FILE_SYSTEM_SIZE (64 * SECTOR_SIZE)  // 32KB total file system size
#define MAX_PATH_LENGTH 256
/* File types */
#define FILE_TYPE_NONE 0
#define FILE_TYPE_BINARY 1
#define FILE_TYPE_TEXT 2
#define FILE_TYPE_DIRECTORY 3

/* File attributes */
#define FILE_ATTR_READONLY 0x01
#define FILE_ATTR_HIDDEN   0x02
#define FILE_ATTR_SYSTEM   0x04
#define FILE_ATTR_EXECUTE  0x08

/* Error codes */
#define FS_SUCCESS 0
#define FS_ERROR_NOT_FOUND -1
#define FS_ERROR_DISK_FULL -2
#define FS_ERROR_NAME_TAKEN -3
#define FS_ERROR_INVALID_NAME -4
#define FS_ERROR_MAX_FILES -5
#define FS_ERROR_PERMISSION -6

/* File entry structure */
typedef struct {
    char filename[MAX_FILENAME_LENGTH];
    uint8_t type;               // File type (binary, text, directory)
    uint8_t attributes;         // File attributes (read-only, hidden, etc.)
    uint16_t size;              // File size in bytes
    uint16_t start_sector;      // Starting sector of file data
    uint16_t creation_date;     // Date in packed format (YYYYYYYMMMMDDDDD)
    uint16_t modified_date;     // Date in packed format
} file_entry_t;

/* File system structure */
typedef struct {
    char signature[8];          // "PhiFS" + version 
    uint16_t total_sectors;     // Total sectors in file system
    uint16_t free_sectors;      // Number of free sectors
    uint16_t total_files;       // Total number of files
    uint16_t free_entries;      // Number of free file entries
    file_entry_t entries[MAX_FILES]; // File entries
} filesystem_t;

/* File handle for open files */
typedef struct {
    int entry_index;            // Index in the file entry table
    uint16_t position;          // Current position in file
    uint8_t mode;               // 0=read, 1=write, 2=append
    uint8_t is_open;            // 1 if file is open, 0 otherwise
} file_handle_t;

/* Extern declarations */
extern filesystem_t* fs; // Declare fs as extern
extern file_handle_t handles[MAX_FILES]; // Declare handles as extern
extern char current_path[MAX_PATH_LENGTH];
/* Function prototypes */
int fs_init();                                   // Initialize file system
int fs_format();                                 // Format file system
int fs_create(const char* filename, uint8_t type); // Create a new file
int fs_delete(const char* filename);             // Delete a file
int fs_rename(const char* oldname, const char* newname); // Rename a file
int fs_open(const char* filename, uint8_t mode); // Open a file, returns handle
int fs_close(int handle);                        // Close a file
int fs_read(int handle, void* buffer, uint16_t size); // Read from a file
int fs_write(int handle, const void* buffer, uint16_t size); // Write to a file
int fs_seek(int handle, uint16_t position);      // Seek to position in file
int fs_get_size(const char* filename);           // Get file size
int fs_list_files(const char* pattern);          // List files matching pattern
int fs_get_free_space();                         // Get free space in bytes

/* Directory functions */
int fs_mkdir(const char* dirname);               // Create a directory
int fs_chdir(const char* dirname);               // Change directory
int fs_rmdir(const char* dirname);               // Remove directory
void fs_get_current_path(char* path_buffer);
void fs_init_path(void);

/* Helper functions */
void fs_get_date(uint16_t* date);                // Get current date in packed format
int fs_find_file(const char* filename);          // Find file by name, returns index
void int_to_string(int value, char* str);
#endif /* FILESYSTEM_H */