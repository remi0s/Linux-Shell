/*  ERGASIA MA8HMATOS
  LEITOYRGIKA SYSTHMATA
       31-12-2016

MASTORAS RAFAIL EVANGELOS
       AEM7918
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

//functions declerations
void read_command(char *buffer);
int split_arguments(char *buffer,char *args[1000][512]);
int execute(int commands,char *args[1000][512]);
int read_file(char *buffer,char *argv[],int offset);


int main(int argc, char *argv[]){
  if (argc <2) {
    //interactive mode
    int i=0,j=0;
    int quit=1;
    while(quit){
      printf("mastoras_7918> ");
      char *buffer=malloc(sizeof(char)*2048); //make a big buffer to avoid segmentation faults
      int commands;   //number of commands
      read_command(buffer);  //read input into buffer
      char *args[1000][512];    //a point to 2d array for the arguments
      commands=split_arguments(buffer,args); //parse the commands and arguments
      quit=execute(commands,args);  //execute all commands, return if there was a quit command
      free(buffer); //free buffer in order to create new next round, to avoid troubles
      for(i=0;i<1000;i++){ //avoiding troubles by making NULL all args
          for(j=0;j<512;j++)
            args[i][j]=NULL;
      }
    }

  }else if(argc==2){
    //batch file mode
    int offset=0;   //this variable indicates the row of the file we last read
    int endOfFile=1,exit2=1,quit=1;   //variables in order to exit programm
    while(endOfFile==1&&quit==1){ //if atleast one becomes 0 the programm ends
                                  //we can easily change it so if programm
                                  //finds quit command will wait for all the other
                                  //commands, by changing the && to ||

      char *buffer=malloc(sizeof(char)*2048);//create a big buffer
      int commands;     //number of commands
      int i=0,j=0;
      endOfFile=read_file(buffer,argv,offset);  //read from file and return if we reached the end of the fail
      offset++;               //next row for the next round
      char *args[1000][512];
      commands=split_arguments(buffer,args); //parse commands and arguments
      exit2=execute(commands,args);         //execute all and return if there was a quit command
      if(exit2==0||endOfFile==0){           //this will only come in use if we need || in the while
        quit=0;
      }
      free(buffer); //free buffer , avoid troubles

      for(i=0;i<1000;i++){ //NULL all args , avoid troubles
          for(j=0;j<512;j++)
            args[i][j]=NULL;
      }
    }

  }else{ //arguments were more than should
    printf("In order to use this programm type : \n");
    printf("%s                 for interactive mode \n",argv[0]);
    printf("%s filename        for batch mode \n",argv[0]);
    exit(0);

  }

  return EXIT_SUCCESS;
}

//READ FILE FUNCTION
int read_file(char *buffer,char *argv[],int offset){
  FILE *file=fopen(argv[1],"r"); //open file for reading
  if (file==0){ //in case there wasnt such a file
    printf("Could not open %s \n",argv[1]);
    exit(0);
  }else{ //if open was sucessfull
    int i=0;
    int j=0;
    int x;


    while  (( x = fgetc( file ) ) != EOF){//while we havent reached the end of file
      if(j<offset){ //jump the rows we already have executed
        if(x=='\n'){
          j++;
        }
      }else{        //new row we havent executed
        if(x=='\n'){   //if we have reached rows end
          x='\0';      //end with \0
          buffer[i]=x;
          break;    //break from while
        }else{
          buffer[i]=x;  //all except '\n' go into buffer
        }
        i++;
      }
    }
    fclose( file ); //close file
    buffer[i]='\0';

    if(i==0&&j==0){ //if both i and j =0 means we hit EOF at the 1st try
      printf("File %s is empty \n",argv[1]); //so the file is empty
    }

    if(strlen(buffer)>512){ //check every row to have max 512 characters
      printf("Only 512 characters are allowed per row\n");
      printf("You have entered %lu characters in row %d\n",strlen(buffer),offset);
      exit(0);
    }

    if(x==EOF){ //if we reach EOF
      x='\0';
      buffer[i]=x;
      return 0; //return 0 in order to stop the programm
    }else{      //else we havent reached EOF yet
      return 1; //keep going
    }
  }
}

//READ INPUT FROM CONSOLE
void read_command(char *buffer){
    fgets(buffer, 2048, stdin); //we use all the buffer lengh in order to avoid problems
    if(strlen(buffer)>0){
      buffer[strlen(buffer)-1] = '\0'; //the last char must be '\0'
    }else{
      buffer[0]='\0'; //if buffer is empty just make an '\0' character
    }

  if(strlen(buffer)>512){ //check buffer lengh to be max 512 characters
    printf("Only 512 characters are allowed\n");
    printf("You have entered %lu characters\n",strlen(buffer));
    exit(0);
  }
}

void removeSpaces(char *str1){
  //an function which removes blanks
  //needed to use in order to avoid trouble with ; ; (empty commands)
    char *str2;
    str2=str1;
    while (*str2==' ') str2++;
    if (str2!=str1) memmove(str1,str2,strlen(str2)+1);
}


#define DELIM " \t\r\n\a"  //a delimeter used in strtok in order to purse args

int split_arguments(char *buffer,char *args[1000][512]){
    char *token;
    int j=0,i=0;
    int commands=0;
    token=strtok(buffer,";"); //find each one of the commands
    while(token!=NULL){
        *args[i]=token;  //and save them at args
        removeSpaces(*args[i]);
        commands++;
        i++;

      token=strtok(NULL,";");
    }

    for(i=0;i<commands;i++){ //now that we have all the commands purse their arguments
      j=0;
      token=strtok(*args[i], DELIM);
      while(token!=NULL){
          args[i][j]=token;
          j++;
          token=strtok(NULL, DELIM);
      }
    }
    args[i][j] = NULL; //the last character must be NULL
    return commands;
}


static int *glob_var; //a global variable to use for shared memmory

int execute(int commands,char *args[1000][512]){
  pid_t pid[commands]; //array of all pids
  int status[commands]; //aray of status
  int i=0;
  int mother=0; //indicator that this is mother
  //create a shared memmory block between mother and childs
  glob_var = mmap(NULL, sizeof *glob_var, PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  *glob_var=0; //0 means there is no quit command yet

  for(i=0;i<commands;i++){ //each command will be a new child process
    pid[i]=fork(); //create child process
    if (pid[i] < 0) {
      //fork failed
      printf("Creating child process failed\n");
      exit(1);
    }else if(pid[i] == 0){

        //child process
        if(strcmp(*args[i],"")==0){
          //empty command do nothing
          exit(1); //close child process
        }
        if((strcmp(*args[i],"quit"))==0){
            //quit entered, change a global flag so mother ends the programm
            *glob_var=1;
            exit(1);
          }else{
            if(execvp(*args[i], args[i])==-1){ //try to execute command
              //if command doesnt exist (returned -1)
              printf("Unknown command\n");
              exit(1);
            }else{
              //if command exists
              perror(*args[i]);
              exit(1);
            }
          }

    }else{
      //mother process
      mother=1;
    }
  }

  if(mother==1){
    //mother process
    for(int i=0;i<commands;i++){
      //wait for all commands to be executed
      do {
        waitpid(pid[i], &status[i], WUNTRACED);
      }  while (!WIFEXITED(status[i]) && !WIFSIGNALED(status[i]));
    }
    if((*glob_var)==1){
      //if quit was entered terminate programm after all commands were executed
      return 0;
    }else{
      return 1; //else keep going bro
    }
  }
  return 1; //for no warnings
}
