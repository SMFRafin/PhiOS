#include "print.h"
#include "input.h"
#include "commands.h"

void kernel_main() {
    set_video_mode();
    clear_screen();

    print_string("\n\n");
    print_string("  _         _   __\n");
    print_string(" |_) |_  o / \\ (_  \n");
    print_string(" |   | | | \\_/ __) \n");
    print_string("\n");
    print_string("type phi -help for list of commands\n");

    while (1) {
        // Always position the prompt at column 0
        int x, y;
        get_cursor_pos(&x, &y);
        set_cursor_pos(0, y);
        
        print_string("|>:");
        char input_buffer[64] = {0};
        get_input(input_buffer, 64);
        

        process_command(input_buffer);

        get_cursor_pos(&x, &y);
        if (x != 0) {
            print_newline();
        }
    }
}