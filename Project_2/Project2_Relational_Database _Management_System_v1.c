/********************************************************************************
 * Author: Meng-Shiun, Tsai
 * Program: Relational Database Management System
    1. Parse SQL instruction (Note that we should deal with infinite input).
    2. Use linked list to store data.
    3. My method:
        1) Use queue to store input string.
        2) Use stack to store the status of database (for undo or redo).
        3) Use stack to implement log_SQL_to_file when user input undo or redo.
********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For establishing database (people and group)
typedef struct node_people {
    char name[20], phone[15], birthday[10], email[50];
    struct node_people *next;
}NODE_PEOPLE;

typedef struct node_group {
    char name[20], group_name[20];
    struct node_group *next;
}NODE_GROUP;

typedef struct database {
    struct node_people *head_people;
    struct node_group *head_group;
    struct database *next;
}DATABASE;

// For UNDO and REDO (store the status of database in the stack)
typedef struct database_status_main {
    struct database *top;
}DATABASE_STATUS_MAIN;

typedef struct database_status_backup {
    struct database *top;
}DATABASE_STATUS_BACKUP;

// For infinite length of input (store each word in the queue)
typedef struct node_word{
    char word[50];
    struct node_word *next;
}NODE_WORD;

typedef struct input {
    struct node_word *head;
    struct node_word *tail;
}INPUT;

// For UNDO and REDO in log.txt
typedef struct node_number {
    int number;
    struct node_number *next;
}NODE_NUMBER;

typedef struct file_line {
    struct node_number *top;
}FILE_LINE;

// Deal with linked list
DATABASE *create_database();
NODE_PEOPLE *create_node_people(char*, char*, char*, char*);
NODE_GROUP *create_node_group(char*, char*);
void database_insert_node(DATABASE*, NODE_PEOPLE*, NODE_GROUP*, int, int);
void database_select_node(DATABASE*, int[], int[], int, NODE_WORD*);
void database_update_node(DATABASE*, int, char*, char*, char*, char*);
void database_delete_node(DATABASE*, int, char*, char*);

// Parse SQL instruction
void database_insert_from_SQL(DATABASE*, INPUT*);
void database_select_from_SQL(DATABASE*, INPUT*);
void database_update_from_SQL(DATABASE*, INPUT*);
void database_delete_from_SQL(DATABASE*, INPUT*);

// Deal with UNDO and REDO
DATABASE_STATUS_MAIN *create_database_status_main();
DATABASE_STATUS_BACKUP *create_database_status_backup();
DATABASE *copy_database(DATABASE*);
void status_push(DATABASE_STATUS_MAIN*, DATABASE_STATUS_BACKUP*, DATABASE*, int);
DATABASE *status_pop(DATABASE_STATUS_MAIN*, DATABASE_STATUS_BACKUP*, int);

// Deal with infinite length of input
INPUT *create_input();
NODE_WORD *create_node_word(char*);
void input_enqueue(INPUT*, NODE_WORD*);
void get_SQL_instruction(INPUT*);
void show_input(NODE_WORD*);

// Deal with file I/O
FILE_LINE *create_file_line();
NODE_NUMBER *create_node_number(int);
void line_push(FILE_LINE*, NODE_NUMBER*);
NODE_NUMBER *line_pop(FILE_LINE*);
void log_SQL_to_file(INPUT*, int, NODE_PEOPLE*, NODE_GROUP*, char[]);
DATABASE *insert_from_file();
void save_to_file(DATABASE*);

int main() {
    INPUT *input = NULL;
    DATABASE *database = insert_from_file();
    DATABASE *original_database = copy_database(database);
    DATABASE_STATUS_MAIN *database_status_main = create_database_status_main();
    DATABASE_STATUS_BACKUP *database_status_backup = create_database_status_backup();

    while(1) {
        input = create_input();
        get_SQL_instruction(input);
        // printf("Repeat input: ");
        // show_input(input->head);

        // Write SQL instruction into log.txt
        if((!strcmp(input->head->word, "INSERT") || !strcmp(input->head->word, "SELECT") || !strcmp(input->head->word, "UPDATE") || !strcmp(input->head->word, "DELETE")) ||
           (!strcmp(input->head->word, "undo") && database_status_main->top != NULL) ||
           (!strcmp(input->head->word, "redo") && database_status_backup->top != NULL)) {
            log_SQL_to_file(input, 2, NULL, NULL, "");
        }

        // Execute SQL instruction according to different action
        if(!strcmp(input->head->word, "QUIT")) {
            log_SQL_to_file(NULL, 3, NULL, NULL, "結束此次紀錄");
            break;

        } else if(!strcmp(input->head->word, "INSERT")) {
            database_insert_from_SQL(database, input);

        } else if(!strcmp(input->head->word, "SELECT")) {
            database_select_from_SQL(database, input);

        } else if(!strcmp(input->head->word, "UPDATE")) {
            database_update_from_SQL(database, input);

        } else if(!strcmp(input->head->word, "DELETE")) {
            database_delete_from_SQL(database, input);

        } else if(!strcmp(input->head->word, "undo") && database_status_main->top != NULL) {
            // Pop the main status of database and Push it into the backup status
            database = status_pop(database_status_main, NULL, 1);
            status_push(NULL, database_status_backup, database, 0);

            // Reset the status of database and Write the information of UNDO into log.txt
            database = (database_status_main->top == NULL) ? copy_database(original_database) : copy_database(database_status_main->top);
            log_SQL_to_file(input, -1, NULL, NULL, " 復原:");

        } else if(!strcmp(input->head->word, "redo") && database_status_backup->top != NULL) {
            // Pop the backup status of database and Push it into the main status
            database = status_pop(NULL, database_status_backup, 0);
            status_push(database_status_main, NULL, database, 1);

            // Write the information of REDO into log.txt
            log_SQL_to_file(input, -1, NULL, NULL, "");
        }

        // Write the status of database into people.txt and group.txt and Store the main status of database into stack
        if(!strcmp(input->head->word, "INSERT") || !strcmp(input->head->word, "UPDATE") || !strcmp(input->head->word, "DELETE") ||
           !strcmp(input->head->word, "undo")   || !strcmp(input->head->word, "redo")) {
            // Save database into people.txt and group.txt
            save_to_file(database);

            // Save the main status of database and Clear the backup status of database
            if(!strcmp(input->head->word, "INSERT") || !strcmp(input->head->word, "UPDATE") || !strcmp(input->head->word, "DELETE")) {
                status_push(database_status_main, NULL, database, 1);
                if(database_status_backup->top != NULL) {
                    database_status_backup = create_database_status_backup();
                }
            }
        }

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

NODE_WORD *create_node_word(char *word) {
    NODE_WORD *node_word = (NODE_WORD*)malloc(sizeof(NODE_WORD));
    strcpy(node_word->word, word);
    node_word->next = NULL;

    return node_word;
}

void input_enqueue(INPUT *input, NODE_WORD *node_word) {
    if(input->head == NULL) {
        input->head = node_word;
        input->tail = node_word;
    } else {
        input->tail->next = node_word;
        input->tail = input->tail->next;
    }
}

void get_SQL_instruction(INPUT *input) {
    int counter=0;
    char character='\0', word[50];
    NODE_WORD *node_word = NULL;

    // Initialize word
    memset(word, '\0', 50);

    // Read each character of input and Store the word into queue (when user input ' ')
    while(1) {
        character = fgetc(stdin);

        if(character != '\n' && character != ' ') {
            word[counter] = character;
            counter++;
        } else {
            node_word = create_node_word(word);
            input_enqueue(input, node_word);

            if(character == ' ') {
                counter=0;
                memset(word, '\0', 50);
            } else if(character == '\n') {
                break;
            }
        }
    }
}

void show_input(NODE_WORD *current) {
    if(current != NULL) {
        printf("%s ", current->word);
        show_input(current->next);
    } else {
        printf("\n");
    }
}

DATABASE *create_database() {
    DATABASE *database = (DATABASE*)malloc(sizeof(DATABASE));
    database->head_people = NULL;
    database->head_group = NULL;
    database->next = NULL;

    return database;
}

NODE_PEOPLE* create_node_people(char *name, char *phone, char *birhtday, char *email) {
    NODE_PEOPLE *node_people = (NODE_PEOPLE*)malloc(sizeof(NODE_PEOPLE));
    strcpy(node_people->name, name);
    strcpy(node_people->phone, phone);
    strcpy(node_people->birthday, birhtday);
    strcpy(node_people->email, email);
    node_people->next = NULL;

    return node_people;
}

NODE_GROUP* create_node_group(char *name, char *group_name) {
    NODE_GROUP *node_group = (NODE_GROUP*)malloc(sizeof(NODE_GROUP));
    strcpy(node_group->name, name);
    strcpy(node_group->group_name, group_name);
    node_group->next = NULL;

    return node_group;
}

void database_insert_node(DATABASE* database, NODE_PEOPLE* node_people, NODE_GROUP* node_group, int is_people, int is_copy_DB) {
    if(is_people == 1) {
        if(database->head_people == NULL) {
            database->head_people = node_people;
        } else {
            NODE_PEOPLE *current_people = database->head_people;
            while(current_people->next != NULL) {
                current_people = current_people->next;
            }
            current_people->next = node_people;
        }
    } else if(is_people == 0) {
        if(database->head_group == NULL) {
            database->head_group = node_group;
        } else {
            NODE_GROUP *current_group = database->head_group;
            while(current_group->next != NULL) {
                current_group = current_group->next;
            }
            current_group->next = node_group;
        }
    }

    if(is_copy_DB == 0) {
        // Write the information of INSERT into log.txt
        log_SQL_to_file(NULL, is_people, node_people, node_group, "新增");
    }
}

void database_select_node(DATABASE *database, int select_columns[], int select_tables[], int select_case, NODE_WORD *node_word) {
    char where_column[15], logic_equal[5], where_value[50], logic_andor[5];
    NODE_PEOPLE *current_people = database->head_people;
    NODE_GROUP *current_group = database->head_group;

    if(select_tables[0] == 1 && select_tables[1] == 0) {
        // CASE ... SELECT the table of people
        while(current_people != NULL) {
            int is_show=0;
            NODE_WORD *current_word = node_word;
            while(current_word != NULL && is_show == 0) {
                int flag=1;
                current_word = current_word->next;
                while(current_word != NULL && strcmp(current_word->word, "OR")) {
                    // Get where_column, =/!=, where_value
                    if(!strcmp(current_word->word, "AND")) {
                        current_word = current_word->next;
                    }
                    strcpy(where_column, current_word->word);
                    current_word = current_word->next;
                    strcpy(logic_equal, current_word->word);
                    current_word = current_word->next;
                    strcpy(where_value, current_word->word);

                    // Update the flag (determining whether this node should be shown out)
                    if(!strcmp(logic_equal, "=")) {
                        if((!strcmp(where_column, "name")     && !strcmp(current_people->name, where_value))     ||
                           (!strcmp(where_column, "phone")    && !strcmp(current_people->phone, where_value))    ||
                           (!strcmp(where_column, "birthday") && !strcmp(current_people->birthday, where_value)) ||
                           (!strcmp(where_column, "email")    && !strcmp(current_people->email, where_value))) {
                            flag &= 1;
                        } else {
                            flag &= 0;
                        }
                    } else if(!strcmp(logic_equal, "!=")) {
                        if((!strcmp(where_column, "name")     && strcmp(current_people->name, where_value))     ||
                           (!strcmp(where_column, "phone")    && strcmp(current_people->phone, where_value))    ||
                           (!strcmp(where_column, "birthday") && strcmp(current_people->birthday, where_value)) ||
                           (!strcmp(where_column, "email")    && strcmp(current_people->email, where_value))) {
                            flag &= 1;
                        } else {
                            flag &= 0;
                        }
                    }
                    current_word = current_word->next;
                }
                is_show |= flag;
            }
            if(is_show == 1 || select_case < 2) {
                // Show columns chosen by user (CASE ... is_show = 1 or no "WHERE" in the input)
                if(select_columns[0] == 1) {
                    printf("%20s", current_people->name);
                }
                if(select_columns[1] == 1) {
                    printf("%15s", current_people->phone);
                }
                if(select_columns[2] == 1) {
                    printf("%10s", current_people->birthday);
                }
                if(select_columns[3] == 1) {
                    printf("%50s", current_people->email);
                }
                printf("\n");
            }
            current_people = current_people->next;
        }
    } else if(select_tables[0] == 0 && select_tables[1] == 1) {
        // CASE ... SELECT the table of group
        while(current_group != NULL) {
            int is_show=0;
            NODE_WORD *current_word = node_word;
            while(current_word != NULL && is_show == 0) {
                int flag=1;
                current_word = current_word->next;
                while(current_word != NULL && strcmp(current_word->word, "OR")) {
                    // Get where_column, =/!=, where_value
                    if(!strcmp(current_word->word, "AND")) {
                        current_word = current_word->next;
                    }
                    strcpy(where_column, current_word->word);
                    current_word = current_word->next;
                    strcpy(logic_equal, current_word->word);
                    current_word = current_word->next;
                    strcpy(where_value, current_word->word);

                    // Update the flag (determining whether this node should be shown out)
                    if(!strcmp(logic_equal, "=")) {
                        if((!strcmp(where_column, "name")       && !strcmp(current_group->name, where_value)) ||
                           (!strcmp(where_column, "group_name") && !strcmp(current_group->group_name, where_value))) {
                            flag &= 1;
                        } else {
                            flag &= 0;
                        }
                    } else if(!strcmp(logic_equal, "!=")) {
                        if((!strcmp(where_column, "name")       && strcmp(current_group->name, where_value)) ||
                           (!strcmp(where_column, "group_name") && strcmp(current_group->group_name, where_value))) {
                            flag &= 1;
                        } else {
                            flag &= 0;
                        }
                    }
                    current_word = current_word->next;
                }
                is_show |= flag;
            }
            if(is_show == 1 || select_case < 2) {
                // Show columns chosen by user (CASE ... is_show = 1 or no "WHERE" in the input)
                if(select_columns[0] == 1) {
                    printf("%20s", current_group->name);
                }
                if(select_columns[4] == 1) {
                    printf("%20s", current_group->group_name);
                }
                printf("\n");
            }
            current_group = current_group->next;
        }
    } else if(select_tables[0] == 1 && select_tables[1] == 1) {
        // CASE ... SELECT the table of people and group at the same time
        if(select_case == 2) {
            // CASE ... "WHERE" is in the input
            while(current_people != NULL) {
                current_group = database->head_group;
                while(current_group != NULL) {
                    int is_show=0;
                    NODE_WORD *current_word = node_word;
                    while(current_word != NULL && is_show == 0) {
                        int flag=1;
                        current_word = current_word->next;
                        while(current_word != NULL && strcmp(current_word->word, "OR")) {
                            // Get where_column, =/!=, where_value
                            if(!strcmp(current_word->word, "AND")) {
                                current_word = current_word->next;
                            }
                            strcpy(where_column, current_word->word);
                            current_word = current_word->next;
                            strcpy(logic_equal, current_word->word);
                            current_word = current_word->next;
                            strcpy(where_value, current_word->word);

                            // Update the flag (determining whether this node should be shown out)
                            if(!strcmp(logic_equal, "=")) {
                                if((!strcmp(current_people->name, current_group->name)) &&
                                   ((!strcmp(where_column, "name")       && !strcmp(current_people->name, where_value))     ||
                                    (!strcmp(where_column, "phone")      && !strcmp(current_people->phone, where_value))    ||
                                    (!strcmp(where_column, "birthday")   && !strcmp(current_people->birthday, where_value)) ||
                                    (!strcmp(where_column, "email")      && !strcmp(current_people->email, where_value))    ||
                                    (!strcmp(where_column, "name")       && !strcmp(current_group->name, where_value))      ||
                                    (!strcmp(where_column, "group_name") && !strcmp(current_group->group_name, where_value)))) {
                                    flag &= 1;
                                } else {
                                    flag &= 0;
                                }
                            } else if(!strcmp(logic_equal, "!=")) {
                                if((!strcmp(current_people->name, current_group->name)) &&
                                   ((!strcmp(where_column, "name")       && strcmp(current_people->name, where_value))     ||
                                    (!strcmp(where_column, "phone")      && strcmp(current_people->phone, where_value))    ||
                                    (!strcmp(where_column, "birthday")   && strcmp(current_people->birthday, where_value)) ||
                                    (!strcmp(where_column, "email")      && strcmp(current_people->email, where_value))    ||
                                    (!strcmp(where_column, "name")       && strcmp(current_group->name, where_value))      ||
                                    (!strcmp(where_column, "group_name") && strcmp(current_group->group_name, where_value)))) {
                                    flag &= 1;
                                } else {
                                    flag &= 0;
                                }
                            }
                            current_word = current_word->next;
                        }
                        is_show |= flag;
                    }
                    if(is_show == 1) {
                        // Show columns chosen by user
                        if(select_columns[0] == 1) {
                            printf("%20s", current_people->name);
                        }
                        if(select_columns[1] == 1) {
                            printf("%15s", current_people->phone);
                        }
                        if(select_columns[2] == 1) {
                            printf("%10s", current_people->birthday);
                        }
                        if(select_columns[3] == 1) {
                            printf("%50s", current_people->email);
                        }
                        if(select_columns[4] == 1) {
                            printf("%20s", current_group->group_name);
                        }
                        printf("\n");
                    }
                    current_group = current_group->next;
                }
                current_people = current_people->next;
            }
        } else {
            // CASE ... "WHERE" is not in the input
            while(current_people != NULL) {
                // Show columns chosen by user
                if(select_columns[0] == 1) {
                    printf("%20s", current_people->name);
                }
                if(select_columns[1] == 1) {
                    printf("%15s", current_people->phone);
                }
                if(select_columns[2] == 1) {
                    printf("%10s", current_people->birthday);
                }
                if(select_columns[3] == 1) {
                    printf("%50s", current_people->email);
                }
                printf("\n");
                current_people = current_people->next;
            }
            printf("\n");
            while(current_group != NULL) {
                // Show columns chosen by user
                if(select_columns[0] == 1) {
                    printf("%20s", current_group->name);
                }
                if(select_columns[4] == 1) {
                    printf("%20s", current_group->group_name);
                }
                printf("\n");
                current_group = current_group->next;
            }
        }
    }
}

void database_update_node(DATABASE *database, int is_people, char *update_column, char *update_value, char *where_column, char *where_value) {
    if(is_people == 1) {
        NODE_PEOPLE *current_people = database->head_people;
        while(current_people != NULL) {
            // Check if matching update condition
            if((!strcmp(where_column, "name")     && !strcmp(current_people->name, where_value))     ||
               (!strcmp(where_column, "phone")    && !strcmp(current_people->phone, where_value))    ||
               (!strcmp(where_column, "birthday") && !strcmp(current_people->birthday, where_value)) ||
               (!strcmp(where_column, "email")    && !strcmp(current_people->email, where_value))) {
                // Update the specific column chosen by user
                if(!strcmp(update_column, "name")) {
                    strcpy(current_people->name, update_value);
                } else if(!strcmp(update_column, "phone")) {
                    strcpy(current_people->phone, update_value);
                } else if(!strcmp(update_column, "birthday")) {
                    strcpy(current_people->birthday, update_value);
                } else if(!strcmp(update_column, "email")) {
                    strcpy(current_people->email, update_value);
                }

                // Write the information of UPDATE into log.txt
                log_SQL_to_file(NULL, 1, current_people, NULL, "更新");
            }
            current_people = current_people->next;
        }
    } else if(is_people == 0) {
        NODE_GROUP *current_group = database->head_group;
        while(current_group != NULL) {
            // Check if matching update condition
            if((!strcmp(where_column, "name")       && !strcmp(current_group->name, where_value)) ||
               (!strcmp(where_column, "group_name") && !strcmp(current_group->group_name, where_value))) {
                // Update the specific column chosen by user
                if(!strcmp(update_column, "name")) {
                    strcpy(current_group->name, update_value);
                } else if(!strcmp(update_column, "group_name")) {
                    strcpy(current_group->group_name, update_value);
                }

                // Write the information of UPDATE into log.txt
                log_SQL_to_file(NULL, 0, NULL, current_group, "更新");
            }
            current_group = current_group->next;
        }
    }
}

void database_delete_node(DATABASE *database, int is_people, char *where_column, char *where_value) {
    if(is_people == 1) {
        NODE_PEOPLE *current_people=database->head_people, *previous_people=NULL;
        while(current_people != NULL) {
            // Check if matching delete condition
            if((!strcmp(where_column, "name")     && !strcmp(current_people->name, where_value))     ||
               (!strcmp(where_column, "phone")    && !strcmp(current_people->phone, where_value))    ||
               (!strcmp(where_column, "birthday") && !strcmp(current_people->birthday, where_value)) ||
               (!strcmp(where_column, "email")    && !strcmp(current_people->email, where_value))) {

                // Write the information of DELETE into log.txt
                log_SQL_to_file(NULL, 1, current_people, NULL, "刪除");

                // Delete node
                if(previous_people == NULL) {
                    // Delete head node
                    database->head_people = current_people->next;
                    free(current_people);
                    current_people = database->head_people;
                } else {
                    // Delete node except for head
                    previous_people->next = current_people->next;
                    free(current_people);
                    current_people = previous_people->next;
                }
                continue;
            }
            previous_people = current_people;
            current_people = current_people->next;
        }
    } else if(is_people == 0) {
        NODE_GROUP *current_group=database->head_group, *previous_group=NULL;
        while(current_group != NULL) {
            // Check if matching delete condition
            if((!strcmp(where_column, "name")       && !strcmp(current_group->name, where_value)) ||
               (!strcmp(where_column, "group_name") && !strcmp(current_group->group_name, where_value))) {

                // Write the information of DELETE into log.txt
                log_SQL_to_file(NULL, 0, NULL, current_group, "刪除");

                // Delete node
                if(previous_group == NULL) {
                    // Delete head node
                    database->head_group = current_group->next;
                    free(current_group);
                    current_group = database->head_group;
                } else {
                    // Delete node except for head
                    previous_group->next = current_group->next;
                    free(current_group);
                    current_group = previous_group->next;
                }
                continue;
            }
            previous_group = current_group;
            current_group = current_group->next;
        }
    }
}

void database_insert_from_SQL(DATABASE *database, INPUT *input) {
    int i=0, column_counter=0, is_people=0;
    char name[20], phone[15], birthday[10], email[50], group_name[20];
    char columns_and_values[10][50];

    NODE_WORD *node_word = input->head;
    NODE_PEOPLE *node_people = NULL;
    NODE_GROUP *node_group = NULL;

    // Initialize all strings
    for(i=0; i<10; i++) {
        memset(columns_and_values[i], '\0', 50);
    }
    memset(name, '\0', 20);
    memset(phone, '\0', 15);
    memset(birthday, '\0', 10);
    memset(email, '\0', 50);
    memset(group_name, '\0', 20);

    // Parse INSERT instruction into column and values
    while(node_word != NULL) {
        if(strcmp(node_word->word, "INSERT") && strcmp(node_word->word, "INTO")  && strcmp(node_word->word, "VALUES") &&
           strcmp(node_word->word, "people") && strcmp(node_word->word, "group") &&
           strcmp(node_word->word, "(")      && strcmp(node_word->word, ")")     && strcmp(node_word->word, ",")) {
            strcpy(columns_and_values[column_counter], node_word->word);
            column_counter++;
        } else if(!strcmp(node_word->word, "people")) {
            is_people = 1;
        } else if(!strcmp(node_word->word, "group")) {
            is_people = 0;
        }
        node_word = node_word->next;
    }

    // Save values according to related column
    column_counter = column_counter / 2;
    for (i=0; i<column_counter; i++) {
        if(!strcmp(columns_and_values[i], "name")) {
            strcpy(name, columns_and_values[i+column_counter]);
        } else if(!strcmp(columns_and_values[i], "phone")) {
            strcpy(phone, columns_and_values[i+column_counter]);
        } else if(!strcmp(columns_and_values[i], "birthday")) {
            strcpy(birthday, columns_and_values[i+column_counter]);
        } else if(!strcmp(columns_and_values[i], "email")) {
            strcpy(email, columns_and_values[i+column_counter]);
        } else if(!strcmp(columns_and_values[i], "group_name")) {
            strcpy(group_name, columns_and_values[i+column_counter]);
        }
    }
    if(strlen(name) == 0) {
        strcpy(name, "NA");
    }
    if(strlen(phone) == 0) {
        strcpy(phone, "NA");
    }
    if(strlen(birthday) == 0) {
        strcpy(birthday, "NA");
    }
    if(strlen(email) == 0) {
        strcpy(email, "NA");
    }
    if(strlen(group_name) == 0) {
        strcpy(group_name, "NA");
    }

    // Insert values into database
    if(is_people == 1) {
        node_people = create_node_people(name, phone, birthday, email);
    } else if(is_people == 0) {
        node_group = create_node_group(name, group_name);
    }
    database_insert_node(database, node_people, node_group, is_people, 0);
}

void database_select_from_SQL(DATABASE *database, INPUT *input) {
    int i=0, select_case=0;
    int select_columns[5]={0}, select_tables[2]={0};
    NODE_WORD *node_word = input->head;

    // Parse SELECT instruction
    for(; node_word!=NULL; node_word=node_word->next) {
        if(!strcmp(node_word->word, "SELECT")) {
            select_case = 0;
        } else if(!strcmp(node_word->word, "FROM")) {
            select_case = 1;
        } else if(!strcmp(node_word->word, "WHERE")) {
            select_case = 2;
        }

        if(select_case == 0) {
            // CASE ... When node_word is in select_columns area
            if(!strcmp(node_word->word, "*")) {
                for(i=0; i<5; i++) {
                    select_columns[i] = 1;
                }
            } else {
                if(!strcmp(node_word->word, "name")) {
                    select_columns[0] = 1;
                } else if(!strcmp(node_word->word, "phone")) {
                    select_columns[1] = 1;
                } else if(!strcmp(node_word->word, "birthday")) {
                    select_columns[2] = 1;
                } else if(!strcmp(node_word->word, "email")) {
                    select_columns[3] = 1;
                } else if(!strcmp(node_word->word, "group_name")) {
                    select_columns[4] = 1;
                }
            }
        } else if(select_case == 1) {
            // CASE ... When node_word is in select_tables area
            if(!strcmp(node_word->word, "people")) {
                select_tables[0] = 1;
            } else if(!strcmp(node_word->word, "group")) {
                select_tables[1] = 1;
            }
        } else if(select_case == 2) {
            // CASE ... When node_word is in where area
            database_select_node(database, select_columns, select_tables, select_case, node_word);
            return;
        }
    }
    // CASE ... "WHERE" is not in the input
    database_select_node(database, select_columns, select_tables, select_case, node_word);
}

void database_update_from_SQL(DATABASE *database, INPUT *input) {
    int is_people=0, counter=0;
    char update_column[15], update_value[50], where_column[15], where_value[50];

    NODE_WORD *node_word = input->head;

    // Initialize all strings
    memset(update_column, '\0', 15);
    memset(update_value, '\0', 50);
    memset(where_column, '\0', 15);
    memset(where_value, '\0', 50);

    // Parse UPDATE instruction
    for(counter=0; node_word!=NULL; counter++, node_word=node_word->next) {
        if(counter == 1) {
            if(!strcmp(node_word->word, "people")) {
                is_people = 1;
            } else if(!strcmp(node_word->word, "group")) {
                is_people = 0;
            }
        } else if(counter == 3) {
            strcpy(update_column, node_word->word);
        } else if(counter == 5) {
            strcpy(update_value, node_word->word);
        } else if(counter == 7) {
            strcpy(where_column, node_word->word);
        } else if(counter == 9) {
            strcpy(where_value, node_word->word);
        }
    }

    // Update data from database
    database_update_node(database, is_people, update_column, update_value, where_column, where_value);
}

void database_delete_from_SQL(DATABASE *database, INPUT *input) {
    int is_people=0, counter=0;
    char where_column[15], where_value[50];

    NODE_WORD *node_word = input->head;

    // Initialize all strings
    memset(where_column, '\0', 15);
    memset(where_value, '\0', 50);

    // Parse DELETE instruction
    for(counter=0; node_word!=NULL; counter++, node_word=node_word->next) {
        if(counter == 2) {
            if(!strcmp(node_word->word, "people")) {
                is_people = 1;
            } else if(!strcmp(node_word->word, "group")) {
                is_people = 0;
            }
        } else if(counter == 4) {
            strcpy(where_column, node_word->word);
        } else if(counter == 6) {
            strcpy(where_value, node_word->word);
        }
    }

    // Update data from database
    database_delete_node(database, is_people, where_column, where_value);
}

DATABASE_STATUS_MAIN *create_database_status_main() {
    DATABASE_STATUS_MAIN *database_status_main = (DATABASE_STATUS_MAIN*)malloc(sizeof(DATABASE_STATUS_MAIN));
    database_status_main->top = NULL;

    return database_status_main;
}

DATABASE_STATUS_BACKUP *create_database_status_backup() {
    DATABASE_STATUS_BACKUP *database_status_backup = (DATABASE_STATUS_BACKUP*)malloc(sizeof(DATABASE_STATUS_BACKUP));
    database_status_backup->top = NULL;

    return database_status_backup;
}

DATABASE *copy_database(DATABASE *database) {
    DATABASE *database_status = create_database();
    NODE_PEOPLE *node_people=NULL, *current_people=database->head_people;
    NODE_GROUP *node_group=NULL, *current_group=database->head_group;
    while(current_people != NULL) {
        node_people = create_node_people(current_people->name, current_people->phone, current_people->birthday, current_people->email);
        database_insert_node(database_status, node_people, NULL, 1, 1);
        current_people = current_people->next;
    }
    while(current_group != NULL) {
        node_group = create_node_group(current_group->name, current_group->group_name);
        database_insert_node(database_status, NULL, node_group, 0, 1);
        current_group = current_group->next;
    }

    return database_status;
}

void status_push(DATABASE_STATUS_MAIN *database_status_main, DATABASE_STATUS_BACKUP *database_status_backup, DATABASE *database, int is_main) {
    // Copy database (in another memory space)
    DATABASE *database_status = copy_database(database);

    if(is_main == 1) {
        if(database_status_main->top == NULL) {
            database_status_main->top = database_status;
        } else {
            database_status->next = database_status_main->top;
            database_status_main->top = database_status;
        }
    } else if(is_main == 0) {
        if(database_status_backup->top == NULL) {
            database_status_backup->top = database_status;
        } else {
            database_status->next = database_status_backup->top;
            database_status_backup->top = database_status;
        }
    }
}

DATABASE *status_pop(DATABASE_STATUS_MAIN *database_status_main, DATABASE_STATUS_BACKUP *database_status_backup, int is_main) {
    if(is_main == 1) {
        DATABASE *top_main = database_status_main->top;
        database_status_main->top = database_status_main->top->next;

        return top_main;
    } else if(is_main == 0) {
        DATABASE *top_backup = database_status_backup->top;
        database_status_backup->top = database_status_backup->top->next;

        return top_backup;
    }
}

FILE_LINE *create_file_line() {
    FILE_LINE *file_line = (FILE_LINE*)malloc(sizeof(FILE_LINE));
    file_line->top = NULL;

    return file_line;
}

NODE_NUMBER *create_node_number(int number) {
    NODE_NUMBER *node_number = (NODE_NUMBER*)malloc(sizeof(NODE_NUMBER));
    node_number->number = number;
    node_number->next = NULL;

    return node_number;
}

void line_push(FILE_LINE *file_line, NODE_NUMBER *node_number) {
    if(file_line->top == NULL) {
        file_line->top = node_number;
    } else {
        node_number->next = file_line->top;
        file_line->top = node_number;
    }
}

NODE_NUMBER *line_pop(FILE_LINE *file_line) {
    NODE_NUMBER *temp_node = file_line->top;

    if(file_line->top != NULL) {
        file_line->top = file_line->top->next;
    }

    return temp_node;
}

void log_SQL_to_file(INPUT *input, int log_case, NODE_PEOPLE *current_people, NODE_GROUP *current_group, char SQL_chinese[]) {
    FILE *fptr = fopen("log.txt", "a");

    // According to case, write the log into file
    if(log_case == -1) {
        // CASE ... UNDO or REDO
        int line_counter=0;
        char word[50], character='\0';
        FILE_LINE *file_line = create_file_line();
        NODE_NUMBER *node_number = NULL;
        FILE *fptr_read = fopen("log.txt", "r");

        if(fptr_read != NULL) {
            // Find number of lines and Determine which line should be copied
            while(fscanf(fptr_read, "%s", word) != EOF) {
                if(!strcmp(word, "INSERT") || !strcmp(word, "SELECT") || !strcmp(word, "UPDATE") || !strcmp(word, "DELETE") ||
                   !strcmp(word, "undo")   || !strcmp(word, "redo")   || !strcmp(word, "QUIT")   || !strcmp(word, "結束此次紀錄") || !strcmp(word, "=>")) {
                    line_counter++;

                    // Use stack to determine which line should be copied (for undo or redo instruction)
                    if(!strcmp(SQL_chinese, " 復原:")) {
                        // CASE ... UNDO
                        if(!strcmp(word, "INSERT") || !strcmp(word, "UPDATE") || !strcmp(word, "DELETE") || !strcmp(word, "redo")) {
                            node_number = create_node_number(line_counter);
                            line_push(file_line, node_number);
                        } else if(!strcmp(word, "undo")) {
                            node_number = line_pop(file_line);
                        }
                    } else {
                        // CASE ... REDO
                        if(!strcmp(word, "undo")) {
                            node_number = create_node_number(line_counter);
                            line_push(file_line, node_number);
                        } else if(!strcmp(word, "redo")) {
                            node_number = line_pop(file_line);
                        }
                    }
                }
            }

            // Write the information of UNDO or REDO into log.txt
            fptr_read = fopen("log.txt", "r");
            for(line_counter=0; fscanf(fptr_read, "%s", word)!=EOF; ) {
                // Count total number of lines
                if(!strcmp(word, "INSERT") || !strcmp(word, "SELECT") || !strcmp(word, "UPDATE") || !strcmp(word, "DELETE") ||
                   !strcmp(word, "undo")   || !strcmp(word, "redo")   || !strcmp(word, "QUIT")   || !strcmp(word, "結束此次紀錄") || !strcmp(word, "=>")) {
                    line_counter++;
                }

                // Write the information of UNDO or REDO into log.txt
                if((!strcmp(SQL_chinese, " 復原:") && line_counter > node_number->number && !strcmp(word, "=>")) ||
                   (!strcmp(SQL_chinese, "")       && line_counter > node_number->number && !strcmp(word, "復原:"))) {
                    fprintf(fptr, "=>%s", SQL_chinese);
                    for(character='\0'; character!='\n'; ) {
                        character = fgetc(fptr_read);
                        fprintf(fptr, "%c", character);
                    }
                } else if(line_counter > node_number->number && strcmp(word, "=>")) {
                    break;
                }
            }
        }
    } else if(log_case == 0) {
        // CASE ... group table
        fprintf(fptr, "=> 資料表group(name , group_name)%s了一筆資料(%s , %s)。\n", SQL_chinese, current_group->name, current_group->group_name);

    } else if(log_case == 1) {
        // CASE ... people table
        fprintf(fptr, "=> 資料表people(name , phone , birthday , name)%s了一筆資料(%s , %s , %s , %s)。\n", SQL_chinese, current_people->name, current_people->phone, current_people->birthday, current_people->email);

    } else if(log_case == 2) {
        // CASE ... Write SQL instruction into log.txt
        NODE_WORD *current_word = input->head->next;

        fprintf(fptr, "%s", input->head->word);
        while(current_word != NULL) {
            fprintf(fptr, " %s", current_word->word);
            current_word = current_word->next;
        }
        fprintf(fptr, "\n");

    } else if(log_case == 3) {
        // CASE ... Write the information of END
        fprintf(fptr, "--------------------------------------- %s ---------------------------------------\n\n\n", SQL_chinese);
    }

    fclose(fptr);
}

DATABASE *insert_from_file() {
    char name[20], phone[15], birthday[10], email[50], group_name[20];
    DATABASE *database = create_database();

    // Read data from people.txt
    char buffer_people[20+15+10+50], *token;
    NODE_PEOPLE *node_people = NULL;
    FILE *fptr_people = fopen("people.txt", "r");
    if(fptr_people != NULL) {
        while(fgets(buffer_people, 20+15+10+50, fptr_people) != NULL) {
            buffer_people[strlen(buffer_people)-1] = '\0';

            // Parse string into name, phone, birthday and email
            token = strtok(buffer_people, " "); // name
            strcpy(name, token);
            token = strtok(NULL, " "); // phone
            strcpy(phone, token);
            token = strtok(NULL, " "); // birthday
            strcpy(birthday, token);
            token = strtok(NULL, " "); // email
            strcpy(email, token);

            // Insert values into database
            node_people = create_node_people(name, phone, birthday, email);
            database_insert_node(database, node_people, NULL, 1, 1);
        }
    }
    fclose(fptr_people);

    // Read data from group.txt
    char buffer_group[20+20];
    NODE_GROUP *node_group = NULL;
    FILE *fptr_group = fopen("group.txt", "r");
    if(fptr_group != NULL) {
        while(fgets(buffer_group, 20+20, fptr_group) != NULL) {
            buffer_group[strlen(buffer_group)-1] = '\0';

            // Parse string into name and group_name
            token = strtok(buffer_group, " "); // name
            strcpy(name, token);
            token = strtok(NULL, " "); // group_name
            strcpy(group_name, token);

            // Insert values into database
            node_group = create_node_group(name, group_name);
            database_insert_node(database, NULL, node_group, 0, 1);
        }
    }
    fclose(fptr_group);

    return database;
}

void save_to_file(DATABASE *database) {
    // Write the status of database (people) into people.txt
    FILE *fptr_people = fopen("people.txt", "w");
    NODE_PEOPLE *current_people = database->head_people;
    if(fptr_people != NULL) {
        while(current_people != NULL) {
            fprintf(fptr_people, "%s %s %s %s\n", current_people->name, current_people->phone, current_people->birthday, current_people->email);
            current_people = current_people->next;
        }
    }
    fclose(fptr_people);

    // Write the status of database (group) into group.txt
    FILE *fptr_group = fopen("group.txt", "w");
    NODE_GROUP *current_group = database->head_group;
    if(fptr_group != NULL) {
        while(current_group != NULL) {
            fprintf(fptr_group, "%s %s\n", current_group->name, current_group->group_name);
            current_group = current_group->next;
        }
    }
    fclose(fptr_group);
}
