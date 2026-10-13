/*
- Programming Assignment 02: ls-v1.6.0 (Recursive Listing -R)
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

// ANSI Color Codes
#define COLOR_RESET   "\033[0m"
#define COLOR_BLUE    "\033[0;34m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_RED     "\033[0;31m"
#define COLOR_PINK    "\033[0;35m"
#define COLOR_REVERSE "\033[7m"

// Display modes
#define MODE_VERTICAL   0  // Default "down then across"
#define MODE_LONG       1  // -l option
#define MODE_HORIZONTAL 2  // -x option

int display_mode = MODE_VERTICAL;
int flag_R = 0;  // -R recursive flag

void do_ls(const char *dir, int print_header);
void print_file_info(const char *dir, const char *filename);
void print_columnar_vertical(const char *dir, char **filenames, int count, int max_len);
void print_columnar_horizontal(const char *dir, char **filenames, int count, int max_len);
void print_colored_filename(const char *dir, const char *filename, int padding);
int is_archive(const char *filename);
int compare_strings(const void *a, const void *b);
int get_term_width(void);

int main(int argc, char *argv[])
{
    int opt;

    // 1. Parse -l, -x, and -R options using getopt
    while ((opt = getopt(argc, argv, "lxR")) != -1)
    {
        switch (opt)
        {
            case 'l':
                display_mode = MODE_LONG;
                break;
            case 'x':
                display_mode = MODE_HORIZONTAL;
                break;
            case 'R':
                flag_R = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [-R] [directory...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // 2. Process directories
    int num_dirs = argc - optind;
    if (num_dirs == 0)
    {
        do_ls(".", flag_R ? 1 : 0);
    }
    else
    {
        for (int i = optind; i < argc; i++)
        {
            // Print header if multiple directories or if recursive mode is enabled
            int print_hdr = (num_dirs > 1 || flag_R) ? 1 : 0;
            do_ls(argv[i], print_hdr);

            if (i < argc - 1)
                printf("\n");
        }
    }
    return 0;
}

// Check if filename has archive extension (.tar, .gz, .zip, etc.)
int is_archive(const char *filename)
{
    const char *exts[] = {".tar", ".gz", ".zip", ".tgz", ".bz2", ".xz", NULL};
    for (int i = 0; exts[i] != NULL; i++)
    {
        size_t len = strlen(filename);
        size_t ext_len = strlen(exts[i]);
        if (len >= ext_len && strcmp(filename + len - ext_len, exts[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

// Print filename with ANSI color based on st_mode and file extension
void print_colored_filename(const char *dir, const char *filename, int padding)
{
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", dir, filename);

    struct stat info;
    if (lstat(path, &info) == -1)
    {
        printf("%-*s", padding, filename);
        return;
    }

    const char *color = COLOR_RESET;

    if (S_ISLNK(info.st_mode))
    {
        color = COLOR_PINK; // Symbolic Link: Pink
    }
    else if (S_ISDIR(info.st_mode))
    {
        color = COLOR_BLUE; // Directory: Blue
    }
    else if (S_ISCHR(info.st_mode) || S_ISBLK(info.st_mode) || S_ISFIFO(info.st_mode) || S_ISSOCK(info.st_mode))
    {
        color = COLOR_REVERSE; // Special Files: Reverse Video
    }
    else if (is_archive(filename))
    {
        color = COLOR_RED; // Tarballs & Archives: Red
    }
    else if (info.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
    {
        color = COLOR_GREEN; // Executable: Green
    }

    printf("%s%s%s", color, filename, COLOR_RESET);

    int len = strlen(filename);
    for (int i = len; i < padding; i++)
    {
        putchar(' ');
    }
}

// Comparison function for qsort
int compare_strings(const void *a, const void *b)
{
    const char *strA = *(const char **)a;
    const char *strB = *(const char **)b;
    return strcmp(strA, strB);
}

// Gets terminal width using ioctl (fallback to 80 if unavailable)
int get_term_width(void)
{
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
    {
        return w.ws_col;
    }
    return 80;
}

void do_ls(const char *dir, int print_header)
{
    DIR *dp = opendir(dir);
    if (dp == NULL)
    {
        perror(dir);
        return;
    }

    if (print_header)
    {
        printf("%s:\n", dir);
    }

    // Dynamic array to store all filenames
    int capacity = 16;
    int count = 0;
    int max_len = 0;
    char **filenames = malloc(capacity * sizeof(char *));

    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL)
    {
        // Skip hidden files
        if (entry->d_name[0] == '.')
            continue;

        if (count >= capacity)
        {
            capacity *= 2;
            filenames = realloc(filenames, capacity * sizeof(char *));
        }

        filenames[count] = strdup(entry->d_name);

        int len = strlen(entry->d_name);
        if (len > max_len)
        {
            max_len = len;
        }

        count++;
    }

    closedir(dp);

    if (count > 0)
    {
        // 1. Sort filenames alphabetically
        qsort(filenames, count, sizeof(char *), compare_strings);

        // 2. Render current directory contents using selected display mode
        if (display_mode == MODE_LONG)
        {
            for (int i = 0; i < count; i++)
            {
                print_file_info(dir, filenames[i]);
            }
        }
        else if (display_mode == MODE_HORIZONTAL)
        {
            print_columnar_horizontal(dir, filenames, count, max_len);
        }
        else
        {
            print_columnar_vertical(dir, filenames, count, max_len);
        }

        // 3. Handle recursion (-R flag)
        if (flag_R)
        {
            for (int i = 0; i < count; i++)
            {
                char full_path[1024];
                snprintf(full_path, sizeof(full_path), "%s/%s", dir, filenames[i]);

                struct stat st;
                if (lstat(full_path, &st) == 0)
                {
                    // Check if entry is a directory (and not symlink)
                    if (S_ISDIR(st.st_mode) && !S_ISLNK(st.st_mode))
                    {
                        printf("\n");
                        do_ls(full_path, 1);
                    }
                }
            }
        }

        // 4. Free allocated dynamic memory
        for (int i = 0; i < count; i++)
        {
            free(filenames[i]);
        }
        free(filenames);
    }
}

// Horizontal ("across") column printing logic (-x)
void print_columnar_horizontal(const char *dir, char **filenames, int count, int max_len)
{
    int term_width = get_term_width();
    int col_width = max_len + 2;
    int current_pos = 0;

    for (int i = 0; i < count; i++)
    {
        if (current_pos + col_width > term_width && current_pos > 0)
        {
            printf("\n");
            current_pos = 0;
        }

        print_colored_filename(dir, filenames[i], col_width);
        current_pos += col_width;
    }
    printf("\n");
}

// Vertical ("down then across") column printing logic (Default)
void print_columnar_vertical(const char *dir, char **filenames, int count, int max_len)
{
    int term_width = get_term_width();
    int col_width = max_len + 2;

    int cols = term_width / col_width;
    if (cols < 1) cols = 1;

    int rows = (count + cols - 1) / cols;

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            int idx = r + (c * rows);

            if (idx < count)
            {
                int pad = (c == cols - 1 || idx + rows >= count) ? 0 : col_width;
                print_colored_filename(dir, filenames[idx], pad);
            }
        }
        printf("\n");
    }
}

// Helper to format permissions (rwx)
void format_mode(mode_t mode, char *str)
{
    strcpy(str, "----------");

    if (S_ISDIR(mode))  str[0] = 'd';
    if (S_ISLNK(mode))  str[0] = 'l';
    if (S_ISCHR(mode))  str[0] = 'c';
    if (S_ISBLK(mode))  str[0] = 'b';

    if (mode & S_IRUSR) str[1] = 'r';
    if (mode & S_IWUSR) str[2] = 'w';
    if (mode & S_IXUSR) str[3] = 'x';

    if (mode & S_IRGRP) str[4] = 'r';
    if (mode & S_IWGRP) str[5] = 'w';
    if (mode & S_IXGRP) str[6] = 'x';

    if (mode & S_IROTH) str[7] = 'r';
    if (mode & S_IWOTH) str[8] = 'w';
    if (mode & S_IXOTH) str[9] = 'x';
}

void print_file_info(const char *dir, const char *filename)
{
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", dir, filename);

    struct stat info;
    if (lstat(path, &info) == -1)
    {
        perror("lstat");
        return;
    }

    char mode_str[11];
    format_mode(info.st_mode, mode_str);

    struct passwd *pw = getpwuid(info.st_uid);
    struct group  *gr = getgrgid(info.st_gid);

    char user[32], group[32];
    snprintf(user, sizeof(user), "%s", pw ? pw->pw_name : "unknown");
    snprintf(group, sizeof(group), "%s", gr ? gr->gr_name : "unknown");

    char *time_str = ctime(&info.st_mtime);
    time_str[strlen(time_str) - 1] = '\0';

    printf("%s %2ld %-8s %-8s %8ld %.12s ",
           mode_str,
           (long)info.st_nlink,
           user,
           group,
           (long)info.st_size,
           time_str + 4);

    print_colored_filename(dir, filename, 0);
    printf("\n");
}