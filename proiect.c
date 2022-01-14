#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

void ls_function(const char *dir, int op_l)
{
    struct dirent *d;
    DIR *dh = opendir(dir);
    if (!dh)
    {
        if (errno == ENOENT)
            perror("Error");

        else
            perror("Unable to read directory.");
        exit(EXIT_FAILURE);
    }

    while ((d = readdir(dh)) != NULL)
    {
        if (d->d_name[0] == '.')
            continue;
        printf("%s ", d->d_name);

        if (op_l)
            printf("\n");
    }
    if (!op_l)
        printf("\n");
}

void rm_function(const char *path)
{
    size_t path_len;
    char *full_path;
    DIR *dir;
    struct stat stat_path, stat_d;
    struct dirent *d;

    stat(path, &stat_path);

    if (S_ISDIR(stat_path.st_mode) == 0)
    {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    if ((dir = opendir(path)) == NULL)
    {
        perror("Cannot delete");
        exit(EXIT_FAILURE);
    }

    path_len = strlen(path);

    while ((d = readdir(dir)) != NULL)
    {

        if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
            continue;

        full_path = calloc(path_len + strlen(d->d_name) + 1, sizeof(char));
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, d->d_name);

        stat(full_path, &stat_d);

        if (S_ISDIR(stat_d.st_mode) != 0)
        {
            rm_function(full_path);
            continue;
        }

        if (unlink(full_path) == 0)
            printf("Removed a file: %s\n", full_path);
        else
            printf("Cannot remove a file: %s\n", full_path);
        free(full_path);
    }

    if (rmdir(path) == 0)
        printf("Removed a directory: %s\n", path);
    else
        printf("Can`t remove a directory: %s\n", path);

    closedir(dir);
}
int lsh_echo(char **args)
{
    int i = 1;
    while (args[i] != NULL)
    {
        printf("%s", args[i]);
        i++;
    }
    return 0;
}
int lsh_cp(char **args)
{
    if (args[1] == NULL || args[2] == NULL)
    {
        printf("Not enough arguments.");
        return 1;
    }
    char *src = args[1];
    char *dest = args[2];

    int auxSrc = open(src, O_RDONLY);
    if (auxSrc < 0)
    {
        perror("Error");
        return 1;
    }
    int auxDest = open(dest, O_CREAT | O_WRONLY, S_IRWXU);
    if (auxDest < 0)
    {
        perror("Error");
        return 1;
    }
    ssize_t reader = -1;
    char buffer[1024];
    while (reader = read(auxSrc, buffer, 1024))
    {
        int content, written = 0;
        while (content = write(auxDest, buffer + written, reader - written))
            written += content;
        if (written < 0)
        {
            perror("Error");
            return 1;
        }
    }
    if (reader < 0)
    {
        perror("Error");
        return 1;
    }
    close(auxSrc);
    close(auxDest);
    return 0;
};

int lsh_ls(char **args)
{
    if (args[1] == NULL)
    {
        ls_function(".", 0);
    }
    else if (args[1] != NULL && args[2] == NULL)
    {
        if (args[1][0] == '-')
        {

            int op_l = 0;
            char *p = args[1] + 1;
            if (*p == 'l')
                op_l = 1;
            else
            {
                perror("Unknown option. Available options: -l");
                return 1;
            }

            ls_function(".", op_l);
        }
        else
            ls_function(args[1], 0);
    }
    else if (args[1] != NULL && args[2] != NULL && args[3] == NULL)
    {
        if (args[1][0] != '-')
        {
            perror("Incorrect usage error.");
            return 1;
        }
        int op_l = 0;
        char *p = args[1] + 1;
        if (*p == 'l')
            op_l = 1;
        else
        {
            perror("Unknown option error.");
            return 1;
        }

        ls_function(args[2], op_l);
    }
    return 0;
}
int lsh_rm(char **args)
{

    if (args[1] == NULL)
    {
        fprintf(stderr, "Missing path");
        return 1;
    }
    struct stat stat_path;

    stat(args[1], &stat_path);
    if (S_ISREG(stat_path.st_mode))
    {
        if (unlink(args[1]) == 0)
        {
            printf("Removed a file: %s\n", args[1]);
        }
        else
        {
            perror("Cannot remove the file.");
            return 1;
        }
    }
    else
        rm_function(args[1]);

    return 0;
}
int lsh_mkdir(char **args)
{
    if (args[1] == NULL)
    {
        perror("Not enough arguments.");
        return 1;
    }
    else
    {
        int result = mkdir(args[1], S_IRWXU);
        if (result == -1)
        {
            perror("Cannot create directory.");
            return 1;
        }
    }
    return 0;
}
int lsh_help()
{
    printf("Available commands:\nls (-l)\ncp\nrm\nmkdir\nversion\nexit\n");
    return 0;
}
int lsh_version()
{
    printf("v1.0 Proiect Sisteme de Operare\nBoghiu Alexandra-Adriana, grupa 234\n");
    return 0;
}
int lsh_exit()
{
    return 1;
}

char *command_name[] = {
    "echo",
    "cp",
    "ls",
    "rm",
    "mkdir",
    "help",
    "version",
    "exit"};

int (*commands[])(char **) = {
    &lsh_echo,
    &lsh_cp,
    &lsh_ls,
    &lsh_rm,
    &lsh_mkdir,
    &lsh_help,
    &lsh_version,
    &lsh_exit};

int lsh_num_builtins()
{
    return sizeof(command_name) / sizeof(char *);
}
#define LSH_TOK_BUFSIZE 1024
#define LSH_TOK_DELIM " \t\r\n\a"
int int_mode = 1;

int lsh_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (!pid)
    {
        if (execvp(args[0], args) == -1)
            perror("Launch error");
        return 1;
    }
    else if (pid < 0) //daca fork da eroare
        perror("Launch error");

    else
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED); 

        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    return 0;
}

int lsh_execute(char **args)
{
    if (args[0] == NULL)
        return 1; 

    for (int i = 0; i < lsh_num_builtins(); i++)
    {
        if (!strcmp(args[0], command_name[i])) 
        {
            return (*commands[i])(args);
        }
    }
    return lsh_launch(args); 
}
char **lsh_split_line(char *line)
{
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens) 
    {

        perror("Allocation error");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM); 

    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize) 
        {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));

            if (!tokens)
            {
                fprintf(stderr, "Allocation error");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM);
    }

    tokens[position] = NULL; 

    return tokens;
}
char *lsh_read_line()
{
    char *line = NULL;
    ssize_t bufsize = 0;

    if (getline(&line, &bufsize, stdin) == -1)
    {
        if (feof(stdin))
            exit(EXIT_SUCCESS); 
        else
        {
            perror("Readline error");
            exit(EXIT_FAILURE); 
        }
    }

    return line;
}
void lsh_loop()
{
    char *line;
    char **args;
    int status;

    do
    {
        printf("dbxcli ");
        line = lsh_read_line();      
        args = lsh_split_line(line); 
        status = lsh_execute(args);  

        free(line);
        free(args);

    } while (!status);
}
int main(int argc, char **argv)
{

    lsh_loop();

    return 0;
}