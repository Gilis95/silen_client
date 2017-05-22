#ifndef PERSON_H
#define PERSON_H
typedef struct person{
    char pin[11];
    char name[20];
    char date_of_birth[11];
    char parent_PIN[2][11];
    short killed;
}person;

void kill_person(const char*);
void give_birth(person*);
void intializeTree();

#endif // PERSON_H
