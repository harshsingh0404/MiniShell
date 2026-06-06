#include "shell.h"
/*builtin commands*/
	char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
						"set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
						"exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", NULL};


  //fetch the first word -->ls -l so, fetch first word ls;
  //return ls

int status;
char *get_command(char *input_string)
{
    static char command[100];
    int i = 0;

    /* skip spaces */
    while (*input_string == ' ')
    {
        input_string++;
    }

    /* copy first word */
    while (*input_string != ' ' &&
           *input_string != '\0' &&
           *input_string != '\n')
    {
        command[i++] = *input_string;
        input_string++;
    }

    command[i] = '\0';

    return command;
}

void extract_external_commands(char **external_commands)
{
    FILE *fp;
    char buffer[100];
    int i = 0;

    /* open file */
    fp = fopen("external_commands.txt", "r");

    if(fp == NULL)
    {
        printf("File not found\n");
        return;
    }

    /* read line by line */
    while(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        /* remove newline character */
        buffer[strcspn(buffer, "\n")] = '\0';

        /* allocate memory */
        external_commands[i] = malloc(strlen(buffer) + 1);

        if(external_commands[i] == NULL)
        {
            printf("Memory allocation failed\n");
            return;
        }

        /* copy command */
        strcpy(external_commands[i], buffer);

        i++;
    }

    /* last element NULL */
    external_commands[i] = NULL;

    fclose(fp);
}


int check_command_type(char *command, char **external_commands)
{
    int i;

    /* check builtin commands */
    for(i = 0; builtins[i] != NULL; i++)
    {
        if(strcmp(command, builtins[i]) == 0)
        {
            return BUILTIN;
        }
    }

    /* check external commands */
    for(i = 0; external_commands[i] != NULL; i++)
    {
        if(strcmp(command, external_commands[i]) == 0)
        {
            return EXTERNAL;
        }
    }
    return NO_COMMAND;
}

void execute_internal_commands(char *input_string)
{
    char buff[1024];

    /* exit */
    if (strcmp(input_string, "exit") == 0)
    {
        exit(0);
    }

    /* cd */
    else if (strcmp(input_string, "cd") == 0)
    {
        chdir(getenv("HOME"));
    }
    else if (strncmp(input_string, "cd ", 3) == 0)
    {
        if (chdir(input_string + 3) == -1)
        {
            perror("cd");
        }
    }

    /* pwd */
    else if (strcmp(input_string, "pwd") == 0)
    {
        getcwd(buff, sizeof(buff));
        printf("%s\n", buff);
    }

    /* echo $$ */
    else if (strcmp(input_string, "echo $$") == 0)
    {
        printf("%d\n", getpid());
    }

    /* echo $? */
    else if (strcmp(input_string, "echo $?") == 0)
    {
        if (WIFEXITED(status))
        {
            printf("%d\n", WEXITSTATUS(status));
        }
        else if (WIFSTOPPED(status))
        {
            printf("%d\n", WSTOPSIG(status));
        }
    }

    /* echo $SHELL */
    else if (strcmp(input_string, "echo $SHELL") == 0)
    {
        char *shell = getenv("SHELL");

        if (shell)
            printf("%s\n", shell);
        else
            printf("SHELL variable not set\n");
    }

    /* normal echo */
    else if (strncmp(input_string, "echo ", 5) == 0)
    {
        printf("%s\n", input_string + 5);
    }
}

void execute_external_commands(char *input_string)
{
    /* Check if pipe is present */
    if (strchr(input_string, '|') == NULL)
    {
        char *argv[20];
        int i = 0;

        /* Convert command string to argv[] */
        argv[i] = strtok(input_string, " ");

        while (argv[i] != NULL)
        {
            i++;
            argv[i] = strtok(NULL, " ");
        }

        /* Execute external command */
        execvp(argv[0], argv);

        perror("execvp");
        exit(1);
    }
    else
    {
        char *cmds[20];
        int cmd_count = 0;

        /* Split input string using pipe delimiter */
        cmds[cmd_count] = strtok(input_string, "|");

        while (cmds[cmd_count] != NULL)
        {
            cmd_count++;
            cmds[cmd_count] = strtok(NULL, "|");
        }

        /* Calculate number of pipes */
        int pipe_count = cmd_count - 1;

        char *commands[20][20];

        /* Convert each command into argv format */
        for (int i = 0; i < cmd_count; i++)
        {
            int j = 0;

            commands[i][j] = strtok(cmds[i], " ");

            while (commands[i][j] != NULL)
            {
                j++;
                commands[i][j] = strtok(NULL, " ");
            }
        }

        int fd[20][2];

        /* Create required pipes */
        for (int i = 0; i < pipe_count; i++)
        {
            pipe(fd[i]);
        }

        /* Create one process per command */
        for (int i = 0; i < cmd_count; i++)
        {
            pid_t pid = fork();

            if (pid == 0)
            {
                /* Connect previous pipe to stdin */
                if (i > 0)
                    dup2(fd[i - 1][0], STDIN_FILENO);

                /* Connect stdout to next pipe */
                if (i < pipe_count)
                    dup2(fd[i][1], STDOUT_FILENO);

                /* Close all pipe descriptors */
                for (int j = 0; j < pipe_count; j++)
                {
                    close(fd[j][0]);
                    close(fd[j][1]);
                }

                /* Execute command */
                execvp(commands[i][0], commands[i]);

                perror("execvp");
                exit(1);
            }
        }

        /* Parent closes all pipes */
        for (int i = 0; i < pipe_count; i++)
        {
            close(fd[i][0]);
            close(fd[i][1]);
        }

        /* Wait for all child processes */
        for (int i = 0; i < cmd_count; i++)
        {
            wait(NULL);
        }
    }
}
