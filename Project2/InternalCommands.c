#include <string.h>
#include "InternalCommands.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
extern char **environ;
FILE *batchfile = NULL;

/*************************************************************
 *METHOD: sets value of batchfile.  Called by myshell.c upon
 *startup. Used predominantly for the pause method, which
 * blocks until reading a return char.
 *************************************************************/
void setBatchfile(FILE * file){
    batchfile = file;
}

/*************************************************************
 *METHOD: executeInternalCommand checks the user -supplied
 *command against internal command list using strcmp.  If found
 *the result is set to some non zero value and returned to the
 *caller.  If the value is 1, the caller is informed to exit.
 *Some commands support redirection, and the value is supplied as
 *such.
 *input: string array containing commands and args
 *input: a string containing the ouptut filename(can be NULL)
 *input: a flag for whether or not to execute in appendMode.
 *************************************************************/
int executeInternalCommand(char** argv, char* outputFile, int appendMode){
    int result = 0;
    
    if(strcmp(argv[0], "quit" )== 0){
        result = 1;
    }
    else if(strcmp(argv[0], "echo") == 0){
        result = 2;
        echo(argv, outputFile, appendMode);
    }
    else if(strcmp(argv[0], "cd") == 0){
        result = 3;
        cd(argv);
    }
    else if(strcmp(argv[0], "clr") == 0){
        result = 4;
        clr();
    }
    
    else if(strcmp(argv[0], "environ") == 0){
        result = 5;
        printEnviron(outputFile, appendMode);
    }
    
    else if(strcmp(argv[0], "dir")==0){
        result = 6;
        printDir(argv, outputFile, appendMode);
    }
    else if(strcmp(argv[0], "help")== 0){
        result = 7;
        shellHelp();
    }
    else if(strcmp(argv[0], "pause")==0){
        result = 8;
        shellPause(argv);
    }
    return result;
}



/*************************************************************
 *METHOD: shellHelp() prints the readme document to the screen.
 *Can be redirected
 *************************************************************/
void shellHelp(){
    FILE * rm;
    char word[500];
    rm = fopen(getenv("README"), "r");
    while(fgets(word, 500, rm)){
        printf("%s", word);
    }
    fclose(rm);
}


/*************************************************************
 *METHOD: printDir is adapted from the tutorial slides listed in
 * lab, and modified so as to print to an optionally supplied
 * outputfile in either appendmode or write mode.
 *input: string array containing directory to print
 *fileName: string contianing name of outputFile (can be NULL)
 *appendMode: flag for whether to use append or write.
 *************************************************************/
void printDir(char **argv, char* fileName, int appendMode){
    DIR *dir;
    struct dirent *s;//Defined in dirent.h – you need to import this!
    char* directory;
    FILE * outputFile = NULL;
    //If we should be writing to a file in append mode
    if(fileName && appendMode){
        outputFile = fopen(fileName, "a");
        //else if we should just be writing to a file in write mode, open it
    } else if(fileName){
        outputFile = fopen(fileName, "w");
    }
    if(argv[1]!= NULL){
        directory= argv[1];
    }
    else{
        directory = strdup(getenv("PWD"));
    }
    //If dir equals NULL, that means there is an error
    dir = opendir(directory);
    //While there is another item in the directory that has not been looked at yet
    while((s = readdir(dir)) != NULL)
    {
        //If writing to a file, write to it
        if(outputFile){
            fprintf(outputFile, "%s\n", s->d_name);
        }
        //else print the contents to the screen.
        else{
            
            printf("%s\n", s->d_name);
        }
    }
    if(outputFile){
        fclose(outputFile);
    }
}
/*************************************************************
 *METHOD: printEnviron prints the environment strings to the
 * screen (or file) using external character environ.
 * If using output, the output is opened first, and the value is
 * printed to the file instead ofthe screen
 * INPUT: string containing name of output file (can be null)
 * OUTPUT: flag for append mode
 *************************************************************/
void printEnviron(char* fileName, int appendMode){
    FILE * outputFile = NULL;
    //If we should be writing to a file in append mode
    if(fileName && appendMode){
        outputFile = fopen(fileName, "a");
    }
    
    //else if we should just be writing to a file in write mode, open it
    else if(fileName){
        outputFile = fopen(fileName, "w");
    }
    int i = 0;
    //while environ has more strings, loop and print!
    while(environ[i]!=NULL){
        if(outputFile){
            fprintf(outputFile, "%s\n", environ[i]);
        }
        else{
            printf("%s\n", environ[i]);
        }
        i++;
    }
    //if opened, close file
    if(outputFile){
        fclose(outputFile);
    }
}

/*************************************************************
 *METHOD: printEnviron prints the environment strings to the
 * screen (or file) using external character environ.
 * If using output, the output is opened first, and the value is
 * printed to the file instead ofthe screen
 * INPUT: string containing name of output file (can be null)
 * INPUT: flag for append mode
 *************************************************************/
void cd(char **argv){
    char * dirName;
    dirName = (argv[1] == NULL) ? getenv("HOME") : argv[1];
    if(chdir(dirName) == 0){
        char pwd[1024];
        getcwd(pwd, 1024);
        setenv("PWD", pwd, 1);
        
    }else{
        printf("Unable change to directory %s\n", dirName);
    }
}

/*************************************************************
 *METHOD: echo prints repeats each word in the user string
 *back to the console (or outputfile) after reading.
 * If using output, the output is opened first, and the value is
 * printed to the file instead ofthe screen
 * INPUT: string array containg each word in the command
 * INPUT: string containing name of output file (can be null)
 * INPUT: flag for append mode
 *************************************************************/
void echo(char **argv, char* fileName, int appendMode){
    FILE * outputFile = NULL;
    //If writing in append mode open the file
    if(fileName && appendMode){
        outputFile = fopen(fileName, "a");
    }
    //If writing in write mode open the file
    else if(fileName){
        outputFile = fopen(fileName, "w");
    }
    int i = 1;
    //while there are words in the string array, print them (to file or console)
    while (argv[i]!=NULL){
        if(outputFile){
            fprintf(outputFile, "%s ", argv[i]);
        }
        else{
            printf("%s ", argv[i]);
        }
        i++;
    }
    //Add a return to the end of the line (stripped by parser)
    if(outputFile){
        fprintf(outputFile, "\n");
    }
    else{
        printf("\n");
    }
    
    //close output file
    if(outputFile){
        fclose(outputFile);
    }
}

/*************************************************************
 *METHOD: clr clears screen using lab instructor supplied string
 *************************************************************/
void clr(){
    printf("\033[H\033[2J");
}

/*************************************************************
 *METHOD: shellPause performs a blocking readln call until
 *user enters the return key.
 *************************************************************/
void shellPause(){
    printf("PAUSED: press return to continue\n");
    FILE * input;
    //If reading from a batch file, get it
    if (batchfile){
        input = batchfile;
    }
    //else read from stdin
    else {
        input = stdin;
    }
        char f;
        while (fgetc(input)!= '\n');
}






