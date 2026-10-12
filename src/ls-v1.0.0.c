/*
- Programming Assignment 02: ls-v1.1.0 (Long Listing Format)
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

int flag_l = 0;

void do_ls(const char *dir);
void print_file_info(const char *dir, const char *filename);

int main(int argc, char *argv[])
{
    int opt;

    // 1. Parse command-line options using getopt
    while ((opt = getopt(argc, argv, "l")) != -1)
    {
        if (opt == 'l')
        {
            flag_l = 1;
        }
    }

    // 2. Process directories (or default to current directory ".")
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

void do_ls(const char *dir)
{
    DIR *dp = opendir(dir);
    if (dp == NULL)
    {
        perror(dir);
        return;
    }

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
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dp);
}

// Helper to format file permissions (rwx)
void format_mode(mode_t mode, char *str)
{
    strcpy(str, "----------");

    // File type
    if (S_ISDIR(mode))  str[0] = 'd';
    if (S_ISLNK(mode))  str[0] = 'l';
    if (S_ISCHR(mode))  str[0] = 'c';
    if (S_ISBLK(mode))  str[0] = 'b';

    // Permissions
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

    // Convert mode to rwx string
    char mode_str[11];
    format_mode(info.st_mode, mode_str);

    // Resolve owner and group names
    struct passwd *pw = getpwuid(info.st_uid);
    struct group  *gr = getgrgid(info.st_gid);

    char user[32], group[32];
    snprintf(user, sizeof(user), "%s", pw ? pw->pw_name : "unknown");
    snprintf(group, sizeof(group), "%s", gr ? gr->gr_name : "unknown");

    // Format time string
    char *time_str = ctime(&info.st_mtime);
    time_str[strlen(time_str) - 1] = '\0'; // trim trailing newline

    // Print formatted long-listing line
    printf("%s %2ld %-8s %-8s %8ld %.12s %s\n",
           mode_str,
           (long)info.st_nlink,
           user,
           group,
           (long)info.st_size,
           time_str + 4, // skips day-of-week (e.g. "Mon ")
           filename);
}