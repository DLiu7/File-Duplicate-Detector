#include "detect_dups.h"

#include <ftw.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <libgen.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include "uthash.h"

#define MD5_HASH_SIZE 33

FileGroup *file_groups = NULL;
EVP_MD_CTX *mdctx;
const EVP_MD *EVP_md5();
char base_path[PATH_MAX];

static int render_file_info(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf) {
    (void)ftwbuf;

    switch (tflag) {
        case FTW_F: {
            if (S_ISREG(sb->st_mode)) {
                struct stat lsb;
                if (lstat(fpath, &lsb) == 0 && S_ISLNK(lsb.st_mode)) {
                    break;
                }

                char md5[MD5_HASH_SIZE];
                if (compute_md5(fpath, md5) == 0) {
                    insertRegFile(md5, sb->st_ino, sb->st_nlink, fpath);
                }
            }
            break;
        }

        case FTW_SL: {
            char link_target[PATH_MAX];
            ssize_t len = readlink(fpath, link_target, sizeof(link_target) - 1);
            if (len == -1) {
                perror("readlink failed");
                break;
            }
            link_target[len] = '\0';

            char full_target_path[PATH_MAX];

            if (link_target[0] == '/') {
                strncpy(full_target_path, link_target, sizeof(full_target_path) - 1);
                full_target_path[sizeof(full_target_path) - 1] = '\0';
            } else {
                char fpath_copy[PATH_MAX];
                strncpy(fpath_copy, fpath, PATH_MAX - 1);
                fpath_copy[PATH_MAX - 1] = '\0';
                char *dir = dirname(fpath_copy);

                snprintf(full_target_path, sizeof(full_target_path), "%s/%s", dir, link_target);
            }

            struct stat target_stat;
            if (stat(full_target_path, &target_stat) == 0 && S_ISREG(target_stat.st_mode)) {
                insertSym(fpath, target_stat.st_ino);
            } else {
                perror("stat failed on symlink target");
            }

            break;
        }

        default:
            break;
    }

    return 0;
}

int compute_md5(const char *path, char *output) {
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    const EVP_MD *md = EVP_md5();
    unsigned char digest[EVP_MAX_MD_SIZE];

    FILE *file = fopen(path, "rb");
    unsigned char buffer[4096];
    size_t bytes_read;
    unsigned int md_len;

    if (!mdctx || !md || !file) {
        if (mdctx) EVP_MD_CTX_free(mdctx);
        if (file) fclose(file);
        return -1;
    }

    EVP_DigestInit_ex(mdctx, md, NULL);
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file))) {
        EVP_DigestUpdate(mdctx, buffer, bytes_read);
    }
    EVP_DigestFinal_ex(mdctx, digest, &md_len);

    for (unsigned int i = 0; i < md_len; i++) {
        sprintf(output + (i * 2), "%02x", digest[i]);
    }
    output[32] = '\0';

    EVP_MD_CTX_free(mdctx);
    fclose(file);
    return 0;
}

PathNode* createPN(const char *path) {
    PathNode *newNode = malloc(sizeof(PathNode));
    if (newNode) {
        strncpy(newNode->path, path, PATH_MAX);
        newNode->path[PATH_MAX - 1] = '\0';
        newNode->next = NULL;
    }
    return newNode;
}

void addPath(PathNode **head, const char *path) {
    PathNode *newNode = createPN(path);
    if (!newNode) return;
    newNode->next = *head;
    *head = newNode;
}

void insertRegFile(const char *md5, ino_t inode, nlink_t ref_count, const char *path) {
    FileGroup *fg = NULL;
    HASH_FIND_STR(file_groups, md5, fg);
    if (!fg) {
        fg = malloc(sizeof(FileGroup));
        if (!fg) return;
        strncpy(fg->md5, md5, 33);
        fg->hard_links = NULL;
        HASH_ADD_STR(file_groups, md5, fg);
    }

    HardLink *hl = NULL;
    HASH_FIND(hh, fg->hard_links, &inode, sizeof(ino_t), hl);
    if (!hl) {
        hl = malloc(sizeof(HardLink));
        if (!hl) return;
        hl->inode = inode;
        hl->path_count = 0;
        hl->paths = NULL;
        hl->soft_links = NULL;
        HASH_ADD(hh, fg->hard_links, inode, sizeof(ino_t), hl);
    }

    addPath(&hl->paths, path);
    hl->path_count++;
}

void insertSym(const char *symlink_path, ino_t target_ino) {
    FileGroup *fg, *tmp_fg;
    struct stat link_stat;

    if (lstat(symlink_path, &link_stat) != 0) {
        perror("lstat failed on symlink");
        return;
    }

    ino_t symlink_ino = link_stat.st_ino;

    for (fg = file_groups; fg != NULL; fg = tmp_fg) {
        tmp_fg = fg->hh.next;

        HardLink *hl, *tmp_hl;
        for (hl = fg->hard_links; hl != NULL; hl = tmp_hl) {
            tmp_hl = hl->hh.next;

            if (hl->inode == target_ino) {
                SoftLink *sl = NULL;
                HASH_FIND(hh, hl->soft_links, &symlink_ino, sizeof(ino_t), sl);

                if (!sl) {
                    sl = malloc(sizeof(SoftLink));
                    if (!sl) {
                        perror("malloc failed for SoftLink");
                        return;
                    }
                    sl->inode = symlink_ino;
                    sl->path_count = 0;
                    sl->paths = NULL;
                    HASH_ADD(hh, hl->soft_links, inode, sizeof(ino_t), sl);
                }

                addPath(&sl->paths, symlink_path);
                sl->path_count++;
                return;
            }
        }
    }
}

void print_output() {
    int file_number = 1;
    FileGroup *fg, *tmp_fg;
    for (fg = file_groups; fg != NULL; fg = tmp_fg) {
        tmp_fg = fg->hh.next;

        printf("File %d:\n", file_number++);
        printf("\tMD5 Hash: %s\n", fg->md5);

        HardLink *hl, *tmp_hl;
        for (hl = fg->hard_links; hl != NULL; hl = tmp_hl) {
            tmp_hl = hl->hh.next;

            printf("\t\tHard Link (%lu): %lu\n", hl->path_count, hl->inode);
            printf("\t\t\tPaths:\n");
            for (PathNode *p = hl->paths; p != NULL; p = p->next)
                printf("\t\t\t\t%s\n", p->path);

            int softlink_number = 1;
            SoftLink *sl, *tmp_sl;
            for (sl = hl->soft_links; sl != NULL; sl = tmp_sl) {
                tmp_sl = sl->hh.next;

                printf("\t\t\tSoft Link %d(%lu): %lu\n", softlink_number++, sl->path_count, sl->inode);
                printf("\t\t\t\tPaths:\n");
                for (PathNode *p = sl->paths; p != NULL; p = p->next)
                    printf("\t\t\t\t\t%s\n", p->path);
            }
        }
    }
}

void freePath(PathNode *head) {
    while (head) {
        PathNode *temp = head;
        head = head->next;
        free(temp);
    }
}

void cleanup() {
    FileGroup *fg, *tmp_fg;
    HASH_ITER(hh, file_groups, fg, tmp_fg) {
        HASH_DEL(file_groups, fg);
        HardLink *hl, *tmp_hl;
        HASH_ITER(hh, fg->hard_links, hl, tmp_hl) {
            HASH_DEL(fg->hard_links, hl);
            freePath(hl->paths);
            SoftLink *sl, *tmp_sl;
            HASH_ITER(hh, hl->soft_links, sl, tmp_sl) {
                HASH_DEL(hl->soft_links, sl);
                freePath(sl->paths);
                free(sl);
            }
            free(hl);
        }
        free(fg);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./detect_dups <directory>\n");
        exit(EXIT_FAILURE);
    }

    char *dir_path = argv[1];
    struct stat sb;

    if (stat(dir_path, &sb) == -1 || !S_ISDIR(sb.st_mode)) {
        printf("Error %d: %s is not a valid directory\n", errno, dir_path);
        exit(EXIT_FAILURE);
    }

    if (!realpath(dir_path, base_path)) {
        perror("realpath failed on base path");
        exit(EXIT_FAILURE);
    }

    OpenSSL_add_all_digests();
    mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) {
        printf("Failed to create MD5 context\n");
        exit(EXIT_FAILURE);
    }

    if (nftw(dir_path, render_file_info, 20, FTW_PHYS) == -1) {
        perror("nftw");
        EVP_MD_CTX_free(mdctx);
        return -1;
    }

    print_output();
    cleanup();
    EVP_MD_CTX_free(mdctx);

    return 0;
}