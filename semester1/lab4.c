#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct List
{
    struct List *next;
    char *string;
} List;

List *addElem(List *list, char *string) {
    if (list == NULL) {
        List *new = malloc(sizeof(List));
        new->string = malloc(strlen(string) + 1);
        strcpy(new->string, string);
        new->next = NULL;
        return new;
    }

    List *current = list;
    while(current->next != NULL) {
        current = current->next;
    }
    current->next = malloc(sizeof(List));
    current->next->string = malloc(strlen(string) + 1);
    strcpy(current->next->string, string);
    current->next->next = NULL;
    return list;
}

void deleteElem(List *list) {
    free(list->string);
    free(list);
}

int main()
{
    char str[BUFSIZ];
    List *list = NULL, *current, *tmp;

    fgets(str, BUFSIZ, stdin);
    while(str[0] != '.'){
        list = addElem(list, str);
        fgets(str, BUFSIZ, stdin);
    }

    current = list;
    while(current != NULL) {
        printf("%s", current->string);
        current = current->next;
    }

    current = list;
    while (current != NULL)
    {
        tmp = current->next;
        deleteElem(current);
        current = tmp;
    }
}