#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>

#define MAX_LINE 80 /* The maximum length of command */
#define MAX_ARG 20  /* The maximum length of an argument */
#define READ_END 0
#define WRITE_END 1

#define FLAGS O_WRONLY | O_CREAT | O_TRUNC
#define MODE S_IRWXU | S_IXGRP | S_IROTH | S_IXOTH

int main (void) {
	char * args [MAX_LINE], * history_args[MAX_LINE]; /* command line arguments */
	char * tmp_ptr, * tmp_ptr_1;
	int should_run = 1;
	int prompt_cnt, history_prompt_cnt, i, fd;
	char ch;
	pid_t pid;
	FILE * in_f;
	int pipe_fd[2];
	//char read_buffer[BUFFER_SIZE];
	bool output_redirection_flag, pipe_flag, input_redirection_flag, concurrent_flag;
	int output_redirection_index, pipe_index, input_redirection_index;
	bool his_output_redirection_flag, his_pipe_flag, his_input_redirection_flag, his_concurrent_flag;
	int his_output_redirection_index, his_pipe_index, his_input_redirection_index;

	for (i = 0; i < MAX_LINE; ++ i) {
		args[i] = (char*) malloc (MAX_ARG * sizeof (char));
		history_args[i] = (char*) malloc (MAX_ARG * sizeof (char));
	}
	history_prompt_cnt = -1; /* mark no command history */

	while (should_run) {
		printf("osh>");
		fflush (stdout);

		/* first parse input arguments */		
		prompt_cnt = 0; /* prompt_cnt should mark NULL in args */
		i = 0;
		ch = getchar();
		concurrent_flag = output_redirection_flag = pipe_flag = input_redirection_flag = false;
		output_redirection_index = pipe_index = input_redirection_index = 0;

		while (1) {
			//printf ("prompt_cnt=%d, i=%d, ch :[%c]\n", prompt_cnt, i, ch);
			if (ch == '\n')
				break;
			else if (ch == ' ') {
				while (ch == ' ')
					ch = getchar();
			} else {
				while (ch != ' ' && ch != '\n') {
					args[prompt_cnt][i++] = ch;
					ch = getchar();
				}
				args[prompt_cnt][i] = '\0';
				if (strcmp (args[prompt_cnt], ">") == 0) {
					output_redirection_flag = true;
					output_redirection_index = prompt_cnt;
				} else if (strcmp (args[prompt_cnt], "|") == 0) {
					pipe_flag = true;
					pipe_index = prompt_cnt;
				} else if (strcmp (args[prompt_cnt], "<") == 0) {
					input_redirection_flag = true;
					input_redirection_index = prompt_cnt;
				} else if (strcmp (args[prompt_cnt], "&") == 0) {
					concurrent_flag = true;
				}
				//printf ("args[%d]: %s\n", prompt_cnt, args[prompt_cnt]);
				prompt_cnt += 1;
				i = 0;
			}
		}
		
		if (prompt_cnt == 0)
			continue;
		else if (strcmp (args[0], "exit") == 0) {
			should_run = 0;
			continue;
		} else if (strcmp (args[0], "!!") == 0) {
			if (history_prompt_cnt == -1) {
				printf("No command in history.\n");
				continue;
			} else {
				for (i = 0; i < history_prompt_cnt; ++ i) {
					strcpy (args[i], history_args[i]);
				}
				prompt_cnt = history_prompt_cnt;	
				output_redirection_flag = his_output_redirection_flag;
				output_redirection_index = his_output_redirection_index;
				input_redirection_flag = his_input_redirection_flag;
				input_redirection_index = his_input_redirection_index;
				pipe_flag = his_pipe_flag;
				pipe_index = his_pipe_index;
				concurrent_flag = his_concurrent_flag;
				for (i = 0; i < prompt_cnt; ++ i) {
					printf("%s ", args[i]);
				}
				printf("\n");
			}
		} else {
			for (i = 0; i < prompt_cnt; ++ i) {
				strcpy (history_args[i], args[i]);
			}
			history_prompt_cnt = prompt_cnt;
			his_concurrent_flag = concurrent_flag;
			his_output_redirection_flag = output_redirection_flag;
			his_output_redirection_index = output_redirection_index;
			his_input_redirection_flag = input_redirection_flag;
			his_input_redirection_index = input_redirection_index;
			his_pipe_flag = pipe_flag;
			his_pipe_index = pipe_index;
		}
		
		if (concurrent_flag) {
			-- prompt_cnt;
		}

		if (output_redirection_flag) {
			fd = open (args[output_redirection_index+1], FLAGS, MODE);
			if (fd == -1) {
				fprintf(stderr, "Open Failed.");
				return 1;
			}
			prompt_cnt = output_redirection_index;
		}

		if (input_redirection_flag) {
			fd = open (args[input_redirection_index+1], O_RDONLY, S_IRUSR);

			if (fd == -1) {
				fprintf (stderr, "Open failed.");
			}
			prompt_cnt = input_redirection_index;
		}

		tmp_ptr = args[prompt_cnt];
		args[prompt_cnt] = NULL; // requirement of execvp()

		/* (1) fork a child process using fork() */
		pid = fork();
		
		if (pid < 0) { /* error occurred */
			fprintf (stderr, "Fork Failed.");
			return 1;
		} else if (pid == 0) {
		/* (2) the child process will invoke execvp() */
			if (pipe_flag) {
				if (pipe (pipe_fd) == -1) {
					fprintf(stderr, "Pipe failed.");
					return 1;
				}

				args[prompt_cnt] = tmp_ptr;
				tmp_ptr = args[pipe_index];
				pid = fork();
				if (pid < 0) {
					fprintf(stderr, "Fork Failed.");
					return 1;
				}

				if (pid > 0) {
				/* parent process will execute the second command */
					wait(NULL);
					args[pipe_index] = tmp_ptr;
					tmp_ptr = args[prompt_cnt];
					args[prompt_cnt] = NULL;
					
					close (pipe_fd[WRITE_END]);
					dup2 (pipe_fd[READ_END], STDIN_FILENO);
					execvp (args[pipe_index+1], args+pipe_index+1);
					return 1;
				} else {
				/* new process will execute the first command */
					args[pipe_index] = NULL; /* requirement of execvp() */
					close (pipe_fd[READ_END]);
					dup2 (pipe_fd[WRITE_END], STDOUT_FILENO);
					execvp (args[0], args);
					return 1;
				}
			} else {
				if (output_redirection_flag) {
					dup2 (fd, STDOUT_FILENO);
				}
				if (input_redirection_flag) {
					dup2 (fd, STDIN_FILENO);
				}
				execvp(args[0], args);
				return 1;
			}
		} else {
		/* (3) parent will invoke wait() unless command include & */
			if (! concurrent_flag)
				waitpid (pid, NULL, 0);
			if (output_redirection_flag)
				close (fd);
			args[prompt_cnt] = tmp_ptr;
		}
	}

	for (i = 0; i < MAX_LINE; ++ i) {
		free (args[i]);
		free (history_args[i]);
	}

	return 0;
}
