#include "systemcalls.h"
#include <stdio.h>      // For printf, perror
#include <stdlib.h>     // For EXIT_FAILURE, EXIT_SUCCESS
#include <unistd.h>     // For fork, execv, _exit, dup2, close
#include <sys/wait.h>   // For waitpid, WIFEXITED, WEXITSTATUS
#include <fcntl.h>      // For open, O_WRONLY, O_CREAT, O_TRUNC

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

 int result = system(cmd);

    // Check if the command was executed successfully
    if (result == 0) {
        return true;  // Success
    } else {
        return false; // Failure
    }

   
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    //command[count] = command[count];

   
/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/

    va_end(args);
     int pid = fork();

    if (pid == -1)
    {
    // error, failed to fork()
        return false;
    } 
    else if (pid == 0)
    {
    
    execv(command[0],command);
            _exit(EXIT_FAILURE);
    }
    else 
    {
        // In the parent process, wait for the child to finish
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            // waitpid failed
            return false;
        }

        // Check if the child process terminated successfully
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }

}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

    va_end(args);

 // Open the output file
    int fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        return false;
    }

    // Fork the process
    int pid = fork();
    if (pid == -1) {
        // Fork failed
        close(fd);
        perror("fork");
        return false;
    } else if (pid == 0) {
        // In the child process

        // Redirect standard output to the file
        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("dup2");
            _exit(EXIT_FAILURE);
        }

        // Close the original file descriptor
        close(fd);

        // Execute the command
        execv(command[0], command);

        // If execv returns, there was an error
        perror("execv");
        _exit(EXIT_FAILURE);
    } else {
        // In the parent process
        close(fd); // Close the file descriptor in the parent
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            // waitpid failed
            perror("waitpid");
            return false;
        }

        // Check if the child process terminated successfully
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }
}
