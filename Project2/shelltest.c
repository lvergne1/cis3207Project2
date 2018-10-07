#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Parse.h"

static int executeInternalCommand(char** argv);
static void clr();

extern char ** environ;
char pwd[1024];
char *home;
int main(int argc, char** argv){
    
    char rawString[200];
    clr();
    
    getcwd(pwd, 1024);
    home = getenv("HOME");
    printf("%s> ", pwd);
    while(fgets(rawString, 200, stdin)){
        char* inputString = strdup(rawString);
        char* inputFile = NULL;
        char* outputFile = NULL;
        int numPipes = 0;
        char *fullCommandsToBeExecuted[10][20];
        int fullArgCount[10];
        int i = 0;
        char *commandsToBeExecuted[10];
        int runInBackground = 0;
        if(strcmp(inputString, "\n")!= 0){
            runInBackground = parseForBackground(&inputString);
            numPipes = parseForPipes(inputString, commandsToBeExecuted);
            parseForRedirectedIO(commandsToBeExecuted, &inputFile, &outputFile, numPipes);
            parseCommandListForArgs(commandsToBeExecuted, fullCommandsToBeExecuted, fullArgCount);
            int resultOfInternalCommand = executeInternalCommand(fullCommandsToBeExecuted[0]);
            if (resultOfInternalCommand==1) break;
            else if(!resultOfInternalCommand){
                int forkResult;
                if((forkResult = fork())== 0){
                    //child
                    printf("FORKED!\n");
                    if(strstr(fullCommandsToBeExecuted[0][0], ".")){
                        strsep(&fullCommandsToBeExecuted[0][0], ".");
                        char* pathName;
                        pathName = strcat(pwd, fullCommandsToBeExecuted[0][0]);
                        if(execv(pathName, fullCommandsToBeExecuted[0])<0){
                            printf("Unable to execute:%s\n", fullCommandsToBeExecuted[0][0]);
                            exit(0);
                        }
                    }else if(strstr(fullCommandsToBeExecuted[0][0], "/")){
                        if(execv(fullCommandsToBeExecuted[0][0], fullCommandsToBeExecuted[0])< 0){
                            printf("Unable to execute absolute path name:%s\n", fullCommandsToBeExecuted[0][0]);
                            exit(0);
                        }
                    }else{
                        if(execvp(fullCommandsToBeExecuted[0][0], fullCommandsToBeExecuted[0])<0){
                            printf("Unable to find %s in %s\n",fullCommandsToBeExecuted[0][0],  getenv("PATH"));
                            exit(0);
                        }
                    }
                }
                else{
                    //parent
                    int status = 0;
                    wait(&status);
                }
            }
        }
        printf("%s> ", pwd);
    }
    printf("\nEXITING SHELL\n");
}

static void clr(){
    printf("\033[H\033[2J");
}

/*NULL BUG ON NO SPACE AND \NL*/
static int executeInternalCommand(char** argv){
    int result = 0;
    if(strcmp(argv[0], "quit" )== 0){
        result = 1;
    }
    else if(strcmp(argv[0], "echo") == 0){
        result = 2;
        int i = 1;
        while (argv[i]!=NULL){
            printf("%s ", argv[i]);
            i++;
        }
        printf("\n");
    }
    else if(strcmp(argv[0], "cd") == 0){
        result = 3;
        char * dirName;
        dirName = (argv[1] == NULL) ? home : argv[1];
        if(chdir(dirName) == 0){
            getcwd(pwd, 1024);
            
        }else{
            printf("Unable to change directory\n");
        }
    }
    
    else if(strcmp(argv[0], "clr") == 0){
        result = 4;
        clr();
    }
    
    else if(strcmp(argv[0], "environ") == 0){
        result = 5;
        int i = 0;
        char* newShell = strcat(pwd, "/");
        setenv("SHELL", newShell, 1);
        while(environ[i]!=NULL){
            printf("%s\n", environ[i]);
            i++;
        }
    }
    
    else if(strcmp(argv[0], "dir")==0){
        result = 6;
        printf("dir\n");
    }
    else if(strcmp(argv[0], "help")== 0){
        result = 7;
        printf("help\n");
    }
    else if(strcmp(argv[0], "pause")==0){
        result = 8;
        printf("pause\n");
    }
    return result   ;
}



