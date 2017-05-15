#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define STOP 0
#define INSERT_PERSON 1
#define KILL_PERSON 2

#define true 1
#define false 0

/* person structure */
typedef struct person{
    char pin[10];
    char name[20];
    char date_of_birth[11];
    char parent_PIN[2][10];
    short killed;
}person;

/* Message used for inserting new person into tree */
typedef struct insert_person_message {
    long int my_msg_type;
    person person;
}insert_person_msg;

/* Message used for killing person with PIN */
typedef struct kill_person_message{
    long int my_msg_type;
    char pin[10];
}kill_person_msg;

/* Message specifying next operation */
typedef struct generic_message{
    long int my_msg_type;
    char operation[1];
}generic_msg;

char* inputString(FILE*, size_t);

int main(void)

{
    int running = 1;
    int msgid;
    generic_msg generic_messages;
    kill_person_msg kill_person_messages;
    insert_person_msg insert_person_message;


    char* info = "-------INFO-------\n";
    char* insert = "1 - Inser newborn child\n";
    char* remove = "2 - Declare person deth\n";

    char* name = "Name:\n";
    char* date_of_birth = "Date of birth:\n";
    char* parent_pin_1 = "First parent pin:\n";
    char* parent_pin_2 = "Second parent pin:\n";
    char* child_pin = "Child pin:\n";

    char* person_pin = "Person pin:\n";


    /* Let us set up the message queue */
    msgid = msgget(ftok("pesho",(key_t)1234), 0660| IPC_PRIVATE);

    if (msgid == -1) {
        perror("msgget failed with error");
        exit(EXIT_FAILURE);
    }
    /* Then the messages are retrieved from the queue, until an end message is
     * encountered. lastly the message queue is deleted
     */

    while(running) {
        write(STDOUT_FILENO,info,strlen(info)*sizeof(char));
        write(STDOUT_FILENO,insert,strlen(insert)*sizeof(char));
        write(STDOUT_FILENO,remove,strlen(remove)*sizeof(char));

        write(STDOUT_FILENO,"> ",sizeof(char));

        /* Read wanted operation to be executed */
        scanf("%ld",&generic_messages.my_msg_type);
        /* Send to server message to prepare for wanted operation */
        if (msgsnd(msgid, &generic_messages, sizeof(short)*2,0) == -1) {
            fprintf(stderr, "msgrcv failed with error: %s\n",
                    strerror(errno));
            exit(EXIT_FAILURE);
        }

        write(STDOUT_FILENO,"> ",sizeof(char));

        switch(generic_messages.my_msg_type){
        case INSERT_PERSON:
            /* read data from console */
            write(STDOUT_FILENO,name,sizeof(char)*strlen(name));
            write(STDOUT_FILENO,"> ",sizeof(char));
            read(STDIN_FILENO,insert_person_message.person.name,sizeof(insert_person_message.person.name));
            write(STDOUT_FILENO,date_of_birth,sizeof(char)*strlen(date_of_birth));
            write(STDOUT_FILENO,"> ",sizeof(char));
            read(STDIN_FILENO,insert_person_message.person.date_of_birth,sizeof(insert_person_message.person.date_of_birth));
            write(STDOUT_FILENO,parent_pin_1,sizeof(char)*strlen(parent_pin_1));
            write(STDOUT_FILENO,"> ",sizeof(char));
            read(STDIN_FILENO,insert_person_message.person.parent_PIN[0], sizeof(insert_person_message.person.parent_PIN[0]));
            fflush(stdin);
            write(STDOUT_FILENO,parent_pin_2,sizeof(char)*strlen(parent_pin_2));
            fflush(stdin);
            write(STDOUT_FILENO,"> ",sizeof(char));
            fflush(stdin);
            read(STDIN_FILENO,insert_person_message.person.parent_PIN[1], sizeof(insert_person_message.person.parent_PIN[1]));
            write(STDOUT_FILENO,child_pin,sizeof(char)*strlen(child_pin));
            write(STDOUT_FILENO,"> ",sizeof(char));
            read(STDIN_FILENO,insert_person_message.person.pin,sizeof(insert_person_message.person.pin));
            insert_person_message.person.killed=0;

            insert_person_message.my_msg_type = generic_messages.my_msg_type;

            /* Send message to server*/
            if (msgsnd(msgid, &insert_person_message,sizeof(insert_person_msg), 0) == -1) {
                perror("msgcrv failed with error");
                exit(EXIT_FAILURE);
            }

            /* Override memmory of insert_person in order not to messup on next query */
            memset(insert_person_message.person.name,0,sizeof(insert_person_message.person.name));
            memset(insert_person_message.person.date_of_birth,0,sizeof(insert_person_message.person.date_of_birth));
            memset(insert_person_message.person.parent_PIN[1],0, sizeof(insert_person_message.person.parent_PIN[1]));
            memset(insert_person_message.person.parent_PIN[0],0, sizeof(insert_person_message.person.parent_PIN[0]));
            memset(insert_person_message.person.pin,0, sizeof(insert_person_message.person.pin));

            break;
        case KILL_PERSON:
            /* read data from console */
            write(STDOUT_FILENO,person_pin,sizeof(char)*strlen(person_pin));
            write(STDOUT_FILENO,"> ",sizeof(char));
            read(STDIN_FILENO,kill_person_messages.pin,sizeof(kill_person_messages.pin));

            kill_person_messages.my_msg_type = generic_messages.my_msg_type;

            /* Send message to server*/
            if (msgsnd(msgid, &kill_person_messages,sizeof(kill_person_messages), 0) == -1) {
                perror("msgcrv failed with error");
                exit(EXIT_FAILURE);
            }

            /* Override memmory of insert_person in order not to messup on next query */
            memset(kill_person_messages.pin,0,sizeof(kill_person_messages.pin));

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
