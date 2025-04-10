#include "systemcalls.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

/**
 * @brief Call the system() function with the command string passed in.
 * 
 * @param cmd Command to run
 * @return true if the command succeeded
 * @return false if the command failed
 */
bool do_system(const char *cmd)
{
    if (cmd == NULL)
        return false;

    int ret = system(cmd);
    return (ret == 0);
}

/**
 * @brief Execute a command with arguments using execv()
 * 
 * @param count Argument count
 * @param ... Command and its arguments as strings (char*)
 * @return true if the command was executed successfully
 * @return false on failure
 */
bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);

    // Build the command array
    char *command[count + 1];
    for (int i = 0; i < count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;

    va_end(args);

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork failed");
        return false;
    }

    if (pid == 0)
    {
        execv(command[0], command);
        perror("execv failed");
        exit(EXIT_FAILURE);
    }

    int status;
    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid failed");
        return false;
    }

    return (WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

/**
 * @brief Execute a command with stdout redirected to a file
 * 
 * @param outputfile File to redirect stdout to
 * @param count Argument count
 * @param ... Command and its arguments as strings (char*)
 * @return true if successful
 * @return false on failure
 */
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);

    char *command[count + 1];
    for (int i = 0; i < count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;

    va_end(args);

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork failed");
        return false;
    }

    if (pid == 0)
    {
        int fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0)
        {
            perror("open failed");
            exit(EXIT_FAILURE);
        }

        if (dup2(fd, STDOUT_FILENO) < 0)
        {
            perror("dup2 failed");
            close(fd);
            exit(EXIT_FAILURE);
        }

        close(fd);

        execv(command[0], command);
        perror("execv failed");
        exit(EXIT_FAILURE);
    }

    int status;
    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid failed");
        return false;
    }

    return (WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
