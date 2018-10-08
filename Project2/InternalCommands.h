//
//  InternalCommands.h
//  
//
//  Created by Leo Vergnetti on 10/3/18.
//

#ifndef InternalCommands_h
#define InternalCommands_h

#include <string.h>
#include <stdio.h>
#include <unistd.h>

int executeInternalCommand(char** argv, char* outputFile, int appendMode);
void setBatchfile(FILE * file);
void cd(char **argv);
void clr();
void echo(char **argv, char* fileName, int appendMode);
void printEnviron(char* fileName, int appendMode);
void shellPause();
void printDir(char **argv, char* fileName, int appendMode);
void shellHelp();
#endif /* InternalCommands_h */

