//
//  Parse.h
//  
//
//  Created by Leo Vergnetti on 9/27/18.
//

#ifndef Parse_h
#define Parse_h

#include <stdio.h>
void testParseForBackground();
void testParseForPipes();
void testParseForRedirectedIO();
void testParseCommandListForArgs();
int parseForBackground(char** inputString);
int parseForPipes(char* inputString, char **commandsToBeExecuted);
int parseForRedirectedIO(char** commandsToBeExecuted, char** inputFile, char** outputFile, int numberOfPipes);
int parseCommandListForArgs(char **stringInput, char *finalCommandList[10][20] , int  fullArgCount[10]);
char* removeWhiteSpace(char* string);
#endif /* Parse_h */
