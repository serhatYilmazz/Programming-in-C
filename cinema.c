#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
*
*   Author: Serhat YILMAZ
*   Date: 26.10.2016
*
*   The program is a console application written in C programming language.
*   It is basic cinema automation which can create new Salon(hall),
*   reserving sits according to student or full fare types, cancel this reserved sits, show sitting plan as a basic
*   visualization on console.
*/

/* Static and unchangable variables */
static const int STUDENT = 7;
static const int FULLFARE = 10;

void readTxt(); /* Reads input from txt file */
void process(char *); /* All process will be here  */
short categorize(char *); /* Categorize input according its command  */

/* Commands that will come from txt file */
void createHall(char *data[]);
void buyTicket(char *data[]);
void cancelTicket(char *data[]);
void showHall(char *);
void statistics();

/* Checking issues the relevant command */
int controlCreateHall(char *, char *);
int controlBuyTicket(char *, char *, int, int);
struct Hall *controlShowHall(char *);
void controlCancelTicket(char *name, char *sit);

/* Methods of Linked List */
void add(char *hallname, int width, int height, char *filmName);
void display();

struct Hall{
    char *hallName;
    char *filmName;
    char **size;
    int width;
    int height;
    int student;
    int fullFare;
    struct Hall *next;
}*hallList; /* User can create more than one hall. We can not know exact number. For that should be used linked lists  */

int main()
{
    hallList = NULL; /* Assigning linked list's initial value as NULL   */
    readTxt();  /*  HERE WE GO -->  */
    /*display();   */
    system("PAUSE");
    return 0;
}


void readTxt(){
    FILE *file = fopen("input.txt", "r");
    char line[250];
    while(fgets(line, sizeof(line), file)){  /*By using fgets() function, can be read a file line by line   */
        process(line);
    }
    fclose(file);
}

void process(char *line){
    /* -------------------------------------------INITIALIZING VARIABLES-------------------------------------------  */
    FILE *fileWrite = fopen("output.txt", "a"); /* Initialization for output file where the output will be printed in  */
    int i = 0;      /* counter for 'for' loops */
    char *token;    /* in "line" string, every characters of pointer should be recorded in 'token' */
    char *data[10]; /* Hold information for a line   */
    int counter = 0;    /* It will count how many whitespace exist in a line  */
    short flag;     /* It will be used for categorizing a line  */
    char tokenBackup[250];  /* It holds line array for future using.  */

    /* -------------------------------------------PROCESS PART OF FUNCTION-------------------------------------------  */
    strcpy(tokenBackup, line);  /* Hold line array   */
    token = strtok(line, " "); /*We have a long line containing lots of strings. It will be divided according to ' ' */
    flag = categorize(token);   /* We will get commands type e.g: if read from .txt CREATEHALL we will get relevant flag*/
    printf("\n");

    while(token != NULL){       /* division starts token by token    */
        data[i] = token;    /*While we are dividing line word by word, we have to save other informations about the command   */
        token = strtok(NULL, " ");
        i++;
    }
    switch(flag){   /* There is a returned flag which will be used here   */

        case 1:
            counter = 0;
            for(i = 0; tokenBackup[i] != '\0'; i++){    /* All cases include this command, it controls how many whitespace
                                                        exists in a line, if it is more than desired number of whitespace,
                                                            then throwing an exception*/
                if(tokenBackup[i] == ' ')
                    counter++;
            }
            if(counter > 4 ){       /* 4 is desired number of whitespace  */
                printf("ERROR :: Hall name and Film name can't include whitespace\n");
                fprintf(fileWrite, "ERROR :: Hall name and Film name can't include whitespace\n");
                fclose(fileWrite);
                return;
            }
            createHall(data);
            break;
        case 2:
            counter = 0;
            for(i = 0; tokenBackup[i] != '\0'; i++){
                if(tokenBackup[i] == ' ')
                    counter++;
            }
            if(counter > 4 ){
                printf("ERROR :: Film Name can't include whitespace\n");
                fprintf(fileWrite, "ERROR :: Film Name can't include whitespace\n");
                fclose(fileWrite);
                return;
            }
            buyTicket(data);
            break;
        case 3:

            counter = 0;
            for(i = 0; tokenBackup[i] != '\0'; i++){
                if(tokenBackup[i] == ' ')
                    counter++;
            }
            if(counter > 2 ){
                printf("ERROR :: Film Name can't include whitespace\n");
                fprintf(fileWrite, "ERROR :: Film Name can't include whitespace\n");
                fclose(fileWrite);
                return;
            }
            cancelTicket(data);
            break;
        case 4:
            counter = 0;
            for(i = 0; tokenBackup[i] != '\0'; i++){
                if(tokenBackup[i] == ' ')
                    counter++;
            }
            if(counter > 1 ){
                printf("ERROR :: Film Name and Hall can't include whitespace\n");
                fprintf(fileWrite, "ERROR :: Film Name and Hall can't include whitespace\n");
                fclose(fileWrite);
                return;
            }
            showHall(data[1]);
            break;
        case 5:
            statistics();
            break;
        default:
            printf("\nInvalid Command\n");      /*  If commands entered wrong, this case occurs   */
            fprintf(fileWrite, "\nInvalid Command\n");
            fclose(fileWrite);
            break;

    }
    fclose(fileWrite);
}

short categorize(char *token){      /* It is categorizing part   */
    /* For CREATEHALL     -> 1
       For BUYTICKET      -> 2
       For CANCELTICKET   -> 3
       For SHOWHALL       -> 4
       For STATISTICS     -> 5
    */
    short flag;
    if(strcmp(token, "CREATEHALL") == 0){
        flag = 1;
    }
    else if(strcmp(token, "BUYTICKET") == 0){
        flag = 2;
    }
     else if(strcmp(token, "CANCELTICKET") == 0){
        flag = 3;
    }
     else if(strcmp(token, "SHOWHALL") == 0){
        flag = 4;
    }
     else if(strcmp(token, "STATISTICS") == 0){
        flag = 5;
    }
    /*printf("Flag = %d", flag);  */
    return flag;
}
/*---------------------------------------------------------------------------------------*/
/* Commands that will come from txt file */
void createHall(char *data[]){
    /* -------------------------------------------INITIALIZING VARIABLES-------------------------------------------  */
    FILE *fileWrite = fopen("output.txt", "a");
    int width = atoi(data[3]);
    int height = atoi(data[4]);
    int flag = controlCreateHall(data[1], data[2]);  /* Sent hall name and film name and controls necessary things  */
    char *hallName;
    char *filmName;

    /* -------------------------------------------PROCESS PART OF FUNCTION-------------------------------------------  */
    if(flag == 0){ /* If necessities did not be satisfied then cancel process  */
        return;
    }
    hallName = strtok(data[1], "\"");   /* Seperation of hall name from double quotes */
    if(hallName == NULL){       /* When hall name seperated from double quotes, return "" means <null> cancel the process   */
        printf("Hall name will be created can't be empty\n");
        fprintf(fileWrite, "Hall name will be created can't be empty\n");
        fclose(fileWrite);
        return;
    }
    filmName = strtok(data[2], "\"");   /* Seperation of film name from double quotes */
    if(filmName == NULL){    /* When film name seperated from double quotes, return "" means <null> cancel the process   */
        printf("Film name will be created can't be empty\n");
        fprintf(fileWrite, "Film name will be created can't be empty\n");
        fclose(fileWrite);
        return;
    }
    add(hallName, width, height, filmName); /* If requirements are satisfied, then add this whole hall to linked list   */
    fclose(fileWrite);
}

void buyTicket(char *data[]){
    /* -------------------------------------------INITIALIZING VARIABLES-------------------------------------------  */
    FILE *fileWrite = fopen("output.txt", "a");
    char *name = strtok(data[1], "\"");  /* Name of film  */
    char *sit;
    int type;   /* Student or FullFare   */
    int howMany;

    /* -------------------------------------------PROCESS PART OF FUNCTION-------------------------------------------  */
    if(name == NULL){
        printf("Film name will be purchased can't be empty");
        fprintf(fileWrite, "Film name will be purchased can't be empty");
        fclose(fileWrite);
        return;
    }
    sit = data[2];  /* Which sit e.g: G10   */

    if(strcmp(data[3], "Student") == 0){    /*  Specifying type of ticket e.g: Student | FullFare  */
        type = STUDENT;
    }
    else{
        type = FULLFARE;
    }
    howMany = atoi(data[4]);    /* How many consecutive ticket will be purchased?  */
    controlBuyTicket(name, sit, howMany, type); /* Necessary assignments will be done there   */
    fclose(fileWrite);
}

void cancelTicket(char *data[]){
     /* -------------------------------------------INITIALIZING VARIABLES-------------------------------------------  */
     FILE *fileWrite = fopen("output.txt", "a");
     char *name = strtok(data[1], "\"");  /* Name of film  */
     char *sit; /* Which sit e.g: G10   */

     /* -------------------------------------------PROCESS PART OF FUNCTION-------------------------------------------  */
     if(name == NULL){  /* When film name seperated from double quotes, return "" means <null> cancel the process   */
        printf("\nFilm name will be cancelled can't be empty");
        fprintf(fileWrite, "\nFilm name will be cancelled can't be empty");
        fclose(fileWrite);
        return;
     }
     sit = data[2];  /* Which sit e.g: G10   */
     controlCancelTicket(name, sit); /* Necessary assignments will be done there   */
     fclose(fileWrite);
}

void showHall(char *name){
    /* -------------------------------------------INITIALIZING VARIABLES-------------------------------------------  */
    FILE *fileWrite = fopen("output.txt", "a");
    char alphabet[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
    char *hallName = strtok(name, "\n\""); /*  Serhat */
    struct Hall *tmp;   /* It iterate whole list to find desired hall  */

    /* -------------------------------------------PROCESS PART OF FUNCTION-------------------------------------------  */
    if(hallName == NULL){    /* When hall name seperated from double quotes, return "" means <null> cancel the process   */
        printf("Hall name will be shown can't be empty\n");
        fprintf(fileWrite, "Hall name will be shown can't be empty\n");
        fclose(fileWrite);
        return;
     }
    tmp = controlShowHall(hallName);    /* Control for hall name   */
    if(tmp == NULL){ /* If there is no hall name as wanted, it do not do anything  */
        printf("ERROR :: There is not any hall named %s\n", hallName);
        fprintf(fileWrite, "ERROR :: There is not any hall named %s\n", hallName);
    }
    else{

        int i, j, k;    /* Necessary counters for printing hall*/
        int y = tmp->height;
        int x = tmp->width;

    FILE *fileO = fopen("output.txt", "a");
    fprintf(fileO, "\n\n%s sitting plan\n\n", hallName);
    for(i = 0; i < y; i++){ /* Height of (rows) hall.   */
        if((i+1)/10 > 0)
            fprintf(fileO, "%d", i+1);
        else
            fprintf(fileO, "%d ", i+1);
        for(j = 0; j < x; j++){ /* Width (columns) of hall */
            fprintf(fileO, "|%c", (tmp->size[i][j] == '\0') ? ' ' : tmp->size[i][j]);
        }
        fprintf(fileO, "|\n");
        for(j = 0; j < 1; j++){
            fprintf(fileO, "  ");
            for(k = 0; k < 2*x+1; k++){
                fprintf(fileO, "-");
            }
        }
        fprintf(fileO, "\n");

    }

        fprintf(fileO, "   ");
        k = 0;
        for(j = 0; j < 2*x; j++){
            if(j % 2 == 0){
                fprintf(fileO, "%c ", alphabet[k]);
                k++;
            }
        }
        fprintf(fileO, "\n");
        for(i = 0; i < x-6; i++){
            fprintf(fileO, " ");
        }
        fprintf(fileO, "C U R T A I N\n");
        fclose(fileO);
        /* For writing console */
        printf("\n%s sitting plan\n\n", hallName);

    for(i = 0; i < y; i++){
        if((i+1)/10 > 0)
            printf("%d", i+1);
        else
            printf("%d ", i+1);
        for(j = 0; j < x; j++){
            printf("|%c", (tmp->size[i][j] == '\0') ? ' ' : tmp->size[i][j]);
        }
        printf("|\n");
        for(j = 0; j < 1; j++){
            printf("  ");
            for(k = 0; k < 2*x+1; k++){
                printf("-");
            }
        }
        printf("\n");

    }

        printf("   ");
        k = 0;
        for(j = 0; j < 2*x; j++){
            if(j % 2 == 0){
                printf("%c ", alphabet[k]);
                k++;
            }
        }
        printf("\n");
        for(i = 0; i < x-6; i++){
            printf(" ");
        }
        printf("C U R T A I N");
    }
    fclose(fileWrite);
}

void statistics(){
     FILE *fileWrite = fopen("output.txt", "a");
     struct Hall *tmp = hallList;
     printf("\nStatistics\n");
     fprintf(fileWrite, "\nStatistics\n");
     while(tmp != NULL){  /* It is control statement for controlling that does  desired film exist?  */
        printf("%s %d student(s), %d full fare(s), sum:%d TL\n", tmp->filmName, (tmp->student == 0) ? 0 : tmp->student, (tmp->fullFare == 0) ? 0 : tmp->fullFare, ((tmp->student * STUDENT) + (tmp->fullFare * FULLFARE)));
        fprintf(fileWrite, "%s %d student(s), %d full fare(s), sum:%d TL\n", tmp->filmName, (tmp->student == 0) ? 0 : tmp->student, (tmp->fullFare == 0) ? 0 : tmp->fullFare, ((tmp->student * STUDENT) + (tmp->fullFare * FULLFARE)));
        tmp = tmp->next;
    }
    fprintf(fileWrite, "\n\n");
    fclose(fileWrite);
}
/*---------------------------------------------------------------------------------------*/

/* Checking issues the relevant command */
int controlCreateHall(char *hallName, char *filmName){
    /* -------------------------------------------INITIALIZING VARIABLES-------------------------------------------  */
    FILE *fileWrite = fopen("output.txt", "a");
    int lengthOfHallName = 0;   /*First determine the length of them and then check if they are in double quotes */
    int flagOfHallName = 1;     /* End of method, if all situations are okey, it returns one (and operation)  */
    int flagOfFilmName = 1;
    int lengthOfFilmName = 0;

    /* -------------------------------------------PROCESS PART OF FUNCTION-------------------------------------------  */
    lengthOfHallName = strlen(hallName);   /* How many characters does hall name include?   */
    lengthOfFilmName = strlen(filmName);  /* How many characters does film name include?   */


     if(hallName[0] != '\"' || hallName[lengthOfHallName - 1] != '\"' ){  /* Checking hall name starts and ends with double quotes */
        printf("\nERROR :: Hall name of %s should starts and end with double quotes or can't include any whitespace..\n", hallName);
        fprintf(fileWrite, "\nERROR :: Hall name of %s should starts and end with double quotes or can't include any whitespace..\n", hallName);
        flagOfHallName = 0;
     }

     if(filmName[0] != '\"' || filmName[lengthOfFilmName - 1] != '\"'){ /* Checking film name starts and ends with double quotes */
        printf("\nERROR :: Film name of %s should starts and end with double quotes or can't include any whitespace..\n", filmName);
        fprintf(fileWrite, "\nERROR :: Film name of %s should starts and end with double quotes or can't include any whitespace..\n", filmName);
        flagOfFilmName = 0;
     }
     fclose(fileWrite);
     return flagOfFilmName && flagOfHallName;
}

int controlBuyTicket(char *name, char *sit, int howMany, int type){
     /* -------------------------------------------INITIALIZING VARIABLES-------------------------------------------  */
     FILE *fileWrite = fopen("output.txt", "a");
     struct Hall *tmp;
     int i, widthIndex, heightIndex;
     int numberOfDigit;
     char alphabet[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
     char *heightChar;
     int widthIndexBackup;

     /* -------------------------------------------PROCESS PART OF FUNCTION-------------------------------------------  */
     tmp = hallList;
     while(tmp != NULL && strcmp(tmp->filmName, name) != 0){  /* It is control statement for controlling that does
                                                                                desired film exist?  If filmName found, then it
                                                                                                    exits the loop
                                                                                                    with an error message.*/
        if(tmp->next == NULL){
            printf("\nERROR :: There is not any film named %s", name);
            fprintf(fileWrite, "\nERROR :: There is not any film named %s", name);
            fclose(fileWrite);
            return 0;
        }
        tmp = tmp->next;
    }

    /* ----------------------TO SPECIFY SITTING LOCATION ----------------------*/
    for(i = 0; i < 26; i++){ /* To find which sit label match with size[][]. For this determined label of sit's index over
                                                                                                            English Alphabet*/
        if(sit[0] == alphabet[i]){
            widthIndex = i;
            break;
        }
    }
    numberOfDigit = strlen(sit); /* Specify number of character in sitting order  */
    heightChar = (char *)malloc((numberOfDigit + 1) * sizeof(char));     /*  +1 is for '\0'   */
    for(i = 1; i < numberOfDigit; i++){
        heightChar[i-1] = sit[i];
    }
    heightIndex = atoi(heightChar) - 1;
    /* --------------------------------------------------------------------------*/

    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<CONTROLLING LOCATIONS' SALABLITY>>>>>>>>>>>>>>>> */
    if(tmp->width < widthIndex || tmp->height < heightIndex){   /* If input exceed hall's sitting plan:  */
        printf("\nERROR: Seat %s is not defined at %s", sit, tmp->hallName);
        fprintf(fileWrite, "\nERROR: Seat %s is not defined at %s", sit, tmp->hallName);
        fclose(fileWrite);
        return 0;
    }
    if((tmp->width - widthIndex) < howMany){    /* If consecutive desires exceed one line's bounds:   */
        printf("\nERROR :: Order must be consecutive, (%s + %d)range is exceeded", sit, howMany);
        fprintf(fileWrite, "\nERROR :: Order must be consecutive, (%s + %d)range is exceeded", sit, howMany);
        fclose(fileWrite);
        return 0;
    }

    widthIndexBackup = widthIndex;
    for(i = 0; i < howMany; widthIndex++, i++){ /* For checking availability desired sit(s)  */
        if(tmp->size[heightIndex][widthIndex] != '\0'){
            printf("\nERROR :: Specified seat(s) in %s are not available! They have been already taken.", tmp->hallName);
            fprintf(fileWrite, "\nERROR :: Specified seat(s) in %s are not available! They have been already taken.", tmp->hallName);
            fclose(fileWrite);
            return 0;
        }
    }
    /* Assigning desired sit(s) */
     printf("\n%s [%s] Seat(s) ", tmp->hallName, tmp->filmName);
     fprintf(fileWrite, "\n%s [%s] Seat(s) ", tmp->hallName, tmp->filmName);
    for(i = 0; i < howMany; widthIndexBackup++, i++){
            printf("%c%d%c", alphabet[widthIndexBackup], heightIndex + 1, (i + 1 < howMany) ? ',' : '\0');
            fprintf(fileWrite, "%c%d%c", alphabet[widthIndexBackup], heightIndex + 1, (i + 1 < howMany) ? ',' : '\0');
            tmp->size[heightIndex][widthIndexBackup] = (type == STUDENT) ? 's' : 'f';
            if(type == STUDENT){
                tmp->student++;
            }
            else{
                tmp->fullFare++;
            }
    }

    printf(" successfully sold\n");
    fprintf(fileWrite, " successfully sold\n");
    fclose(fileWrite);
    return 1;
    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>> */

}

struct Hall *controlShowHall(char *name){
    /*To find which hall wanted to be shown */
     struct Hall *tmp = hallList;
     while(tmp != NULL && strcmp(tmp->hallName, name) != 0){  /* It is control statement for controlling that does desired film exist? */
        if(tmp->next == NULL){
            return NULL;
        }
        tmp = tmp->next;
    }

    return tmp;
}

void controlCancelTicket(char *name, char *sit){
    /* -------------------------------------------INITIALIZING VARIABLES-------------------------------------------  */
    FILE *fileWrite = fopen("output.txt", "a");
    struct Hall *tmp = hallList;
    int numberOfDigit;
    char *heightChar;
    int i, widthIndex, heightIndex;
    char alphabet[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
    /* -------------------------------------------PROCESS PART OF FUNCTION-------------------------------------------  */
     while(tmp != NULL && strcmp(tmp->filmName, name) != 0){  /* It is control statement for controlling that does
                                                                                desired film exist?  If filmName found, then it
                                                                                                    exits the loop
                                                                                                    with an error message.*/
        if(tmp->next == NULL){
            printf("\nERROR :: There is not any film named %s", name);
            fprintf(fileWrite, "\nERROR :: There is not any film named %s", name);
            fclose(fileWrite);
            return;
        }
        tmp = tmp->next;
    }

    /* ----------------------TO SPECIFY SITTING LOCATION ----------------------*/
    for(i = 0; i < 26; i++){ /* To find which sit label match with size[][]. For this determined label of sit's index over
                                                                                                            English Alphabet*/
        if(sit[0] == alphabet[i]){ /* widthIndex will have which letter was wanted  */
            widthIndex = i;
            break;
        }
    }

    numberOfDigit = strlen(sit); /* Specify number of character in sitting order  */
    heightChar = (char *)malloc((numberOfDigit + 1) * sizeof(char));     /*  +1 is for '\0'   */
    for(i = 1; i < numberOfDigit; i++){
        heightChar[i-1] = sit[i];
    }
    sit[numberOfDigit - 1] = '\0';
    heightIndex = atoi(heightChar) - 1;
    /* --------------------------------------------------------------------------*/
    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<CONTROL FOR RECYCLING>>>>>>>>>>>>>>>>>>>>>>>>>>*/
    if(tmp->width < widthIndex || tmp->height < heightIndex){
        printf("ERROR: Seat %s is not defined at %s", sit, tmp->hallName);
        fprintf(fileWrite, "ERROR: Seat %s is not defined at %s", sit, tmp->hallName);
        fclose(fileWrite);
        return;
    }

    if(tmp->size[heightIndex][widthIndex] == '\0'){ /*If desired sit have not been sold: */
        printf("\nERROR: Seat %s in %s was not sold.", sit, tmp->hallName);
        fprintf(fileWrite, "\nERROR: Seat %s %cin %s was not sold.", sit,' ',  tmp->hallName);
        fclose(fileWrite);
        return;
    }
    /* If relevant sit belongs to either a student or a full fare */
    if(tmp->size[heightIndex][widthIndex] == 's'){
        tmp->student--;
    }
    else{
        tmp->fullFare--;
    }
    tmp->size[heightIndex][widthIndex] = '\0';

    printf("\n%s [%s] Purchase cancelled. Seat %s is now free.", tmp->hallName, tmp->filmName, sit);
    fprintf(fileWrite, "\n%s [%s] Purchase cancelled. Seat %s is now free.", tmp->hallName, tmp->filmName, sit);
    fclose(fileWrite);
    /*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>><>>>>>>>>>>>>>>>>>>>>>>>>>>*/
}
/*---------------------------------------------------------------------------------------*/
/*  Linked List Methods    */

void add(char *hallName, int width, int height, char *filmName){
    int i, j, x, y;
    if(hallList == NULL){  /* If there is no created hall:  */
        hallList = (struct Hall*)malloc(sizeof(struct Hall));
        hallList->size = (char **)malloc(height * sizeof(char *)); /* Initializing double dimensional dynamic array   */

        for(i = 0; i < height; i++){ /*Assigning columns to each row. (Memory allocation) */
            hallList->size[i] = (char *)malloc(width * sizeof(char));
        }

        hallList->height = height;
        hallList->width = width;

        hallList->hallName = (char *)malloc((strlen(hallName)+1) * sizeof(char));
        strcpy(hallList->hallName, hallName);

        hallList->filmName = (char *)malloc((strlen(filmName)+1) * sizeof(char));
        strcpy(hallList->filmName, filmName);

        hallList->student = 0;
        hallList->fullFare = 0;

        y = height;
        x = width;

        for(i = 0; i < y; i++){
            for(j = 0; j < x; j++){
                hallList->size[i][j] = '\0';
            }
        }

        hallList->next = NULL;
    }
    else{   /* If list has at least one hall: */
        struct Hall *tmp;
        tmp = (struct Hall*)malloc(sizeof(struct Hall));
        tmp->size = (char **)malloc(height * sizeof(char *));

        for(i = 0; i < height; i++){
            tmp->size[i] = (char *)malloc(width * sizeof(char));
        }

        tmp->height = height;
        tmp->width = width;

        tmp->hallName = (char *)malloc((strlen(hallName)+1) * sizeof(char));
        strcpy(tmp->hallName, hallName);

        tmp->filmName = (char *)malloc((strlen(filmName)+1) * sizeof(char));
        strcpy(tmp->filmName, filmName);

        tmp->student = 0;
        tmp->fullFare = 0;

        y = height;
        x = width;


        for(i = 0; i < y; i++){
            for(j = 0; j < x; j++){
                tmp->size[i][j] = '\0';
            }
        }

        tmp->next = hallList;
        hallList = tmp;
    }
}

void display(){ /*For checking presence of halls  */
    struct Hall *tmp = hallList;
    printf("\n\n");
    while(tmp != NULL){
        printf("Name: %s width: %d\theight: %d\n**************************\n", tmp->hallName, tmp->width, tmp->height);
        tmp = tmp->next;
    }


}
