Feature 2


What is the crucial difference between the stat() and lstat() system calls? In the context of 
the ls command, when is it more appropriate to use lstat()?

Difference: stat() follows symbolic links and returns info about the target file. lstat() does not follow links—it returns info about the symbolic link itself.
When ls uses lstat(): Always use lstat() in ls so symbolic links are identified as links (l) rather than showing properties of the file they point to (and to avoid breaking on broken links). 


The st_mode field in struct stat is an integer that contains both the file type (e.g., regular 
file, directory) and the permission bits. Explain how you can use bitwise operators (like &) and 
predefined macros (like S_IFDIR or S_IRUSR) to extract this information.

File Type: Mask st_mode with S_IFMT using & to check against constants like S_IFDIR (or use macro helper S_ISDIR(st_mode)).   
Permissions: Use & with permission flags (e.g., mode & S_IRUSR for owner read, mode & S_IWUSR for owner write). If the result is non-zero, the permission is enabled.


Feature 3


Explain the general logic for printing items in a "down then across" columnar format. Why is 
a simple single loop through the list of filenames insufficient for this task?

General Logic:Find the longest filename and add spacing (like 2 spaces) to get the column width.   Divide the terminal width by the column width to see how many columns fit on screen.   
Divide total files by columns to figure out how many rows are needed.   
Print row by row. For each row, print the file for that row, then skip ahead by the number of rows to get the file for the next column on the same line.   
Why a Single Loop Fails:
Terminals can only print line-by-line from left to right and top to bottom. A simple single loop prints files sequentially (0, 1, 2, 3...) across a line. To fill columns downwards first, you have to jump across your array to pick out the files that belong on the current line before sending a newline character.


What is the purpose of the ioctl system call in this context? What would be the limitations of 
your program if you only used a fixed-width fallback (e.g., 80 columns) instead of detecting 
the terminal size?

Purpose of ioctl:
The ioctl system call asks the Linux terminal driver for the exact width of the current terminal window in characters.   
Limitations of Fixed-Width Fallback (80 columns):
On larger terminals: It wastes empty space on the right side of the screen by wrapping early and making the file list unnecessarily long vertically.   
On smaller terminals: If the window is narrower than 80 characters, long lines break onto new lines, messing up the column alignment.


Feature 4


Compare the implementation complexity of the "down then across" (vertical) printing logic 
versus the "across" (horizontal) printing logic. Which one requires more pre-calculation and 
why?

The "down then across" (vertical) printing logic requires significantly more pre-calculation.   Horizontal ("Across") Logic: Operates in a single simple loop from left to right (0 to N-1). It only needs to know the column width to keep track of the current screen position and issue a newline (\n) when full.   
Vertical ("Down Then Across") Logic: Requires multi-step index math upfront. Before printing a single character, it must calculate terminal width, column width, total columns, and total rows. Because terminal output is line-by-line, it must calculate custom array offsets (row + col * rows) to jump through memory and collect items for the current line.


Describe the strategy you used in your code to manage the different display modes (-l, -x, 
and default). How did your program decide which function to call for printing?

State Management Strategy:
Defined an integer state variable (display_mode) with named macro constants (MODE_VERTICAL, MODE_LONG, MODE_HORIZONTAL).   
Option Parsing:
Inside main(), getopt() checks command-line switches and updates display_mode whenever -l or -x is detected.   
Function Selection / Routing:
Inside do_ls(), after directory entries are gathered, the code checks display_mode:   
If display_mode == MODE_LONG, it calls print_file_info() directly.   
If display_mode == MODE_HORIZONTAL, it calls print_columnar_horizontal().   
Otherwise (default), it falls back to calling print_columnar_vertical().