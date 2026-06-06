#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "shell.h"

extern pid_t pid;        /* Child process id from main.c */
extern char prompt[50]; /* Prompt from main.c */

/* Handle Ctrl+C and Ctrl+Z */
void signal_handler(int signum)
{
    if (signum == SIGINT)
    {
        /* Print prompt on Ctrl+C */
        if (pid == 0)
        {
            printf("\n%s", prompt);
            fflush(stdout);
        }
    }
    else if (signum == SIGTSTP)
    {
        /* Print prompt on Ctrl+Z */
        if (pid == 0)
        {
            printf("\n%s", prompt);
            fflush(stdout);
        }
    }
}

void scan_input(char *prompt, char *input_str)
{
    char *external_commands[152];

    /* Load external commands */
    extract_external_commands(external_commands);

    /* Register signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTSTP, signal_handler);

    while (1)
    {
        /* Display prompt */
	    char cwd[1024];
	    char hostname[100];
	    char *username;
	    char *home;
	    
	    username = getenv("USER");
	    home = getenv("HOME");
	    
	    gethostname(hostname, sizeof(hostname));
	    getcwd(cwd, sizeof(cwd));
	    
	    if (strncmp(cwd, home, strlen(home)) == 0)
	    {
		    printf(ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET "@" ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET ":" ANSI_COLOR_BLUE "~%s"
				    ANSI_COLOR_RESET "$ ", username, hostname, cwd + strlen(home));
	    }
	    else
	    {
		    printf(ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET "@" ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET ":" ANSI_COLOR_BLUE "%s" ANSI_COLOR_RESET "$ ", username, hostname, cwd);
	    }
	    
	    fflush(stdout);
        /* Read user input */
        scanf(" %[^\n]", input_str);

        /* Change prompt */
        if (strncmp(input_str, "PS1=", 4) == 0)
        {
            if (input_str[4] != ' ')
            {
                strcpy(prompt, input_str + 4);
            }
            else
            {
                printf("ERROR : Invalid PS1 command\n");
            }
        }
        else
        {
            /* Extract command */
            char *cmd = get_command(input_str);

            /* Check command type */
            int ret = check_command_type(cmd, external_commands);

            /* Execute built-in command */
            if (ret == BUILTIN)
            {
                execute_internal_commands(input_str);
            }
            /* Execute external command */
            else if (ret == EXTERNAL)
            {
		    /* Create child process */
		    pid = fork();
		    if (pid > 0)
		    {
			    
			    waitpid(pid, &status, WUNTRACED);
			    
			    if (WIFSTOPPED(status))
			    {
				    printf("\n[%d]+ Stopped\t%s\n", pid, input_str);
			    }
			    pid = 0;
		    }
		    else if (pid == 0)
		    {
			    /* Child gets default signal actions */
			    signal(SIGINT, SIG_DFL);
			    signal(SIGTSTP, SIG_DFL);

			    execute_external_commands(input_str);
			    exit(0);
		    }
		    else
		    {
			    perror("fork");
		    }	
            }
            /* Invalid command */
            else
            {
                printf("%s : Command not found\n", cmd);
            }
        }
    }
}
