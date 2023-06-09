# Project 2: Processes and the Shell

This is Project 2 of [CSE-34341-SVS-Spring-2023](https://github.com/patrick-flynn/CSE34341-SVS-Sp2023/blob/main/index.md)

## Student

* Tram Trinh (htrinh@nd.edu)

# Project Goals

The goals of this project are:

- To learn the relationship between the kernel, the shell, and user-level programs.
- To learn how to employ the Unix process management system calls.
- To gain more experience in rigorous error handling.

# Essential Requirements

myshell is a program that is capable of executing, managing, and monitoring user level programs. This program will be similar in purpose and design to everyday shells like bash or tcsh, although the syntax will be slightly different. myshell will be invoked without any arguments, and will support several different commands.

myshell should print out a prompt like myshell> when it is ready to accept input. It must read a line of input, accepting several possible commands:

- list-files
- change-dir
- print-dir
- copy-file
- start-process
- wait-for-any-child
- wait-for-a-child
- run-process
- kill-process
- quit
- exit

## list-files

The built in list-files command should cause the shell to list the contents of the current directory, displaying each filename, type (use F for file and D for directory) and permission bits (see below) , and size in bytes – roughly the same information as ls -l

For example:
 
> <samp>myshell> list-files
 
> <samp>D .         user[rwx]group[r-x]other[r-x] 0 bytes</samp>
 
> <samp>D ..        user[rwx]group[r-x]other[r-x] 0 bytes</samp>
 
> <samp>F words.txt user[rw-]group[r--]other[r--] 105 bytes</samp>
 
> <samp>F myshell.c user[rw-]group[r--]other[r--] 2836 bytes</samp>
 
> <samp>... and so on ...
 

## change-dir

The built in change-dir command should cause the shell to change its working directory to the named directory:

> <samp>myshell> change-dir /tmp</samp>

 
## print-dir
 
The built in  print-dir command should cause the shell to print the current working directory:

> <samp>myshell> print-dir</samp>
 
> <samp>/escnfs/home/htrinh</samp>


## copy-file

The built in  copy-file command should duplicate one file or directory to another:

> <samp>myshell> copy-file old.c new.c</samp>
 
> <samp>copy-file: copied 2836 bytes from old.c to new.c</samp>
 

## start-process

The start-process command should start another program with command line arguments, print out the process number of the running program, and then accept another line of input. 

For example:

> <samp>myshell> start-process cp data.txt copy.txt</samp>
 
> <samp>myshell: process 346 started</samp>
 
> <samp>myshell></samp>
 

## wait-for-any-child

The wait-for-any-child command causes the shell to wait for any child process to exit. When this happens, indicate whether the exit was normal or abnormal, along with the exit code or signal number and name, respectively. Display any errors encountered. 

For example:

> <samp>myshell> wait-for-any-child</samp>
 
> <samp>myshell: process 502 exited normally with status 5</samp>

> <samp>myshell> wait-for-any-child</samp>
 
> <samp>myshell: process 347 exited abnormally with signal 11: Segmentation fault.</samp>

> <samp>myshell> wait-for-any-child</samp>
 
> <samp>myshell: No children.</samp>
 

## wait-for-a-child

The wait-for-a-child command does the same thing, but waits for a specific child process to exit:
 
> <samp>myshell> wait-for-a-child 346</samp>
 
> <samp>myshell: process 346 exited normally with status 0</samp>
 
> <samp>myshell> wait-for-a-child 346</samp>

> <samp>myshell: No such process.</samp>
 

## run-process

The run-process command should combine the behavior of start-process and wait-for-a-child. run-process should start a program, wait for that particular process to finish, and print the exit status. 
 
For example:
 
> <samp>myshell> run-process date</samp>

> <samp>Mon Jan 19 11:51:57 EST 2023</samp>
 
> <samp>myshell: process 348 exited normally with status 0</samp>
 

## kill-process

The kill-process command should kill the process whose pid is given.
 
> <samp>myshell> kill-process 346</samp>
 
> <samp>myshell: process 346 has been killed</samp>
 
> <samp>myshell> kill-process 346</samp>
 
> <samp>myshell: unable to kill process 346</samp>
 

## Other

After each command completes, the program will continue to print a prompt and accept another line of input. The shell should exit with status zero if the command is quit or exit or the input reaches end-of-file. If the user types a blank line, simply print another prompt and accept a new line of input. If the user types any other command, the shell should print a reasonable error message:

 
> <samp>myshell> bargle ls -la</samp>
 
> <samp>myshell: unknown command: bargle</samp>


