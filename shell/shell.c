//
// Created by srdczk on 20-3-13.
//
#include "../include/shell.h"
#include "../include/stdio.h"
#include "../include/syscall.h"
#include "../include/string.h"
#include "../include/fs.h"
#include "../include/clock.h"

// 最大支持输入 64 大小的命令
#define CMD_LEN 0x40
// 最多支持 15 个参数
#define MAX_ARG_NR 0x10

static char res_path[MAX_PATH_LEN] = {0};

static char cmd_line[CMD_LEN] = {0};
// 记录当前命令
char cwd_cache[64] = {0};

void print_header() {
    printf("[srdczk@localhost %s]$ ", cwd_cache);
}

static void readline(char *buf, u32 cnt) {
    char *pos = buf;
    while (read(0, pos, 1) != -1 && (pos - buf) < cnt) {
        switch (*pos) {
            case 'l' - 'a':
                *pos = 0;
                clear();
                print_header();
                // 之前输入的保留
                printf("%s", buf);
                break;
            case 'u' - 'a':
                while (pos != buf) {
                    if (pos != buf + 1) putchar('\b');
                    *(pos--) = '\0';
                }
                break;
            case '\n':
            case '\r':
                putchar('\n');
                return;
            case '\b':
                if (buf[0] != '\b') {
                    pos--;
                    putchar('\b');
                }
                break;
            default:
                putchar(*pos);
                pos++;
                break;
        }
    }
    printf("Not support!\n");
}

static int parse_cmd(char *str, char **av, char token) {
    int index = 0;
    while (index < MAX_ARG_NR) {
        av[index++] = NULL;
    }
    char *next = str;
    int ac = 0;
    while (*next) {
        while (*next == token) next++;
        if (!*next) break;
        av[ac] = next;
        while (*next && *next != token) next++;
        if (*next == token) *next++ = '\0';
        if (ac >= MAX_ARG_NR) break;
        ac++;
    }
    return ac;
}

char *av[MAX_ARG_NR];
// 转换路径中的 . 和 ..
static void swish_path(char *old_path, char *new_path) {

    char name[MAX_FILE_NAME_LEN] = {0};
    char *sub_path = old_path;
    sub_path = path_parse(sub_path, name);
    if (name[0] == 0) {
        new_path[0] = '/';
        new_path[1] = '\0';
        return;
    }
    new_path[0] = '\0';
    strcat("/", new_path);
    while (name[0]) {
        if (!strcmp("..", name)) {
            char *slash_ptr = strrchr(new_path, '/');
            if (slash_ptr != new_path) {
                *slash_ptr = '\0';
            } else {
                *(slash_ptr + 1) = '\0';
            }
        } else if (strcmp(".", name)) {
            if (strcmp(new_path, "/")) {
                strcat("/", new_path);
            }
            strcat(name, new_path);
        }
        memset(name, 0, MAX_FILE_NAME_LEN);
        if (sub_path) {
            sub_path = path_parse(sub_path, name);
        }
    }
}

void make_abs_path(char *path, char *res) {
    char abs_path[MAX_PATH_LEN] = {0};
    if (path[0] != '/') {
        memset(abs_path, '\0', MAX_PATH_LEN);
        if (getcwd(abs_path, MAX_PATH_LEN)) {
            if (!((abs_path[0] == '/') && (abs_path[1] == '\0'))) {
                strcat("/", abs_path);
            }
        }
    }
    strcat(path, abs_path);
    swish_path(abs_path, res);
}
// 基础命令
void pwd(int ac, char **av) {
    if (ac != 1) {
        printf("pwd: no argument support!\n");
        return;
    } else {
        if (getcwd(res_path, MAX_PATH_LEN)) {
            printf("%s\n", res_path);
        } else {
            printf("pwd: get current directory fail!\n");
        }
    }
}

char *cd(int ac, char **av) {
    if (ac > 2) {
        printf("cd: only support 1 argument!\n");
        return NULL;
    }
    if (ac == 1) {
        res_path[0] = '/';
        res_path[1] = '\0';
    } else make_abs_path(av[1], res_path);
    if (chdir(res_path) == -1) {
        printf("cd: no such directory!\n");
        return NULL;
    }
    return res_path;
}

void ls(int ac, char **av) {
    char* pathname = NULL;
    stat f_stat;
    memset((char *) &f_stat, '\0', sizeof(stat));
    bool long_info = 0;
    u32 arg_path_nr = 0;
    u32 arg_index = 1;
    while (arg_index < ac) {
        if (av[arg_index][0] == '-') {
            if (!strcmp("-l", av[arg_index])) {
                long_info = 1;
            } else if (!strcmp("-h", av[arg_index])) {
                printf("usage: -l list all infomation about the file.\n-h for help\nlist all files in the current dirctory if no option\n");
                return;
            } else {
                printf("ls: invalid option %s\nTry `ls -h' for more information.\n", av[arg_index]);
                return;
            }
        } else {	     // ls的路径参数
            if (arg_path_nr == 0) {
                pathname = av[arg_index];
                arg_path_nr = 1;
            } else {
                printf("ls: only support one path\n");
                return;
            }
        }
        arg_index++;
    }

    if (pathname == NULL) {	 // 若只输入了ls 或 ls -l,没有输入操作路径,默认以当前路径的绝对路径为参数.
        if (NULL != getcwd(res_path, MAX_PATH_LEN)) {
            pathname = res_path;
        } else {
            printf("ls: getcwd for default path failed\n");
            return;
        }
    } else {
        make_abs_path(pathname, res_path);
        pathname = res_path;
    }

    if (file_stat(pathname, &f_stat) == -1) {
        printf("ls: cannot access %s: No such file or directory\n", pathname);
        return;
    }
    if (f_stat.st_type == DIRECTORY) {
        dir *d = opendir(pathname);
        dir_entry *dir_e = NULL;
        char sub_pathname[MAX_PATH_LEN] = {0};
        u32 pathname_len = strlen(pathname);
        u32 last_char_index = pathname_len - 1;
        memcpy(pathname, sub_pathname, pathname_len);
        if (sub_pathname[last_char_index] != '/') {
            sub_pathname[pathname_len] = '/';
            pathname_len++;
        }
        rewinddir(d);
        if (long_info) {
            char ftype;
            printf("total: %d\n", f_stat.st_size);
            while((dir_e = readdir(d))) {
                ftype = 'd';
                if (dir_e->type == REGULAR) {
                    ftype = '-';
                }
                sub_pathname[pathname_len] = 0;
                strcat(sub_pathname, dir_e->filename);
                memset(&f_stat, '\0', sizeof(stat));
                if (file_stat(sub_pathname, &f_stat) == -1) {
                    printf("ls: cannot access %s: No such file or directory\n", dir_e->filename);
                    return;
                }
                printf("%c  %d  %d  %s\n", ftype, dir_e->i_no, f_stat.st_size, dir_e->filename);
            }
        } else {
            while((dir_e = readdir(d))) {
                printf("%s ", dir_e->filename);
            }
            printf("\n");
        }
        closedir(d);
    } else {
        if (long_info) {
            printf("-  %d  %d  %s\n", f_stat.st_ino, f_stat.st_size, pathname);
        } else {
            printf("%s\n", pathname);
        }
    }
}

void clear_screen(int ac, char **av) {
    if (ac != 1) {
        printf("clear: not support argument!\n");
        return;
    }
    clear();
}

int shell_mkdir(int ac, char **av) {
    int res = -1;
    if (ac != 2) {
        printf("mkdir: must have 1 argument!\n");
        return res;
    } else {
        make_abs_path(av[1], res_path);
        if (strcmp("/", res_path) && mkdir(res_path) != -1) {
            res = 0;
        } else {
            printf("mkdir: %s fail\n", av[1]);
        }
        return res;
    }
}

int shell_rmdir(int ac, char **av) {
    int res = -1;
    if (ac != 2) {
        printf("rmdir: must have 1 argument!\n");
        return res;
    } else {
        make_abs_path(av[1], res_path);
        if (strcmp("/", res_path) && rmdir(res_path) != -1) {
            res = 0;
        } else printf("rmdir %s fail!\n", av[1]);
    }
    return res;
}

int rm(int ac, char **av) {
    int res = -1;
    if (ac != 2) {
        printf("rm: must have 1 argument!\n");
        return res;
    } else {
        make_abs_path(av[1], res_path);
        if (strcmp("/", res_path) && unlink(res_path) != -1) {
            res = 0;
        } else printf("rm %s fail!\n", av[1]);
    }
    return res;
}


void shell() {
    cwd_cache[0] = '/';
    while (1) {
        print_header();
        memset(cmd_line, '\0', 64);
        memset(res_path, '\0', MAX_PATH_LEN);
        readline(cmd_line, CMD_LEN);
        int ac = parse_cmd(cmd_line, av, ' ');
        if (ac > 0) {
            int len = strlen(av[ac - 1]);
            av[ac - 1][len - 1] = '\0';
        }
        if (!strcmp("ls", av[0])) {
            ls(ac, av);
        } else if (!strcmp("cd", av[0])) {
            if (cd(ac, av)) {
                memset(cwd_cache, '\0', 64);
                strcpy(res_path, cwd_cache);
            }
        } else if (!strcmp("pwd", av[0])) {
            pwd(ac, av);
        } else if (!strcmp("clear", av[0])) {
            clear_screen(ac, av);
        } else if (!strcmp("mkdir", av[0])) {
            shell_mkdir(ac, av);
        } else if (!strcmp("rmdir", av[0])) {
            shell_rmdir(ac, av);
        } else if (!strcmp("rm", av[0])) {
            rm(ac, av);
        } else {
            int pid = fork();
            if (!pid) {
                make_abs_path(av[0], res_path);
                av[0] = res_path;
                stat f_stat;
                memset(&f_stat, '\0', sizeof(stat));
                if (file_stat(av[0], &f_stat) == -1) {
                    printf("program Not exist!\n");
                } else {
                    exec(av[0], av);
                }
                while (1);
            } else {
                // 父进程一般先被调用, 让其延迟
                int delay = 200000000;
                while (delay--);
            }
        }
    }
}