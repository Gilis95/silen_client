#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "src/headers/person.h"
#include "src/headers/communication_header.h"

#define true 1
#define false 0

char* inputString(FILE*, size_t);

int main(void)

{
    int running = 1;
    int msgid;
    long int msg_to_recieve = 0;
    generic_msg generic_messages;
    kill_person_msg kill_person_messages;
    person_msg person_message;


    char* info = "-----------INFO-----------\n";
    char* list = "1 - List family tree\n";
    char* insert = "2 - Inser newborn child\n";
    char* remove = "3 - Declare person deth\n";
    char* stop = "666 - Quit client and stop server\n";

    char* name = "Name:\n";
    char* date_of_birth = "Date of birth:\n";
    char* parent_pin_1 = "First parent pin:\n";
    char* parent_pin_2 = "Second parent pin:\n";
    char* child_pin = "Child pin:\n";

    char* person_pin = "Person pin:\n";


    /* Let us set up the message queue */
    msgid = msgget(ftok("dqvola",(key_t)666), 0660| IPC_PRIVATE);

    if (msgid == -1) {
        perror("msgget failed with error");
        exit(EXIT_FAILURE);
    }
    /* Then the messages are retrieved from the queue, until an end message is
     * encountered. lastly the message queue is deleted
     */

    while(running) {
        write(STDOUT_FILENO,info,strlen(info)*sizeof(char));
        write(STDOUT_FILENO,list,strlen(list)*sizeof(char));
        write(STDOUT_FILENO,insert,strlen(insert)*sizeof(char));
        write(STDOUT_FILENO,remove,strlen(remove)*sizeof(char));
        write(STDOUT_FILENO,stop,strlen(stop)*sizeof(char));

        write(STDOUT_FILENO,"> ",sizeof(char));

        /* Read wanted operation to be executed */
        scanf("%ld",&generic_messages.my_msg_type);
        /* Send to server message to prepare for wanted operation */
        if (msgsnd(msgid, &generic_messages, sizeof(short)*2,0) == -1) {
            fprintf(stderr, "msgrcv failed with error: %s\n",
                    strerror(errno));
            exit(EXIT_FAILURE);
        }

        switch(generic_messages.my_msg_type){
        case LIST:
            while(1){
                if (msgrcv(msgid, &person_message, sizeof(person_msg),
                           msg_to_recieve, 0) == -1) {
                    perror("msgcrv failed with error");
                    exit(EXIT_FAILURE);
                }
                if(person_message.my_msg_type == ITERATE_END){
                    break;
                }
                printf("Person name: %s\n",person_message.person.name);
                printf("Person date of birth: %s\n",person_message.person.date_of_birth);
                printf("First parent pin: %s\n",person_message.person.parent_PIN[0]);
                printf("Second parent pin: %s\n",person_message.person.parent_PIN[1]);
                printf("Person pin: %s\n",person_message.person.pin);
                printf("Is person dead: %hd\n",person_message.person.killed);


                /* Override memmory of insert_person in order not to messup on next query */
                memset(person_message.person.name,0,sizeof(person_message.person.name));
                memset(person_message.person.date_of_birth,0,sizeof(person_message.person.date_of_birth));
                memset(person_message.person.parent_PIN[1],0, sizeof(person_message.person.parent_PIN[1]));
                memset(person_message.person.parent_PIN[0],0, sizeof(person_message.person.parent_PIN[0]));
                memset(person_message.person.pin,0, sizeof(person_message.person.pin));
            }
            break;
        case INSERT_PERSON:
            /* read data from console */
            write(STDOUT_FILENO,name,sizeof(char)*strlen(name));
            write(STDOUT_FILENO,"> ",sizeof(char));
            scanf("%s",person_message.person.name);
            write(STDOUT_FILENO,date_of_birth,sizeof(char)*strlen(date_of_birth));
            write(STDOUT_FILENO,"> ",sizeof(char));
            scanf("%s",person_message.person.date_of_birth);
            write(STDOUT_FILENO,parent_pin_1,sizeof(char)*strlen(parent_pin_1));
            write(STDOUT_FILENO,"> ",sizeof(char));
            scanf("%s",person_message.person.parent_PIN[0]);
            write(STDOUT_FILENO,parent_pin_2,sizeof(char)*strlen(parent_pin_2));
            write(STDOUT_FILENO,"> ",sizeof(char));
            scanf("%s",person_message.person.parent_PIN[1]);
            write(STDOUT_FILENO,child_pin,sizeof(char)*strlen(child_pin));
            write(STDOUT_FILENO,"> ",sizeof(char));
            scanf("%s",person_message.person.pin);
            person_message.person.killed=0;

            person_message.my_msg_type = generic_messages.my_msg_type;

            /* Send message to server*/
            if (msgsnd(msgid, &person_message,sizeof(person_msg), 0) == -1) {
                perror("msgcrv failed with error");
                exit(EXIT_FAILURE);
            }

            /* Override memmory of insert_person in order not to messup on next query */
            memset(person_message.person.name,0,sizeof(person_message.person.name));
            memset(person_message.person.date_of_birth,0,sizeof(person_message.person.date_of_birth));
            memset(person_message.person.parent_PIN[1],0, sizeof(person_message.person.parent_PIN[1]));
            memset(person_message.person.parent_PIN[0],0, sizeof(person_message.person.parent_PIN[0]));
            memset(person_message.person.pin,0, sizeof(person_message.person.pin));

            break;
        case KILL_PERSON:
            /* Override memmory of kill person in order not to messup characters */
            memset(kill_person_messages.pin,0,sizeof(kill_person_messages.pin));


            /* read data from console */
            write(STDOUT_FILENO,person_pin,sizeof(char)*strlen(person_pin));
            write(STDOUT_FILENO,"> ",sizeof(char));
            scanf("%s",kill_person_messages.pin);

            kill_person_messages.my_msg_type = generic_messages.my_msg_type;

            /* Send message to server*/
            if (msgsnd(msgid, &kill_person_messages,sizeof(kill_person_messages), 0) == -1) {
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
