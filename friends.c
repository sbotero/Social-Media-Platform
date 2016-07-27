#include "friends.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "error.h"

/*
 * Create a new user with the given name.  Insert it at the tail of the list 
 * of users whose head is pointed to by *user_ptr_add.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if a user by this name already exists in this list.
 *   - 2 if the given name cannot fit in the 'name' array
 *       (don't forget about the null terminator).
 */
int create_user(const char *name, User **user_ptr_add) {
    if (strlen(name) >= MAX_NAME) {
        return 2;
    }

    User *new_user = malloc(sizeof(User));
    if (new_user == NULL) {
        perror("malloc");
        exit(1);
    }
    strncpy(new_user->name, name, MAX_NAME); // name has max length MAX_NAME - 1

    for (int i = 0; i < MAX_NAME; i++) {
        new_user->profile_pic[i] = '\0';
    }

    new_user->first_post = NULL;
    new_user->next = NULL;
    for (int i = 0; i < MAX_FRIENDS; i++) {
        new_user->friends[i] = NULL;
    }

    // Add user to list
    User *prev = NULL;
    User *curr = *user_ptr_add;
    while (curr != NULL && strcmp(curr->name, name) != 0) {
        prev = curr;
        curr = curr->next;
    }

    if (*user_ptr_add == NULL) {       // bug fixed 03/04/2016. Now correct on repeat of 1st name
        *user_ptr_add = new_user;
        return 0;
    } else if (curr != NULL) {
        free(new_user);
        return 1;
    } else {
        prev->next = new_user;
        return 0;
    }
}


/* 
 * Return a pointer to the user with this name in
 * the list starting with head. Return NULL if no such user exists.
 *
 * NOTE: You'll likely need to cast a (const User *) to a (User *)
 * to satisfy the prototype without warnings.
 */
User *find_user(const char *name, const User *head) {
/*    const User *curr = head;
    while (curr != NULL && strcmp(name, curr->name) != 0) {
        curr = curr->next;
    }

    return (User *)curr;
*/
    while (head != NULL && strcmp(name, head->name) != 0) {
        head = head->next;
    }

    return (User *)head;
}


/*
 * Print the usernames of all users in the list starting at curr.
 * Names should be printed to standard output, one per line.
 */
char *list_users(User *curr) {
    int total_length = strlen("User List\r\n");

    User *counting_curr = curr;
    // loop through to get the total amount of characters
    while (counting_curr != NULL){
        // add 3 for '\t', '\n', '\r'
        total_length += 3;
        // add the length of the name
        total_length += strlen(counting_curr->name);
        counting_curr = counting_curr->next;
    }
    // add 1 for the null terminating character
    total_length += 1;

    // initialize a buffer that will hold everything that needs to be returned
    char *buffer;
    malloc_err(&buffer, sizeof(char) * total_length);
    strncpy(buffer, "User List\r\n", strlen("User List\r\n") + 1);

    // loop through to add all the users into buffer
    while(curr != NULL){
        strncat_err(&buffer, "\t", 2);
        strncat_err(&buffer, curr->name, strlen(curr->name) + 1);
        strncat_err(&buffer, "\r\n", 3);
        curr = curr->next;
    }
    return buffer;
}



/* 
 * Make two users friends with each other.  This is symmetric - a pointer to 
 * each user must be stored in the 'friends' array of the other.
 *
 * New friends must be added in the first empty spot in the 'friends' array.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if the two users are already friends.
 *   - 2 if the users are not already friends, but at least one already has
 *     MAX_FRIENDS friends.
 *   - 3 if the same user is passed in twice.
 *   - 4 if at least one user does not exist.
 *
 * Do not modify either user if the result is a failure.
 * NOTE: If multiple errors apply, return the *largest* error code that applies.
 */
int make_friends(const char *name1, const char *name2, User *head) {
    User *user1 = find_user(name1, head);
    User *user2 = find_user(name2, head);

    if (user1 == NULL || user2 == NULL) {
        return 4;
    } else if (user1 == user2) { // Same user
        return 3;
    }

    int i, j;
    for (i = 0; i < MAX_FRIENDS; i++) {
        if (user1->friends[i] == NULL) { // Empty spot
            break;
        } else if (user1->friends[i] == user2) { // Already friends.
            return 1;
        }
    }

    for (j = 0; j < MAX_FRIENDS; j++) {
        if (user2->friends[j] == NULL) { // Empty spot
            break;
        } 
    }

    if (i == MAX_FRIENDS || j == MAX_FRIENDS) { // Too many friends.
        return 2;
    }

    user1->friends[i] = user2;
    user2->friends[j] = user1;
    return 0;
}




/*
 *  Print a post.
 *  Use localtime to print the time and date.
 */
void print_post(char **container, Post *post) {
    if (post != NULL) {

        // add the author to the container
        strncat_err(container, "From: ", strlen("From: ") + 1);
        strncat_err(container, post->author, strlen(post->author) + 1);
        strncat_err(container, "\r\n", strlen("\r\n") + 1);

        // add the date
        strncat_err(container, "Date: ", strlen("Date: ") + 1);
        strncat_err(container, asctime(localtime(post->date)), 
            strlen(asctime(localtime(post->date))) + 1);
        strncat_err(container, "\r\n", strlen("\r\n") + 1);
    
        // add the message
        strncat_err(container, post->contents, strlen(post->contents) + 1);
        strncat_err(container, "\r\n", strlen("\r\n") + 1);
    }
}


/* 
 * Print a user profile.
 * For an example of the required output format, see the example output
 * linked from the handout.
 * Return:
 *   - 0 on success.
 *   - 1 if the user is NULL.
 */
char *print_user(User *user) {

    if (user == NULL) {
      return NULL;
    }

    // loop through to get the total length of all characters 
    int total_length = strlen("Name: \r\n\r\n");
    char *dashed_line = "------------------------------------------\r\n";
    total_length += strlen(user->name);
    total_length += strlen(dashed_line);
    total_length += strlen("Friends:\r\n");
    for (int i = 0; i < MAX_FRIENDS && user->friends[i] != NULL; i++) {
        total_length += strlen("\r\n");
        total_length += strlen(user->friends[i]->name);
    }
    total_length += strlen(dashed_line);
    total_length += strlen("Posts:\r\n");
    Post *counter_curr = user->first_post;
    while (counter_curr != NULL) {
        total_length += strlen("From: \r\n");
        total_length += strlen(counter_curr->author);
        total_length += strlen("Date: \r\n");
        total_length += strlen(asctime(localtime(counter_curr->date)));
        total_length += (strlen(counter_curr->contents) + 2); //2 for '\r', '\n'
        counter_curr = counter_curr->next;
        if(counter_curr != NULL){
            total_length += strlen("\r\n===\r\n\r\n");
        }
    }
    total_length += strlen(dashed_line);
    total_length += 1; // for the null terminating character

    // loop through to put in all the characters into the buffer
    char *buffer;
    malloc_err(&buffer, sizeof(char) * total_length);

    strncpy(buffer, "Name: ", strlen("Name: ") + 1);
    strncat_err(&buffer, user->name, strlen(user->name) + 1);
    strncat_err(&buffer, "\r\n\r\n", strlen("\r\n\r\n") + 1);
    strncat_err(&buffer, dashed_line, strlen(dashed_line) + 1);
    strncat_err(&buffer, "Friends:\r\n", strlen("Friends:\r\n") + 1);
    for (int i = 0; i < MAX_FRIENDS && user->friends[i] != NULL; i++) {
        strncat_err(&buffer, user->friends[i]->name, 
            strlen(user->friends[i]->name) + 1);
        strncat_err(&buffer, "\r\n", strlen("\r\n") + 1);
    }
    strncat_err(&buffer, dashed_line, strlen(dashed_line) + 1);
    strncat_err(&buffer, "Posts:\r\n", strlen("Posts:\r\n") + 1);
    Post *curr = user->first_post;
    while (curr != NULL) {
        print_post(&buffer, curr);
        curr = curr->next;
        if(curr != NULL){
            strncat_err(&buffer, "\r\n===\r\n\r\n", 
                strlen("\r\n===\r\n\r\n") + 1);
        }
    }
    strncat_err(&buffer, dashed_line, strlen(dashed_line) + 1);
    return buffer;
}


/*
 * Make a new post from 'author' to the 'target' user,
 * containing the given contents, IF the users are friends.
 *
 * Insert the new post at the *front* of the user's list of posts.
 *
 * Use the 'time' function to store the current time.
 *
 * 'contents' is a pointer to heap-allocated memory - you do not need
 * to allocate more memory to store the contents of the post.
 *
 * Return:
 *   - 0 on success
 *   - 1 if users exist but are not friends
 *   - 2 if either User pointer is NULL
 */
int make_post(const User *author, User *target, char *contents) {
    if (target == NULL || author == NULL) {
        return 2;
    }

    int friends = 0;
    for (int i = 0; i < MAX_FRIENDS && target->friends[i] != NULL; i++) {
        if (strcmp(target->friends[i]->name, author->name) == 0) {
            friends = 1;
            break;
        }
    }

    if (friends == 0) {
        return 1;
    }

    // Create post
    Post *new_post = malloc(sizeof(Post));
    if (new_post == NULL) {
        perror("malloc");
        exit(1);
    }
    strncpy(new_post->author, author->name, MAX_NAME);
    new_post->contents = contents;
    new_post->date = malloc(sizeof(time_t));
    if (new_post->date == NULL) {
        perror("malloc");
        exit(1);
    }
    time(new_post->date);
    new_post->next = target->first_post;
    target->first_post = new_post;

    return 0;
}

