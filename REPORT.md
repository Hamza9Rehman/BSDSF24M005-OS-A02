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


Feature 5


Why is it necessary to read all directory entries into memory before you can sort them? What 
are the potential drawbacks of this approach for directories containing millions of files?

Why It Is Necessary:
The readdir() system call returns directory entries one by one in whatever order the underlying file system stores them (un-ordered). Algorithms like qsort() need access to the full dataset in memory to make comparative swaps between any two elements across the list. Because you cannot perform swaps or random-access lookups directly on the directory stream, all entries must be stored in memory first before sorting can take place.   
Potential Drawbacks for Millions of Files:
High Memory Consumption: Allocating memory for millions of file name strings and array pointers can consume gigabytes of RAM, potentially triggering Out-Of-Memory (OOM) errors.   
Latency / Delay: The user experiences a noticeable delay because ls cannot print a single filename until every file in the directory has been read, allocated, and sorted.


Explain the purpose and signature of the comparison function required by qsort(). How does 
it work, and why must it take const void * arguments?

Function Signature: int compare_strings(const void *a, const void *b);

Purpose:
qsort() is a generic C standard library function that can sort any data type (integers, structs, strings). Because qsort() does not know what type of data it is sorting, it relies on your custom comparison function to compare two elements.   
How It Works:
It takes two pointers, casts them to string pointers (const char **), and compares the underlying strings using strcmp():   
Returns a negative integer if the first string comes before the second alphabetically.   
Returns 0 if both strings are identical.   
Returns a positive integer if the first string comes after the second alphabetically.   
Why It Must Take const void *:
void * is C's generic pointer type, allowing qsort() to accept array elements of any data type. The const keyword promises that the comparison function will not modify the data elements while comparing them. 


Feature 6


How do ANSI escape codes work to produce color in a standard Linux terminal? Show the 
specific code sequence for printing text in green.

How ANSI Escape Codes Work:
ANSI escape codes are special non-printable character sequences sent directly to the terminal's standard output stream. When the terminal emulator encounters the escape sequence starting with \033[ (or \e[), it intercepts it as an instruction to change display styling (such as text color, background, or text weight) rather than printing the characters to screen. All subsequent text remains styled until a reset code (\033[0m) is sent to return the terminal to default settings.

Specific Sequence for Green Text:
Start Green: \033[0;32m (or \033[32m)
Reset Color: \033[0m

Example in C:
printf("\033[0;32m%s\033[0m\n", filename);


To color an executable file, you need to check its permission bits. Explain which bits in the 
st_mode field you need to check to determine if a file is executable by the owner, group, or 
others.

To determine if a file is executable, you must inspect the permission bits within the st_mode field of struct stat using bitwise AND (&) operations against the POSIX execution permission macros:

Owner Execute Permission: Check S_IXUSR bit (0100 octal).
int is_owner_exec = (info.st_mode & S_IXUSR);

Group Execute Permission: Check S_IXGRP bit (0010 octal).
int is_group_exec = (info.st_mode & S_IXGRP);

Others Execute Permission: Check S_IXOTH bit (0001 octal).
int is_others_exec = (info.st_mode & S_IXOTH);

Combined Check in ls:
To check if any user class (owner, group, or others) has execute permission on the file, combine the macros using bitwise OR (|):
if (info.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
    // File is executable by at least one entity (Print in Green)
}