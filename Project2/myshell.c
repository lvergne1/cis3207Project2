
#include "myshell.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "Parse.h"
#include "InternalCommands.h"
#include <fcntl.h>

extern char** environ;

/*************************************************************
 *METHOD: startShell calls initEnvironment, and begins
 *the main loop, sending any commandline args
 *************************************************************/
void startShell(int argc , char** argv){
    initEnvironment();
    mainLoop(argc, argv);
}

/*************************************************************
 *METHOD: initEnvironment sets the pwd environment variable,
 *as well as setting the shell environment variable (passed to future children)
 *************************************************************/
void initEnvironment(){
    char pwd[1024];
    getcwd(pwd, 1024);
    char* pwdcpy = strdup(pwd);
    setenv("README", strcat(pwdcpy, "/readme"), 1);
    setenv("SHELL", strcat(pwd, "/myshell"), 1);
}

/*************************************************************
 *METHOD: mainLoop contains the loop logic for myshell.c
 *The method first checks the cmdlne args to see if there
 *was a file present.  If so, it attempts to open the batch file.
 *If this fails, input is set to stdin and the user is informed.
 *Within the loop, input is taken from the user terminated by
 *a \n. This string is then parsed and split into the number
 *of commands to be piped, as well as any input and output
 *redirection. Then the array is further parsed into each
 *individual command line argument. Then executeInternalCommand
 *is called. If it does not register to an ICmd, it is passed to
 *a loop in order to chain multiple piped commands.  Finally
 *IO is suitably redirected, and the command is executed.
 *May be executed in background.
 *input int argc: int containing number of command line args
 *input char** argv: string array containing commandline args
 *************************************************************/
void mainLoop(int argc, char** argv){
    int batchfileMode = 0;
    FILE *fp = 0;
    //If attempting to execute from batch file
    if(argc > 1){
        batchfileMode = 1;
       // open the file specified in the arg
        fp = fopen(argv[1], "r");
        //If the open fails, inform the user and read from stdin
        if(fp == NULL){
            printf("ERROR opening %s : using stdin\n", argv[1]);
            batchfileMode = 0;
            fp = stdin;
        }
        else{
            //sets batchfile pointer in internal command (else pause command waits for \n)
            setBatchfile(fp);
        }
    }
    //otherwise set the file to stdin
    else{
        fp = stdin;
    }
    //an array to hold raw user input via fgets
    char rawString[500];
    //clear the screen and print the current directory
    clr();
    printf("%s> ", getenv("PWD"));
    
    //Continue looping while fgets doesn't return NULL (EOFcondition)
    while(fgets(rawString, 500, fp)){
        //If in batchfile mode, print the string read from the file
        if(batchfileMode){
            printf("%s", rawString);
        }
        //Vars for loop execution inputFile, outputFile, appendmode etc
        char* inputString = strdup(rawString);
        char* inputFile = NULL;
        char* outputFile = NULL;
        int appendMode;
        int numPipes = 0;
        char *fullCommandsToBeExecuted[10][20];
        int fullArgCount[10];
        int i = 0;
        char *commandsToBeExecuted[10];
        int runInBackground = 0;
        
        //Do not parse an empty command!
        if(strcmp(inputString, "\n")!= 0){
            //Parse the inputstring, logging the number of pipes, append mode, and background execution.
            runInBackground = parseForBackground(&inputString);
            numPipes = parseForPipes(inputString, commandsToBeExecuted);
            appendMode = parseForRedirectedIO(commandsToBeExecuted, &inputFile, &outputFile, numPipes);
            parseCommandListForArgs(commandsToBeExecuted, fullCommandsToBeExecuted, fullArgCount);
            //Attempt to execute an internal command. executeInternalCommand returns 0 if it does not find a cmd
            int resultOfInternalCommand = executeInternalCommand(fullCommandsToBeExecuted[0], outputFile, appendMode);
            //IF not an internal command prepare to execute external command
            if (!resultOfInternalCommand){
                int forkResult;
                //create an array of pipes (for chaining piped commands)
                int pipes[numPipes][2];
                //BEGIN loop and loop for each command in the list (numPipes +1)
                for(int i = 0; i <= numPipes; i++){
                    //if we are currently executing the last command in the list, we don't need a new pipe.
                    //otherwise pipe it!
                    if(i != numPipes){
                        pipe(pipes[i]);
                    }
                    /*executing here only requires the current pipe and the last pipe.
                     *This should only be executed by parent.  If no future children
                     *will require these pipes, close them!
                     */
                    if(i >= 2){
                        close(pipes[i-2][0]);
                        close(pipes[i-2][1]);
                        
                    }
                    //If something went wrong, inform the user, otherwise we are the child!
                    if ((forkResult = fork()) < 0){
                        printf("Error occured during fork\n");
                    } else if(forkResult == 0){
                        //child
                        //set(create) "PARENT" environment string
                        setenv("PARENT", getenv("SHELL"), 1);
                        //If we are currently at the first item in the pipe chain, we can take our input from a file
                        if(i == 0 && inputFile){
                            redirectInput(inputFile);
                        }
                        //if not the first command, we should redirect our input to the pipe.
                        if(numPipes){//ENSURE we are not a single command
                            if(i != 0){ //and we are not the first command
                                //close stdin, redirect our input to the previous pipe, and close the remaining pipes.
                                close(0);
                                dup2(pipes[i-1][0], 0);
                                close(pipes[i-1][0]);
                                close(pipes[i-1][1]);
                                
                            } else{
                                //IF we are the first command, close the read pipe (we won't use it)
                                close(pipes[i][0]);
                            }
                            
                            //if not the last command, redirect output (as above) and close pipes
                            if(i != numPipes){
                                close(1);
                                dup2(pipes[i][1], 1);
                                close(pipes[i][1]);
                                close(pipes[i][0]);
                            }else{
                                //If the last command, close the write portion of the previous pipe, we won't need it!
                                close(pipes[i-1][1]);
                            }
                        }
                        //If we are at the final pipe in the list (or if we are executing a lone command), redirect the output to the file
                        if (i == numPipes && outputFile){
                            redirectOutput(outputFile, appendMode);
                        }
                        //finally call execute command on the current child
                       executeCommand(fullCommandsToBeExecuted[i]);
                    }
                    //PARENT
                }
                if (numPipes > 0){
                    
                    //If we opened a pipe, we must close it. The parent closed all pipes execept for the last one. CLOSE!
                    close(pipes[numPipes-1][0]);
                    close(pipes[numPipes-1][1]);
                }
                //If we shouldn't run in the background, wait for all children to finish.
                if(!runInBackground){
                    int status = 0;
                    while(wait(&status)>0);
                }
            }
            //If we executed the quit command, we should break from the loop
            if(resultOfInternalCommand == 1) break;
        }
        //otherwise, print the current directory and prepare for next execution
        printf("%s> ", getenv("PWD"));
    }
    //exited the loop, say bye!
    printf("\nEXITING SHELL\n");
}


/*************************************************************
 *METHOD: redirect output opens a file for outputRedirection.
 *closes current stdout, and uses the file in place.
 *If appendmode is set, we set it to append.
 *otherwise set to overwrite existing file
 
 *input: output file: char*
 *input: appendMode: int (0 means overwrite)
 *************************************************************/
void redirectOutput(char* outputFile, int append){
    int newstdout;
    if(append){
        newstdout = open(outputFile,O_WRONLY|O_APPEND|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
    }
    else{
    newstdout = open(outputFile,O_WRONLY|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
        ftruncate(newstdout, 0);
    }
 close(1);
 dup2(newstdout, 1);
 close(newstdout);
 }

/*************************************************************
 *METHOD: redirectInput opens the inputFile for read only
 *input: inputFile : string has value of inputFile (may be unset)
 *************************************************************/
void redirectInput(char *inputFile){
    int newstdin = open(inputFile, O_RDONLY);
    close(0);
    dup2(newstdin, 0);
    close(newstdin);
}
/*************************************************************
 *METHOD: executeCommand uses exec call to run
 * the appropriate command.  If the command contains a (.), Then
 *we look in current directory for the command.
 * If the command has no (.) but has a /, we treat it as an
 *absolute pathname, and run the command from that directory
 *Otherwise, we use execvp to run in the path environment variable.
 *the process exits if unable to find command.
 *************************************************************/
void executeCommand(char* argv[20]){
    //If we have a (.)
    if(strstr(argv[0], ".")){
        //remove it
        strsep(&argv[0], ".");
        char* pathName;
        //set pathname to the environment variable
        pathName = strcat(getenv("PWD"), argv[0]);
        if(execv(pathName, argv)<0){
            printf("Unable to execute:%s\n", argv[0]);
            exit(0);
        }
    //IF no (.) but have a /, run it from the given pathname
    }else if(strstr(argv[0], "/")){
        if(execv(argv[0], argv)< 0){
            printf("Unable to execute absolute path name:%s\n", argv[0]);
            exit(0);
        }
    //Otherwise run it form the path strings
    }else{
        if(execvp(argv[0], argv)<0){
            printf("Unable to find %s in %s\n",argv[0],  getenv("PATH"));
            exit(0);
        }
    }
}
