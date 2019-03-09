#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define TRUE 1
#define FALSE 0
#define BUF_SIZE 1024

// Skeleton for the stack
struct PeerNode {
    int data;
    struct PeerNode *next;
    struct PeerNode *prev;
};

int peek();

void push(int data);

void pop();

int empty();

void display();

void create();

void stack_size();

struct PeerNode *stack = NULL; //Our stack, Empty at the beginning
static int size_of_the_stack = 0;

/*
 * Check if str starts with beginning
 */
int starts_with(char *str, char *beginning) {
    size_t len1 = strlen(str), len2 = strlen(beginning);
    if (len2 > len1) {
        return FALSE;
    }
    int res = TRUE;
    for (int i = 0; i < len2; ++i) {
        res = res * (str[i] == beginning[i]);
    }
    return res;
}

/*
 * Peek at the top element
 */
int peek() {
    if (stack == NULL) {
        printf("Stack is not created\n");
    } else if (!size_of_the_stack) {
        printf("Stack is empty\n");
    } else {
        printf("%d is on top of the stack\n", stack->data);

        return stack->data;
    }
}

/*
 * Push element onto a stack
 */
void push(int data) {
    if (stack == NULL) {
        //No stack exists
        printf("Stack is not created\n");
    } else {
        //Create new node
        struct PeerNode *new_node = (struct PeerNode *) malloc(sizeof(struct PeerNode));
        if (new_node == NULL) {
            printf("No space for the new node\n");
        }
        new_node->data = data;
        new_node->prev = NULL;
        new_node->next = NULL;

        //Change state of the stack
        if (size_of_the_stack == 0) {
            stack = new_node;
        } else {
            struct PeerNode *old = stack;
            stack->prev = new_node;
            new_node->next = stack;
            stack = new_node;
        }

        ++size_of_the_stack;
        printf("%d is pushed onto the stack\n", data);

    }

}

/*
 * Silent pop without notifications
 */
void silent_pop() {
    if (stack == NULL) {
        //No stack exists
        return;
    } else if (!size_of_the_stack) {
        return;
    } else {
        struct PeerNode *old_node = stack;
        stack = stack->next;
        free(old_node);

        if (stack != NULL) {
            stack->prev = NULL;
        } else {
            //If stack is now empty create free PeerNode
            stack = (struct PeerNode *) malloc(sizeof(struct PeerNode));
        }
        --size_of_the_stack;

    }
}

/*
 * Pop top element from stack
 */
void pop() {
    if (stack == NULL) {
        //No stack exists
        printf("Stack is not created\n");
    } else if (!size_of_the_stack) {
        printf("Stack is empty\n");;
    } else {
        struct PeerNode *old_node = stack;
        stack = stack->next;
        free(old_node);

        if (stack != NULL) {
            stack->prev = NULL;
        } else {
            //If stack is now empty create free PeerNode
            stack = (struct PeerNode *) malloc(sizeof(struct PeerNode));
        }
        printf("Top element popped\n");
        --size_of_the_stack;

    }
}

/*
 * Check if stack is empty, 0 if not and 1 if yes
 */
int empty() {
    if (stack == NULL) {
        printf("Stack is not created\n");
        return 1;
    } else if (size_of_the_stack == 0) {
        printf("Stack is empty\n");
        return 1;
    } else {
        printf("Stack is not empty\n");
        return 0;
    }
}

/*
 * String representations of the stack
 */
void display() {
    if (stack == NULL) {
        printf("Stack is not created\n");
    } else if (size_of_the_stack == 0) {
        printf("Stack is empty\n");
    } else {
        struct PeerNode *cur = stack;
        printf("Current state of the stack is: \n");
        do {
            printf("%d ", cur->data);
            cur = cur->next;
        } while (cur != NULL);
        printf("\n");
    }
}

/*
 * Creates empty stack
 */
void create() {
    if (stack == NULL) {
        //If stack is empty
        stack = (struct PeerNode *) malloc(sizeof(struct PeerNode));
    } else {
        //If stack is not empty - clear
        while (size_of_the_stack != 0) {
            silent_pop();
        }
    }
    printf("Stack is created\n");
}

/*
 * Output stack size
 */
void stack_size() {
    if (stack != NULL) {
        printf("Size of the stack is: %d\n", size_of_the_stack);
    } else {
        printf("Stack is not created\n");
    }
};

int main(void) {
    int fds[2];
    char buf[BUF_SIZE];
    // Create pipe
    if (pipe(fds)) {
        printf("Error in pipe creation\n");
        exit(EXIT_FAILURE);
    }

    int pid = fork();
    if (pid > 0) {
        // Parent Process - Client
        close(fds[0]); // Close read side

        while (TRUE) {
            // Get command, remove \n n the end
            fgets(buf, BUF_SIZE, stdin);
            buf[strlen(buf) - 1] = '\0';
            // Send command via pipe
            write(fds[1], &buf, sizeof(char) * BUF_SIZE);
        }
    } else if (pid == 0) {
        // Child Process - Server
        close(fds[1]); // Close write side

        while (TRUE) {
            // Read from pipe (suspends if pipe is empty)
            read(fds[0], &buf, sizeof(char) * BUF_SIZE);

            // Proceed with commands
            printf("-- ");
            if (!strcmp(buf, "create")) {
                create();
            } else if (starts_with(buf, "push")) {
                int data;
                sscanf(buf, "push %d", &data);
                push(data);
            } else if (!strcmp(buf, "pop")) {
                pop();
            } else if (!strcmp(buf, "peek")) {
                int data = peek();
            } else if (!strcmp(buf, "empty")) {
                empty();
            } else if (!strcmp(buf, "display")) {
                display();
            } else if (!strcmp(buf, "size")) {
                stack_size();
            } else if (!strcmp(buf, "help")) {
                printf("Possible commands are:\n");
                printf("help - show this\n");
                printf("create - creates stack\n");
                printf("peek - peek at the first element of the stack\n");
                printf("push X - pushes integer X onto the stack\n");
                printf("pop - removes top element from the stack\n");
                printf("empty - checks if stack is empty\n");
                printf("display - string representation of the current stack\n");
                printf("size - output current stack size\n");
            } else {
                printf("Wrong command, if you need command print 'help'\n");
            }
        }
    } else {
        exit(EXIT_FAILURE);
    }
}