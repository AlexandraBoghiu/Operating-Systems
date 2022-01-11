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
            perror("Directory does not exist.");

        else
            perror("Unable to read directory.");
        exit(EXIT_FAILURE);
    }

    while ((d = readdir(dh)) != NULL)
    {
        if (d->d_name[0] == '.') //ignoram hidden files
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

    // stat for the path
    stat(path, &stat_path); //stat() gets status information about "path" and places it in the area of memory pointed to by the buf argument.

    // if path does not exists or is not dir - exit with status -1
    if (S_ISDIR(stat_path.st_mode) == 0)
    {
        perror("Not a directory");
        exit(EXIT_FAILURE);
    }

    // nu se poate sterge
    if ((dir = opendir(path)) == NULL)
    {
        perror("Not possible.");
        exit(EXIT_FAILURE);
    }

    // lungimea path-ului
    path_len = strlen(path);

    //trec prin fisierele/folderele din folderul curent
    while ((d = readdir(dir)) != NULL)
    {

        if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, "..")) //sar peste . si .., nu vreau sa mai merg recursiv atunci
            continue;

        // urmeaza sa creez full path pentru fisierul/folderul curent
        full_path = calloc(path_len + strlen(d->d_name) + 1, sizeof(char));
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, d->d_name);

        // stat pt d ca sa verific daca e fisier sau folder ulterior
        stat(full_path, &stat_d);

        // sterg un folder recursiv
        if (S_ISDIR(stat_d.st_mode) != 0)
        {
            rm_function(full_path);
            continue;
        }

        if (unlink(full_path) == 0) //sterg fisier (folderele trebuie sa nu aiba fisiere pentru a le putea sterge)
            printf("Removed a file: %s\n", full_path);
        else
            printf("Cannot remove a file: %s\n", full_path);
        free(full_path);
    }

    // sterg folderul gol
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
        return 0;
    }
    char *src = args[1];
    char *dest = args[2];

    int auxSrc = open(src, O_RDONLY);
    if (auxSrc < 0)
    {
        printf("Error: %d", errno);
        return 0;
    }
    int auxDest = open(dest, O_CREAT | O_WRONLY, S_IRWXU);
    if (auxDest < 0)
    {
        printf("Error: %d", errno);
        return 0;
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
            printf("Error: %d", errno);
            return 0;
        }
    }
    if (reader < 0)
    {
        printf("Error: %d", errno);
        return 0;
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
        { //daca avem vreo optiune (l)

            int op_l = 0;
            char *p = args[1] + 1;
            if (*p == 'l')
                op_l = 1;
            else
            {
                perror("Unknown option error.");
                exit(EXIT_FAILURE);
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
            perror("Incorrect Usage error.");
            exit(EXIT_FAILURE);
        }
        int op_l = 0;
        char *p = args[1] + 1;
        if (*p == 'l')
            op_l = 1;
        else
        {
            perror("Unknown option error.");
            exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
    }
    struct stat stat_path;
    // stat for the path
    stat(args[1], &stat_path);      //stat() gets status information about "path" and places it in the area of memory pointed to by the buf argument.
    if (S_ISREG(stat_path.st_mode)) //daca sterg un file
    {
        if (unlink(args[1]) == 0)
        {
            printf("Removed a file: %s\n", args[1]);
        }
        else
        {
            perror("Cannot remove the file.");
            exit(EXIT_FAILURE);
        }
    }
    else
        rm_function(args[1]);
}
int lsh_mkdir(char **args)
{
    if (args[1] == NULL)
    {
        perror("Not enough arguments.");
        exit(EXIT_FAILURE);
    }
    else
    {
        int result = mkdir(args[1], S_IRWXU);
        if (result == -1)
        {
            perror("Cannot create directory.");
            exit(EXIT_FAILURE);
        }
    }
}
int lsh_version()
{
    printf("Proiect Sisteme de Operare\nBoghiu Alexandra-Adriana, grupa 234\n");
    return 0;
}

char *command_name[] = {
    "echo",
    "cp",
    "ls",
    "rm",
    "mkdir",
    "version"};

int (*commands[])(char **) = {
    &lsh_echo,
    &lsh_cp,
    &lsh_ls,
    &lsh_rm,
    &lsh_mkdir,
    &lsh_version};

int lsh_num_builtins()
{
    return sizeof(command_name) / sizeof(char *);
}
#define LSH_TOK_BUFSIZE 1024
#define LSH_TOK_DELIM " \t\r\n\a"
int int_mode = 1;

int lsh_launch(char **args) //primeste argumentele facute anterior
{
    pid_t pid, wpid;
    int status;

    pid = fork(); //creez un proces nou
    if (!pid)     //proces-copil
    {
        if (execvp(args[0], args) == -1) //execvp ruleaza noul program "peste" cel vechi (il  inlocuieste)
                                         //execvp se asteapta sa primeasca un nume de program si un array
                                         //v -> array de argumente unde primul e numele programului
                                         //p -> in loc sa dau full path, dau numai numele programului si las sistemul sa il caute
            perror("lsh");               //daca returneaza -1, eroare
        exit(EXIT_FAILURE);
    }
    else if (pid < 0) //daca fork da eroare
        perror("lsh");

    else
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED); //parintele asteapta dupa copil
            //wuntraced -> pastreaza statusul procesului-copil care s-a oprit
        } while (!WIFEXITED(status) && !WIFSIGNALED(status)); //WIFEXITED(status) verifica daca procesul-copil s-a terminat cu succes cu exit
    //WIFSIGNALED(status) verifica daca procesul-copil s-a terminat cu succes (nu a primit semnale "nerezolvate")
    return 1;
}

int lsh_execute(char **args)
{
    if (args[0] == NULL)
        return 1; //comanda fara nimic de fapt

    for (int i = 0; i < lsh_num_builtins(); i++)
    {
        if (!strcmp(args[0], command_name[i])) //imi cauta in comenzi si daca o gaseste, executa
        {
            if (args[0] == "version")
                lsh_version();
            else
                return (*commands[i])(args);
        }
    }
    return lsh_launch(args); //daca nu gaseste o comanda, imi face procesul
}
char **lsh_split_line(char *line)
{
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens) //nu s-a alocat memorie corect
    {

        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM); //incep sa separ textul

    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize) //daca am nevoie de mai multa memorie, o realoc
        {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));

            if (!tokens)
            {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM);
    }

    tokens[position] = NULL; //am terminat

    return tokens;
}
char *lsh_read_line()
{
    char *line = NULL;
    ssize_t bufsize = 0;

    if (getline(&line, &bufsize, stdin) == -1)
    {
        if (feof(stdin))
            exit(EXIT_SUCCESS); //am ajuns la sfarsitul fisierului (EOF)
        else
        {
            perror("readline error");
            exit(EXIT_FAILURE); //eroare
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
        line = lsh_read_line();      //citesc comanda
        args = lsh_split_line(line); //separ comanda in program si argumente
        status = lsh_execute(args);  //execut

        free(line);
        free(args);

    } while (status);
}
int main(int argc, char **argv)
{

    lsh_loop();

    return EXIT_SUCCESS;
}