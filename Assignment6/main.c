// Copyright 2021 Matt Zagame
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>

void print_details(struct stat);

int main(int argc, char *argv[]) {
    struct stat statbuf;
    struct dirent *entry;
    DIR *dirp;
    int i;

    if (argc > 1) {
        for (i = 1; i < argc; i++) {
            if (lstat(argv[i], &statbuf) == -1) {
                perror("lstat() failed, exiting");
                exit(1);
            }
            printf("FILENAME:\t\t\t%s\n", argv[i]);
            print_details(statbuf);
        }
    } else {
        // open directory for reading
        if ((dirp = opendir(".")) == NULL) {
            perror("opendir() failed, exiting");
            exit(1);
        }
        // get file status on each directory entry
        while ((entry = readdir(dirp)) != NULL) {
            if ((lstat(entry->d_name, &statbuf)) == -1) {
                perror("lstat() failed, exiting");
                exit(1);
            }
            printf("FILENAME:\t\t\t%s\n", entry->d_name);
            print_details(statbuf);
        }
    }
    return 0;
}

void print_details(struct stat statbuf) {
    struct passwd *pwd;
    struct group *grp;
    struct tm *mtime;
    short mode;
    char file_type[19], perm[12], mtime_buf[19];
    const char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
    "Aug", "Sep", "Oct", "Nov", "Dec"};

    if ((pwd = getpwuid(statbuf.st_uid)) == NULL) {
        perror("getpwuid() failed, exiting");
        exit(1);
    }
    if ((grp = getgrgid(statbuf.st_gid)) == NULL) {
        perror("getgrgid() failed, exiting");
        exit(1);
    }

    // get file type
    mode = statbuf.st_mode;
    switch (mode & __S_IFMT) {
        case __S_IFREG:
            sprintf(file_type, "ordinary");
            break;
        case __S_IFDIR:
            sprintf(file_type, "directory");
            break;
        case __S_IFLNK:
            sprintf(file_type, "symbolic link");
            break;
        case __S_IFIFO:
            sprintf(file_type, "named pipe");
            break;
        case __S_IFSOCK:
            sprintf(file_type, "Unix Domain Socket");
            break;
        case __S_IFCHR:
            sprintf(file_type, "character device");
            break;
        case __S_IFBLK:
            sprintf(file_type, "block device");
            break;
        default:
            sprintf(file_type, "?");
            break;
    }
    // get file permissions
    sprintf(perm, (mode & S_IRUSR) ? "r" : "-");
    sprintf(perm + strlen(perm), (mode & S_IWUSR) ? "w" : "-");
    sprintf(perm + strlen(perm), (mode & S_IXUSR) ? "x " : "- ");
    sprintf(perm + strlen(perm), (mode & S_IRGRP) ? "r" : "-");
    sprintf(perm + strlen(perm), (mode & S_IWGRP) ? "w" : "-");
    sprintf(perm + strlen(perm), (mode & S_IXGRP) ? "x " : "- ");
    sprintf(perm + strlen(perm), (mode & S_IROTH) ? "r" : "-");
    sprintf(perm + strlen(perm), (mode & S_IWOTH) ? "w" : "-");
    sprintf(perm + strlen(perm), (mode & S_IXOTH) ? "x" : "-");

    // setup time since last modification
    mtime = localtime(&statbuf.st_mtime);
    sprintf(mtime_buf, "%s %02d %02d:%02d %04d", months[mtime->tm_mon],
            mtime->tm_mday, mtime->tm_hour, mtime->tm_min, mtime->tm_year +
            1900);

    // print directory entry contents
    printf("FILE_TYPE:\t\t\t%s\nPERMISSIONS:\t\t\t%s\nOWNER_NAME:\t\t\t%s\n"
           "GROUP_NAME:\t\t\t%s\nDATE_OF_LAST_MODIFICATION:\t%s\nLINK_COUNT:"
           "\t\t\t%lu\nSIZE_IN_BYTES:\t\t\t%ld\nINODE_NUMBER:\t\t\t%lu\n\n",
           file_type, perm, pwd->pw_name, grp->gr_name, mtime_buf,
           statbuf.st_nlink, statbuf.st_size, statbuf.st_ino);
}
