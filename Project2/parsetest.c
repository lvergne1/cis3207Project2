#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

int main(){
  char *string = strdup( "Hello everyone let's split it up");
  char *stringb[50];
  int i = 0;
  while((stringb[i] = strsep(&string, " "))!=NULL)
    {
      printf("%s\n", stringb[i]);
      i++;
    }

  i = 0;
  while(stringb[i]!= NULL){
    printf("%s\n", stringb[i]);
    i++;
  }
  printf("%d\n", i);
}
