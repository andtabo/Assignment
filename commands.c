//
// Created by andre on 18/05/19.
//
#define _GNU_SOURCE
//#define _POSIX_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>


void commandline(char *pmpt , char* line);


#define MAX_ARGS 255


char *command = NULL;
char *commands[MAX_ARGS];
char *direction = NULL;
char *directions[MAX_ARGS];
char *token = NULL;
char *args[MAX_ARGS];

int commandIndex;
int directionIndex;
int tokenIndex;

int savestdio = 0;
int savestdo = 0;
int rw = 0;



void HandleOneTokenCommands(char *token){
#define VARS 3

    // =================================================================================================================
    //---------------------------------- Check which function is required using string compare -------------------------
    // =================================================================================================================
    if(strcmp(token , "exit") == 0){                            // exit function
        exit(0);
    }
    else if(strcmp(token , "all") == 0){                        // print all function
        printall();
    }
    else{                                                       // variable assignment " = "
        char *minitoken = NULL, *args[VARS];
        int tokenIndex;

        // tokenize using = to get which env var in arg 1 and the value it will be set to in arg 2.
        minitoken = strtok(token, "=");
        for (tokenIndex = 0; minitoken != NULL && tokenIndex < VARS; tokenIndex++) {
            args[tokenIndex] = minitoken;
            minitoken = strtok(NULL, "=");
        }
        if(tokenIndex == 2){        //check if value is a reference to another env var

            if(args[1][0] == '$'){
                int length = strlen(args[1]);
                int index = 0;
                char *variablename[length];

                for(int i=1;i<length;i++)
                {
                    variablename[index]=&args[1][i];            //get reference name
                    index++;
                }
                char *variablevalue = getvar(*variablename);    // get  reference value
                if(strcmp(variablevalue,"NOT FOUND") == 0){     //if invalid reference
                    printf("Variable Assignment Failed\n");
                    fflush(stdout);
                }else{
                    updatevar(args[0], variablevalue);          //update env val with ref val
                }
            }else{
                //printf("%s", args[1]);
                updatevar(args[0], args[1]);                    //update env val with user input
            }
        }else{
            //--------------------------- using command binaries ------------------------------------------------------
            pid_t pid;
            int status;

            args[1] = NULL;
            //args[2] = NULL;

            //printf("I will try another binary \n");
            if ((pid = fork()) < 0)
                perror("fork() error");
            else if (pid == 0) {
                // ---------------------------- CHILD PROCESS -------------------
                int child = getpid();
                //printf("Child of pid '%d' existing\n", child);
                int a = execvp(args[0], args );      //&args[0]
                if(a < 0){
                    printf("Non existent command:\n %s\n '%s' detected\n", args[0] ,args[1]);
                    kill(child, SIGKILL);
                }
                exit(0);
                //----------------------------------------------------------------
            }
            //else{
                wait(NULL);
                //printf("parent has forked child with pid of %d\n", (int) pid);
//                if ((pid = wait(&status)) == -1) {
//                    perror("wait() error");
//                }else{
//                    printf("parent is done waiting\n");
//                }
//                if(WIFEXITED(status)) {
//                    //printf("child exited with status of %d\n", WEXITSTATUS(status));
//                }else if (WIFSIGNALED(status)) {
//                    printf("child was terminated by signal %d\n",WTERMSIG(status));
//                }else if (WIFSTOPPED(status)) {
//                    printf("child was stopped by signal %d\n", WSTOPSIG(status));
//                }else puts("reason unknown for child termination");
//            }
        }

    }

}


void HandleMultiTokenCommands(char *tokens[255], int tokennumber) {

    for (int i = 1; i < tokennumber; i++) {
        //printf("TOKEN OUTPUT: %s\n",tokens[i]);
        if (tokens[i][0] =='$') {               // detect and referenced values by checking first char of each token
            int length = strlen(tokens[i]);
            char *variablename[length];

            for (int j = 1; j < length; j++)            // for; number of characters in each word inputted by the user
            {
                variablename[j-1] = &tokens[i][j];
            }

            char *variablevalue = getvar(*variablename);        // get reference
            if (strcmp(variablevalue, "NOT FOUND") == 0) {     // error if reference is invalid
                //printf("%s", *variablename);
                printf("Variable Assignment Failed\n");
                fflush(stdout);
                break;
            } else {

                strcpy(tokens[i], variablevalue);                // replace $reference with env var

            }
        }
    }


            // ----------------------------------------- PRINT FUNCTION --------------------------------------------------------
    if (strcmp(tokens[0], "print") == 0) {                // print a sentence as inputted by the user (including spaces)
        for (int i = 1; i < tokennumber; i++) {

            printf("%s", tokens[i]);                       // print normal user input
            printf(" ");
        }
        printf("\n");                                           //when finished
        fflush(stdout);

        // ------------------------------------ CHANGE DIRECTORY FUNCTION ----------------------------------------------
    } else if (strcmp(tokens[0], "chdir") == 0) {
        if (tokennumber == 2) {
            int response = chdir(tokens[1]);        //change lib function and check return value
            if (response == 0) {
                updateCWD();                        //if return == success; change current work dir env var
            } else {
                printf("Invalid path given\n");     //if return == fail;  output error
                fflush(stdout);
            }
        } else {
            printf("Invalid Input\n");              //if more than two tokens; error (space located in directory, hence invalid)
            fflush(stdout);
        }
        // ------------------------------------ SOURCE FUNCTION ----------------------------------------------

    } else if (strcmp(tokens[0], "source") == 0) {

        FILE *fp = fopen (tokens[1], "r");      //initialize an object of the type FILE, which contains all the information necessary to control the stream

        char *line=NULL;
        size_t len = 0;
        ssize_t read=0;
        int length;
        int count;

            if (!fp) {          //if not found, display error
                //fprintf (stderr, "error: file open failed '%s'.\n", tokens[1]);
            }else {
                while((read = getline(&line, &len,fp))!= -1) {

                    if(line[read-2] == '\n'){
                        line[read-2] = '\0';
                    }else if(line[read-1] == '\n'){
                        line[read-1] = '\0';
                    }


                    length = strlen(getvar("PROMPT"));
                    char pmpt[length + 1];
                    strcpy(pmpt, getvar("PROMPT"));
                    strcat(pmpt, ">");

                    commandline(pmpt, line);

                    free(line);
                }
            }
    }
    else {
        //-------------------------------- using command binaries ------------------------------------------------------
        pid_t pid;
        int status;

        if ((pid = fork()) < 0)                     //new process is forked and its state is stored in pid
            perror("fork() error");
        else if (pid == 0) {
            // ---------------------------- CHILD PROCESS -------------------
            int child = getpid();                   // retrieve process identifier
            int a = execvp(tokens[0], tokens);      // execute the function and return its exit state

            if (a < 0) {
                //printf("Non existent command '%s'detected, with parameter '%s'.", tokens[0],tokens[1]);
                kill(child, SIGKILL);               //if function returns an error, kill the child
            }
            exit(1);                                //exit successfully
            //----------------------------------------------------------------
        }else do{
            if ((pid = waitpid(pid, &status, WNOHANG)) == -1)
                perror("wait() error");
            else if (pid == 0) {
                //printf("child still processing\n");
                // parent process whilst child is still running
            }
            else {
                if (WIFEXITED(status))
                    printf("child exited with status of %d\n", WEXITSTATUS(status));
                else puts("child did not exit successfully");
            }
        } while (pid == 0);
//                              OPTIONAL EXIT STATE
//            if (WIFEXITED(status)) {
//                printf("Child exited with status of %d\n", WEXITSTATUS(status));
//            } else if (WIFSIGNALED(status)) {
//                printf("child was terminated by signal %d\n", WTERMSIG(status));
//            } else if (WIFSTOPPED(status)) {
//                printf("child was stopped by signal %d\n", WSTOPSIG(status));
//            } else puts("reason unknown for child termination");

    }
}



void commandline( char *pmpt , char* line ) {

    // ========================== Check if there are multiple commands via pipe "|" ================================
    command = strtok(line, "|");    //breaks string into a series of tokens using pipe delimiter and returns first token

    for (commandIndex = 0; command != NULL && commandIndex < MAX_ARGS - 1; commandIndex++) {
        commands[commandIndex] = command;                   //store each returned token in an array
        command = strtok(NULL, "|");                        //get next token
    }
    commands[commandIndex] = NULL;                          //Null terminate the array of tokens (commands)
//

//char ** commands2 = comman
//printf("Command1: %s\n",commands[0]);
//printf("Command2: %s\n",commands[1]);
//printf("Command3: %s\n",commands[2]);
// ======= Creation of pipe

    int pipefd[2];
    //pipe(pipefd);
    pipefd[0] = open("test", O_RDWR | O_CREAT);
    pipefd[1] = open("test", O_RDWR | O_CREAT);

    if (pipe(pipefd)==-1)
    {
        fprintf(stderr, "Pipe Failed" );
    }

    // ======= For each command detected;
    for(int i = 0; i < commandIndex; i++) {

        if(i == 1) {

        }
        //=========execute on the preset pipe structure to connect different commands



        if(commandIndex > 1) {

            if (i > 1){
                commands[i] = commands[i] + 1;
            }

                if( i == 0) {                               //output to pipe
                    printf("start pipe system %d \n",i);
                    printf("command : %s \n", commands[i]);
                    savestdo = dup(1);                  // save terminal stdout

                    dup2(pipefd[1],STDOUT_FILENO);      // redirect stdout to output to pipe
                    //printf("testing file test");
                    //fflush(stdout);
                }
                else if ( i == 1){

                    savestdio = dup(0);                 // save terminal stdin
                    //printf("command : %s \n", commands[i]);

                    //close(0);
                    //dup(pipefd[0]);
                    dup2(pipefd[0], STDIN_FILENO);      // set stdin to read from pipe
                    //strcpy(commands[1], "tr : '\\n'");

                    //printf("in pipe system %d \n",i);

                    if (i == (commandIndex -1)) {         //output to terminal but read from pipe
                        //printf("last pipe in the system %d \n",i);
                        //dup(1);
                        dup2(savestdo, 1);      //restore terminal output
                        //printf("command : %s \n", commands[i]);

                        //commands[2] = "grep usr";
                    }
                }else if (i == (commandIndex -1)) {         //output to terminal but read from pipe
                    //dup(1);
                    dup2(savestdo, 1);      //restore terminal output
                    printf("last pipe in the system %d \n",i);
                    //printf("command : %s \n", commands[i]);
                }else{
                    //printf("command : %s \n", commands[i]);//output and read to and from the pipe
                    //printf("in pipe system %d \n",i);
                }
        }


        int direL = 0;
        char d = 'o';
        int lineL = strlen(commands[i]);
        int overwrite = 0;
        //printf("TOKENIZING %s\n",commands[i]);

        direction = strtok(commands[i], "<");    //breaks string into a series of tokens using direction delimiter and returns first token

        for (directionIndex = 0; direction != NULL && directionIndex < MAX_ARGS - 1; directionIndex++) {
            directions[directionIndex] = direction;                   //store each returned token in an array

            direL = direL + strlen(direction);
            d = 'r';
            //printf("Directions %d has %s\n", directionIndex, directions[directionIndex]);
            direction = strtok(NULL, "<");                            //get next token
        }
        if(directionIndex == 1)             // if did not find '<'; try to find '>'
        {
            direL = 0;
            //printf("commands of %d has: %s\n",i,commands[i]);

            direction = strtok(commands[i], ">");    //breaks string into a series of tokens using pipe delimiter and returns first token

            for (directionIndex = 0; direction != NULL && directionIndex < MAX_ARGS - 1; directionIndex++) {
                directions[directionIndex] = direction;                   //store each returned token in an array
                direL = direL + strlen(direction);
                d = 'w';
                direction = strtok(NULL, ">");
            }
        }
        overwrite = lineL - direL;                  // find the how many direction symbols were found.

        if(overwrite == 2){
            d = 'a';
        }
        if(overwrite > 0 && overwrite < 3){
            if(d == 'r'){                           // prepare for read
                directions[1] = directions[1]+1;
                int file_desc = open(directions[1], O_RDWR);
                savestdio = dup(0);                 //relocate 'read from terminal' to lowest free file descriptor
                rw = 0;
                read(file_desc, directions[1], 255);
                dup2(file_desc,rw);

                strcat(directions[0], directions[1]);

            }else if(d == 'w'){
                directions[1] = directions[1]+1;
                int file_desc = open(directions[1], O_RDWR | O_CREAT);
                savestdio = dup(1);
                rw = 1;
                dup2(file_desc,rw);

            }else if(d == 'a'){
                directions[1] = directions[1]+1;
                int file_desc = open(directions[1], O_RDWR | O_APPEND | O_CREAT);
                savestdio = dup(1);
                rw = 1;
                dup2(file_desc,rw);
            }
        }else if(overwrite == 3){
            directions[1] = directions[1]+1;
            strcat(directions[0], directions[1]);
        }

        directions[directionIndex] = NULL;

        //printf("%s",directions[0]);
        //Null terminate the array of  (commands)



        // ================ For each command detected; Collect groups of char separated by a space =====================

        token = strtok(directions[0], " ");

        //printf("Input data before'<' contains: %s\n",directions[0]);
        for (tokenIndex = 0; token != NULL && tokenIndex < MAX_ARGS - 1; tokenIndex++) {
            args[tokenIndex] = token;
            token = strtok(NULL, " ");

        }
        args[tokenIndex] = NULL;                            //Null terminate array of tokens (words)
        // -------------------------- Handle commands according to number of arguments ----------------------------

        if (tokenIndex == 1) {
            HandleOneTokenCommands(args[0]);                //handle single token commands; eg. USER=newUser
        } else {
            // printf("args: %s\n", args[0]);
            //printf("args: %s\n", args[1]);
            //printf("args: %s\n", args[2]);

            HandleMultiTokenCommands(args, tokenIndex);     //handle multi token commands; eg.
        }
    }


    strcpy(pmpt, getvar("PROMPT"));
    strcat(pmpt,">");

    //printf("%d",rw);
    if(savestdio != 0 )
        dup2(savestdio,rw);      //restores stdin or stdout to terminal

    if(commandIndex > 1) {
        close(pipefd[0]);       //close both pipe ends
        close(pipefd[1]);
    }
}

