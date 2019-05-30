//
// Created by andre on 18/05/19.
//

#include <string.h>
#include <stdlib.h>

int variables = 8;
char **valuenames;
char **values;

void updateCWD(){
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    values[2] = cwd;
}

void setvar(){
    valuenames = malloc(8*sizeof(*valuenames));
    values = malloc(8*sizeof(*valuenames));

    valuenames[0] = "PATH";
    valuenames[1] = "PROMPT";
    valuenames[2] = "CWD";
    valuenames[3] = "USER";
    valuenames[4] = "HOME";
    valuenames[5] = "SHELL";
    valuenames[6] = "TERMINAL";
    valuenames[7] = "EXITCODE";

    values[0] = getenv("PATH");
    values[1] = "bajdaqoxra-1.0";
    updateCWD();
    values[3] = getenv("USERNAME");
    values[4] = getenv("HOME");
    //TODO shell location
    values[5] = getenv("SHELL");
    values[6] = getenv("TERM");
    values[7] = "0";

}

char *getvar(char *strname){
    for(int i = 0; i < variables ; i++ ){
        if (strcmp(valuenames[i] , strname ) == 0){
            return values[i];
        }
    }
    return "NOT FOUND";
}

void printall() {
    for (int i = 0; i < variables; i++) {
        printf("%s=%s\n", valuenames[i], values[i]);
        fflush(stdout);

    }
}

void addvar(char *strname, char *val){
    int current_size = sizeof(valuenames);
    int new_size = current_size+1;

    //free(valuenames);

    char **old = realloc(valuenames,new_size*sizeof(*valuenames));
    old[new_size-1] = strname;
    valuenames = old;

    char **oldvalues = realloc(values,new_size*sizeof(*values));
    oldvalues[new_size-1] = val;
    values = oldvalues;

    variables++;

}

void updatevar(char *strname, char *val){
    //printf(" start of update var: %s",val);

    for(int i = 0; i < variables ; i++ ){
        if (strcmp(valuenames[i] , strname ) == 0){
            strcpy(values[i],val);
            //printf("values :%s",valuenames[i]);
            //printf("val :%s",val);

            return;
        }
    }
    addvar(strname, val);
}

void my_forkexec(char file[20],pid_t PID){
    if (PID == 0){
        printf("Child process: \n");
        fflush(stdout);

        execvp(file, values);
    }
}