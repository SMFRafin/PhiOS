/* filesystem.c - Simple File System for PhiOS */
#include "filesystem.h"
#include "print.h"
#include "string.h"


#undef fs
#undef handles
char current_path[MAX_PATH_LENGTH] = "/";

filesystem_t* fs = (filesystem_t*)0x20000;  // File system metadata at 0x20000
file_handle_t handles[MAX_FILES];

/* Pointers to memory for our file system */
static uint8_t* fs_data = (uint8_t*)0x21000;       // File data starting at 0x21000

static int current_directory = -1;  // -1 is root directory

#define NULL ((void*)0)

/* Custom implementation of memset */
void* memset(void* dest, int value, uint32_t count) {
    unsigned char* ptr = (unsigned char*)dest;
    while (count--) {
        *ptr++ = (unsigned char)value;
    }
    return dest;
}

/* Custom implementation of strcpy */
char* strcpy(char* dest, const char* src) {
    char* original = dest;
    while ((*dest++ = *src++));
    return original;
}

char *strcat(char* dest, const char* src) {
    char* original = dest;
    while (*dest) {
        dest++;
    }
    while ((*dest++ = *src++));
    return original;
}
/* Custom implementation of memcpy */
void* memcpy(void* dest, const void* src, uint32_t count) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    while (count--) {
        *d++ = *s++;
    }
    return dest;
}

/* Custom implementation of int_to_string */
void int_to_string(int value, char* str) {
    char buffer[12]; // Enough for 32-bit integers
    int i = 0;
    int is_negative = 0;

    if (value < 0) {
        is_negative = 1;
        value = -value;
    }

    do {
        buffer[i++] = (value % 10) + '0';
        value /= 10;
    } while (value > 0);

    if (is_negative) {
        buffer[i++] = '-';
    }

    buffer[i] = '\0';

    // Reverse the string
    int j;
    for (j = 0; j < i; j++) {
        str[j] = buffer[i - j - 1];
    }
    str[j] = '\0';
}

/* Initialize the file system */
int fs_init() {
    // Check if file system is already initialized
    if (strcmp(fs->signature, "PhiFS10") == 0) {
        // Already initialized, just verify structure
        if (fs->total_sectors != (FILE_SYSTEM_SIZE / SECTOR_SIZE)) {
            // Invalid file system size
            return fs_format();
        }
        return FS_SUCCESS;

    }
    // Not initialized, format the file system
    return fs_format();
}

/* Format the file system */
int fs_format() {
    // Initialize file system structure
    memset(fs, 0, sizeof(filesystem_t));
    strcpy(fs->signature, "PhiFS10");
    fs->total_sectors = FILE_SYSTEM_SIZE / SECTOR_SIZE;
    fs->free_sectors = fs->total_sectors - 1;  // One sector for file system metadata
    fs->total_files = 0;
    fs->free_entries = MAX_FILES;
    
    // Initialize file handles
    for (int i = 0; i < MAX_FILES; i++) {
        handles[i].is_open = 0;
    }
    
    // Clear file data area
    memset(fs_data, 0, FILE_SYSTEM_SIZE - sizeof(filesystem_t));
    
    print_string("\nFile system formatted successfully.\n");
    return FS_SUCCESS;
}

/* Find a file by name, returns file index or error */
int fs_find_file(const char* filename) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs->entries[i].type != FILE_TYPE_NONE && 
            strcmp(fs->entries[i].filename, filename) == 0) {
            return i;
        }
    }
    return FS_ERROR_NOT_FOUND;
}

/* Find a free file entry, returns index or error */
static int fs_find_free_entry() {
    if (fs->free_entries == 0) {
        return FS_ERROR_MAX_FILES;
    }
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs->entries[i].type == FILE_TYPE_NONE) {
            return i;
        }
    }
    
    // This should never happen if free_entries > 0
    return FS_ERROR_MAX_FILES;
}

/* Find free sectors for a file of size 'bytes' */
static int fs_find_free_sectors(uint16_t bytes, uint16_t* start_sector) {
    uint16_t sectors_needed = (bytes + SECTOR_SIZE - 1) / SECTOR_SIZE;
    
    if (sectors_needed > fs->free_sectors) {
        return FS_ERROR_DISK_FULL;
    }
    
    // Simple allocation strategy - find first free block
    // This is inefficient but simple for demonstration
    uint8_t* allocation_map = (uint8_t*)(fs + 1);  // Allocation map right after file system structure
    
    uint16_t consecutive = 0;
    uint16_t start = 0;
    
    for (uint16_t i = 0; i < fs->total_sectors; i++) {
        uint8_t byte = allocation_map[i / 8];
        uint8_t bit = 1 << (i % 8);
        
        if ((byte & bit) == 0) {
            // Free sector
            if (consecutive == 0) {
                start = i;
            }
            consecutive++;
            
            if (consecutive >= sectors_needed) {
                *start_sector = start;
                return sectors_needed;
            }
        } else {
            // Used sector
            consecutive = 0;
        }
    }
    
    return FS_ERROR_DISK_FULL;
}

/* Mark sectors as used in allocation map */
static void fs_mark_sectors(uint16_t start, uint16_t count, int used) {
    uint8_t* allocation_map = (uint8_t*)(fs + 1);
    
    for (uint16_t i = 0; i < count; i++) {
        uint16_t sector = start + i;
        uint8_t* byte = &allocation_map[sector / 8];
        uint8_t bit = 1 << (sector % 8);
        
        if (used) {
            *byte |= bit;  // Mark as used
        } else {
            *byte &= ~bit; // Mark as free
        }
    }
    
    // Update free sector count
    if (used) {
        fs->free_sectors -= count;
    } else {
        fs->free_sectors += count;
    }
}

/* Get current date in packed format (YYYYYYYMMMMDDDDD) */
void fs_get_date(uint16_t* date) {
    *date = (25 << 9) | (5 << 5) | 10;  // May 7, 2020
}

/* Create a new file */
int fs_create(const char* filename, uint8_t type) {
    // Check if filename is valid
    if (strlen(filename) == 0 || strlen(filename) > MAX_FILENAME_LENGTH - 1) {
        return FS_ERROR_INVALID_NAME;
    }
    
    // Check if file already exists
    if (fs_find_file(filename) >= 0) {
        return FS_ERROR_NAME_TAKEN;
    }
    
    // Find a free entry
    int entry_index = fs_find_free_entry();
    if (entry_index < 0) {
        return entry_index;  // Error code
    }
    
    // Initialize file entry
    file_entry_t* entry = &fs->entries[entry_index];
    strcpy(entry->filename, filename);
    entry->type = type;
    entry->attributes = 0;
    entry->size = 0;
    entry->start_sector = 0;  // No data yet
    
    // Set creation and modification dates
    fs_get_date(&entry->creation_date);
    entry->modified_date = entry->creation_date;
    
    // Update file system metadata
    fs->total_files++;
    fs->free_entries--;
    
    return entry_index;
}

/* Delete a file */
int fs_delete(const char* filename) {
    // Find the file
    int index = fs_find_file(filename);
    if (index < 0) {
        return index;  // Error code
    }
    
    file_entry_t* entry = &fs->entries[index];
    
    // Check if file is open
    for (int i = 0; i < MAX_FILES; i++) {
        if (handles[i].is_open && handles[i].entry_index == index) {
            return FS_ERROR_PERMISSION;  // Can't delete open file
        }
    }
    
    // Free sectors used by file
    uint16_t sectors = (entry->size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    if (sectors > 0) {
        fs_mark_sectors(entry->start_sector, sectors, 0);  // Mark as free
    }
    
    // Clear entry
    memset(entry, 0, sizeof(file_entry_t));
    
    // Update file system metadata
    fs->total_files--;
    fs->free_entries++;
    
    return FS_SUCCESS;
}

/* Open a file, returns handle or error code */
int fs_open(const char* filename, uint8_t mode) {
    // Find the file
    int index = fs_find_file(filename);
    if (index < 0) {
        return index;  // Error code
    }
    
    // Check if write mode requested on read-only file
    if ((mode == 1 || mode == 2) && 
        (fs->entries[index].attributes & FILE_ATTR_READONLY)) {
        return FS_ERROR_PERMISSION;
    }
    
    // Find a free handle
    int handle_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (!handles[i].is_open) {
            handle_index = i;
            break;
        }
    }
    
    if (handle_index < 0) {
        return FS_ERROR_MAX_FILES;  // No free handles
    }
    
    // Initialize handle
    file_handle_t* handle = &handles[handle_index];
    handle->entry_index = index;
    handle->position = (mode == 2) ? fs->entries[index].size : 0;  // Start at end if append mode
    handle->mode = mode;
    handle->is_open = 1;
    
    return handle_index;
}

/* Close a file */
int fs_close(int handle) {
    if (handle < 0 || handle >= MAX_FILES || !handles[handle].is_open) {
        return FS_ERROR_NOT_FOUND;
    }
    
    handles[handle].is_open = 0;
    return FS_SUCCESS;
}

/* Read from a file */
int fs_read(int handle, void* buffer, uint16_t size) {
    if (handle < 0 || handle >= MAX_FILES || !handles[handle].is_open) {
        return FS_ERROR_NOT_FOUND;
    }
    
    file_handle_t* file = &handles[handle];
    file_entry_t* entry = &fs->entries[file->entry_index];
    
    // Check if we're trying to read past the end of file
    if (file->position >= entry->size) {
        return 0;  // EOF
    }
    
    // Calculate how many bytes we can actually read
    uint16_t bytes_to_read = size;
    if (file->position + bytes_to_read > entry->size) {
        bytes_to_read = entry->size - file->position;
    }
    
    // Calculate sector and offset
    uint16_t start_sector = entry->start_sector;
    uint16_t offset_in_file = file->position;
    uint16_t sector_offset = offset_in_file / SECTOR_SIZE;
    uint16_t byte_offset = offset_in_file % SECTOR_SIZE;
    
    // Get pointer to data in memory
    uint8_t* data_ptr = fs_data + (start_sector + sector_offset) * SECTOR_SIZE + byte_offset;
    
    // Copy data to buffer
    memcpy(buffer, data_ptr, bytes_to_read);
    
    // Update position
    file->position += bytes_to_read;
    
    return bytes_to_read;
}

/* Write to a file */
int fs_write(int handle, const void* buffer, uint16_t size) {
    if (handle < 0 || handle >= MAX_FILES || !handles[handle].is_open) {
        return FS_ERROR_NOT_FOUND;
    }
    
    file_handle_t* file = &handles[handle];
    file_entry_t* entry = &fs->entries[file->entry_index];
    
    // Check if file is read-only
    if (file->mode == 0) {
        return FS_ERROR_PERMISSION;
    }
    
    // Check if we need to allocate more space for the file
    uint16_t new_size = file->position + size;
    if (new_size > entry->size) {
        uint16_t old_sectors = (entry->size + SECTOR_SIZE - 1) / SECTOR_SIZE;
        uint16_t new_sectors = (new_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
        
        if (new_sectors > old_sectors) {
            // Need to allocate more sectors
            if (entry->start_sector == 0) {
                // First allocation for this file
                uint16_t start_sector;
                int result = fs_find_free_sectors(new_size, &start_sector);
                if (result < 0) {
                    return result;  // Error code
                }
                
                // Mark sectors as used
                fs_mark_sectors(start_sector, new_sectors, 1);
                
                // Update file entry
                entry->start_sector = start_sector;
            } else {
                return FS_ERROR_DISK_FULL;
            }
        }
        
        // Update file size
        entry->size = new_size;
        fs_get_date(&entry->modified_date);
    }
    
    // Calculate sector and offset
    uint16_t start_sector = entry->start_sector;
    uint16_t offset_in_file = file->position;
    uint16_t sector_offset = offset_in_file / SECTOR_SIZE;
    uint16_t byte_offset = offset_in_file % SECTOR_SIZE;
    
    // Get pointer to data in memory
    uint8_t* data_ptr = fs_data + (start_sector + sector_offset) * SECTOR_SIZE + byte_offset;
    
    // Copy data from buffer
    memcpy(data_ptr, buffer, size);
    
    // Update position
    file->position += size;
    
    return size;
}

/* Seek to position in file */
int fs_seek(int handle, uint16_t position) {
    if (handle < 0 || handle >= MAX_FILES || !handles[handle].is_open) {
        return FS_ERROR_NOT_FOUND;
    }
    
    file_handle_t* file = &handles[handle];
    file_entry_t* entry = &fs->entries[file->entry_index];
    
    // Check if position is valid
    if (position > entry->size) {
        position = entry->size;  // Can't seek past end of file
    }
    
    file->position = position;
    return position;
}

/* Get size of a file */
int fs_get_size(const char* filename) {
    int index = fs_find_file(filename);
    if (index < 0) {
        return index;  // Error code
    }
    
    return fs->entries[index].size;
}

/* List files matching a pattern */
int fs_list_files(const char* pattern) {
    int count = 0;
    
    print_string("\nFilename        Type Size    Date       Attr\n");
    print_string("--------------------------------------------\n");
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs->entries[i].type != FILE_TYPE_NONE) {
            // Simple pattern matching
            if (pattern == NULL || pattern[0] == '*' || 
                strncmp(fs->entries[i].filename, pattern, strlen(pattern)) == 0) {
                
                file_entry_t* entry = &fs->entries[i];
                
                // Print filename with padding
                print_string(entry->filename);
                int padding = 16 - strlen(entry->filename);
                for (int j = 0; j < padding; j++) {
                    print_char(' ');
                }
                
                // Print file type
                const char* type_str = "????";
                switch (entry->type) {
                    case FILE_TYPE_BINARY: type_str = "BIN "; break;
                    case FILE_TYPE_TEXT:   type_str = "TXT "; break;
                    case FILE_TYPE_DIRECTORY: type_str = "DIR "; break;
                }
                print_string(type_str);
                
                // Print size
                char size_str[8];
                int_to_string(entry->size, size_str);
                print_string(size_str);
                padding = 8 - strlen(size_str);
                for (int j = 0; j < padding; j++) {
                    print_char(' ');
                }
                
                // Print date (MM/DD/YY format)
                uint16_t date = entry->modified_date;
                uint8_t day = date & 0x1F;
                uint8_t month = (date >> 5) & 0xF;
                uint16_t year = 2000 + ((date >> 9) & 0x7F);
                
                char date_str[12];
                int_to_string(month, date_str);
                print_string(date_str);
                print_char('/');
                int_to_string(day, date_str);
                print_string(date_str);
                print_char('/');
                int_to_string(year, date_str);
                print_string(date_str);
                
                // Print attributes
                print_string("  ");
                if (entry->attributes & FILE_ATTR_READONLY) print_char('R'); else print_char('-');
                if (entry->attributes & FILE_ATTR_HIDDEN)   print_char('H'); else print_char('-');
                if (entry->attributes & FILE_ATTR_SYSTEM)   print_char('S'); else print_char('-');
                if (entry->attributes & FILE_ATTR_EXECUTE)  print_char('X'); else print_char('-');
                
                print_newline();
                count++;
            }
        }
    }
    
    if (count == 0) {
        print_string("No files found.\n");
    } else {
        print_char('\n');
        print_string("Total files: ");
        char num_str[8];
        int_to_string(count, num_str);
        print_string(num_str);
        print_newline();
    }
    
    return count;
}

/* Get free space in bytes */
int fs_get_free_space() {
    return fs->free_sectors * SECTOR_SIZE;
}

/* Create a directory */
int fs_mkdir(const char* dirname) {
    return fs_create(dirname, FILE_TYPE_DIRECTORY);
}
void fs_init_path(void) {
    strcpy(current_path, "/");
}
int fs_chdir(const char* dirname) {
    if (strcmp(dirname, "..") == 0) {
        // Go up one level
        if (strcmp(current_path, "/") != 0) {
            // We're not at root, so we need to go up
            
            // Find the last slash in the path
            int i = strlen(current_path) - 1;
            
            // Skip the trailing slash if it exists
            if (current_path[i] == '/') {
                i--;
            }
            
            // Find the previous slash
            while (i >= 0 && current_path[i] != '/') {
                i--;
            }
            
            // Cut the path at this point
            if (i >= 0) {
                current_path[i+1] = '\0';
                
                // If we're back at root, make sure path is "/"
                if (i == 0) {
                    current_path[1] = '\0';
                }
            }
        }
        
        current_directory = -1;  // Back to root for now
        return FS_SUCCESS;
    }
    
    int index = fs_find_file(dirname);
    if (index < 0) {
        return index;  // Error code
    }
    
    // Check if it's a directory
    if (fs->entries[index].type != FILE_TYPE_DIRECTORY) {
        return FS_ERROR_NOT_FOUND;
    }
    
    // Update current directory
    current_directory = index;
    
    // Update path
    if (strcmp(current_path, "/") != 0) {
        // Only add slash if we're not at root
        strcat(current_path, "/");
    }
    strcat(current_path, dirname);
    
    return FS_SUCCESS;
}

/* Remove a directory */
int fs_rmdir(const char* dirname) {
    int index = fs_find_file(dirname);
    if (index < 0) {
        return index;  // Error code
    }
    
    // Check if it's a directory
    if (fs->entries[index].type != FILE_TYPE_DIRECTORY) {
        return FS_ERROR_NOT_FOUND;
    }
    
    // Check if it's the current directory
    if (index == current_directory) {
        return FS_ERROR_PERMISSION;
    }
    
    // Delete the directory (uses same logic as deleting a file)
    return fs_delete(dirname);
}