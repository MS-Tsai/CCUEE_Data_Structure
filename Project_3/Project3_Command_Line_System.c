#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For establishing data system (store each node in the stack)
typedef struct path_node {
    int is_dir, is_root;
    char name[100];
    struct path_node *next, *previous_layer, *next_layer_head;
}PATH_NODE;

typedef struct path {
    struct path_node *top;
}PATH;

// For infinite length of input (store each word in the queue)
typedef struct input_node{
    char word[100];
    struct input_node *next;
}INPUT_NODE;

typedef struct input {
    struct input_node *head;
    struct input_node *tail;
}INPUT;

// Deal with data system
PATH *create_path();
PATH_NODE *create_path_node(char*, int, int);
void path_push(PATH*, PATH_NODE*);
void path_pop(PATH*);
int create_dir_or_file(PATH_NODE*, char[], int, int);
void remove_dir_or_file(PATH_NODE*, INPUT_NODE*, int);
void cd(PATH*, INPUT*);
void dir(PATH_NODE*);
void find(PATH*, PATH_NODE*, INPUT_NODE*);
void copy(PATH_NODE*, PATH_NODE*, INPUT_NODE*);
void copy_node(PATH_NODE*, PATH_NODE*, int*, int*);
PATH_NODE *is_path_exist(PATH_NODE*, char path_string[]);
void show_current_path(PATH_NODE*);

// Deal with infinite length of input
INPUT *create_input();
INPUT_NODE *create_input_node(char*);
void input_enqueue(INPUT*, INPUT_NODE*);
INPUT *get_user_input();

// Deal with Bonus
// void bonus(PATH_NODE*, INPUT*);
// void get_absolute_path(PATH_NODE*, char[]);

int main() {
    INPUT *input = NULL;
    PATH *path = create_path();

    // Initialize the root node in data system
    PATH_NODE *root_node = create_path_node("DS", 1, 1);
    path_push(path, root_node);
    // system("mkdir C:\\DS && cd C:\\DS");

    while(1) {
        // Show current path
        show_current_path(path->top);
        printf(">");

        // Get user input
        input = get_user_input();

        // Execute instruction according to different input
        if(!strcmp(input->head->word, "mkdir")) {
            create_dir_or_file(path->top, input->head->next->word, 1, 0);

        } else if(!strcmp(input->head->word, "rmdir")) {
            remove_dir_or_file(path->top, input->head->next, 1);

        } else if(!strcmp(input->head->word, "cd")) {
            cd(path, input);

        } else if(!strcmp(input->head->word, "dir")) {
            dir(path->top);

        } else if(!strcmp(input->head->word, "del")) {
            remove_dir_or_file(path->top, input->head->next, 0);

        } else if(!strcmp(input->head->word, "find")) {
            PATH *finding_path = create_path();
            find(finding_path, root_node, input->head->next);
            free(finding_path);

        } else if(!strcmp(input->head->word, "copy")) {
            copy(root_node, path->top, input->head->next);

        } else {
            printf("Error : Invalid instruction !\n");
        }
        printf("\n");

        // Bonus
        // bonus(path->top, input);

        free(input);
    }

    return 0;
}

INPUT *create_input() {
    INPUT *input = (INPUT*)malloc(sizeof(INPUT));
    input->head = NULL;
    input->tail = NULL;

    return input;
}

INPUT_NODE *create_input_node(char *word) {
    INPUT_NODE *input_node = (INPUT_NODE*)malloc(sizeof(INPUT_NODE));
    strcpy(input_node->word, word);
    input_node->next = NULL;

    return input_node;
}

void input_enqueue(INPUT *input, INPUT_NODE *input_node) {
    if(input->head == NULL) {
        input->head = input_node;
        input->tail = input_node;
    } else {
        input->tail->next = input_node;
        input->tail = input->tail->next;
    }
}

INPUT *get_user_input() {
    int counter=0;
    char character='\0', word[100];
    INPUT *input = create_input();
    INPUT_NODE *input_node = NULL;

    // Initialize word
    memset(word, '\0', 100);

    // Read each character of input and Store the word into queue (when user input ' ')
    while(1) {
        character = fgetc(stdin);

        if(character != '\n' && character != ' ') {
            word[counter] = character;
            counter++;
        } else {
            input_node = create_input_node(word);
            input_enqueue(input, input_node);

            if(character == ' ') {
                counter=0;
                memset(word, '\0', 100);
            } else if(character == '\n') {
                break;
            }
        }
    }

    return input;
}

PATH *create_path() {
    PATH *path = (PATH*)malloc(sizeof(PATH));
    path->top = NULL;

    return path;
}

PATH_NODE *create_path_node(char *name, int is_dir, int is_root) {
    PATH_NODE *path_node = (PATH_NODE*)malloc(sizeof(PATH_NODE));
    strcpy(path_node->name, name);
    path_node->is_dir = is_dir;
    path_node->is_root = is_root;
    path_node->next = NULL;
    path_node->previous_layer = NULL;
    path_node->next_layer_head = NULL;

    return path_node;
}

void path_push(PATH *path, PATH_NODE *path_node) {
    if(path->top == NULL) {
        path->top = path_node;
    } else {
        path_node->previous_layer = path->top;
        path->top = path_node;
    }
}

void path_pop(PATH *path) {
    if(path->top->is_root != 1) {
        path->top = path->top->previous_layer;
    }
}

int create_dir_or_file(PATH_NODE *current_path, char name[], int is_dir, int is_copy) {
    PATH_NODE *path_node = create_path_node(name, is_dir, 0);

    if(current_path->next_layer_head == NULL) {
        current_path->next_layer_head = path_node;
    } else {
        PATH_NODE *current_node = current_path->next_layer_head;
        while(1) {
            if(!strcmp(current_node->name, name) && current_node->is_dir == is_dir) {
                // CASE ... Check if the folder/file name is existed or not
                if(is_copy != 1) {
                    printf("Error : %s %s existed !\n", ((is_dir == 1) ? "Folder" : "File"), name);
                }
                return 0;
            } else if(current_node->next == NULL) {
                // CASE ... No duplicated folder/file
                current_node->next = path_node;
                break;
            }
            current_node = current_node->next;
        }
    }

    if(is_copy != 1) {
        printf("%s %s created %s", ((is_dir == 1) ? "Folder" : "File"), name, ((is_dir == 1) ? "!\n" : "@ "));
        if(is_dir != 1) {
            show_current_path(current_path);
            printf("\n");
        }
    }

    return 1;
}

void remove_dir_or_file(PATH_NODE *current_path, INPUT_NODE* input_node, int is_dir) {
    if(current_path->next_layer_head != NULL) {
        PATH_NODE *temp = NULL;
        PATH_NODE *previous_node = NULL;
        PATH_NODE *current_node = current_path->next_layer_head;
        while(current_node != NULL) {
            // Check if the folder/file name is existed or not
            if(!strcmp(current_node->name, input_node->word) && current_node->is_dir == is_dir) {
                if(previous_node == NULL) {
                    current_path->next_layer_head = current_node->next;
                } else {
                    previous_node->next = current_node->next;
                }
                free(current_node);
                printf("%s %s removed !\n", ((is_dir == 1) ? "Folder" : "File"), input_node->word);
                return ;
            }
            previous_node = current_node;
            current_node = current_node->next;
        }
    }
    printf("Error : %s %s not found !\n", ((is_dir == 1) ? "Folder" : "File"), input_node->word);
}

void cd(PATH *path, INPUT *input) {
    INPUT_NODE *input_node = input->head->next;

    if(!strcmp(input_node->word, ".>")) {
        // CASE ... Create a new file
        create_dir_or_file(path->top, input_node->next->word, 0, 0);

    } else if(!strcmp(input_node->word, "..")) {
        // CASE ... Back to previous layer
        path_pop(path);

    } else {
        // CASE ... Go to next layer chosen by user
        PATH_NODE *current_node = path->top->next_layer_head;
        while(current_node != NULL) {
            // Check if the folder/file name is existed or not
            if(!strcmp(current_node->name, input_node->word) && current_node->is_dir == 1) {
                path_push(path, current_node);
                return ;
            }
            current_node = current_node->next;
        }
        printf("Error : Folder %s not existed !\n", input_node->word);
    }
}

void dir(PATH_NODE *current_path) {
    int dir_counter=0, file_counter=0;
    PATH_NODE *current_node = NULL;

    // Find the number of folders under current path
    for(dir_counter=0, current_node=current_path->next_layer_head; current_node!=NULL; current_node=current_node->next) {
        if(current_node->is_dir == 1) {
            dir_counter++;
            printf("\t<DIR>\t%s\n", current_node->name);
        }
    }

    // Find the number of files under current path
    for(file_counter=0, current_node=current_path->next_layer_head; current_node!=NULL; current_node=current_node->next) {
        if(current_node->is_dir == 0) {
            file_counter++;
            printf("\t<File>\t%s\n", current_node->name);
        }
    }

    // Show statistic results
    printf("\t%4d file(s)\n", file_counter);
    printf("\t%4d folder(s)\n", dir_counter);
}

void find(PATH *finding_path, PATH_NODE *current_node, INPUT_NODE *input_node) {
    if(current_node != NULL) {
        // Create a temp node according to current node and Push this node into finding_path
        PATH_NODE *temp_node = create_path_node(current_node->name, current_node->is_dir, current_node->is_root);
        path_push(finding_path, temp_node);

        // Check if the name of top of finding_path is equal to user input (yes/no ... show/continue)
        if(!strcmp(finding_path->top->name, input_node->word)) {
            show_current_path(finding_path->top);
            printf("\n");
        }

        // Continue to find next layer
        find(finding_path, current_node->next_layer_head, input_node);

        // Pop the top of finding_path
        path_pop(finding_path);

        // Continue to find next node (same layer)
        find(finding_path, current_node->next, input_node);
    }
}

void copy(PATH_NODE *root_node, PATH_NODE *current_node, INPUT_NODE *input_node) {
    char src_path[100], tgt_path[100], *token;
    PATH_NODE *src_current_node=NULL, *tgt_current_node=NULL;

    // Initialize source path and target path
    strcpy(src_path, input_node->word);
    strcpy(tgt_path, input_node->next->word);

    // Check if both source path and target path inputed by user are existed
    src_current_node = is_path_exist(current_node, src_path);
    tgt_current_node = is_path_exist(root_node, tgt_path);
    if(src_current_node != NULL && tgt_current_node != NULL) {
        int copy_result=1, dir_counter=0, file_counter=0;

        // Check the source node is existed in the target path
        copy_result = create_dir_or_file(tgt_current_node, src_current_node->name, src_current_node->is_dir, 1);

        if(copy_result == 1) {
            // Go to copied source node in the target path
            tgt_current_node = tgt_current_node->next_layer_head;
            while(tgt_current_node != NULL) {
                if(!strcmp(src_current_node->name, tgt_current_node->name) && src_current_node->is_dir == tgt_current_node->is_dir) {
                    break;
                }
                tgt_current_node = tgt_current_node->next;
            }

            // Start to copy node
            (src_current_node->is_dir == 1) ? dir_counter++ : file_counter++;
            copy_node(src_current_node->next_layer_head, tgt_current_node, &dir_counter, &file_counter);

            // Show statistic results
            if(dir_counter != 0 && file_counter != 0) {
                printf("%d dir(s) and %d file(s) copied to %s\\%s\n", dir_counter, file_counter, tgt_path, src_current_node->name);
            } else if(dir_counter != 0) {
                printf("%d dir(s) copied to %s\\%s\n", dir_counter, tgt_path, src_current_node->name);
            } else if(file_counter != 0) {
                printf("File %s copied to %s\n", src_current_node->name, tgt_path);
            }

        } else {
            printf("Error : %s %s existed @ %s !\n", ((src_current_node->is_dir == 1) ? "Folder" : "File"), src_current_node->name, tgt_path);
        }
    } else if(src_current_node == NULL && tgt_current_node == NULL) {
        printf("Error : Both source path (%s) and target path (%s) are not existed !\n", src_path, tgt_path);

    } else if(src_current_node == NULL) {
        printf("Error : Source path (%s) is not existed !\n", src_path);

    } else if(tgt_current_node == NULL) {
        printf("Error : Target path (%s) is not existed !\n", tgt_path);

    }
}

void copy_node(PATH_NODE *src_current_node, PATH_NODE *tgt_current_node, int *dir_counter, int *file_counter) {
    if(src_current_node != NULL) {
        // Create target node according to the information of source node
        create_dir_or_file(tgt_current_node, src_current_node->name, src_current_node->is_dir, 1);
        (src_current_node->is_dir == 1) ? (*dir_counter)++ : (*file_counter)++;

        // Continue to copy next layer
        copy_node(src_current_node->next_layer_head, tgt_current_node->next_layer_head, dir_counter, file_counter);

        // Continue to copy next node (same layer)
        copy_node(src_current_node->next, tgt_current_node, dir_counter, file_counter);
    }
}

PATH_NODE *is_path_exist(PATH_NODE *check_node, char path_string[]) {
    char temp_path_string[strlen(path_string)], *token;

    // Save original path into temporary memory space
    strcpy(temp_path_string, path_string);

    // Parse given path
    token = strtok(temp_path_string, "\\"); // first folder
    if(!strcmp(token, "DS:") && strlen(path_string) < 4) {
        return NULL;
    } else {
        if(!strcmp(token, "DS:")) {
            token = strtok(NULL, "\\");         // next folder or NULL
        }

        // According to given path, find the corresponding node
        while(token != NULL && strlen(token) != 0 && check_node != NULL) {
            check_node = check_node->next_layer_head;
            while(check_node != NULL) {
                if(!strcmp(check_node->name, token)) {
                    break;
                }
                check_node = check_node->next;
            }
            token = strtok(NULL, "\\");
        }

        return check_node;
    }
}

void show_current_path(PATH_NODE *current_path) {
    if(current_path->is_root != 1) {
        if(current_path->previous_layer->is_root == 1) {
            printf("DS:\\%s", current_path->name);
        } else {
            show_current_path(current_path->previous_layer);
            printf("\\%s", current_path->name);
        }
    } else {
        printf("DS:\\");
    }
}
/*
void bonus(PATH_NODE *current_path, INPUT *input) {
    char absolute_path[100]="C:\\", instruction[100]="";
    INPUT_NODE *input_node = input->head;

    // Get absolute path according to current node
    get_absolute_path(current_path, absolute_path);

    // Execute system instruction according to user input
    if(!strcmp(input_node->word, "mkdir") || !strcmp(input_node->word, "rmdir") || !strcmp(input_node->word, "del") || !strcmp(input_node->word, "dir")) {
        if(strcmp(input_node->word, "dir")) {
            strcat(absolute_path, input_node->next->word);
        }
        strcat(instruction, input_node->word);
        strcat(instruction, " ");
        strcat(instruction, absolute_path);

    } else if(!strcmp(input_node->word, "cd")) {
        input_node = input_node->next;
        if(!strcmp(input_node->word, ".>")) {
            strcat(absolute_path, input_node->next->word);
            strcat(instruction, "cd ");
            strcat(instruction, ".> ");
            strcat(instruction, absolute_path);
        } else if(!strcmp(input_node->word, "..")) {
            strcat(instruction, "cd ..");
        } else {
            strcat(instruction, "cd ");
            strcat(instruction, absolute_path);
        }

    } else if(!strcmp(input_node->word, "copy")) {
        int i=0;
        char src_absolute_path[100]="C:\\", tgt_absolute_path[100]="C:\\DS\\", *last_src_node;

        // Get absolute source path and absolute target path
        get_absolute_path(current_path, src_absolute_path);
        strcat(src_absolute_path, input_node->next->word);
        strcat(tgt_absolute_path, input_node->next->next->word+4);
        last_src_node = strrchr(input_node->next->word, '\\');
        if(last_src_node == NULL) {
            strcat(tgt_absolute_path, "\\");
            strcat(tgt_absolute_path, input_node->next->word);
        } else {
            strcat(tgt_absolute_path, last_src_node);
        }

        if(strrchr(input_node->next->word, '.') == NULL) {
            strcat(instruction, "mkdir ");
            strcat(instruction, tgt_absolute_path);
            strcat(instruction, " && ");
            strcat(instruction, "xcopy /E ");
        } else {
            strcat(instruction, "copy ");
        }
        strcat(instruction, src_absolute_path);
        strcat(instruction, " ");
        strcat(instruction, tgt_absolute_path);

    }
    printf("%s\n", instruction);
    system(instruction);
}

void get_absolute_path(PATH_NODE *current_path, char absolute_path[]) {
    if(current_path != NULL) {
        get_absolute_path(current_path->previous_layer, absolute_path);
        strcat(absolute_path, current_path->name);
        strcat(absolute_path, "\\");
    }
}
*/
