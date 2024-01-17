#include "../header/header.h"
/**
    Student Name: Xu Jiang
    simple source file containing the data structure to store dictionary
*/
typedef struct SingleWord{
    char str[51];
}Word;
typedef struct Dictionary{
    Word *words;
    int size;
}Dict;

