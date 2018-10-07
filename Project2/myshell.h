

#ifndef myshell_h
#define myshell_h

#include <stdio.h>

void startShell(int argc, char** argv);
void initEnvironment();
void mainLoop(int argc, char** argv);
void executeCommand(char* argv[20]);
void redirectInput(char *inputFile);
void redirectOutput(char* outputFile, int append);
#endif /* myshell_h */
