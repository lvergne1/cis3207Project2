#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
extern char **environ;

int main(){
  int i = 0;
  char input[1024];
  char buff[1024];
  int size = sizeof(buff);
  i = 0;
  while(environ[i]!= NULL){
    printf("Env %d: %s\n",i,  environ[i]);
    i++;
 }
  /*  while(i < 10){
    getcwd(buff, size);
    printf("%s\n", buff);
    fgets(input, sizeof(input), stdin);
    char* token = strtok(input, "\n");
    printf("\n%d\n",chdir(token));
    getcwd(buff, size);
    printf("%s\n", buff);
    i++;
    }*/
}
