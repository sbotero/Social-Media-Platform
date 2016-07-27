#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "friends.h"
#include "error.h"
#include <sys/time.h>
#include <sys/types.h>

#define INPUT_BUFFER_SIZE 256
#define INPUT_ARG_MAX_NUM 12
#define DELIM " \n"


/* 
 * Print a formatted error message to stderr.
 */
void error(char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

/* 
 * Read and process commands
 * Return:  -1 for quit command
 *          0 otherwise
 */

 ////////NEEED TO EDIT THIS/////////////////////////
int process_args(int cmd_argc, char **cmd_argv, User **user_list_ptr, char *name, 
    int client_fd) {
    User *user_list = *user_list_ptr;

    if (cmd_argc <= 0) {
        return 0;
    } else if (strcmp(cmd_argv[0], "quit") == 0 && cmd_argc == 1) {
        return -1;

    } else if (strcmp(cmd_argv[0], "list_users") == 0 && cmd_argc == 1) {
        char *ret_list = list_users(user_list);
        //printf("%s\n", ret_list); // will need to comment this out later
        // need to write to client with this return value
        write_err(client_fd, ret_list, (strlen(ret_list) * sizeof(char)) + 1);
        free(ret_list);
        return 20;

    } else if (strcmp(cmd_argv[0], "make_friends") == 0 && cmd_argc == 2) {
        switch (make_friends(cmd_argv[1], name, user_list)) {
            case 0:
                return 20;
            case 1:
                //error("users are already friends");
                return 3;
                break;
            case 2:
                //error("at least one user you entered has the max number of friends");
                return 4;
                break;
            case 3:
                //error("you must enter two different users");
                return 5;
                break;
            case 4:
                //error("at least one user you entered does not exist");
                return 6;
                break;
        }
    } else if (strcmp(cmd_argv[0], "post") == 0 && cmd_argc >= 3) {
        // first determine how long a string we need
        int space_needed = 0;
        for (int i = 3; i < cmd_argc; i++) {
            space_needed += strlen(cmd_argv[i]) + 1;
        }

        // allocate the space
        char *contents = malloc(space_needed);
        if (contents == NULL) {
            perror("malloc");
            exit(1);
        }

        // copy in the bits to make a single string
        strcpy(contents, cmd_argv[3]);
        for (int i = 4; i < cmd_argc; i++) {
            strcat(contents, " ");
            strcat(contents, cmd_argv[i]);
        }

        User *author = find_user(name, user_list);
        User *target = find_user(cmd_argv[1], user_list);
        switch (make_post(author, target, contents)) {
            case 0:
                return 20;
            case 1:
                //error("the users are not friends");
                return 7;
                break;
            case 2:
                //error("at least one user you entered does not exist");
                return 8;
                break;
        }
    } else if (strcmp(cmd_argv[0], "profile") == 0 && cmd_argc == 2) {
        User *user = find_user(cmd_argv[1], user_list);
        char *ret_pr_user = print_user(user);
        if (ret_pr_user == NULL) {
            free(ret_pr_user);
            return 9;
        }
        else {
            // need to write the contents of ret_pr_user to client
            //printf("%s\n", ret_pr_user);
            write_err(client_fd, ret_pr_user, (strlen(ret_pr_user) * sizeof(char)) + 1);
            free(ret_pr_user);
            return 20;
            // have a return value indicating everything was written succesfully
        }
    }
    else {
        //error("Incorrect syntax");
        // write to client saying incorrect syntax
        return 10;
    }
    return 0;
}


/*
 * Tokenize the string stored in cmd.
 * Return the number of tokens, and store the tokens in cmd_argv.
 * username_stored is passed in incase the username needs to be inputted
 * based on certain functions signatures
 */
int tokenize(char *cmd, char **cmd_argv) {
    int cmd_argc = 0;
    char *next_token = strtok(cmd, DELIM);    
    while (next_token != NULL) {
        if (cmd_argc >= INPUT_ARG_MAX_NUM - 1) {
            error("Too many arguments!"); // <--- need to change this syntax error
            cmd_argc = 0;
            break;
        }
        cmd_argv[cmd_argc] = next_token;
        cmd_argc++;
        next_token = strtok(NULL, DELIM);
    }

    /*// need to insert the username into the cmd_argv if the first argument
    // is make_friends or make_post
    if ((strcmp(cmd_argv[0], "make_friends") == 0) || 
        (strcmp(cmd_argv[0], "make_post") == 0)){

        // need to shift everything one step to the right in the array apart
        // from the first string
        for (int i = cmd_argc; i > 1; i--){
            cmd_argv[cmd_argc] = cmd_argv[cmd_argc - 1];
        }
        // put in the username into cmd_argv[1]
        cmd_argv[1] = username_stored;
        cmd_argc++;
    }*/


    return cmd_argc;
}

 void creating_new_client(Client **ptr_to_head, int listenfd){

    //Client *temp = *ptr_to_head;

    Client *p = malloc(sizeof(struct client));
    if ((p->fd = accept(listenfd, NULL, NULL)) < 0) {
      perror("accept");
      exit(1);
    } 
    p->name[0] = '\0';
    p->next = *ptr_to_head;
    *ptr_to_head = p;
    p->inbuf = 0;
    p->room = INPUT_BUFFER_SIZE;
    memset(p->buffer, 0, INPUT_BUFFER_SIZE);
    p->after = p->buffer;
    write_err(p->fd, "What is your user name?\r\n", 25*sizeof(char));
}

void removeclient(Client *p, Client **head)
{
    if (*head == p){
        (*head) = p->next;
        p->next = NULL;
        close(p->fd);
        free(p);
    }
    else {

        Client *traverser = *head;
        while (traverser != NULL && traverser->next != p){
            traverser = traverser->next;
        }
        if (traverser){
            traverser->next = p->next;
            p->next = NULL;
            close(p->fd);
            free(p);
        }
    }
}



