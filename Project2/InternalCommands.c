#include <string.h>
#include "InternalCommands.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
extern char **environ;

/*NULL BUG ON NO SPACE AND \NL*/
/*************************************************************
 *************************************************************/

int executeInternalCommand(char** argv){
    int result = 0;
    
    if(strcmp(argv[0], "quit" )== 0){
        result = 1;
    }
    else if(strcmp(argv[0], "echo") == 0){
        result = 2;
        echo(argv);
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
        printEnviron(argv);
    }
    
    else if(strcmp(argv[0], "dir")==0){
        result = 6;
        printf("dir\n");
        printDir(argv);
    }
    else if(strcmp(argv[0], "help")== 0){
        result = 7;
        printf("help\n");
    }
    else if(strcmp(argv[0], "pause")==0){
        result = 8;
        shellPause();
    }
    return result   ;
}

/*************************************************************
 *************************************************************/
void printDir(char **argv){
    DIR *dir;
    struct dirent *s;//Defined in dirent.h – you need to import this!
    char* directory;
    if(argv[1]!= NULL){
        directory= argv[1];
    }
    else{
        directory = strdup(getenv("PWD"));
    }
    dir = opendir(directory);//If dir equals NULL, that means there is an error
    while((s = readdir(dir)) != NULL)//While there is another item in the directory that has not been looked at yet
    {
        printf("%s\n", s->d_name);
    }
}
/*************************************************************
 *************************************************************/
void printEnviron(){
    int i = 0;
    while(environ[i]!=NULL){
        printf("%s\n", environ[i]);
        i++;
    }
}

/*************************************************************
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
 *************************************************************/
void echo(char **argv){
    int i = 1;
    while (argv[i]!=NULL){
        printf("%s ", argv[i]);
        i++;
    }
    printf("\n");
}

/*************************************************************
 *************************************************************/
void clr(){
    printf("\033[H\033[2J");
}

/*************************************************************
 *************************************************************/
void shellPause(){
    printf("PAUSED: press return to continue\n");
    char f;
    while (fgetc(stdin)!= '\n');
}





