/*************************************************************
 * Author: Meng-Shiun, Tsai
 * Program: Christmas Tree
    1. Write a program to implement Binary Search Tree (BST).
    2. Use AVL tree to balance the height of BST.
*************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node {
    int number;
    struct node *next;         // For queue;
    struct node *left, *right; // For tree;
}NODE;

typedef struct tree {
    int counter;
    struct node *root;
}TREE;

typedef struct queue {
    struct node *head, *tail;
}QUEUE;

// Deal with Tree
TREE *create_tree();
NODE *create_node(int);
void insert_node(TREE*, NODE*, NODE*);
void delete_node(TREE*, NODE*, NODE*, int);
TREE *insert_from_file();

// Deal with depth-first-search (traversal)
void preorder(FILE*, TREE*, NODE*, int*);
void inorder(FILE*, TREE*, NODE*, int*);
void postorder(FILE*, TREE*, NODE*, int*);

// Deal with Breadth-first-search (traversal)
QUEUE *create_queue();
void enqueue(QUEUE*, NODE*);
NODE *dequeue(QUEUE*);
void BFS(FILE*, TREE*, NODE*);

// Deal with AVL balance
NODE *avl_left_rotate(NODE*);
NODE *avl_right_rotate(NODE*);
int get_height(NODE*);
void avl_balance(TREE*, NODE*, NODE*);

// Deal with file I/O
void print_out_basic(TREE*);
void print_out_main(TREE*);

int main() {
    int previous_height=0, current_height=0;
    char input[15], *input_mode, *input_number;
    TREE *tree = insert_from_file();
    NODE *new_node = NULL;

    // Print out basic.txt
    print_out_basic(tree);

    while(1) {
        printf("Command: ");
        fgets(input, 15, stdin);
        input_mode = strtok(input, " ");

        if(!strcmp(input_mode, "QUIT\n")) {
            break;
        } else if(!strcmp(input_mode, "insert") || !strcmp(input_mode, "delete")) {
            input_number = strtok(NULL, " ");

            if(!strcmp(input_mode, "insert")) {
                new_node = create_node(atoi(input_number));
                insert_node(tree, tree->root, new_node);
            } else if(!strcmp(input_mode, "delete")) {
                delete_node(tree, NULL, tree->root, atoi(input_number));
            }

            print_out_basic(tree);

        } else if(!strcmp(input_mode, "balance\n")) {
            do {
                // Make sure whether tree is balanced or not (height would be fixed)
                previous_height = get_height(tree->root);
                avl_balance(tree, NULL, tree->root);
                current_height = get_height(tree->root);
            } while(previous_height != current_height);

            print_out_main(tree);
        }
        printf("\n");
    }

    return 0;
}

TREE *create_tree() {
    TREE *tree = (TREE*)malloc(sizeof(TREE));
    tree->counter = 0;
    tree->root = NULL;

    return tree;
}

NODE *create_node(int number) {
    NODE *node = (NODE*)malloc(sizeof(NODE));
    node->number = number;
    node->next = NULL;
    node->left = NULL;
    node->right = NULL;

    return node;
}

void insert_node(TREE *tree, NODE *current_node, NODE *new_node) {
    if(tree->root == NULL) {
        tree->root = new_node;
        (tree->counter)++;
    } else {
        if(new_node->number < current_node->number) {
            if(current_node->left == NULL) {
                // CASE ... The number of new node is smaller than the number of current node and the left node of current node is NULL
                current_node->left = new_node;
                (tree->counter)++;
            } else {
                // CASE ... Continue to search the left node of current node
                insert_node(tree, current_node->left, new_node);
            }
        } else if(new_node->number > current_node->number) {
            if(current_node->right == NULL) {
                // CASE ... The number of new node is higher than the number of current node and the right node of current node is NULL
                current_node->right = new_node;
                (tree->counter)++;
            } else {
                // CASE ... Continue to search the right node of current node
                insert_node(tree, current_node->right, new_node);
            }
        } else {
            // CASE ... Thu number of new node is equal to the number of current node (insert into the left node of current node)
            NODE *temp_node = current_node->left;
            current_node->left = new_node;
            new_node->left = temp_node;
            (tree->counter)++;
        }
    }
}

void delete_node(TREE *tree, NODE *previous_node, NODE *current_node, int number) {
    if(current_node != NULL) {
        if(number < current_node->number) {
            // CASE ... Continue to search the left node of current node
            delete_node(tree, current_node, current_node->left, number);
        } else if(number > current_node->number) {
            // CASE ... Continue to search the right node of current node
            delete_node(tree, current_node, current_node->right, number);
        } else {
            // CASE ... Deleted node is found
            if(current_node->left == NULL || current_node->right == NULL) {
                // CASE ... Left node or right node is NULL
                if(previous_node == NULL) {
                    tree->root = (current_node->left == NULL) ? current_node->right : current_node->left;
                } else if(previous_node->left == current_node) {
                    previous_node->left = (current_node->left == NULL) ? current_node->right : current_node->left;
                } else if(previous_node->right == current_node) {
                    previous_node->right = (current_node->left == NULL) ? current_node->right : current_node->left;
                }
                free(current_node);

            } else {
                // CASE ... Both left node and right node are not NULL

                // Find the rightmost node under the left node of current node
                NODE *previous_rightmost_node=NULL, *rightmost_node_under_current_left=current_node->left;
                while(rightmost_node_under_current_left->right != NULL) {
                    previous_rightmost_node = rightmost_node_under_current_left;
                    rightmost_node_under_current_left = rightmost_node_under_current_left->right;
                }

                // Exchange current node with rightmost node and Set rightmost node to NULL
                if(previous_rightmost_node == NULL) {
                    // CASE ... The right node of the left node of current node is NULL
                    current_node->left->right = current_node->right;
                    if(previous_node == NULL) {
                        tree->root = current_node->left;
                    } else if(previous_node->left == current_node) {
                        previous_node->left = current_node->left;
                    } else if(previous_node->right == current_node) {
                        previous_node->right = current_node->left;
                    }
                    free(current_node);
                } else {
                    // CASE ... The right node of the left node of current node is not NULL
                    current_node->number = rightmost_node_under_current_left->number;
                    previous_rightmost_node->right = rightmost_node_under_current_left->left;
                    free(rightmost_node_under_current_left);
                }
            }
            (tree->counter)--;
        }
    }
}

TREE *insert_from_file() {
    int number=0;
    TREE *tree = create_tree();
    NODE *new_node = NULL;
    FILE *fptr_read = fopen("input.txt", "r");

    // Read number from input.txt and Insert into tree
    while(fscanf(fptr_read, "%d", &number) != EOF) {
        new_node = create_node(number);
        insert_node(tree, tree->root, new_node);
    }
    fclose(fptr_read);

    return tree;
}

void preorder(FILE *fptr, TREE *tree, NODE *current_node, int *counter) {
    if(current_node != NULL) {
        fprintf(fptr, "%d%s", current_node->number, (++(*counter) == tree->counter) ? "" : ", ");
        preorder(fptr, tree, current_node->left, counter);
        preorder(fptr, tree, current_node->right, counter);
    }
}

void inorder(FILE *fptr, TREE *tree, NODE *current_node, int *counter) {
    if(current_node != NULL) {
        inorder(fptr, tree, current_node->left, counter);
        fprintf(fptr, "%d%s", current_node->number, (++(*counter) == tree->counter) ? "" : ", ");
        inorder(fptr, tree, current_node->right, counter);
    }
}

void postorder(FILE *fptr, TREE *tree, NODE *current_node, int *counter) {
    if(current_node != NULL) {
        postorder(fptr, tree, current_node->left, counter);
        postorder(fptr, tree, current_node->right, counter);
        fprintf(fptr, "%d%s", current_node->number, (++(*counter) == tree->counter) ? "" : ", ");
    }
}

QUEUE *create_queue() {
    QUEUE *queue = (QUEUE*)malloc(sizeof(QUEUE));
    queue->head = NULL;
    queue->tail = NULL;

    return queue;
}

void enqueue(QUEUE *queue, NODE *node) {
    if(node != NULL) {
        if(queue->head == NULL) {
            queue->head = node;
            queue->tail = node;
        } else {
            queue->tail->next = node;
            queue->tail = queue->tail->next;
        }
    }
}

NODE *dequeue(QUEUE *queue) {
    NODE *temp_node = queue->head;
    if(queue->head == queue->tail) {
        queue->head = NULL;
        queue->tail = NULL;
    } else {
        queue->head = queue->head->next;
    }
    return temp_node;
}

void BFS(FILE *fptr, TREE *tree, NODE *current_node) {
    if(current_node != NULL) {
        int counter=0;

        // For root node
        QUEUE *queue = create_queue();
        enqueue(queue, current_node);

        // For other nodes except for root node
        while(current_node != NULL) {
            current_node = dequeue(queue);
            if(current_node != NULL) {
                fprintf(fptr, "%d%s", current_node->number, ((++counter) == tree->counter) ? "" : ", ");
                enqueue(queue, current_node->left);
                enqueue(queue, current_node->right);
            }
        }

        free(queue);
    }
}

void print_out_basic(TREE *tree) {
    int counter=0;
    FILE *fptr = fopen("basic.txt", "w");

    // Show current tree (using BSF)
    fprintf(fptr, "tree: ");
    BFS(fptr, tree, tree->root);

    // Show inorder
    fprintf(fptr, "\n\ninorder: ");
    inorder(fptr, tree, tree->root, &counter);

    // Show preorder
    fprintf(fptr, "\npreorder: ");
    counter=0;
    preorder(fptr, tree, tree->root, &counter);

    // Show postorder
    fprintf(fptr, "\npostorder: ");
    counter=0;
    postorder(fptr, tree, tree->root, &counter);

    // Show Breadth-first-search (BFS)
    fprintf(fptr, "\nBreadth-first-search: ");
    BFS(fptr, tree, tree->root);

    fclose(fptr);
}

NODE *avl_left_rotate(NODE *current_node) {
    // Left rotation for current node
    NODE *temp_node = current_node->right;
    current_node->right = temp_node->left;
    temp_node->left = current_node;

    return temp_node;
}

NODE *avl_right_rotate(NODE *current_node) {
    // Right rotation for current node
    NODE *temp_node = current_node->left;
    current_node->left = temp_node->right;
    temp_node->right = current_node;

    return temp_node;
}

int get_height(NODE *current_node) {
    if(current_node == NULL) {
        return 0;
    } else {
        if(get_height(current_node->left) >= get_height(current_node->right)) {
            return 1 + get_height(current_node->left);
        } else {
            return 1 + get_height(current_node->right);
        }
    }
}

void avl_balance(TREE *tree, NODE *previous_node, NODE *current_node) {
    if(current_node != NULL) {
        // Search imbalanced node in postorder
        avl_balance(tree, current_node, current_node->left);
        avl_balance(tree, current_node, current_node->right);

        if(get_height(current_node->left) - get_height(current_node->right) > 1 || get_height(current_node->right) - get_height(current_node->left) > 1) {
            // CASE ... Imbalanced node is found
            NODE *rotated_node = NULL;

            if(get_height(current_node->left) - get_height(current_node->right) > 1) {
                if(get_height(current_node->left->left) >= get_height(current_node->left->right)) {
                    // LL mode (directly do right rotation for current node)
                    rotated_node = avl_right_rotate(current_node);
                } else {
                    // LR mode (do left rotation for the left node of current node and do right rotation for current node)
                    current_node->left = avl_left_rotate(current_node->left);
                    rotated_node = avl_right_rotate(current_node);
                }
            } else if(get_height(current_node->right) - get_height(current_node->left) > 1) {
                if(get_height(current_node->right->left) >= get_height(current_node->right->right)) {
                    // RL mode (do right rotation for the right node of current node and do left rotation for current node)
                    current_node->right = avl_right_rotate(current_node->right);
                    rotated_node = avl_left_rotate(current_node);
                } else {
                    // RR mode (directly do left rotation for current node)
                    rotated_node = avl_left_rotate(current_node);
                }
            }

            // Reconnect the link between rotated node and previous node
            if(previous_node == NULL) {
                tree->root = rotated_node;
            } else if(previous_node->left == current_node) {
                previous_node->left = rotated_node;
            } else if(previous_node->right == current_node) {
                previous_node->right = rotated_node;
            }
        }
    }
}

void print_out_main(TREE *tree) {
    FILE *fptr = fopen("main.txt", "w");

    fprintf(fptr, "Balance tree: ");
    BFS(fptr, tree, tree->root);
    fprintf(fptr, "\nHeight: %d", get_height(tree->root));

    fclose(fptr);
}
