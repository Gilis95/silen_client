#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctype.h>
#include "src/headers/person.h"
#include "src/headers/communication_header.h"

#define true 1
#define false 0

char *get_input_string(FILE *, size_t);
char *get_string_with_fixed_lenght(FILE *, char *, size_t, size_t);
int is_date_valid(int , int , int);
char *get_pin(FILE *, char *) ;
char *get_birth_date(FILE *, char *);

int main(void) {
    int running = 1;
    int msgid;
    long int msg_to_recieve = 0;
    generic_msg generic_messages;
    kill_person_msg kill_person_messages;
    person_msg person_message;


    char* info = "-----------INFO-----------\n";
    char* list = "1 - List family tree\n";
    char* insert = "2 - Insert newborn child\n";
    char* remove = "3 - Declare person dead\n";
    char* stop = "666 - Quit client and stop server\n";

    char* name = "Name:\n";
    char* date_of_birth = "Date of birth:\n";
    char* parent_pin_1 = "First parent pin:\n";
    char* parent_pin_2 = "Second parent pin:\n";
    char* child_pin = "Child pin:\n";

    char* person_pin = "Person pin:\n";


    /* Let us set up the message queue */
    msgid = msgget(ftok("dqvola", (key_t)666), 0660 | IPC_PRIVATE);

    if (msgid == -1) {
        perror("msgget failed with error");
        exit(EXIT_FAILURE);
    }

    /*
     * Then the messages are retrieved from the queue, until an end message is
     * encountered. Lastly the message queue is deleted.
     */

    while (running) {
        write(STDOUT_FILENO, info, strlen(info)*sizeof(char));
        write(STDOUT_FILENO, list, strlen(list)*sizeof(char));
        write(STDOUT_FILENO, insert, strlen(insert)*sizeof(char));
        write(STDOUT_FILENO, remove, strlen(remove)*sizeof(char));
        write(STDOUT_FILENO, stop, strlen(stop)*sizeof(char));

        write(STDOUT_FILENO, "> ", sizeof(char));

        /* Read wanted operation to be executed */
        scanf("%ld", &generic_messages.my_msg_type);

        /* Send to server message to prepare for wanted operation */
        if (msgsnd(msgid, &generic_messages, sizeof(short) * 2, 0) == -1) {
            fprintf(stderr, "msgrcv failed with error: %s\n",
                    strerror(errno));
            exit(EXIT_FAILURE);
        }

        switch (generic_messages.my_msg_type) {
        case LIST:
            while (1) {
                if (msgrcv(msgid, &person_message, sizeof(person_msg),
                           msg_to_recieve, 0) == -1) {
                    perror("msgcrv failed with error");
                    exit(EXIT_FAILURE);
                }

                if (person_message.my_msg_type == ITERATE_END) {
                    break;
                }

                printf("----------------------------------------------------------\n");
                printf("Person name: %s\n", person_message.person.name);
                printf("Person date of birth: %s\n", person_message.person.date_of_birth);
                printf("First parent pin: %s\n", person_message.person.parent_PIN[0]);
                printf("Second parent pin: %s\n", person_message.person.parent_PIN[1]);
                printf("Person pin: %s\n", person_message.person.pin);
                printf("Is person dead: %hd\n", person_message.person.killed);

                /* Override memmory of insert_person in order not to messup on next query */
                memset(person_message.person.name, 0, sizeof(person_message.person.name));
                memset(person_message.person.date_of_birth, 0, sizeof(person_message.person.date_of_birth));
                memset(person_message.person.parent_PIN[1], 0, sizeof(person_message.person.parent_PIN[1]));
                memset(person_message.person.parent_PIN[0], 0, sizeof(person_message.person.parent_PIN[0]));
                memset(person_message.person.pin, 0, sizeof(person_message.person.pin));
            }

            printf("----------------------------------------------------------\n");
            break;

        case INSERT_PERSON:
            strcpy(person_message.person.name , get_string_with_fixed_lenght(stdin, name, 2, 20));
            strcpy(person_message.person.date_of_birth , get_birth_date(stdin, date_of_birth));
            strcpy(person_message.person.parent_PIN[0] , get_pin(stdin, parent_pin_1));
            strcpy(person_message.person.parent_PIN[1] , get_pin(stdin, parent_pin_2));
            strcpy(person_message.person.pin , get_pin(stdin, child_pin));
            person_message.person.killed = 0;

            person_message.my_msg_type = generic_messages.my_msg_type;

            /* Send message to server*/
            if (msgsnd(msgid, &person_message, sizeof(person_msg), 0) == -1) {
                perror("msgcrv failed with error");
                exit(EXIT_FAILURE);
            }

            /* Override memmory of insert_person in order not to messup on next query */
            memset(person_message.person.name, 0, sizeof(person_message.person.name));
            memset(person_message.person.date_of_birth, 0, sizeof(person_message.person.date_of_birth));
            memset(person_message.person.parent_PIN[1], 0, sizeof(person_message.person.parent_PIN[1]));
            memset(person_message.person.parent_PIN[0], 0, sizeof(person_message.person.parent_PIN[0]));
            memset(person_message.person.pin, 0, sizeof(person_message.person.pin));

            break;

        case KILL_PERSON:
            /* Override memmory of kill person in order not to messup characters */
            memset(kill_person_messages.pin, 0, sizeof(kill_person_messages.pin));


            strcpy(kill_person_messages.pin, get_pin(stdin, person_pin));

            kill_person_messages.my_msg_type = generic_messages.my_msg_type;

            /* Send message to server*/
            if (msgsnd(msgid, &kill_person_messages, sizeof(kill_person_messages), 0) == -1) {
                perror("msgcrv failed with error");
                exit(EXIT_FAILURE);
            }

            break;

        case STOP:
            running = 0;
        }

    }

    if (msgctl(msgid, IPC_RMID, 0) == -1) {
        perror("msgctl(IPC_RMID) failed");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}


char *get_input_string(FILE* fp, size_t size) {
    //The size is extended by the input with the value of the provisional
    char *str;
    int ch;
    size_t len = 0;
    str = realloc(NULL, sizeof(char) * size); //size is start size

    if (!str)return str;

    fgetc(fp);

    while (EOF != (ch = fgetc(fp)) && ch != '\n') {
        str[len++] = ch;

        if (len == size) {
            str = realloc(str, sizeof(char) * (size += 16));

            if (!str)return str;
        }
    }

    str[len++] = '\0';

    return realloc(str, sizeof(char) * len);
}

/**
  * @param fp descriptor from which input will be gotten
  * @param message_to_print message which describe to user what input is expected
  * @param min_size minimal expected size of client input
  * @param max_size maximum expected size of client input
  *
  * @return pointer to the read string
  */
char *get_string_with_fixed_lenght(FILE *fp, char *message_to_print, size_t min_size, size_t max_size) {
    char *string = "";

    do {
        if (string != NULL) {
            memset(string, 0, strlen(string)*sizeof(char));
        }

        write(STDOUT_FILENO, message_to_print, sizeof(char)*strlen(message_to_print));
        write(STDOUT_FILENO, "> ", sizeof(char));
        fflush(fp);
        usleep(10);
        string = get_input_string(fp, min_size);
    } while (strlen(string) > max_size || strlen(string) < min_size);

    return string;
}

int is_date_valid(int dd, int mm, int yy) {
    if (mm < 1 || mm > 12) {
        return false;
    }

    if (dd < 1) {
        return false;
    }

    int days = 31;

    switch (mm) {
    case 2:
        days = 28;

        if (yy % 400 == 0 || (yy % 4 == 0 && yy % 100 != 0)) {
            days = 29;
        }

    case 4:
    case 6:
    case 9:
    case 11:
        days = 30;
    }

    if (dd > days) {
        return false;
    }

    return true;
}

char *get_pin(FILE* fp, char *pin_message) {
    char *pin = "";
    int is_valid = false;
    int mm, dd, yy;
    int i;

    while (is_valid == false) {
        /** overrides old data */
        if (pin != NULL) {
            memset(pin, 0, strlen(pin)*sizeof(char));
        }

        /** get string whith fixed lenght from FD */
        pin = get_string_with_fixed_lenght(fp, pin_message, 10, 10);
        is_valid = true;

        for (i = 0; i < strlen(pin); i++) {
            if (!isdigit(*(pin + i))) {
                is_valid = false;
                break;
            }
        }

        if (is_valid == true) {
            /** get year month and day from pin */
            yy = (pin[0] - '0') * 10 + (pin[1] - '0');
            mm = (pin[2] - '0') * 10 + (pin[3] - '0');
            dd = (pin[4] - '0') * 10 + (pin[5] - '0');

            /** check if date of birth is valid */
            is_valid = is_date_valid(dd, mm, yy);
        }


    }

    return pin;
}

char *get_birth_date(FILE* fp, char *date_of_birth_message) {
    char *date = "";
    int is_valid = false;
    int yy, mm, dd;
    int i;

    while (is_valid == false) {
        if (date != NULL) {
            memset(date, 0, strlen(date)*sizeof(char));
        }

        /** get string whith fixed lenght from FD */
        date = get_string_with_fixed_lenght(fp, date_of_birth_message, 10, 10);

        /** temporary set validity to true */
        is_valid = true;

        for (i = 0; i < strlen(date); i++) {
            if (!isdigit(*(date + i)) && !(i == 2 || i == 5)) {
                /** oops not digit symbol different then delimeters */
                is_valid = false;
                break;
            }
        }

        /** check whether there symbol different than digit */
        if (is_valid == true) {
            /** get year month and day from string */
            dd = (date[0] - '0') * 10 + (date[1] - '0');
            mm = (date[3] - '0') * 10 + (date[4] - '0');
            yy = (date[6] - '0') * 1000 + (date[7] - '0') * 100 + (date[8] - '0') * 10 + (date[9] - '0');

            /** check whether client gave us valid date */
            is_valid = is_date_valid(dd, mm, yy);

        }
    }

    return date;
}
