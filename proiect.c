#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

int lsh_comanda1(char **args);
int lsh_comanda2(char **args);


char *command_name[] = {
    "comanda1",
    "comanda2"
};

int(*commands[]) (char **) = {
    &lsh_comanda1,
    &lsh_comanda2
};

int lsh_num_builtins() {
    return sizeof(command_name) / sizeof(char *);
}
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
int int_mode = 1;

int lsh_execute(char **args)
{
    if (args[0] == NULL)
    return 1; //comanda fara nimic de fapt

    for(int i = 0; i < lsh_num_builtins(); i++) 
    {
        if (!strcmp(args[0], command_name[i])) //imi cauta in comenzi si daca o gaseste, executa
        return (*commands[i])(args);
    }
    return lsh_launch(args); //daca nu gaseste o comanda, imi face procesul
}
int lsh_launch(char **args) //primeste argumentele facute anterior
{
    pid_t pid, wpid;
    int status;

    pid = fork(); //creez un proces nou
    if (!pid) //proces-copil
    {
        if (execvp(args[0], args) == -1) //execvp ruleaza noul program "peste" cel vechi (il  inlocuieste)
        //execvp se asteapta sa primeasca un nume de program si un array
        //v -> array de argumente unde primul e numele programului
        //p -> in loc sa dau full path, dau numai numele programului si las sistemul sa il caute
            perror("lsh"); //daca returneaza -1, eroare
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