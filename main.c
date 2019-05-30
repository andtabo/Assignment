#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "ExternalFiles/linenoise-master/linenoise.c"
#include "variables.c"
#include "commands.c"
#include "commandline.c"
//including personal and public libraries

// =================================================================================================================
// ----------------------------------------------- Init Global Variables -----------------------------------------------
// =================================================================================================================
char *readline;

int main(int argc, char **argv){

    setvar();       //creates the specified environmental variables from setvar function located in "variables.c"

    // =================================================================================================================
    // --------- while loop in which our prompt will accept inputs and carry out the requried functions ----------------
    // =================================================================================================================

    //while ((line = linenoise(getvar("PROMPT"))) != NULL)

    int length = strlen(getvar("PROMPT"));
    char pmpt[length+1];
    strcpy(pmpt, getvar("PROMPT"));
    strcat(pmpt,">");

    while ((readline = linenoise(pmpt)) != NULL)
    {
        commandline(pmpt, readline);

//        // ========================== Check if there are multiple commands via pipe "|" ================================
//        command = strtok(line, "|");    //breaks string into a series of tokens using pipe delimiter and returns first token
//
//        for (commandIndex = 0; command != NULL && commandIndex < MAX_ARGS - 1; commandIndex++) {
//            commands[commandIndex] = command;                   //store each returned token in an array
//            command = strtok(NULL, "|");                        //get next token
//        }
//        commands[commandIndex] = NULL;                          //Null terminate the array of tokens (commands)
//
//        // ================ For each command detected; Collect groups of char separated by a space =====================
//        for(int i = 0 ; i < commandIndex; i++) {
//            token = strtok(commands[i], " ");
//
//            for (tokenIndex = 0; token != NULL && tokenIndex < MAX_ARGS - 1; tokenIndex++) {
//                args[tokenIndex] = token;
//                token = strtok(NULL, " ");
//            }
//
//            args[tokenIndex] = NULL;                            //Null terminate array of tokens (words)
//
//            // -------------------------- Handle commands according to number of arguments -----------------------------
//            if (tokenIndex == 1) {
//                HandleOneTokenCommands(args[0]);                //handle single token commands; eg. USER=newUser
//            }
//            else{
//                HandleMultiTokenCommands(args, tokenIndex);     //handle multi token commands; eg.
//            }
//        }
//        strcpy(pmpt, getvar("PROMPT"));
//        strcat(pmpt,">");
    }

    linenoiseFree(readline);                           // Free allocated memory
    // =================================================================================================================
}