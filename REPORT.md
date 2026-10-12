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


