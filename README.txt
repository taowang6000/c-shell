##Introduction
This is a simplified version of Unix Shell.


##Supportted functions
Support four built-in commands "exit", "cd", "pwd" and "set".

Realized a external commands "myls", whose function is to list files under a
directory. The output of the command is very similar to "ls -l".

All the existing external command in the current Operating System can be 
supported.

Support multiple pipes without the pipe number's limitation.

Support backgroud execution(command line with &).

Support I/O redirection in a command.


##Limitations
1. The maximum number of a command's parameter is currently set to 9.

2. Pipe and I/O redirection can't coexist in one command. Pipe and background
process should not appear together.

3. Special characters such as "*", "$" are not supported.


##Method to run the program
1. Use the makefile accompanied with the source file c_shell.cpp and myls.cpp,
type in the command "make c_shell.x" and "make myls", two exceutable file 
"c_shell.x" and "myls" will appear in the working directory.

2. Put the file "myls" to any directory you want (optional).

3. Launch the file "c_shell.x", a symbol "$ " will appear.

4. To use the command "myls", a "MYPATH" environment variable need to be set.
Input the command "set MYPATH=DIRECTORY_NAME", the "DIRECTORY_NAME" means the 
full name of the directory where the file "myls" resides in.

5. The shell is ready to be used now.
