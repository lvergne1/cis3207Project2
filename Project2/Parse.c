
#include "Parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


/***********************************************************
*METHOD parseForBackground checks if the method should
*run in the backround.  If yes, it updates the inputString,
*and returns Yes. Else returns No
*INPUT: userInput: String
*OUTPUT: 0 if no, 1 if yes
 *USES: strstr, strsep in <string.h>
***********************************************************/
int parseForBackground(char** inputString){
    int runInBackground = 0;
//remove any newline chars
    *inputString = strsep(inputString, "\n");
//use strstr to see if string has "&"
    if(strstr(*inputString, "&")){
        //It does, so remove it and set runInBackground
        *inputString = strsep(inputString, "&");
        runInBackground = 1;
    }
    return runInBackground;
}


/***********************************************************
 METHOD parseForPipes takes the userInput and commandstoBeExecuted,
 array.  Uses strsep to parse the input string.  Each string is
 copied to the commandsToBeExecuted array.
 NOTE: commands still contain possible redirects
 INPUT: user input: string, commandsToBeExecuted: string array
 OUTPUT: number of pipes: integer
 SIDE EFFECT: populates commandsToBeExecuted
 ***********************************************************/
int parseForPipes(char* inputString, char **commandsToBeExecuted){
    int numberOfPipes = 0;
    while((commandsToBeExecuted[numberOfPipes] = strsep(&inputString, "|"))!=NULL){
        numberOfPipes++;
    }
    numberOfPipes -= 1;
    return numberOfPipes;
}
/***********************************************************
 METHOD parseForRedirectedInput
 INPUT: fileName: string pointer, string array containing
 commands to be piped.
 OUTPUT: string name of file to get input from (OR NULL)
 SIDEEFFECT: filename and < are removed from array.
 ***********************************************************/
int parseForRedirectedIO(char** commandsToBeExecuted, char** inputFile, char** outputFile, int numberOfPipes){
    char* firstCommandToBeExecuted;
    char* lastCommandToBeExecuted;
    int appendMode = 0;
    
    if (strstr(commandsToBeExecuted[0], "<")){
        firstCommandToBeExecuted = strsep(&commandsToBeExecuted[0], "<");
        *inputFile = strsep(&commandsToBeExecuted[0], "<");
        commandsToBeExecuted[0] = firstCommandToBeExecuted;
    }
    if(strstr(commandsToBeExecuted[numberOfPipes], ">")){
        if(strstr(commandsToBeExecuted[numberOfPipes], ">>")){
            appendMode = 1;
            lastCommandToBeExecuted = strsep(&commandsToBeExecuted[numberOfPipes], ">");
            strsep(&commandsToBeExecuted[numberOfPipes], ">");
        }
        else{
            lastCommandToBeExecuted = strsep(&commandsToBeExecuted[numberOfPipes], ">");
        }
        *outputFile = strsep(&commandsToBeExecuted[numberOfPipes], ">");
        commandsToBeExecuted[numberOfPipes] = lastCommandToBeExecuted;
    }
    else if(*inputFile != NULL){
        if(strstr(*inputFile, ">")){
            char* inputFileCopy = strdup(*inputFile);
            
            if(strstr(*inputFile, ">>")){
                appendMode = 1;
                *inputFile = strsep(&inputFileCopy, ">");
                strsep(&inputFileCopy, ">");
            }
            else{
                *inputFile = strsep(&inputFileCopy, ">");
            }
            *outputFile = strsep(&inputFileCopy, ">");
        }
    }
    *inputFile = *inputFile == NULL ? NULL : removeWhiteSpace(*inputFile);
    *outputFile = *outputFile == NULL ? NULL : removeWhiteSpace(*outputFile);
    return appendMode;
}


/*****************************************************************
 METHOD: populate argvarray
 *****************************************************************/
int parseCommandListForArgs(char **stringInput, char *finalCommandList[10][20], int fullArgCount[10]){
    int numberOfArgs = 0;
    int i = 0;
    int j = 0;
    while(stringInput[i]!=NULL){
        while((finalCommandList[i][j] = strsep(&stringInput[i], " "))!=NULL){
            if(*finalCommandList[i][j] != '\0'){
                j++;
            }
        }
        fullArgCount[i] = j;
        j = 0;
        i++;
    }
    return numberOfArgs;
}

/*****************************************************************
 METHOD: removeWhiteSpace
 *****************************************************************/
char* removeWhiteSpace(char* string){
    char * stringCopy = strdup(string);
    int i = 0;
    while (isspace(string[i])){
        string++;
    }
    while(!isspace(string[i]) && string[i] != '\0'){
        i++;
    }
    string[i] = '\0';
    return string;
    
}



/*****************************************************************
 *****************************************************************
                PARSE TESTS
 *****************************************************************
 *****************************************************************/

/*****************************************************************
 METHOD testParseForBackgroud briefly tests capabilities of
 parseForBackground method.  passes 2 strings to parseForBackground
 one with an ampersand, and one without.  Returns the value of the
 call (ie if the method should run in background or not) and the
 modified inputString.
 ****************************************************************/
void testParseForBackground(){
    char* string =strdup("helloworld&");
    printf("FROM DRIVER PRE: %s\n", string);
    printf("Run in Background?: %d\n", parseForBackground(&string));
    printf("FROM DRIVER POST: %s\n", string);
    string = strdup("helloworld");
    printf("FROM DRIVER PRE: %s\n", string);
    printf("Run in Background?: %d\n", parseForBackground(&string));
    printf("FROM DRIVER POST: %s\n", string);
}

/*****************************************************************
 METHOD testParseForPipes briefly tests capabilities of
 parseForPipes ethod.  passes 2 strings to parseForPipes
 one with pipes, and one without. Calls parseForBackground as well
 to verify compatibility.
 ****************************************************************/
void testParseForPipes(){
    char* inputString = strdup("nop|1pipe|2pipes&\n");
    int i = 0;
    printf("INPUT STRING: %s\n", inputString);
    printf("Run in background: %d\n", parseForBackground(&inputString));
    char *commandsToBeExecuted[10];
    printf("Number of Pipes: %d\n", parseForPipes(inputString, commandsToBeExecuted));
    while(commandsToBeExecuted[i]!= NULL){
        printf("From Driver: %s\n", commandsToBeExecuted[i]);
        i++;
    }
    inputString = strdup("nop\n");
    i = 0;
    printf("INPUT STRING: %s\n", inputString);
    printf("Run in background: %d\n", parseForBackground(&inputString));
    printf("Number of Pipes: %d\n", parseForPipes(inputString, commandsToBeExecuted));
    while(commandsToBeExecuted[i]!= NULL){
        printf("From Driver: %s\n", commandsToBeExecuted[i]);
        i++;
    }
}

/*****************************************************************
 METHOD testParseForRedirectedIO tests capabilities of parseForRedirectedIO
 method.  Creates a string array with various possible inputs, running
 through parser up to parseForRedirectedIO.  The output should be
 whether the command should be run in background, the number of pipes
 required, the commands to be run, and the input and output files
 made without any spaces
 ****************************************************************/
void testParseForRedirectedIO(){
    /*No pipes no Input*/
    char* stringChoices[10] = {
        "no pipesnoIOredirect\n","nopipes < infile.txt\n",
        "nopipes > outfile.txt & \n", "nopipes <infile.txt > outfile.txt\n",
        "nopipes outfile first >outfile.txt < infile.txt \n",
        "firstcommand no redirect| second command | thirdcommand&\n",
        "firstcommand with infile< aninfile.txt| second command | thirdcommand \n",
        "firstcommand with infile and outfile< aninfile.txt| second command | thirdcommand arg 1 > out.txt& \n"};
    
    int j = 0;
    
    while(stringChoices[j]!= NULL){
        int i = 0;
        int numPipes = 0;
        char* inputFile = NULL;
        char* outputFile = NULL;
        char *inputString = strdup(stringChoices[j]);
        printf("\n\nINPUT STRING: %s\n", inputString);
        printf("Run in background: %d\n", parseForBackground(&inputString));
        char *commandsToBeExecuted[10];
        printf("Number of Pipes: %d\n", numPipes = parseForPipes(inputString, commandsToBeExecuted));
        parseForRedirectedIO(commandsToBeExecuted, &inputFile, &outputFile, numPipes);
        printf("inFile:%s\n", inputFile);
        printf("outFile:%s\n", outputFile);
        while(commandsToBeExecuted[i]!= NULL){
            printf("Command %d:%s\n", i, commandsToBeExecuted[i]);
            i++;
        }
        j++;
    }
}

/*****************************************************************
 METHOD testParseCommandListForArgs tests capabilities of parseCommandListForArgs.
 method.  Creates a string array with various possible inputs, running
 through parser up to parseCommandListForArgs.  The output should include
 if the command should be run in background, the number of pipes
 required, the commands to be run, the input and output files
 made without any spaces, and it should print the double array containing
 the argv arrays, as well as the array containing the argc values for
 each piped command.
 ****************************************************************/
void testParseCommandListForArgs(){
    char* stringChoices[10] = {
        "no pipes no IOredirect\n","nopipes < infile.txt\n",
        "nopipes > outfile.txt & \n", "nopipes <infile.txt > outfile.txt\n",
        "nopipes outfile first >outfile.txt < infile.txt \n",
        "firstcommand no redirect| second command | thirdcommand&\n",
        "firstcommand with infile< aninfile.txt| second command | thirdcommand \n",
        "firstcommand with infile and outfile< aninfile.txt| second command | thirdcommand arg 1 > out.txt& \n"};
    
    int j = 0;
    char *fullCommandsToBeExecuted[10][20];
    int fullArgCount[10];
    while(stringChoices[j]!= NULL){
        printf("*****\n*******\n****\n");
        int i = 0;
        int numPipes = 0;
        char* inputFile = NULL;
        char* outputFile = NULL;
        char *inputString = strdup(stringChoices[j]);
        printf("INPUT STRING: %s\n", inputString);
        printf("Run in background: %d\n", parseForBackground(&inputString));
        char *commandsToBeExecuted[10];
        printf("Number of Pipes: %d\n", numPipes = parseForPipes(inputString, commandsToBeExecuted));
        parseForRedirectedIO(commandsToBeExecuted, &inputFile, &outputFile, numPipes);
        printf("inFile:%s\n", inputFile);
        printf("outFile:%s\n", outputFile);
        while(commandsToBeExecuted[i]!= NULL){
            printf("Command %d:%s\n", i, commandsToBeExecuted[i]);
            i++;
        }
        printf("\n");
        parseCommandListForArgs(commandsToBeExecuted, fullCommandsToBeExecuted, fullArgCount);
        i = 0;
        int k = 0;
        while(i <= numPipes){
            printf("Command %d has %d args\n",i+1, fullArgCount[i]);
            while(fullCommandsToBeExecuted[i][k]!= NULL){
                printf("%s\n", fullCommandsToBeExecuted[i][k]);
                k++;
            }
            k=0;
            printf("\n");
            i++;
        }
        j++;
    }
}

