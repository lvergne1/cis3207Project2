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

int executeInternalCommand(char ** argv);
void cd(char **argv);
void clr();
void echo(char **argv);
void printEnviron();
void shellPause();
void printDir(char ** argv);
#endif /* InternalCommands_h */

