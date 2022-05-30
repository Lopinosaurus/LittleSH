#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>

// All already-built function are featured in GNU C Library, available in the Git repository.

// Max char accepted
#define MAXCOM 1000
// Max cmds possibly entered
#define MAXLIST 100
#define clear() printf("\033[H\033[J")

// Kind of neofetch when launching the shell
void shell_fetch()
{
    clear();
    printf("\n\n\n\n=================""========================");
    printf("\n\n\n\t----WELCOME TO LITTLESHELL----");
    printf("\n\n\t-WORK IN PROGRESS, VERSION 1.0-");
    printf("\n\n\n\n===================""=======================");
    char* username = getenv("USER");
    printf("\n\n\nWelcome, @%s", username);
    printf("\n");
    sleep(1);
    clear();
}


// Display help menu
void help_menu()
{
    printf("\nDisplaying help menu: "
           "\nLittle Shell 1.0:"
           "\ncd : jump into directory"
           "\nls : display files and directories"
           "\nexit : leave the shell"
           "\n>pipe handling");

    return;
}


// Read input entered by user
int input_dump(char* str)
{
    char* arg;

    arg = readline("\n>>> ");
    if (strlen(arg) != 0)
    {
        // both are featured in history.h
        add_history(arg);
        strcpy(str, arg);
        return 0;
    }
    else
    {
        return 1;
    }
}

// cwd: Current dir
void get_cwd()
{
    char cwd[1024];
    // featured in unistd.h
    getcwd(cwd, sizeof(cwd));
    // featured in stdio.h
    printf(cwd);
}

// Function where the system command is executed
void exec_cmd(char** parsed)
{
    // getting pid, type in types.h
    pid_t pid = fork();

    if (pid == -1)
    {
        printf("\nError: child forking impossible... (PID is equal to -1)");
        return;
    }
    else if (pid == 0)
    {
        if (execvp(parsed[0], parsed) < 0)
        {
            printf("\nError : impossible command (PID equal to 0)");
        }
        exit(0);
    }
    else
    {
        // featured in wait.h
        wait(NULL);
        return;
    }
}

void exec_piped_arg(char** parsed, char** parsed_pipe)
{
    int pipefd[2];
    pid_t pid1, pid2;

    if (pipe(pipefd) < 0)
    {
        printf("\nError : cannot init pipe");
        return;
    }
    pid1 = fork();
    if (pid1 < 0)
    {
        printf("\nError : could not fork child");
        return;
    }

    if (0 == pid1)
    {
        // fun featured in unistd.h
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        if (execvp(parsed[0], parsed) < 0)
        {
            printf("\nError : invalid command");
            exit(0);
        }
    }
    else
    {
        pid2 = fork();

        if (pid2 < 0)
        {
            printf("\nError : could not fork child");
            return;
        }

        if (0 == pid2)
        {
            // featured in unistd.h
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            if (execvp(parsed_pipe[0], parsed_pipe) < 0)
            {
                printf("\nError : invalid command");
                exit(0);
            }
        }
        else
        {
            wait(NULL);
            wait(NULL);
        }
    }
}

// execute commands
int cmd_executer(char** parsed_line)
{
    int not_cmd = 4, i, sw_arg = 0;
    char* cmd_list[not_cmd];
    cmd_list[0] = "exit";
    cmd_list[1] = "cd";
    cmd_list[2] = "help";

    for (i = 0; i < not_cmd; i++)
    {
        if (strcmp(parsed_line[0], cmd_list[i]) == 0)
        {
            sw_arg = i + 1;
            break;
        }
    }

    switch (sw_arg)
    {
    case 1:
        printf("\nQuiting LittleSH...\n");
        exit(0);
    case 2:
        // featured in unistd.h, yeah I kinda cheat :)
        chdir(parsed_line[1]);
        return 1;
    case 3:
        help_menu();
        return 1;
    default:
        printf("\nError: command not found...\n");
        break;
    }

    return 0;
}

// pipe parser
int pipe_parser(char* str, char** piped_string)
{
    int i;
    for (i = 0; i < 2; i++)
    {
        // featured in string.h
        piped_string[i] = strsep(&str, "|");
        if (piped_string[i] == NULL)
            break;
    }

    if (piped_string[1] == NULL)
        return 0;
    else
    {
        return 1;
    }
}

// command parser, based on space char
void global_parser(char* str, char** parsed)
{
    int i;

    for (i = 0; i < MAXLIST; i++) {
        // featured in string.h
        parsed[i] = strsep(&str, " ");

        if (parsed[i] == NULL)
            break;
        // featured in string.h
        if (strlen(parsed[i]) == 0)
            i--;
    }
}

int processString(char* str, char** parsed, char** parsed_pipe)
{

    char* piped_str[2];
    int piped = 0;

    piped = pipe_parser(str, piped_str);

    if (piped)
    {
        global_parser(piped_str[0], parsed);
        global_parser(piped_str[1], parsed_pipe);

    }

    else
    {
        global_parser(str, parsed);
    }

    if (cmd_executer(parsed))
        return 0;
    else
        return 1 + piped;
}

int main()
{
    char typed_input[MAXCOM], *parsed_cmd[MAXLIST];
    char* piped_and_parsed[MAXLIST];
    int proc = 0;
    // initiating shell
    shell_fetch();

    // Yes, it's ugly but you do not have choice
    while (1)
    {
        get_cwd();
        if (input_dump(typed_input))
            continue;
        proc = processString(typed_input,
        parsed_cmd, piped_and_parsed);
        
        if (1 == proc)
            exec_cmd(parsed_cmd);

        if (2 == proc)
            exec_piped_arg(parsed_cmd, piped_and_parsed);
    }
    return 0;
}