/*
- Programming Assignment 02: ls-v1.2.0 (Column Display: Down Then Across)
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

int flag_l = 0;

void do_ls(const char *dir);
void print_file_info(const char *dir, const char *filename);
void print_columnar(char **filenames, int count, int max_len);
int get_term_width(void);

int main(int argc, char *argv[])
{
    int opt;

    // 1. Parse -l flag
    while ((opt = getopt(argc, argv, "l")) != -1)
    {
        if (opt == 'l')
        {
            flag_l = 1;
        }
    }

    // 2. Process directories
    if (optind == argc)
    {
        do_ls(".");
    }
    else
    {
        for (int i = optind; i < argc; i++)
        {
            if (argc - optind > 1)
                printf("%s:\n", argv[i]);

            do_ls(argv[i]);

            if (i < argc - 1)
                printf("\n");
        }
    }
    return 0;
}

// Gets terminal width using ioctl (fallback to 80 if unavailable)
int get_term_width(void)
{
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
    {
        return w.ws_col;
    }
    return 80; // Default fallback
}

void do_ls(const char *dir)
{
    DIR *dp = opendir(dir);
    if (dp == NULL)
    {
        perror(dir);
        return;
    }

    // Dynamic array to store filenames
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

        if (flag_l)
        {
            print_file_info(dir, entry->d_name);
        }
        else
        {
            // Expand dynamic array capacity if full
            if (count >= capacity)
            {
                capacity *= 2;
                filenames = realloc(filenames, capacity * sizeof(char *));
            }

            // Store copy of filename
            filenames[count] = strdup(entry->d_name);

            // Track longest filename length
            int len = strlen(entry->d_name);
            if (len > max_len)
            {
                max_len = len;
            }

            count++;
        }
    }

    closedir(dp);

    // If default mode (not -l) and files exist, print in columns
    if (!flag_l && count > 0)
    {
        print_columnar(filenames, count, max_len);

        // Free allocated memory
        for (int i = 0; i < count; i++)
        {
            free(filenames[i]);
        }
        free(filenames);
    }
}

// "Down then across" column printing logic
void print_columnar(char **filenames, int count, int max_len)
{
    int term_width = get_term_width();
    int col_width = max_len + 2; // 2 spaces buffer between columns

    // Calculate number of columns and rows
    int cols = term_width / col_width;
    if (cols < 1) cols = 1;

    int rows = (count + cols - 1) / cols; // Integer division ceiling formula

    // Print row by row
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            int idx = r + (c * rows);

            if (idx < count)
            {
                // Print filename left-aligned, padded to col_width
                if (c == cols - 1 || idx + rows >= count)
                {
                    printf("%s", filenames[idx]); // Last column in line doesn't need trailing space
                }
                else
                {
                    printf("%-*s", col_width, filenames[idx]);
                }
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

    printf("%s %2ld %-8s %-8s %8ld %.12s %s\n",
           mode_str,
           (long)info.st_nlink,
           user,
           group,
           (long)info.st_size,
           time_str + 4,
           filename);
}