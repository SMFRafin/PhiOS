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
    // Create a new file
    if (fs_create(filename, type) != FS_SUCCESS) {
        print_string("Error creating file\n");
        return;
    }

    print_string("File created successfully\n");
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
    else if (strncmp(input,"mkdir",5)==0)
    {
        const char* dirname = input + 6; // skip "mkdir "
        if (fs_mkdir(dirname) == FS_SUCCESS) {
            print_string("\nDirectory created successfully\n");
        } else {
            print_string("\nError creating directory\n");
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