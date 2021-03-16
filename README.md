# Implementation-of-a-Linux-Shell
C++ Implementation of a Linux Shell

YouTube Video for Demo: https://youtu.be/hpprR58c0v8

For this programming assignment, I implemented my shell by having my modified skeleton code in the main function, and have that main function call many functions that I had made before it. The functions that I made are sectioned neatly at the top of my program, in which it includes functions such as to execute a single command, overwrite stdin or stdout, and more. Like most people, I also wrote functions to split/parse a line I needed, to trim a string’s whitespaces, and to  turn a vector of strings into char pointers. 
Some functions I wrote that may be unique compared to other students may be the functions I wrote for my input/output redirection (in_redirect()/out_redirect), to check if a process is a background process (is_bg_process()), and to test if a given command is an “echo” command (test_for_echo()). 
In my main function, I made altercations to the skeleton code given by writing separate code for if it were only one command inputted, and for if there are multiple commands (signified by | for piping needed). 
I made sure to frequently clean up my code by keeping it neat and commented, which made it easy to figure out the order that my code was running, and how to fix it when a bug was present.
A bonus feature I added was that I colored the shell prompt to yellow, but I know that it doesn’t give any bonus points. I just thought it made the execution of my program look nicer since it emulated a real shell. Thanks for reading!
