#ifndef COMMUNICATION_HEADER_H
#define COMMUNICATION_HEADER_H
#include "person.h"

/* Message used for inserting new person into tree */
typedef struct person_message {
    long int my_msg_type;
    person person;
} person_msg;

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
#endif // COMMUNICATION_HEADER_H
#ifndef COMMUNICATION_CONSTANTS
#define COMMUNICATION_CONSTANTS

#define STOP 666
#define INSERT_PERSON 2
#define KILL_PERSON 3
#define LIST 1
#define ITERATE_END 555


#endif
