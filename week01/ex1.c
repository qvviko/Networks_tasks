#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

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
    if (stack == NULL || size_of_the_stack == 0) {
        exit(EXIT_FAILURE);
    }
    return stack->data;
}

/*
 * Push element onto a stack
 */
void push(int data) {
    if (stack == NULL) {
        //No stack exists
        exit(EXIT_FAILURE);
    }
    //Create new node
    struct Node *new_node = (struct Node *) malloc(sizeof(struct Node));
    if (new_node == NULL) {
        exit(EXIT_FAILURE);
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
    if (stack == NULL || size_of_the_stack) {
        //No stack exists
        exit(EXIT_FAILURE);
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
    printf("Size of the stack is: %d", size_of_the_stack);
};

int main(void) {
    int pid = fork();

    if (pid > 0) {
        // Parent Process - Client
    } else if (pid == 0) {
        // Child Process - Server
    } else {
        exit(EXIT_FAILURE);
    }
}


/*
 *     int pid = fork(), n = 811;
    if (pid > 0) {
        printf("Hello from parent [%d - %d]\n", pid, n);
    } else if (pid == 0) {
        printf("Hello from child [%d - %d]\n", pid, n);
    } else {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
 */