#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/signal.h>
#include <sys/wait.h>

/* list-files command */
void list_files() {
    DIR *d = opendir(".");
    if (!d) {
        printf("myshell: unable to list files from current directory: %s\n", strerror(errno));
        return;
    }
    char type;
    char perm_bits[10];
    struct stat s;

    for (struct dirent *e = readdir(d); e; e = readdir(d)) {
   		if (stat(e->d_name, &s) == 0) { // check if stat succeeds

            /* File types */
            if (S_ISDIR(s.st_mode))
                type = 'D';
            else 
                type = 'F';

            /* Permission bits */
            // User
            perm_bits[0] = (s.st_mode & S_IRUSR) ? 'r' : '-';
            perm_bits[1] = (s.st_mode & S_IWUSR) ? 'w' : '-';
            perm_bits[2] = (s.st_mode & S_IXUSR) ? 'x' : '-';
            // Group
            perm_bits[3] = (s.st_mode & S_IRGRP) ? 'r' : '-';
            perm_bits[4] = (s.st_mode & S_IWGRP) ? 'w' : '-';
            perm_bits[5] = (s.st_mode & S_IXGRP) ? 'x' : '-';
            // Other
            perm_bits[6] = (s.st_mode & S_IROTH) ? 'r' : '-';
            perm_bits[7] = (s.st_mode & S_IWOTH) ? 'w' : '-';
            perm_bits[8] = (s.st_mode & S_IXOTH) ? 'x' : '-';
            perm_bits[9] = '\0';
      
            printf("%-15c\t%-15s\tuser[%c%c%c]group[%c%c%c]other[%c%c%c]\t%ld bytes\n", type, e->d_name, perm_bits[0]
                    , perm_bits[1], perm_bits[2], perm_bits[3], perm_bits[4], perm_bits[5], perm_bits[6]
                    , perm_bits[7], perm_bits[8], (long)s.st_size);
        }
	}
	closedir(d);
}

/* change-dir command */
void change_dir(char *words[], int nwords) {
    if (nwords != 2) {
        printf("myshell> usage: change-dir <dir>\n");
        return;
    }
    // check if chdir succeeds
    if (chdir(words[1]) < 0) {
        printf("myshell: unable to change working directory to %s\n", words[1]);
        return;
    }
    else printf("myshell: working directory has been changed to %s\n", words[1]);
};

/* print-dir command */
void print_dir() {
    char *wdir = getcwd(NULL, 0);
    if (!wdir) {
        printf("myshell: unable to get working directory: %s\n", strerror(errno));
        return;
    }

    printf("%s\n", wdir);
    free(wdir);
}

/* copy-file command */
void copy_one_file(char *source, char *target, long *bytes_count);
void tree_copy(char *source, char *target, int *dirs_count, int *files_count, long *bytes_count);

void copy_file(char *words[], int nwords) {
    if (nwords != 3) {
        printf("myshell> usage: copyfile <source> <target>\n");
        return;
    }
    int dirs_count = 0;
    int files_count = 0;
    long bytes_count = 0;
    struct stat buf;
    /* Check if source is directory or file */
    if (stat(words[1], &buf) < 0) {
            printf("myshell: unable to open directory %s: %s\n", words[1], strerror(errno));
            return;
    }
    if (S_ISDIR(buf.st_mode)) {
        if (strncmp(words[1], "../", 3) == 0 && strncmp(words[2], "./", 2) == 0) { // make sure it does not copy directory to itself
            printf("myshell: unable to copy a directory %s, into itself, %s\n", words[1], words[2]);
            return;
        }
        tree_copy(words[1], words[2], &dirs_count, &files_count, &bytes_count); // otherwise, do treecopy
    }
    else if (S_ISREG(buf.st_mode)) {
        files_count++;
        copy_one_file(words[1], words[2], &bytes_count);
    }

    if (bytes_count > 0) {
        if (dirs_count == 0)
            printf("copy-file: copied %ld bytes from %s to %s\n", bytes_count, words[1], words[2]);
        else
            printf("copy-file: copied %d directories, %d files, and %ld bytes from %s to %s\n", dirs_count, files_count, bytes_count, words[1], words[2]);
    }
    else return;
}   

// Copy directory
void tree_copy(char *source, char *target, int *dirs_count, int *files_count, long *bytes_count) {
    struct stat buf; // pointer to area which information should be written
    
    /* Open source directory */
    DIR *source_dir = opendir(source);
    if (!source_dir) {
        printf("myshell: unable to open %s: %s\n", source, strerror(errno));
        return;
    }

    /* Check if target directory already exist */
    if (strcmp(target, ".") != 0 && strcmp(target, "..") != 0 && strcmp(target, "~") != 0) {
        DIR *target_dir = opendir(target);
        if (target_dir) { // directory exists
            printf("myshell: %s already exits.\n", target);
            closedir(target_dir);
            return;
        }
        else if (ENOENT == errno) { // directory does not exist
            /* Create target directory */
            printf("%s -> %s\n", source, target);
            mkdir(target, S_IRWXU);
            *dirs_count = *dirs_count + 1;
        }
        else { // opendir failed
            printf("myshell: %s directory is not valid: %s\n", target, strerror(errno));
            return;
        }
    }

    /* Create target path from source path */
    char source_path[BUFSIZ];
    char target_path[BUFSIZ];
    
    // Iterate through the source directory
    for (struct dirent *entry = readdir(source_dir); entry; entry = readdir(source_dir)) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Create target path from source path
        sprintf(source_path, "%s/%s", source, entry->d_name);
        sprintf(target_path, "%s/%s", target, entry->d_name);

        // Check if current source path is valid
        if (stat(source_path, &buf) < 0) {
            printf("myshell: unable to open directory %s: %s\n", source_path, strerror(errno));
            return;
        }

        // Check if the current source path is a directory
        if (S_ISDIR(buf.st_mode)) {
            // Recursive case
            tree_copy(source_path, target_path, dirs_count, files_count, bytes_count);
        }

        // If not directory, check if it is a file
        else if (S_ISREG(buf.st_mode)) {
            *files_count = *files_count + 1;
            copy_one_file(source_path, target_path, bytes_count);
        }

        // Neither a directory nor a file
        else {
            printf("myshell: %s is neither a directory nor a file\n", source_path);
            return;
        }
    }

    /* Close source directory */
    closedir(source_dir);  
}

// Copy file
void copy_one_file(char *source, char *target, long *bytes_count) {
    /* Open source file */
    int source_fd = open(source, O_RDONLY, 0);
    if (source_fd < 0) {
        printf("myshell: unable to open %s: %s\n", source, strerror(errno));
        return;
    }

    /* Check if target file already exists */
    if (access(target, F_OK) == 0) {
        printf("myshell: %s already exists\n", target);
        return;
    }

    /* Create target file */
    int target_fd = creat(target, S_IRWXU);
    if (target_fd < 0) {
        printf("myshell: unable to create %s: %s\n", target, strerror(errno));
        return;
    }

    /* Copy file from source to target */
    printf("%s -> %s\n", source, target);
    char buffer[4096];
    int nread, nwrite;

    while ((nread = read(source_fd, buffer, sizeof(buffer))) > 0) {
        // Write to file
        nwrite = write(target_fd, buffer, nread);
        // Check if write is valid 
        if (nwrite < 0) {
            if (errno != EINTR) { // If not Interupted system call
                printf("myshell: unable to write to file %s: %s\n", target, strerror(errno));
                return;
            }
        }

        // Write the remaining to file
        while (nread != nwrite) {
            int remain = write(target_fd, buffer + nwrite, nread - nwrite);
             // Check if write is valid 
            if (remain < 0) {
                if (errno != EINTR) { // If not Interupted system call
                    printf("myshell: write to file %s partially succeed: %s\n", target, strerror(errno));
                    return;
            }
            nwrite += remain;
            }
        }
        *bytes_count += nread;
    }

    /* Close source and target files */
    close(source_fd);
    close(target_fd);
}

/* start-process command */
int start_process(char *words[], int nwords) {
    if (nwords < 2) {
        printf("myshell> usage: start-process <agrs>\n");
        return -1;
    }

    pid_t pid = fork();

    if (pid < 0) { // parent (error)
        printf("myshell: fork error: %s\n", strerror(errno));
        return -1;
    } else if (pid == 0) { // child
        if (execvp(words[1], &words[1]) < 0) { // execute commands
            printf("myshell: unable to start process: %s\n", strerror(errno));
            return -1;
        }
    } else  // parent (success)
        printf("myshell: process %d started\n", pid);
    
    return pid;
}

/* wait-for-any-child command */
void wait_for_any_child() {
    int status;
    int pid = wait(&status);

    if (pid < 0) { // there is no process
        printf("myshell: no children\n");

    } else {
        // check exit status
        if (status == 0) { // exit normally
            printf("myshell: process %d exited normally with status 0\n", pid);
            return;
        } else { // not exit normally
            printf("myshell: process %d exited abnormally with signal %d: %s\n", pid, status, strerror(errno));
            return;
        }
    }

};

/* wait-for-a-child command */
void wait_for_a_child(pid_t child) {
    int status;
    pid_t pid = waitpid(child, &status, 0);

    if (pid < 0) { // check if there is a process
        printf("myshell: no such process\n");
        return;
    } else {
        // check exit status
         if (status == 0) { // exit normally
            printf("myshell: process %d exited normally with status 0\n", pid);
            return;
        } else { // not exit normally
            printf("myshell: process %d exited abnormally with signal %d: %s\n", pid, status, strerror(errno));
            return;
        }
    }
    return;
}

/* run-process command */
void run_process(char *words[], int nwords) {
    if (nwords != 2) {
        printf("myshell> usage: run-process <args>\n");
        return;
    }
    // start a process
    pid_t pid = (pid_t)start_process(words, nwords);

    // wait for process to finish
    if (pid > 0) 
        wait_for_a_child(pid);
}

/* kill-process command */
void kill_process(char *words[], int nwords) {
    if (nwords != 2) {
        printf("myshell> usage: kill-process <pid>\n");
        return;
    }
    pid_t pid = atoi(words[1]);
    if (kill(pid, SIGTERM) < 0) {
        printf("myshell: unable to kill process %d\n", pid);
        return;
    }
    else printf("myshell: process %d has been killed\n", pid);
}

int main(int argc, char *argv[]) {

    /* Read one line of text after printing the prompt */
    char *input_line;

    while (1) {
        /* Prints prompy initially */
        printf("myshell> ");
        fflush(stdout); // force the output

        input_line = calloc(4096, sizeof(char)); // allocate memory for reading command lines

        if (fgets(input_line, 4096, stdin) == NULL) {//stop if fgets() returns null, indicating end-of-file
            printf("\n");
            free(input_line);
            break;
        }
        // check if the input line is valid
        if (strlen(input_line) > BUFSIZ) {
            printf("myshell: too many characters in the input line\n");
            continue;
        }

        /* Breaking the input line into words */
        char *words[128];
        int nwords = 0;
        int flag_break = 0;

        /* Obtain the first word */
        words[0] = strtok(input_line, " \t\n");
        if (words[0] != NULL)
            nwords++;
        else continue;

        /* Get the rest of input line */
        while ((words[nwords] = strtok(0," \t\n")) != NULL) {
            nwords++;
            // check if there are too many words
            if (nwords > sizeof(words)) {
                printf("myshell: too many words in the input line\n");
                flag_break = 1;
                break;
            }
        }
        // continue if there are too many words
        if (flag_break)
            continue;
        
        words[nwords] = 0;

        /* Built-in commands */
        if (strcmp(words[0],"list-files")==0)
            list_files();

        else if (strcmp(words[0],"change-dir")==0)
            change_dir(words, nwords);

        else if (strcmp(words[0],"print-dir")==0)
            print_dir();

        else if (strcmp(words[0],"copy-file")==0)
            copy_file(words, nwords);

        else if (strcmp(words[0],"start-process")==0) {
            int pid = start_process(words, nwords);
            (void)pid;
        }

        else if (strcmp(words[0],"wait-for-any-child")==0)
            wait_for_any_child();

        else if (strcmp(words[0],"wait-for-a-child")==0) {
            if (nwords != 2) 
                printf("myshell> usage: wait-for-a-child <pid>\n");

            else {
                pid_t pid = (pid_t)(atoi(words[1]));
                wait_for_a_child(pid);
            }
        }

        else if (strcmp(words[0],"run-process")==0)
            run_process(words, nwords);

        else if (strcmp(words[0],"kill-process")==0)
            kill_process(words, nwords);

        else if (((strcmp(words[0],"exit")) && (strcmp(words[0],"quit"))) == 0) {
            free(input_line);
            exit(EXIT_SUCCESS);
        }
        else 
            printf("myshell: unknown command: %s\n", words[0]);

        free(input_line);
    }

    return EXIT_SUCCESS;
}
