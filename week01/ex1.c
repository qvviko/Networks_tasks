#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define TRUE 1
#define BUF_SIZE 1024
struct Node {
    int data;
    struct Node *next;
    struct Node *prev;
};

int peek();

void push(int data);

void pop();

int empty();

void display();

void create();

void stack_size();

struct Node *stack = NULL; //Our stack, Empty at the beginning
static int size_of_the_stack = 0;

/*
 * Peek at the top element
 */
int peek() {
    if (stack == NULL) {
        printf("Stack is not created");
    } else if (!size_of_the_stack) {
        printf("Stack is empty");
    }
    return stack->data;
}

/*
 * Push element onto a stack
 */
void push(int data) {
    if (stack == NULL) {
        //No stack exists
        printf("Stack is not created\n");
    }
    //Create new node
    struct Node *new_node = (struct Node *) malloc(sizeof(struct Node));
    if (new_node == NULL) {
        printf("No space for the new node");
    }
    new_node->data = data;
    new_node->prev = NULL;
    new_node->next = NULL;

    //Change state of the stack
    if (size_of_the_stack == 0) {
        stack = new_node;
    } else {
        struct Node *old = stack;
        stack->prev = new_node;
        new_node->next = stack;
        stack = new_node;
    }

    ++size_of_the_stack;
}

/*
 * Pop top element from stack
 */
void pop() {
    if (stack == NULL) {
        //No stack exists
        printf("Stack is not created");
    } else if (!size_of_the_stack) {
        printf("Stack is empty");;
    } else {
        struct Node *old_node = stack;
        stack = stack->next;
        free(old_node);

        if (stack != NULL) {
            stack->prev = NULL;
        } else {
            //If stack is now empty create free Node
            stack = (struct Node *) malloc(sizeof(struct Node));
        }

    }
    --size_of_the_stack;
}

/*
 * Check if stack is empty, 0 if not and 1 if yes
 */
int empty() {
    return size_of_the_stack == 0;
}

void display() {
    if (stack == NULL) {
        printf("Stack is not created");
    } else if (size_of_the_stack == 0) {
        printf("Stack is empty");
    } else {
        struct Node *cur = stack;
        printf("Current state of the stack is: ");
        do {
            printf("%d ", cur->data);
            cur = cur->next;
        } while (cur != NULL);
    }
}

/*
 * Creates empty stack
 */
void create() {
    if (stack == NULL) {
        //If stack is empty
        stack = (struct Node *) malloc(sizeof(struct Node));
    } else {
        //If stack is not empty - clear
        while (!empty()) {
            pop();
        }
    }
}

void stack_size() {
    if (stack != NULL) {
        printf("Size of the stack is: %d", size_of_the_stack);
    } else {
        printf("Stack is not created");
    }
};

int main(void) {
    int fds[2];
    char buf[BUF_SIZE];
    if (pipe(fds)) {
        printf("Error in pipe creation");
        exit(EXIT_FAILURE);
    }

    int pid = fork();
    if (pid == 0) {
        // Parent Process - Client
        close(fds[0]); // Close read side

        while (TRUE) {
            printf(">");
            fgets(buf, BUF_SIZE, stdin);
            buf[strlen(buf) - 1] = '\0';
            write(fds[1], &buf, sizeof(char) * BUF_SIZE);
        }
    } else if (pid > 0) {
        // Child Process - Server
        close(fds[1]); // Close write side

        while (TRUE) {

            read(fds[0], &buf, sizeof(char) * BUF_SIZE);
            if (!strcmp(buf, "create")) {
                create();
                printf("Stack is created\n");
            } else if (!strcmp(buf, "push")) {
                int data;
                sscanf(buf, "push %d", &data);
                push(data);
                printf("%d is pushed onto the stack", data);
            } else if (!strcmp(buf, "pop")) {
                pop();
                printf("Top element popped");
            } else if (!strcmp(buf, "peek")) {
                int data = peek();
                printf("%d is on top of the stack", data);
            } else if (!strcmp(buf, "empty")) {
                if (empty()) {
                    printf("Stack is empty\n");
                } else {
                    printf("Stack is not empty\n");
                }
            } else if (!strcmp(buf, "display")) {
                display();
            } else if (!strcmp(buf, "size")) {
                stack_size();
            } else if (!strcmp(buf, "help")) {
                printf("Possible commands are:\n");
                printf("help - show this\n");
                printf("create - creates stack\n");
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