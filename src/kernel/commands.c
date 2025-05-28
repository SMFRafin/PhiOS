//commands.c
#include "commands.h"
#include "print.h"
#include "string.h"
#include <stddef.h> // For NULL
#include "filesystem.h" // For fs and handles

void reboot() {
    asm volatile (
        "ljmp $0xFFFF, $0x0000"
        :::
    );
}

void shutdown() {
    asm volatile (
        "movw $0x2000, %%ax\n\t"
        "movw $0x604, %%dx\n\t"
        "outw %%ax, %%dx"
        ::: "ax", "dx"
    );
}


void print_command(const char* cmd, const char* desc) {
    print_string(" ");
    print_string(cmd);

    // Calculate padding needed (aim for 15 character column width)
    int padding = 15 - strlen(cmd);
    for (int i = 0; i < padding; i++) {
        print_char(' ');
    }

    print_string(desc);
    print_newline();
}

void init_file_system() {
    // Initialize the file system
    if (fs_init() != FS_SUCCESS) {
        print_string("Error initializing file system\n");
        return;
    }

    // Format the file system
    if (fs_format() != FS_SUCCESS) {
        print_string("Error formatting file system\n");
        return;
    }

    print_string("File system initialized and formatted successfully\n");
}

void create_file(const char* filename, uint8_t type) {
    int result=fs_create(filename,type);
    if(result<0)
    {
        print_string("Error creating file");
        return;
    }
    print_string("File created successfully\n");
}

void delete_file(const char* filename)
{
    if(fs_delete(filename)!=FS_SUCCESS)
    {
        print_string("Error deleteing file");
        return;
    }
    print_string("File deteled");
}

void write_to_file(const char* filename, const char* content) {
    int handle = fs_open(filename, 1); // Mode 1 = write mode
    

    if (handle < 0) {
        int result = fs_create(filename, FILE_TYPE_TEXT);
        if (result < 0) {
            print_string("Error creating file\n");
            return;
        }
        
        handle = fs_open(filename, 1);
        if (handle < 0) {
            print_string("Error opening file\n");
            return;
        }
    }
    
    // Write content to the file
    int bytes_written = fs_write(handle, content, strlen(content));
    if (bytes_written < 0) {
        print_string("Error writing to file\n");
        fs_close(handle);
        return;
    }
    
    // Close the file
    fs_close(handle);
    
    print_string("Successfully wrote ");
    char num_str[8];
    int_to_string(bytes_written, num_str);
    print_string(num_str);
    print_string(" bytes to ");
    print_string(filename);
    print_newline();
}


void read_from_file(const char* filename) {
    // Open the file for reading
    int handle = fs_open(filename, 0); // Mode 0 = read mode
    if (handle < 0) {
        print_string("Error opening file for reading\n");
        return;
    }
    
    // Get file size
    int file_size = fs_get_size(filename);
    if (file_size < 0) {
        print_string("Error getting file size\n");
        fs_close(handle);
        return;
    }
    

    char buffer[MAX_FILE_SIZE];
    if (file_size > MAX_FILE_SIZE - 1) {
        print_string("File too large to read\n");
        fs_close(handle);
        return;
    }
    
    // Read file content
    int bytes_read = fs_read(handle, buffer, file_size);
    if (bytes_read < 0) {
        print_string("Error reading from file\n");
        fs_close(handle);
        return;
    }
    
    // Null-terminate the buffer to make it a valid string
    buffer[bytes_read] = '\0';
    
    // Close the file
    fs_close(handle);
    
    // Display file content
    print_string("\n--- Begin file content ---\n");
    print_string(buffer);
    print_string("\n--- End file content ---\n");
}

void print_phi_logo() {
    int x, y;
    get_cursor_pos(&x, &y);

    set_cursor_pos(16, y + 2);
    print_string(":===+@@@@+===-");

    // Line 2
    set_cursor_pos(15, y + 3);
    print_string(":-=+***@@@@*+++=-:");

    // Line 3
    set_cursor_pos(11, y + 4);
    print_string("=%@@%+:. .@@@@:  :=%@@#=");

    // Line 4
    set_cursor_pos(10, y + 5);
    print_string("%@@@#     .@@@@:     #@@@%");

    // Line 5
    set_cursor_pos(9, y + 6);
    print_string("=@@@@-     .@@@@:     -@@@@+");

    // Line 6
    set_cursor_pos(9, y + 7);
    print_string("+@@@@:     .@@@@:     :@@@@+");

    // Line 7
    set_cursor_pos(9, y + 8);
    print_string(".@@@@*     .@@@@:     +@@@@.");

    // Line 8
    set_cursor_pos(10, y + 9);
    print_string(".*@@@#-   .@@@@:   :*@@@*.");

    // Line 9
    set_cursor_pos(13, y + 10);
    print_string(":=+*#***@@@@#**##*=:");

    // Line 10
    set_cursor_pos(16, y + 11);
    print_string(":===+@@@@+===-");

    // Version text
    set_cursor_pos(16, y + 13);
    print_string("PhiOS - 0.2.0 by SMF");

    // Move cursor to a good position for the next prompt
    set_cursor_pos(0, y + 15);
}

// Function to list files using our file system
void list_directories() {
    fs_list_files(NULL);
}

// Function to execute a program at a specific memory location
void execute_program(unsigned short segment, unsigned short offset) {
    print_string("\nExecuting program...\n");

    asm volatile (
        "pushw %%ds\n\t"        // Save current data segment
        "movw %0, %%ds\n\t"     // Load new data segment
        "lcall *%1\n\t"         // Call far (segment:offset)
        "popw %%ds"             // Restore data segment
        :
        : "r" (segment), "m" (offset)
        : "memory"
    );

    print_string("\nProgram execution complete\n");
}

// Function to run a program using our file system
void run_program(const char* filename) {
    // Try to open the file
    int handle = fs_open(filename, 0); // Open in read mode
    
    if (handle < 0) {
        print_string("\nProgram not found: ");
        print_string(filename);
        print_newline();
        return;
    }
    
    // Check if it has execute permission
    int index = handles[handle].entry_index;
    if (!(fs->entries[index].attributes & FILE_ATTR_EXECUTE)) {
        print_string("\nFile is not executable: ");
        print_string(filename);
        print_newline();
        fs_close(handle);
        return;
    }
    
    print_string("\nLoading program: ");
    print_string(filename);
    print_newline();
    
    // Allocate memory at 0x5000 for the program
    unsigned short program_segment = 0x5000;
    unsigned short program_offset = 0;
    
    // Read the file into memory
    char* buffer = (char*)(program_segment << 4);
    int size = fs_read(handle, buffer, MAX_FILE_SIZE);
    
    fs_close(handle);
    
    if (size <= 0) {
        print_string("Error loading program\n");
        return;
    }
    
    // Execute the program
    execute_program(program_segment, program_offset);
}
void process_command(const char* input) {
    if (strcmp(input, "reboot") == 0) {
        reboot();
    }  else if (strcmp(input, "cls") == 0) {
        clear_screen();
    } else if (strcmp(input, "phi -help") == 0) {
        print_string("\nCommands:\n");
        print_command("reboot", "Restarts the system");
        print_command("dirs", "Lists files in system");
        print_command("cls", "Clears the screen");
        print_command("phi -v", "Shows PhiOS version");
        print_command("phi -phi", "Displays PhiOS logo");
        print_command("say <text>", "Echoes text to screen");
        print_command("shutdown", "Powers off the system");
        print_command("run <filename>", "Runs a program");
        print_command("dirs", "Lists files in system");
        print_command("help", "Displays this help message");
        print_command("run <filename>", "Runs a program");
        print_command("initfs", "Initializes the file system");
        print_command("touch <filename>", "Creates a new text file");
        print_command("rnm <oldname> <newname>", "Creates a new text file");
        print_command("write <filename> <content>", "Writes content to a file");
        print_command("read <filename>", "Reads content from a file");
        print_command("mkdir <dirname>", "Creates a new directory");
        print_command("rm <filename>", "Deletes the file");
        print_command("rmdir <directory name>", "Deletes the directory");
        print_command("cd <directory name>", "Change directory");
        
        
        print_newline();
    } else if (strcmp(input, "phi -v") == 0) {
        print_string("\nPhiOS 0.2.0\n");
    } else if (strcmp(input, "phi -phi") == 0) {
        print_string("\n");
        print_phi_logo();
    } else if (strncmp(input, "say", 3) == 0) {
        print_string("\n");
        print_string(input + 4);  // skip "say "
        print_newline();
    } 
    else if (strncmp(input, "run", 3) == 0) {
        const char* filename = input + 4; // skip "run "
        run_program(filename);
    } else if (strcmp(input, "dirs") == 0) {
        list_directories();
    } else if (strcmp(input, "help") == 0) {
        print_string("\nType 'phi -help' for a list of commands\n");
    }
    else if (strcmp(input, "initfs") == 0) {
        init_file_system();
    }
    else if (strncmp(input, "touch", 5) == 0) {
        const char* filename = input + 6; // skip "touch "
        create_file(filename, FILE_TYPE_TEXT);
    }
    else if (strncmp(input, "write", 5) == 0) {
        // Check if there's at least one space after "write"
        char* first_space = strchr(input + 5, ' ');
        if (first_space == NULL) {
            print_string("\nUsage: write <filename> content\n");
            return;
        }
        
        // Look for the second space that separates filename and content
        char* second_space = strchr(first_space + 1, ' ');
        if (second_space == NULL) {
            print_string("\nUsage: write <filename> content\n");
            return;
        }
        
        // Temporarily null-terminate to extract the filename
        *second_space = '\0';
        char* filename = first_space + 1;
        char* content = second_space + 1;
        
        // Call the write function
        write_to_file(filename, content);
        
        // Restore the space if needed
        *second_space = ' ';
    }

    else if (strncmp(input, "read", 4) == 0) {
        const char* filename = input + 5; // skip "read "
        read_from_file(filename);
    }
    else if (strncmp(input,"mkdir",5)==0)
    {
        const char* dirname = input + 6; // skip "mkdir "
        int result=fs_mkdir(dirname);
        if(result<0)
        {
            print_string("\nError creating directory\n");
        }
        else{
            print_string("\nDirectory created\n");
        }
    }

    else if(strncmp(input,"rmdir",5)==0)
    {
        const char* dirname = input + 6; // skip "rmdir "
        if (fs_rmdir(dirname) == FS_SUCCESS) {
            print_string("\nDirectory removed successfully\n");
        } else {
            print_string("\nError removing directory\n");
        }
    }
    else if(strncmp(input,"cd",2)==0)
    {
        const char* dirname=input+3;
        if(fs_chdir(dirname)!=FS_SUCCESS)
        {
            print_string("\nError changing directory\n");
        }
        else{
            print_string("\nChanged Directory to: ");
            print_string(current_path);
            print_newline();
        }
    }
    else if(strncmp(input,"rm",2)==0)
    {
        const char* filename=input+3;
        delete_file(filename);
    }

    else if(strncmp(input,"rnm",3)==0)
    {
        // Check if there's at least one space after "rnm"
        char* first_space = strchr(input + 3, ' ');
        if (first_space == NULL) {
            print_string("\nUsage: rnm <oldname> <newname>\n");
            return;
        }
        
        // Look for the second space that separates oldname and newname
        char* second_space = strchr(first_space + 1, ' ');
        if (second_space == NULL) {
            print_string("\nUsage: rnm <oldname> <newname>\n");
            return;
        }
        
        // Temporarily null-terminate to extract the old and new names
        *second_space = '\0';
        char* oldname = first_space + 1;
        char* newname = second_space + 1;
        
        // Call the rename function
        int result = fs_rename(oldname, newname);
        
        // Restore the space if needed
        *second_space = ' ';
        
        if (result == FS_SUCCESS) {
            print_string("\nFile renamed successfully\n");
        } else {
            print_string("\nError renaming file\n");
        }
    }
    else if (strcmp(input, "exit") == 0) {
        print_string("\nExiting...\n");
    }
    else if (strcmp(input, "shutdown") == 0) {
        shutdown();
    } else {
        print_string("\nInvalid Command\n");
    }
}