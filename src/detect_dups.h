#ifndef DETECT_DUPS_H
#define DETECT_DUPS_H

// A header file for the detect_dups program
// This file contains the function declarations and structure definitions

#define _XOPEN_SOURCE 500 // for nftw

#include <ftw.h>        // File tree walking
#include <sys/stat.h>   // File status
#include <unistd.h>     // POSIX API (readlink)
#include <limits.h>     // PATH_MAX constant
#include <errno.h>      // Error numbers
#include <dirent.h>     // Directory operations
#include <stdio.h>      // Standard I/O
#include <stdlib.h>     // Standard library
#include <string.h>     // String operations
#include <fcntl.h>      // File control

// Digest contains the OpenSSL library
#include <openssl/evp.h>// OpenSSL EVP API
#include <openssl/md5.h> // MD5 digest

#include "uthash.h"

// define the structure required to store the file paths

typedef struct PathNode {
    char path[PATH_MAX]; // buffer to hold the file path
    struct PathNode *next; // pointer to the next node
} PathNode;

// softlink struct to store the soft links
typedef struct SoftLink {
    ino_t inode;
    PathNode *paths;
    size_t path_count;
    UT_hash_handle hh;
} SoftLink;

// hard link struct to store the hard links and soft links
typedef struct HardLink {
    ino_t inode;
    PathNode *paths;
    size_t path_count;
    SoftLink *soft_links;
    UT_hash_handle hh;
} HardLink;

// filegroup struct to store the file groups
typedef struct FileGroup {
    char md5[33];
    HardLink *hard_links;
    UT_hash_handle hh;
} FileGroup;

extern FileGroup *file_groups; // Global MD5 → HardLink mapping

// process nftw files using this function
static int render_file_info(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf);

// add any other function you may need over here

// Hashing and grouping
int compute_md5(const char *path, char *output_hash);
void insertRegFile(const char *md5, ino_t inode, nlink_t ref_count, const char *path);
void insertSym(const char *symlink_path, ino_t target_ino);

// Memory management
void addPath(PathNode **head, const char *path);
PathNode *createPN(const char *path);
void freePath(PathNode *head);
void cleanup();

#endif