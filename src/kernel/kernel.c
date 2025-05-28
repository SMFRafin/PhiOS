#include "print.h"
#include "input.h"
#include "commands.h"
#include "filesystem.h" 



void kernel_main() {

    set_video_mode();
    clear_screen();
    // Initialize the path to root
    fs_init_path();
    print_string("  _         _   __\n");
    print_string(" |_) |_  o / \\ (_  \n");
    print_string(" |   | | | \\_/ __) \n");
    print_string("\n");
    print_string("type phi -help for list of commands\n");
    fs_init();

    // Start the shell loop
    shell_loop();
}

// Add the display_prompt function
void display_prompt() {
    // Always position the prompt at column 0
    int x, y;
    get_cursor_pos(&x, &y);
    set_cursor_pos(0, y);
    
    print_string("|>:");
    print_string(current_path);
    

    if (current_path[strlen(current_path)-1] != '/') {
        print_char('/');
    }
    
    print_string(" ");
}

// Create a shell_loop function
void shell_loop() {
    while (1) {
        display_prompt();
        
        char input_buffer[64] = {0};
        get_input(input_buffer, 64);
        
        process_command(input_buffer);
        
        // Make sure we have a newline before the next prompt
        int x, y;
        get_cursor_pos(&x, &y);
        if (x != 0) {
            print_newline();
        }
    }
}
