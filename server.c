#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "friends.h"
#include "error.h"
#include <sys/time.h>
#include <sys/types.h>

#ifndef PORT
  #define PORT 53315
#endif


/************************************************************************
NOTE:
I have used snippets of code from muffin map but have tried to modify it 
where I felt it was neccessary. I have also implemented my own code
************************************************************************/

int setup(void) {
  int on = 1, status;
  struct sockaddr_in self;
  int listenfd;
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

  // Make sure we can reuse the port immediately after the
  // server terminates.
  status = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
    (const char *) &on, sizeof(on));
  if(status == -1) {
    perror("setsockopt -- REUSEADDR");
  }

  self.sin_family = AF_INET;
  self.sin_addr.s_addr = INADDR_ANY;
  self.sin_port = htons(PORT);
  memset(&self.sin_zero, 0, sizeof(self.sin_zero));  // Initialize sin_zero to 0

  if (bind(listenfd, (struct sockaddr *)&self, sizeof(self)) == -1) {
    perror("bind"); // probably means port is in use
    exit(1);
  }

  if (listen(listenfd, 5) == -1) {
    perror("listen");
    exit(1);
  }
  return listenfd;
}

/*Search the first inbuf characters of buf for a network newline ("\r\n").
  Return the location of the '\r' if the network newline is found,
  or -1 otherwise.
  Definitely do not use strchr or any other string function in here. (Why not?)
*/

  int find_network_newline(const char *buf, int inbuf) {
  // Step 1: write this function
   for (int i = 0; i < inbuf; i++){
    if ((buf[i] == '\r'))
     return i;
 }
  	return -1; // return the location of '\r' if found
  }

  int main() {
    
    int listenfd, nbytes, tokens_ret, where;
    char *cmd_argv_temp[INPUT_ARG_MAX_NUM]; // used to store the tokens 
    int delete_client = 0; // used to show if a client has been deleted or not

  // Create the heads of the empty data structure
  User *user_list = NULL;
  Client *client_list = NULL; 
  Client *p;

  listenfd = setup();

  // took the next couple of bits from muffin man
  while (1) {
    fd_set fdlist;
    int maxfd = listenfd;
    FD_ZERO(&fdlist);
    FD_SET(listenfd, &fdlist);
    for (p = client_list; p; p = p->next) {
      FD_SET(p->fd, &fdlist);
      if (p->fd > maxfd)
        maxfd = p->fd;
    }
    if (select(maxfd + 1, &fdlist, NULL, NULL, NULL) < 0) {
      perror("select");
    } 
    else {
      if (FD_ISSET(listenfd, &fdlist)){
        creating_new_client(&client_list, listenfd);
      }
      for (p = client_list; p; p = p->next)
        if (FD_ISSET(p->fd, &fdlist))
          break;
    /*
     * it's not very likely that more than one client will drop at
     * once, so it's not a big loss that we process only one each
     * select(); we'll get it later...
     */
      if (p){
        nbytes = read_err(p->fd, p->after, p->room);
        delete_client = 0;
        p->inbuf += nbytes;
        where = find_network_newline(p->buffer, p->inbuf);
        if (where >= 0){
          p->buffer[where] = '\0';
          if (strlen(p->name) == 0){

            if(strlen(p->buffer) < MAX_NAME)
              strncpy(p->name, p->buffer, MAX_NAME);
            else{
              // error message
              // hold the error message
              char error_message[128];
              sprintf(error_message, "Name truncated to %d characters!\r\n", MAX_NAME - 1);
              write_err(p->fd, error_message, 19*sizeof(char));
              strncpy(p->name, p->buffer, MAX_NAME - 1);
              p->buffer[MAX_NAME - 1] = '\0';
            }

            char *new_welc_msg = "Welcome.\r\nGo ahead and enter user commands\r\n";
            char *welc_back_msg = "Welcome back.\r\nGo ahead and enter user commands\r\n";
            int create_user_return = create_user(p->name, &user_list);        
            if (create_user_return == 1){
              write_err(p->fd, welc_back_msg, strlen(welc_back_msg) * sizeof(char));
            }
            else if (create_user_return == 0) {
              write_err(p->fd, new_welc_msg, strlen(new_welc_msg) * sizeof(char));
            } 
          }
          else {
            tokens_ret = tokenize(p->buffer, cmd_argv_temp);
            int ret_val = process_args(tokens_ret, cmd_argv_temp, &user_list, 
              p->name, p->fd);
            switch(ret_val){
              case 3:
                // need to write a message saying "You are already friends\r\n"
                write_err(p->fd, "You are already friends\r\n", 
                strlen("You are already friends\r\n") * sizeof(char));
                break;
              case 4:
                // write "Either you or your friend has the max number of friends.\r\n"
                write_err(p->fd, "Either you or your friend has the max number of friends.\r\n", 
                strlen("Either you or your friend has the max number of friends.\r\n") * sizeof(char));
                break;
              case 5:
                // "You can't friend yourself\r\n"
                write_err(p->fd, "You can't friend yourself\r\n", 
                strlen("You can't friend yourself\r\n") * sizeof(char));
                break;
              case 6:
                //"The user you entered does not exist\r\n"
                write_err(p->fd, "The user you entered does not exist\r\n", 
                strlen("The user you entered does not exist\r\n") * sizeof(char));
                break;
              case 7:
                //"You can only post to your friends\r\n"
                write_err(p->fd, "You can only post to your friends\r\n", 
                strlen("You can only post to your friends\r\n") * sizeof(char));
                break;
              case 8:
                //"The user you want to post to does not exist\r\n"
                write_err(p->fd, "The user you want to post to does not exist\r\n", 
                strlen("The user you want to post to does not exist\r\n") * sizeof(char));
                break;
              case 9:
                // "User not found\r\n"
                write_err(p->fd, "User not found\r\n", 
                strlen("User not found\r\n") * sizeof(char));
                break;
              case -1:
                removeclient(p, &client_list);
                delete_client = 1;
                break; // CAN I DO THIS HERE?
              case 20:
                break; // this means command was done successfully
              default:
                // "Incorrect syntax\r\n";
                write_err(p->fd, "Incorrect syntax\r\n", 
                strlen("Incorrect syntax\r\n") * sizeof(char));
                break;
            }
          }
          if (!delete_client){
            memmove(p->buffer, p->buffer + where + 2, INPUT_BUFFER_SIZE - (where+2));
            memset(p->buffer + (INPUT_BUFFER_SIZE - (where+2)), 0, where + 2);
            p->inbuf -= (where + 2);           
          }
        }
        if (!delete_client){
          p->room = INPUT_BUFFER_SIZE - p->inbuf;
          p->after = p->buffer + p->inbuf;
        }
      }
        }
    }
  return 0;
}


