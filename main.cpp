#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h> 
#include <fcntl.h>
#include <vector>
#include <ctime>
#include <algorithm>
using namespace std;

// Global variables-----------------------------
    //Current date/time on current system
    time_t now = time(0);
    tm *ltm = localtime(&now);

    const string WHITESPACE = " \n\r\t\f\v";
    bool is_echo = false;
    bool output_redirect = false;
    bool input_redirect = false;
    int pid;
//----------------------------------------------

// My Created Functions----------------------------------------------------------
    // Check to see if a line has this character (used for < or >)
    bool has_character(string line, char c) {
        bool character_presence = false;

        // Split line into characters
        vector<char> v(line.begin(), line.end());

        // Go through each character of line and compare
        for (int i=0; i<v.size(); i++){
            if (v.at(i) == c) {
                character_presence = true;
            }
        }
        return character_presence;
    }

    // Trim white spaces around a string (useful for when parsing line for pipes)
    string trim (string line) {
        size_t start = line.find_first_not_of(WHITESPACE);
        string lefttrim = (start == std::string::npos) ? "" : line.substr(start);
        size_t end = lefttrim.find_last_not_of(WHITESPACE);
        return (end == std::string::npos) ? "" : lefttrim.substr(0, end + 1);
    }

    // Parses CommandLine by Spaces
    vector<string> split (string line, string del) {
        vector<string> v;
        string delimiter = del; // can be " ", or "|" for pipes
        
        // While theres more in the line to parse
        while(line.size()){ 
            // Find index up to the next space
            int index = line.find(delimiter);

            // Index is valid
            if(index!=string::npos){
                // Special case: echo command------------------------------
                if(line.substr(0,index)=="echo") {
                    // Add echo to vector
                    v.push_back(line.substr(0,index));
                    line = line.substr(index+delimiter.size());    // Move our cursor up
                    if (line.size()==0) { // If line is now empty
                        v.push_back(line);
                    }
                    index = line.find(delimiter);
                    // Check for quotations
                    if((line.at(0)==34)||(line.at(0)==39)) {       // first character is " or '
                        v.push_back(line.substr(1,line.size()-2)); // push back everything in quotes
                    } else { // not a quote, so -e
                        v.push_back(line.substr(0,index));
                        line = line.substr(index+delimiter.size()); // Move our cursor up
                        v.push_back(line.substr(1,line.size()-2));  // push back everything in quotes
                    }
                    goto endoffunction;
                }
                // Special case: awk command-------------------------------
                if(line.substr(0,index)=="awk") {
                     // Add awk to vector
                    v.push_back(line.substr(0,index));
                    line = line.substr(index+delimiter.size());    // Move our cursor up
                    // Push back rest of line besides quotations
                    v.push_back(line.substr(1,line.size()-2)); // push back everything in quotes
                    goto endoffunction;
                }
                // Everything else besides echo or awk---------------------
                v.push_back(trim(line.substr(0,index))); // Add token to vector
                line = line.substr(index+delimiter.size()); // Move our cursor up
                if(line.size()==0) { // If line is now empty
                    v.push_back(line);
                }
            } else { // If index=error cause there were no more delimiters
                v.push_back(trim(line));
                line = "";
            }
        }
        endoffunction:
        return v;
    }
    
    // Turn vector of strings into char pointers
    char** vec_to_char_array (vector<string>& parts) {
        char** result = new char* [parts.size() + 1]; //add 1 for the NULL
        for (int i=0; i<parts.size(); i++) {
            result[i] = (char*) parts[i].c_str(); // turns each part into char pointer
        }
        result [parts.size()] = NULL;
        return result;
    }
    
    // Input Redirection
    string in_redirect(string inputline) {
        // Command < inputFile
        vector<string> sides = split(inputline, "<");

        // Turn string inputFile into a char*
        char* inputFile = (char*)sides.at(1).c_str();

        //Open file
        int fd2 = open(inputFile, O_CREAT|O_RDONLY, S_IRUSR|S_IRGRP|S_IROTH);
        if (fd2 < 0) {
            perror("Couldn't open input file.\nTermination status: ");
            exit(0);
        }

        // Overwrites stdin
        dup2(fd2, 0); 
        close(fd2);
        
        // Return command to be executed, example: "grep /init" from "grep /init < inputFile"
        return sides.at(0);
    }

    // Output Redirection, return line to execute
    string out_redirect(string inputline) {
        // Command > outputFile
        vector<string> sides = split(inputline, ">");

        // Turn string outputFile into a char*
        char* outputFile = (char*)sides.at(1).c_str();
        int fd = open(outputFile, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        dup2(fd, 1); // overwrite fd with stdin
        close(fd);

        // Return command to be executed, example: "ps aux" from "ps aux > outputFile"
        return sides.at(0);
    }
    
    // Execute command from given line (will call split and vec_to_char_array)
    void execute_command(string inputline) {
        // As long as it's not an echo command
        input_redirect = false;
        output_redirect = false;
        if (!is_echo) {
            // See if the command wants to do I/O Redirection
            if (has_character(inputline, '<')) {
                input_redirect = true;
            }
            if (has_character(inputline, '>')) {
                output_redirect = true;
            }
        }
        
        // Trigger I/O Redirection Execution, Or Regular Execution
        if (output_redirect) {
            string command = out_redirect(inputline);
            execute_command(command);
        } else if (input_redirect) {
            string command = in_redirect(inputline);
            execute_command(command);
        } else { // Regular Execution of a Command
            vector<string> parts = split(inputline, " ");
            char** args = vec_to_char_array(parts);
            if (execvp(args[0], args) < 0) {
                printf("*** ERROR: exec failed\n");
                exit(1);
            }
        }
    }
    
    // Before splitting with "|", test if it's an echo command
    void test_for_echo(string inputline) {
        vector<string> parts = split(inputline, " ");
        if(parts.at(0)=="echo") 
            is_echo = true;
    }
    
    // Check if a process  is flagged to be a bg process
    bool is_bg_process(string line) {
        // Split line into characters
        vector<char> v(line.begin(), line.end());
        // Check to see if last character is "&"
        if (v.at( v.size()-1 ) == 38){
            return true;
        }else{
            return false;
        }
    }
//--------------------------------------------------------------------------------

// Main Shell Function --------------------------------------------------------------------------------
int main() {
    char* username = getenv("USER");
    vector<int> bgs; // list of background processes

    // Reset stdin and stdout
    int stdin = dup(0);
    int stdout = dup(1);

    while (true) {
        /* Check on background processes ---------------------------------*/
        for (int i=0; i<bgs.size(); i++) {
            // If returns -1, process is still running
            if (waitpid(bgs[i], 0, WNOHANG) == bgs[i]) { 
                // process is done, so we remove from list
                bgs.erase( bgs.begin() + i );              // remove ith
                i--;                                       // cancels with i++
            }
        }

        /* Print a prompt ------------------------------------------------*/
        cout << "\033[1;33m" << username << "@" << 1+ltm->tm_mon << "/" << ltm->tm_mday << "/"
         << 1900+ltm->tm_year << "|" << ltm->tm_hour << ":" << ltm->tm_min << "$ \033[0m";

        /* Get a line from standard input --------------------------------*/
        string inputline;
        getline(cin, inputline); 

        /* Exiting -------------------------------------------------------*/
        if (inputline == string("exit")) {
            cout << "Exiting Shell..." << endl;
            break;
        }
        
        /* Not Exiting ---------------------------------------------------*/
        // Check if it should be a background process
        bool bg = is_bg_process(inputline);
        if(bg){
            inputline = inputline.substr(0, inputline.size()-1);
        }

        // Check for echo
        vector<string> commands;
        test_for_echo(inputline);
        if(!is_echo) {
            commands = split(inputline, "|"); // Preparing the input command for execution
        } else {                              // is an echo command
            commands.push_back(inputline);
        }

        // How many commands?
        if (commands.size() == 1) {         // If theres only one command (no piping needed)
            pid = fork();
            if (pid == 0) {                 //child process
                execute_command(commands.at(0));
            } else {                        // parent process
                if (!bg) {                  // wait only if not a background process
                    waitpid(pid,0,0);
                } else {
                    bgs.push_back(pid);
                }
            }
        } else { // Multiple commands (piping needed)
            for(int i=0; i<commands.size(); i++) {
                // Set up the pipe
                int fds[2];
                pipe(fds);

                // Create child process
                int prid = fork();

                // Child vs. Parent
                if (prid == 0) { // child 
                    if (i < commands.size()-1) {      // If we're not on the last command
                        dup2(fds[1],1);               // 1.redirect the output to the next level 
                    }
                    execute_command(commands.at(i));  // 2. execute command at this level
                } else { // parent
                    if (i == commands.size()-1) {     // If we are on the last command, wait only for last child
                        waitpid(prid,0,0);            // 1. wait for the child process running the current level command
                    }
                    dup2(fds[0],0);                   // 2. redirect input from the child process 
                    close( fds[1] );                  // fds[1] must be closed, otherwise the next level will wait
                }
            }
        }

        /* Reset so it doesn't spam ---------------------------*/
        dup2(stdin, 0);
        dup2(stdout, 1);
    }
}