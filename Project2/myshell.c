
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
 *************************************************************/
void startShell(int argc , char** argv){
    initEnvironment();
    mainLoop(argc, argv);
}

/*************************************************************
 *************************************************************/
void initEnvironment(){
    char* pwd = strdup(getenv("PWD"));
    setenv("SHELL", strcat(pwd, "/myshell"), 1);
}

/*************************************************************
 *************************************************************/
void mainLoop(int argc, char** argv){
    int executionMode = 0;
    FILE *fp = 0;
    if(argc > 1){
        executionMode = 1;
        fp = fopen(argv[1], "r");
        printf("FILENAME:%s\n", argv[1]);
        if(fp == NULL){
            printf("ERROR opening %s : using stdin\n", argv[1]);
        }
    }
    else{
        fp = stdin;
    }
    char rawString[500];
    clr();
    printf("%s> ", getenv("PWD"));
    
    while(fgets(rawString, 500, fp)){
        if(executionMode){
            printf("%s", rawString);
        }
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
        
        if(strcmp(inputString, "\n")!= 0){
            runInBackground = parseForBackground(&inputString);
            numPipes = parseForPipes(inputString, commandsToBeExecuted);
            appendMode = parseForRedirectedIO(commandsToBeExecuted, &inputFile, &outputFile, numPipes);
            parseCommandListForArgs(commandsToBeExecuted, fullCommandsToBeExecuted, fullArgCount);
            int resultOfInternalCommand = executeInternalCommand(fullCommandsToBeExecuted[0]);
            if (!resultOfInternalCommand){
                int forkResult;
                int pipes[numPipes][2];
                for(int i = 0; i <= numPipes; i++){
                    if(i != numPipes){
                        pipe(pipes[i]);
                    }
                    if(i >= 2){
                        close(pipes[i-2][0]);
                        close(pipes[i-2][1]);
                        
                    }
                    if ((forkResult = fork()) < 0){
                        printf("Error occured during fork\n");
                    } else if(forkResult == 0){
                        //child
                        if(i == 0 && inputFile){
                            redirectInput(inputFile);
                        }
                        //if not the first command, redirect input
                        if(numPipes){
                            if(i != 0){
                                close(0);
                                dup2(pipes[i-1][0], 0);
                                close(pipes[i-1][0]);
                                close(pipes[i-1][1]);
                                
                            } else{
                                close(pipes[i][0]);
                            }
                            
                            //if not the last command, redirect output
                            if(i != numPipes){
                                close(1);
                                dup2(pipes[i][1], 1);
                                close(pipes[i][1]);
                                close(pipes[i][0]);
                            }else{
                                close(pipes[i-1][1]);
                            }
                        }
                        if (i == numPipes && outputFile){
                            redirectOutput(outputFile, appendMode);
                        }
                       executeCommand(fullCommandsToBeExecuted[i]);
                    }
                }
                if (numPipes > 0){
                    close(pipes[numPipes-1][0]);
                    close(pipes[numPipes-1][1]);
                }
                if(!runInBackground){
                    int status = 0;
                    while(wait(&status)>0);
                }
            }
            if(resultOfInternalCommand == 1) break;
        }
        printf("%s> ", getenv("PWD"));
    }
    printf("\nEXITING SHELL\n");
}


/*************************************************************
 *************************************************************/
void redirectOutput(char* outputFile, int append){
    int newstdout;
    if(append){
        newstdout = open(outputFile,O_WRONLY|O_APPEND|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
    }
    else{
    newstdout = open(outputFile,O_WRONLY|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
    }
 close(1);
 dup2(newstdout, 1);
 close(newstdout);
 }

/*************************************************************
 *************************************************************/
void redirectInput(char *inputFile){
    int newstdin = open(inputFile, O_RDONLY);
    close(0);
    dup2(newstdin, 0);
    close(newstdin);
}
/*************************************************************
 *************************************************************/
void executeCommand(char* argv[20]){
    if(strstr(argv[0], ".")){
        strsep(&argv[0], ".");
        char* pathName;
        pathName = strcat(getenv("PWD"), argv[0]);
        if(execv(pathName, argv)<0){
            printf("Unable to execute:%s\n", argv[0]);
            exit(0);
        }
    }else if(strstr(argv[0], "/")){
        if(execv(argv[0], argv)< 0){
            printf("Unable to execute absolute path name:%s\n", argv[0]);
            exit(0);
        }
    }else{
        if(execvp(argv[0], argv)<0){
            printf("Unable to find %s in %s\n",argv[0],  getenv("PATH"));
            exit(0);
        }
    }
}
