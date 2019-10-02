Assignment 3 : Improving the SHELL

This is a continuation to Assignment 2. This SHELL has added functionality in the form of input and output redirection, piping and process management. 

<------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------>

Input-Output Redirection:
In input redirection using a '<' symbol one can give a file as input to a function, similarly using a '>' once can give a file to which the funtion will write the output to.

<------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------>

Piping:
To pipe one can separate commands using a '|' symbol so the the output of the left command is fed to the right command as input.

<------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------->

Process Management:
This SHELL can be used to handle process running in shell. 
Commands:
1.'jobs' is used to print list of all jobs along with their pid and status 
2.'kjob <process num> <signal num>' is used to send signal <signal num> to process <process num> 
3.'fg <job num>' is used to move background process to foreground
4.'bg <job num>' is used to switch stopped background process to running state
5.'overkill' kills all background processes at once
6.'CTRL-C' kills the current foreground process
7.'CTRL-Z' suspends the current foreground job and moves it to background


<------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------->

Miscellaneous:
Some miscellaneous built ins are also provided like quit, setenv, unsetenv.
Commands:
1.'setenv <var> <val>' sets variable <var> to value <val>
2.'unsetenv <var>' removes variable <var> from list of environment variables
3.'quit' exits the shell

