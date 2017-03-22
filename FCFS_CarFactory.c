#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
* @author: Serhat YILMAZ
* @date: 09.12.2016
*
*   The program is simulation of car factory. First departments are established. Then cars come one by one then they are pass through
*   departments and produced. Everything is hierarchical. If one department is full, then car come after will wait the car its front.
*
*   The code is taking inputs as command line arguments. Therefore please enter your command as (argv[1])
*   >>..exe *.txt
*
*   
*/

struct TopOperation{
    char *name;
    char *type;
    int dirtyBit;
    struct Department *topDept; /*It holds top of department (first department in the list  */
    struct Car *car;
    struct TopOperation *next;  /*It shows next top operation */
    int isFull;

}*queueRear, *queueFront;

struct ManufacturedCar{
    struct Car *car;
    int factroyTime;

}*manufacturedCars;

struct Department{
    char *name;
    int time;
    int dirtyBit; /* */
    struct Department *next;
    struct Car *car;
    char *processedCar;
    int numberOfCarsProcessed;

};

struct DoneDepartments{
    struct DoneDepartments *front;
    struct DoneDepartments *rear;
    struct DoneDepartments *next;
    int count;
    int counter;
    char *type;
};

struct Car{
    char *name;
    char *model;
    int time;
    int startTime;
    int finishTime;
    int isPlaced;
    struct DoneDepartments *doneDept;
    struct TopOperation *operation;
    struct Department *department;
    struct Car *next;

}*carFront, *carRear;

/* Initial Issues */
void readText(int, char *);
int process(char *, int, int, char *);
short categorize(char *);
int findTotalFactoryTime(char *);

/* Commands Issues */
void addDept(char **);
int produce(char **, int, int);
void report(char **, char *);
void reportDepartments(int);
void reportCar(char **);
void reportCars(int);
void printReportTitles(char *);
struct Car *getCar(char **data);

/*Stack Issues */
void push(char **data, struct TopOperation *tmp);
void addProcessedCar(struct Department *, char *);
void display();

int transmit(struct TopOperation *, int, int);
int placeIt(struct TopOperation *, int, int, struct Car *);
void enqueueCar(struct Car *);
void dequeueCar(struct Car *);
float calculatePercent(int);

void reset();
int main(int argc, char *argv[])
{
    queueFront = NULL;
    queueRear = NULL;
    carRear = NULL;
    carFront = NULL;
    readText(0, argv[1]);

    getchar();


    return 0;
}

/*-----------------------------------INITIAL ISSUES----------------------------------- */
/*It reads file and send the necessary things to process() function */
void readText(int count, char *argv){

    FILE *file = fopen(argv, "r");
    char line[250];
    char lineBackup[250];
    char *splitProduce;
    int counter = 0;
    int totalFactoryTime = 0;
    while(fgets(line, sizeof(line), file)){
        if(count == 0){
            strcpy(lineBackup, line);
            splitProduce = strtok(lineBackup, " ");
            if(strcmp(splitProduce, "AddDept") == 0){
                strcpy(lineBackup, line);
                totalFactoryTime += findTotalFactoryTime(lineBackup);
                manufacturedCars = (struct ManufacturedCar*)malloc(sizeof(struct ManufacturedCar));
                manufacturedCars->factroyTime = totalFactoryTime;
                manufacturedCars->car = NULL;
            }
            if(strcmp(splitProduce, "Produce") == 0)
                continue;
                counter = process(line, counter, count, argv);
        }
        else{
            strcpy(lineBackup, line);
            splitProduce = strtok(lineBackup, " ");
            if(strcmp(splitProduce, "Produce") == 0){
                counter = process(line, counter, count, argv);
                if(counter >= count){
                    return;
                }

            }
        }
    }
    if(counter < count){
        while(counter < count){
            if(carFront != NULL)
                counter = transmit(carFront->operation, counter, count);
            else{
                break;
            }
        }
    }
}

int findTotalFactoryTime(char *line){
    char *splitProduce;
    int i = 0;
    int totalFactoryTime = 0;
    char *data[4];
    splitProduce = strtok(line, " ");

            while(splitProduce != NULL){
                data[i] = splitProduce;
                splitProduce = strtok(NULL, " ");
                i++;
            }
            totalFactoryTime = atoi(data[3]);
    return totalFactoryTime;
}

int process(char *line, int counter, int until, char *file){
    /*----------------------INITIALIAZING VARIABLES----------------------*/
    char *token; /*It holds splitting data and continue to split them. */
    char *data[4]; /*It holds splitting data to process them. */
    int i = 0; /*It is counter. */
    short category;
    /*----------------------PROCESS----------------------*/

    token = strtok(line, "\n");
    token = strtok(token, " ");

    while(token != NULL){
        data[i++] = token;
        token = strtok(NULL, " ");
    }

    category = categorize(data[0]);
    switch(category){
        case 1:
            addDept(data);
            break;
        case 2:
            counter = produce(data, counter, until);
            break;
        case 3:
            display();
            break;
        case 4:
            report(data, file);
            break;
        default:
            printf("Invalid command\n");

    }
    return counter;
}

/*It categorize commands */
short categorize(char *data){
    /*
        AddDept --> 1
        Produce --> 2
        PrintFactory --> 3
        Report --> 4
    */
    if(strcmp(data, "AddDept") == 0){
        return 1;
    }
    else if(strcmp(data, "Produce") == 0){
        return 2;
    }
    else if(strcmp(data, "PrintFactory") == 0){
        return 3;
    }
    else if(strcmp(data, "Report") == 0){
        return 4;
    }
    return -1;
}

/* ----------------------------------------------------------------------*/

/* ************************ DEPARTMENT ISSUES************************ */
void addDept(char **data){
    /*----------------------BINDING PROCESS---------------------- */
    struct TopOperation *tmp = (struct TopOperation*)malloc(sizeof(struct TopOperation));
    tmp->type =(char *) malloc(strlen(data[2]) * sizeof(char));
    strcpy(tmp->type, data[2]);
    tmp->dirtyBit = 0;
    tmp->isFull = 0;
    push(data, tmp);

    tmp->next = NULL;

    if(queueFront == NULL && queueRear == NULL){
        queueFront = queueRear = tmp;
        printf("Department %s has been created.\n", data[2]);
        return;
    }

    queueRear->next = tmp;
    queueRear = tmp;
    printf("Department %s has been created.\n", data[2]);
    /*printf("many: %d    type: %s    time: %d\n", howMany, type, time); */
}

int transmit(struct TopOperation *tmp, int counter, int until){
    struct Department *tmpDept;
    struct Car *tmpCar = carFront;
    struct DoneDepartments *doneDept;
    char s[25];
    int OK = 0;
    char carProperty[50];
    while(tmpCar != NULL){

        tmp = tmpCar->operation;
        strcpy(s, tmpCar->model);

        if(tmpDept != tmpCar->department){ /*If there is a situation to prevent a car that comes after another car jump into next department */
            tmpDept = tmpCar->department;
        }

        doneDept = tmpCar->doneDept->front;
        /*Iterating the car's department which include the car now. */
         while(strcmp(tmpCar->operation->type, doneDept->type) != 0){
                    doneDept = doneDept->next;
            }
            if(tmpCar->startTime != counter && doneDept->counter != counter){ /*Program controls a department in one loop
                                                                                    but number of department times. Therefore
                                                                                if a car controls in one counter (T) time then it is done for now.*/
                doneDept->count++;
                doneDept->counter = counter;
                if(doneDept->count > tmpDept->time){
                        doneDept->count = tmpDept->time;
                }
            }
        /*If car's expected finish time has come, then it is transmitted into manufacturedCars. */
        if(tmpCar->finishTime == counter){ /*If a car's production is done  */
            tmpDept->dirtyBit = 0;
            tmpDept->numberOfCarsProcessed++;
            sprintf(carProperty, "%d. %s %s\n", tmpDept->numberOfCarsProcessed, tmpDept->car->name, tmpDept->car->model);
            addProcessedCar(tmpDept, carProperty);
            dequeueCar(tmpCar);
            tmpDept->car = NULL;
            tmp->isFull = 0;
            tmpCar = carFront;
            continue;
        }
        /*If all cars produced (transmitted into manufacturedCars) then this function is done. */
        if(carFront == NULL){
            printf("All Cars are produced.\n");
            return counter;
        }

        /*We first check car's department for priority of queue of cars. */
        /*If its department is full and it is time to iterate it, it does necessary things. */
        while(tmpDept != NULL){
            if(tmpDept->dirtyBit == 1){
                if(tmpDept->time + tmpDept->car->startTime <= counter && tmp->next != NULL && tmp->next->isFull == 0){
                    tmpDept->car->isPlaced = 0;
                    tmpDept->numberOfCarsProcessed++;
                    sprintf(carProperty, "%d. %s %s\n", tmpDept->numberOfCarsProcessed, tmpDept->car->name, tmpDept->car->model);
                    addProcessedCar(tmpDept, carProperty);
                    tmp->car = tmpDept->car;
                    tmpDept->dirtyBit = 0;
                    tmpDept->car = NULL;
                    tmp->isFull = 0;/*A slot was discharged. */
                    OK = 1;
                    placeIt(tmp->next, counter, until, tmp->car);
                    tmp->car = NULL;
                    break;
                }
            }
            tmpDept = tmpDept->next;
        }

        if(tmpCar->next != NULL && tmpCar->next->operation != NULL ){

            tmpCar = tmpCar->next;
            continue;
        }
        /*If desired situation that if a department is full then do necessary iteration process starting
                                                                                        with beginning of queue. */
        if(until == counter)
            return counter;

        if(OK == 0){
            tmpCar = carFront;
            counter++;
        }
        else if(OK == 1){

            return counter;
        }
    }
    return counter;
}

/*It is place cars into departments if it exists any empty place in departments. */
int placeIt(struct TopOperation *tmp, int counter, int until, struct Car *aCar){
    struct Department *tmpDept;
    struct DoneDepartments *tmpDoneDept;
    int OK = 0;
    struct Department *controlTmp;
    while(tmp != NULL){
        tmpDept = tmp->topDept;
        while(tmpDept != NULL){ /*First controlling is car's placing department is empty, if it is, then it is placed. */
            if(tmpDept->dirtyBit == 0 && aCar->startTime <= counter){
                aCar->isPlaced = 1;

                /*Every department have a delay includes 0. Then every delay is added to finish time to determine when a car is done. */
                /*In the other words if a car waits to enter a department it adds how much time it waits to its finish time  */
                if(aCar->operation != NULL){ /*If the car is already in a process */
                    aCar->finishTime += counter - (aCar->operation->topDept->time + aCar->startTime);
                }
                else{ /*If the car's first department is: */
                    aCar->finishTime += counter - aCar->time;
                }

                /*BINDING FOR CAR */
                tmpDoneDept = (struct DoneDepartments*)malloc(sizeof(struct DoneDepartments));
                tmpDoneDept->type = (char *)malloc(strlen(tmp->type) * sizeof(char));
                strcpy(tmpDoneDept->type, tmp->type);
                tmpDoneDept->next = NULL;
                tmpDoneDept->counter = -1;
                tmpDoneDept->count = 0;
                if(aCar->doneDept->front == NULL && aCar->doneDept->rear == NULL){
                    tmpDoneDept->count++;
                    aCar->doneDept->front = aCar->doneDept->rear = tmpDoneDept;
                }
                else{
                    aCar->doneDept->rear->next = tmpDoneDept;
                    aCar->doneDept->rear = tmpDoneDept;
                    tmpDoneDept->count++;
                }/*   End of BINDING FOR CAR */

                aCar->operation = tmp;
                aCar->department = tmpDept;
                tmpDept->car = aCar;
                tmpDept->car->startTime = counter; /*It assigns finish time after done each department to car */
                tmpDept->dirtyBit = 1;
                OK = 1;
                break;
            }
            tmpDept = tmpDept->next;

        }
        if(tmp->isFull == 0){
            controlTmp = tmp->topDept;
            while(controlTmp != NULL){
                if(controlTmp->dirtyBit == 1 && controlTmp->next == NULL){
                        tmp->isFull = 1;
                }
                else if(controlTmp->dirtyBit == 0){ /*If first department is empty but second is full, then exit from controlling. */
                    break;
                }
                controlTmp = controlTmp->next;
            }
        }
         if(until == counter)
            return counter;
        if(OK == 0){
            counter++;
            continue;
        }
        else if(OK == 1){
            return counter;
        }
    }
    return counter;

}

int produce(char **data, int counter, int until){
    struct TopOperation *tmp = queueFront;
    struct Car *aCar;
    char gg[25];
    aCar = getCar(data);
    strcpy(gg, aCar->model);
        while(tmp != NULL ){
            if(tmp->isFull == 1 && aCar->isPlaced == 0){ /*Firstly controlling first department is full or not AND is the car
                                                                           placed? */
                counter = transmit(tmp, counter, until);
            }
            if((tmp->isFull == 0 && aCar->isPlaced == 0) || (counter == until)){ /*If the first department is not full and car is not placed yet: */
                counter = placeIt(tmp, counter, until, aCar);
                if(counter >= until){
                    transmit(tmp, counter, until);
                    return counter;
                }
            }

            if(aCar->isPlaced == 1){/*If necessary iteration is done but still the car isn't placed, then it looks same department
                                    one more time, this time the purpose is placed the car into a place between the departments */
                tmp = tmp->next;
            }


        }
        return counter;
}
struct Car *getCar(char **data){

    /*----------------------INITIALIAZING VARIABLES----------------------*/
    int time;
    struct Car *car = (struct Car*)malloc(sizeof(struct Car));
    char s[25];
    /*----------------------PROCESS----------------------*/
    time = atoi(data[1]);
    car->name = (char *)malloc((strlen(data[2]) + 1) * sizeof(char));
    car->model = (char *)malloc((strlen(data[3]) + 1) * sizeof(char));
    car->doneDept = (struct DoneDepartments*)malloc(sizeof(struct DoneDepartments));
    strcpy(car->name, data[2]);
    strcpy(car->model, data[3]);
    car->time = time;
    car->startTime = time;/*Time that will be updated when the car changes department */
    car->isPlaced = 0;
    car->doneDept->front = NULL;
    car->doneDept->rear = NULL;
    car->doneDept->next = NULL;
    car->doneDept->count = 0;
    car->doneDept->counter = -1;
    car->operation = NULL;  /*Department in which the car process in */
    car->department = NULL;
    car->finishTime = manufacturedCars->factroyTime + time;/*It specified the finish time of the car. */
    strcpy(s, car->model);
    enqueueCar(car);
    return car;

}

void report(char **data, char *file){
    if(strcmp(data[2], "Car") == 0){
       readText(atoi(data[1]), file);
       reportCar(data);
       reset();
    }
    else if(strcmp(data[2], "Cars") == 0){
        readText(atoi(data[1]), file);
        reportCars(atoi(data[1]));
        reset();
    }
    else if(strcmp(data[2], "Departments") == 0){
        readText(atoi(data[1]), file);
        reportDepartments(atoi(data[1]));
        reset();
    }
}

void reportDepartments(int until){
    struct TopOperation *tmp = queueFront;
    struct Department *dpt;
    char text[250];
    printf("\nCommand: Report Departments %d\n", until);
    while(tmp != NULL){
        dpt = tmp->topDept;
        sprintf(text, "|Report for Department \"%s\"|", dpt->name);
        printReportTitles(text);
            while(dpt != NULL){
            if(dpt->car != NULL)
                printf("I am currently processing %s %s\n%s%s", dpt->car->name, dpt->car->model, (dpt->processedCar == NULL) ? "" : "Processed Cars\n",(dpt->processedCar == NULL) ? "" : dpt->processedCar);
            else{
                printf("%s is now free.\n", dpt->name);
                if(dpt->processedCar != NULL){
                    printf("Processed Cars\n%s", dpt->processedCar);
                }
            }
            if(dpt->next != NULL){
                sprintf(text, "|Report for Department \"%s\"|", dpt->next->name);
                printReportTitles(text);
            }
                dpt = dpt->next;
        }
        tmp = tmp->next;
    }

}

void reportCars(int until){
    struct Car *tmpCar = manufacturedCars->car;
    struct TopOperation *tmpOperation;
    float percent;
    char text[250];
    int timing = 0;
    struct DoneDepartments *tmp;
    printf("\nCommand: Report Cars %d\n", until);
    if(tmpCar != NULL){
        while(tmpCar != NULL){
            sprintf(text, "|Report for %s %s|", tmpCar->name, tmpCar->model);
            printReportTitles(text);
            tmp = tmpCar->doneDept->front;
            while(tmp != NULL){
                printf("%s: %d, ", tmp->type, tmp->count);
                timing += tmp->count;
                tmp = tmp->next;
            }

            tmpOperation = queueFront;

            while(tmpCar->operation != NULL && strcmp(tmpOperation->type, tmpCar->operation->type) != 0){
                tmpOperation = tmpOperation->next;
            }
            while(tmpCar->operation != NULL && tmpOperation->next != NULL){
                printf("%s: %d, ", tmpOperation->next->type, 0);
                tmpOperation = tmpOperation->next;
            }
            percent = calculatePercent(timing);
            printf("| Start Time: %d | Complete: %.2f%% | %s\n", tmpCar->time, percent, (percent == 100.0) ? "Complete" : "Not complete");
            timing = 0;
            tmpCar = tmpCar->next;
        }
    }
    tmpCar = carFront;
    if(tmpCar != NULL){
        while(tmpCar != NULL){
            sprintf(text, "|Report for %s %s|", tmpCar->name, tmpCar->model);
            printReportTitles(text);
            tmp = tmpCar->doneDept->front;
            while(tmp != NULL){
                printf("%s: %d, ", tmp->type, tmp->count);
                timing += tmp->count;
                tmp = tmp->next;
            }
            tmpOperation = queueFront;
            while(tmpCar->operation != NULL && strcmp(tmpOperation->type, tmpCar->operation->type) != 0){
                tmpOperation = tmpOperation->next;
            }
            while(tmpCar->operation != NULL && tmpOperation->next != NULL){
                printf("%s: %d, ", tmpOperation->next->type, 0);
                tmpOperation = tmpOperation->next;
            }
            percent = calculatePercent(timing);
            printf("| Start Time: %d | Complete: %.2f%% | %s\n", tmpCar->time, percent, (percent == 100.0) ? "Complete" : "Not complete");
            timing = 0;
            tmpCar = tmpCar->next;
        }
    }
}

void reportCar(char **data){
    struct Car *tmpCar = carFront;
    struct TopOperation *tmpOperation;
    int isFound = 0;
    char text[250];
    float percent;
    int timing = 0;
    struct DoneDepartments *tmp;
    while(tmpCar != NULL){
        if(strcmp(tmpCar->model, data[3]) == 0){
            isFound = 1;
            break;
        }
        tmpCar = tmpCar->next;
    }
    if(isFound == 0){
        tmpCar = manufacturedCars->car;
        while(tmpCar != NULL){
            if(strcmp(tmpCar->model, data[3]) == 0){
                isFound = 1;
                break;
            }
        tmpCar = tmpCar->next;
        }
    }
    if(isFound == 0){
        printf("There is no car named %s\n", data[3]);
        return;
    }

    printf("\nCommand: Report Car %s %s\n", data[1], data[3]);
    sprintf(text, "|Report for %s %s|", tmpCar->name, tmpCar->model);
    printReportTitles(text);
    tmp = tmpCar->doneDept->front;
    while(tmp != NULL){
        printf("%s: %d, ", tmp->type, tmp->count);
        timing += tmp->count;
        tmp = tmp->next;
    }

    tmpOperation = queueFront;
    while(tmpCar->operation != NULL && strcmp(tmpOperation->type, tmpCar->operation->type) != 0){
        tmpOperation = tmpOperation->next;
    }
    while(tmpCar->operation != NULL && tmpOperation->next != NULL){
        printf("%s: %d, ", tmpOperation->next->type, 0);
        tmpOperation = tmpOperation->next;
    }
    percent = calculatePercent(timing);
    printf("| Start Time: %d | Complete: %.2f%% | %s\n", tmpCar->time, percent, (percent == 100.0) ? "Complete" : "Not complete");

}

float calculatePercent(int timing){
    return (float)(timing * 100)/ (float)manufacturedCars->factroyTime;
}

void printReportTitles(char *text){
    int i;
        for(i = 0; i < strlen(text); i++){
            printf("-");
        }
        printf("\n");
        printf("%s\n", text);
        for(i = 0; i < strlen(text); i++){
            printf("-");
        }
        printf("\n");
}

/*When a new Report comes, it resets everything to avoid lack of memory  */
void reset(){
    struct TopOperation *tmp = queueFront;
    struct Department *tmpDept;
    struct Car *tmpCar;
    struct Car *freeCar;

    while(tmp != NULL){
        tmpDept = tmp->topDept;
        while(tmpDept != NULL){
            tmpDept->car = NULL;
            tmpDept->dirtyBit = 0;
            tmpDept->numberOfCarsProcessed = 0;
            tmpDept->processedCar = NULL;
            tmpDept = tmpDept->next;
        }
        tmp->isFull = 0;
        tmp->dirtyBit = 0;
        tmp->car = NULL;
        tmp = tmp->next;
    }
    if(manufacturedCars->car != NULL){
        tmpCar = manufacturedCars->car;
        while(tmpCar != NULL){

            freeCar = tmpCar;
            tmpCar = tmpCar->next;
            free(freeCar);
        }
    }
    manufacturedCars->car = NULL;
    if(carFront != NULL){
        while(carFront != NULL){
            freeCar = carFront;
            if(carFront == carRear){
                carFront = carRear = NULL;
                return;
            }

            carFront = carFront->next;
            free(freeCar);
        }
    }
}
/*While iterating cars into departments, it needed to be followed an order, it is provided by queue of cars */
void enqueueCar(struct Car *car){
    car->next = NULL;
    if(carFront == NULL && carRear == NULL){
        carFront = (struct Car *)malloc(sizeof(struct Car));
        carRear = (struct Car *)malloc(sizeof(struct Car));
        carFront = carRear = car;
        return;
    }
    carRear->next = car;
    carRear = car;
}

/*When a car's production is done, this function take the car, exit them queue of production and add to manufactured car's list */
void dequeueCar(struct Car *car){
    struct Car *tmp = carFront;
    struct Car *notNullTmpCar;
    if(tmp == NULL){
        printf("There is no car\n");
    }
    else if(carFront == carRear){
        carFront = carRear = NULL;
    }
    else{
        carFront = carFront->next;
        tmp->operation = NULL;
        tmp->next = NULL;
    }
    if(manufacturedCars->car == NULL){

        manufacturedCars->car = tmp;
    }
    else{
        notNullTmpCar = manufacturedCars->car;
        while(notNullTmpCar->next != NULL){
            notNullTmpCar = notNullTmpCar->next;
        }
        notNullTmpCar->next = tmp;
    }
}
/* ************************************************************************ */

/*When a department is founded, then its siblings are founded by using stack. first in shows null, last in is shown by topOperation. */
void push(char **data, struct TopOperation *tmp){
    /*----------------------INITIALIAZING VARIABLES----------------------*/
    int howMany;
    char *type;
    int time;
    int i;
    char name[25];
    struct Department *top;
    /*----------------------PROCESS----------------------*/
    howMany = atoi(data[1]);
    type = (char *) malloc(strlen(data[2]) * sizeof(char));
    strcpy(type, data[2]);
    time = atoi(data[3]);
    top = NULL;

    for(i = 0; i < howMany; i++){
        if(top == NULL){
            top = (struct Department*)malloc(sizeof(struct Department));
            sprintf(name, "%s%d", type, (howMany - i));
            top->name = (char *)malloc((strlen(name) + 1) * sizeof(char));
            strcpy(top->name, name);
            top->time = time;
            top->numberOfCarsProcessed = 0;
            top->processedCar = NULL;
            top->dirtyBit = 0;
            top->car = NULL;
            top->next = NULL;
        }
        else{
            struct Department *tmpDept = (struct Department*)malloc(sizeof(struct Department));
            sprintf(name, "%s%d", type, (howMany - i));
            tmpDept->name = (char *)malloc((strlen(name) + 1) * sizeof(char));
            strcpy(tmpDept->name, name);
            tmpDept->time = time;
            tmpDept->numberOfCarsProcessed = 0;
            tmpDept->processedCar = NULL;
            tmpDept->car = NULL;
            tmpDept->dirtyBit = 0;
            tmpDept->next = top;
            top = tmpDept;
        }
    }
    tmp->topDept = top;

}

void addProcessedCar(struct Department *tmpDept, char *carProperty){
    if(tmpDept->processedCar == NULL){
        tmpDept->processedCar = (char *)malloc((strlen(carProperty) + 1) * sizeof(char));
        strcpy(tmpDept->processedCar, carProperty);
    }
    else{
        tmpDept->processedCar = (char *)realloc(tmpDept->processedCar, (strlen(carProperty) + strlen(tmpDept->processedCar) + 1));
        strcat(tmpDept->processedCar, carProperty);
    }
}

void display(){

    struct TopOperation *tmp = queueFront;
    struct Department *tmp1;
    int counter = 0, i;
    while(tmp != NULL){
        tmp1 = tmp->topDept;
        printf("- ");
        while(tmp1 != NULL){
            printf("%s ", tmp1->name);
            counter += strlen(tmp1->name);
            counter += 1;
            tmp1 = tmp1->next;
        }
        printf("\n");

        for(i = 0; i < counter + 1; i++){
            printf(" ");
        }

        tmp = tmp->next;
    }
    printf("\n");

}
