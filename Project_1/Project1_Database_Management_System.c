/****************************************
 * Author: Meng-Shiun, Tsai
 * Program: Database Management System
    1. Parse SQL instruction
    2. Use linked list to store data
****************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node {
    char name[50], phone[15], birthday[10], email[50];
    struct node *next;
}NODE;

typedef struct database {
    int counter;
    struct node *head;
}DATABASE;

// Deal with linked list
DATABASE* create_database();
NODE* create_node(char*, char*, char*, char*);
void insert_node(DATABASE*, NODE*);
void select_node(NODE*, int[], char*, char*);
void update_node(NODE*, char*, char*, char*, char*);
void delete_node(DATABASE*, char*, char*);

// Parse SQL instruction
void insert_from_SQL(DATABASE*, char*);
void select_from_SQL(DATABASE*, char*);
void update_from_SQL(DATABASE*, char*);
void delete_from_SQL(DATABASE*, char*);

// Deal with file I/O
void insert_from_file(DATABASE*);
void save_to_file(NODE*);

int main(int argc, const char * argv[]) {
    char input[200];
    DATABASE *database = create_database();

    // Read data from people.txt
    insert_from_file(database);

    // Execute instruction according the input
    while(1) {
        fgets(input, 200, stdin);

        if (input[0] == 'Q') {
            break;
        } else if(input[0] == 'I') {
            insert_from_SQL(database, input);
        } else if(input[0] == 'S') {
            select_from_SQL(database, input);
        } else if(input[0] == 'U') {
            update_from_SQL(database, input);
        } else if(input[0] == 'D') {
            delete_from_SQL(database, input);
        }

        if(input[0] == 'I' || input[0] == 'U' || input[0] == 'D') {
            save_to_file(database->head);
        }
    }

    return 0;
}

DATABASE* create_database() {
    DATABASE *database = (DATABASE*)malloc(sizeof(DATABASE));
    database->head = NULL;
    database->counter = 0;

    return database;
}

NODE* create_node(char *name, char *phone, char *birhtday, char *email) {
    NODE *node = (NODE*)malloc(sizeof(NODE));
    strcpy(node->name, name);
    strcpy(node->phone, phone);
    strcpy(node->birthday, birhtday);
    strcpy(node->email, email);
    node->next = NULL;

    return node;
}

void insert_node(DATABASE *database, NODE *node) {
    if(database->head == NULL) {
        database->head = node;
    } else {
        NODE *current = database->head;
        while(current->next != NULL) {
            current = current->next;
        }
        current->next = node;
    }
    (database->counter)++;
}

void select_node(NODE *current, int select_columns[], char *where_column, char *where_value) {
    while(current != NULL) {
        // Check if matching select condition
        if(strlen(where_value) == 0 ||
           (!strcmp(where_column, "name")     && !strcmp(current->name, where_value))     ||
           (!strcmp(where_column, "phone")    && !strcmp(current->phone, where_value))    ||
           (!strcmp(where_column, "birthday") && !strcmp(current->birthday, where_value)) ||
           (!strcmp(where_column, "email")    && !strcmp(current->email, where_value))) {
            // Show columns chosen by user
            if(select_columns[0] == 1) {
                printf("%20s", current->name);
            }
            if(select_columns[1] == 1) {
                printf("%15s", current->phone);
            }
            if(select_columns[2] == 1) {
                printf("%10s", current->birthday);
            }
            if(select_columns[3] == 1) {
                printf("%30s", current->email);
            }
            printf("\n");
        }
        current = current->next;
    }
}

void update_node(NODE *current, char *update_column, char *update_value, char *where_column, char *where_value) {
    while(current != NULL) {
        // Check if matching update condition
        if((!strcmp(where_column, "name")     && !strcmp(current->name, where_value))     ||
           (!strcmp(where_column, "phone")    && !strcmp(current->phone, where_value))    ||
           (!strcmp(where_column, "birthday") && !strcmp(current->birthday, where_value)) ||
           (!strcmp(where_column, "email")    && !strcmp(current->email, where_value))) {
            // Update the specific column chosen by user
            if(!strcmp(update_column, "name")) {
                strcpy(current->name, update_value);
            } else if(!strcmp(update_column, "phone")) {
                strcpy(current->phone, update_value);
            } else if(!strcmp(update_column, "birthday")) {
                strcpy(current->birthday, update_value);
            } else if(!strcmp(update_column, "email")) {
                strcpy(current->email, update_value);
            }
        }
        current = current->next;
    }
}

void delete_node(DATABASE *database, char *where_column, char *where_value) {
    NODE *current=database->head, *previous=NULL;
    while(current != NULL) {
        // Check if matching delete condition
        if((!strcmp(where_column, "name")     && !strcmp(current->name, where_value))     ||
           (!strcmp(where_column, "phone")    && !strcmp(current->phone, where_value))    ||
           (!strcmp(where_column, "birthday") && !strcmp(current->birthday, where_value)) ||
           (!strcmp(where_column, "email")    && !strcmp(current->email, where_value))) {
            if(previous == NULL) {
                // Delete head node
                database->head = current->next;
                free(current);
                current = database->head;
            } else {
                // Delete node except for head
                previous->next = current->next;
                free(current);
                current = previous->next;
            }
            (database->counter)--;
            continue;
        }
        previous = current;
        current = current->next;
    }
}

void insert_from_file(DATABASE *database) {
    char input[125], name[50], phone[15], birthday[10], email[50], *token;
    NODE *node = NULL;
    FILE *fptr = fopen("people.txt", "r");

    if (fptr != NULL) {
        // Read peopole.txt and Save name, phone, birthday, email to database
        while(fgets(name, 50, fptr) != NULL) {
            // Read name, phone, birthday and email from people.txt
            name[strlen(name)-1] = '\0';

            fgets(phone, 15, fptr);
            phone[strlen(phone)-1] = '\0';

            fgets(birthday, 10, fptr);
            birthday[strlen(birthday)-1] = '\0';

            fgets(email, 50, fptr);
            email[strlen(email)-1] = '\0';

            // Insert name, phone, birthday, email to database
            node = create_node(name, phone, birthday, email);
            insert_node(database, node);

            // Reset name, phone, birthday, email
            memset(name, '\0', 50);
            memset(phone, '\0', 15);
            memset(birthday, '\0', 10);
            memset(email, '\0', 50);
        }
    }

    fclose(fptr);
}

void insert_from_SQL(DATABASE *database, char *input) {
    int i=0, token_condition, column_counter=0;
    char name[20], name_else[30], phone[15], birthday[10], email[50];
    char columns_and_values[8][50], *token;
    NODE *node = NULL;

    // Initialize all strings
    for(i=0; i<8; i++) {
        memset(columns_and_values[i], '\0', 50);
    }
    memset(name, '\0', 20);
    memset(name_else, '\0', 30);
    memset(phone, '\0', 15);
    memset(birthday, '\0', 10);
    memset(email, '\0', 50);

    // Parse insert instruction into column and values
    token = strtok(input, " ");
    token_condition = 0;
    while(token != NULL) {
        if(token_condition == 1) {
            strcpy(columns_and_values[column_counter], token);
            column_counter++;
            token_condition = 0;
        } else {
            if(!strcmp(token, "INSERT") || !strcmp(token, "INTO") || !strcmp(token, "people") || !strcmp(token, "values") || !strcmp(token, ")") || !strcmp(token, ")\n")) {
                token_condition = 0;
            } else if(!strcmp(token, "(") || !strcmp(token, ",")) {
                token_condition = 1;
            } else {
                strcat(name_else, " ");
                strcat(name_else, token);
            }
        }
        token = strtok(NULL, " ");
    }

    // Save values according to related column
    column_counter = column_counter / 2;
    for (i=0; i<column_counter; i++) {
        if(!strcmp(columns_and_values[i], "name")) {
            strcpy(name, columns_and_values[i+column_counter]);
            strcat(name, name_else);
        } else if(!strcmp(columns_and_values[i], "phone")) {
            strcpy(phone, columns_and_values[i+column_counter]);
        } else if(!strcmp(columns_and_values[i], "birthday")) {
            strcpy(birthday, columns_and_values[i+column_counter]);
        } else if(!strcmp(columns_and_values[i], "email")) {
            strcpy(email, columns_and_values[i+column_counter]);
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

    // Insert values into database
    node = create_node(name, phone, birthday, email);
    insert_node(database, node);
}

void select_from_SQL(DATABASE *database, char *input) {
    int i=0, token_condition=0, column_counter=0;
    int select_columns[4]={0};
    char where_column[10], where_value[50], *token;
    NODE *node = NULL;

    // Initialize all strings
    memset(where_column, '\0', 10);
    memset(where_value, '\0', 50);

    // Parse select instruction
    token = strtok(input, " ");
    token_condition = 0;
    while(token != NULL) {
        if(token_condition == 1) {
            if(!strcmp(token, "*")) {
                for(i=0; i<4; i++) {
                    select_columns[i] = 1;
                }
            } else {
                if(!strcmp(token, "name")) {
                    select_columns[0] = 1;
                } else if(!strcmp(token, "phone")) {
                    select_columns[1] = 1;
                } else if(!strcmp(token, "birthday")) {
                    select_columns[2] = 1;
                } else if(!strcmp(token, "email")) {
                    select_columns[3] = 1;
                }
            }
            token_condition = 0;
        } else {
            if(!strcmp(token, "SELECT") || !strcmp(token, ",")) {
                token_condition = 1;
            } else if(!strcmp(token, "WHERE")) {
                // WHERE column
                token = strtok(NULL, " ");
                strcpy(where_column, token);

                // WHERE value
                token = strtok(NULL, " ");
                token = strtok(NULL, "\n");
                strcpy(where_value, token);
                break;
            } else {
                token_condition = 0;
            }
        }
        token = strtok(NULL, " ");
    }

    // Select from database
    select_node(database->head, select_columns, where_column, where_value);
}

void update_from_SQL(DATABASE *database, char *input) {
    char update_column[10], update_value[50], where_column[10], where_value[50], *token;

    // Initialize all strings
    memset(update_column, '\0', 10);
    memset(update_value, '\0', 50);
    memset(where_column, '\0', 10);
    memset(where_value, '\0', 50);

    // Parse update instruction
    token = strtok(input, " "); // UPDATE
    token = strtok(NULL, " ");  // people
    token = strtok(NULL, " ");  // SET

    token = strtok(NULL, " ");  // update_column
    strcpy(update_column, token);
    token = strtok(NULL, " ");  // =
    token = strtok(NULL, " ");  // update_value
    strcpy(update_value, token);
    token = strtok(NULL, " ");  // update_value or WHERE
    while(strcmp(token, "WHERE")) {
        strcat(update_value, " ");
        strcat(update_value, token);
        token = strtok(NULL, " ");
    }

    token = strtok(NULL, " ");   // where_column
    strcpy(where_column, token);
    token = strtok(NULL, " ");   // =
    token = strtok(NULL, "\n");  // where_value
    strcpy(where_value, token);

    // Update data from database
    update_node(database->head, update_column, update_value, where_column, where_value);
}

void delete_from_SQL(DATABASE *database, char *input) {
    char where_column[10], where_value[50], *token;

    // Initialize all strings
    memset(where_column, '\0', 10);
    memset(where_value, '\0', 50);

    // Parse delete instruction
    token = strtok(input, " "); // DELETE
    token = strtok(NULL, " ");  // FROM
    token = strtok(NULL, " ");  // people

    token = strtok(NULL, " ");  // WHERE
    token = strtok(NULL, " ");   // where_column
    strcpy(where_column, token);
    token = strtok(NULL, " ");  // =
    token = strtok(NULL, "\n");    // where_value
    strcpy(where_value, token);

    // Update data from database
    delete_node(database, where_column, where_value);
}

void save_to_file(NODE *current){
    FILE *fptr = fopen("people.txt", "w");

    if(current == NULL) {
        fprintf(fptr, "");
    } else {
        while(current != NULL) {
            fputs(current->name, fptr);
            fputs("\n", fptr);
            fputs(current->phone, fptr);
            fputs("\n", fptr);
            fputs(current->birthday, fptr);
            fputs("\n", fptr);
            fputs(current->email, fptr);
            fputs("\n", fptr);

            current = current->next;
        }
    }

    fclose(fptr);
}
