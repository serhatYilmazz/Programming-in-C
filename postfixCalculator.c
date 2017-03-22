#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
*
*   Serhat YILMAZ
*   16.11.2016
*
*   In this program, reading from text file, firstly categorize calculation or printing commands, then categorize integer
*   or hex calculation. Both of calculation the given infix notation is converted to postfix notation by pushing stack, doing
*   necessary operation, popping used operators and operands, then write them into a queue.
*   When print command comes, it prints everything to text file and to console.
*   (Note: Please if a line capacity is exceeded 100000 characters, it can be handled it just by increasing
*   line and lineBackup variables, then everything will be calculated correctly.

*   (Note: Please enter input file name as "input1.txt")

*/

/*Initial Issues */
void readText();
void process(char *, FILE *);
short categorizeCommand(char *);
short categorizeCalculation(char *);

/*Calculation Functions*/
void intCalculator();
void hexCalculator();
short isOperator(char);
short isOperand(char);
short isHexOperand(char);
short GetOperatorPrecedence(char);
short HasHigherPrecedence(char, char);
char *convertToHex(char *);

/*Calculator*/
void processOperation();
void processHexOperation();
void add();
void addHex();
void subtract();
void subtractHex();
void multiply();
void multiplyHex();
void divide();
void divideHex();
short isHexControl(char *);

/*Queue Functions */
void create();
void enqueue();
struct Node *dequeue();
void isQueueEmpty();
void displayQueue(FILE *);

/*Stack Functions */
void push(char *);
void pushCharacter(char);
void pop(char);
char *getTop(struct Node *);
void isStackEmpty();
void displayStack(struct Node*);
short isEmpty(struct Node*);

struct Node{
    char *token;
    long value;
    struct Node *next;

}*StackOfOperand, *StackOfOperation, *queueRear, *queueFront;

int main()
{
    StackOfOperand = NULL;
    StackOfOperation = NULL;
    readText();
    return 0;
}

/*It reads file and send the necessary things to process() function then it closes the file */
void readText(){
    FILE *file = fopen("in2.txt", "r");
    FILE *fileOut = fopen("output.txt", "w");
    int i = 0;
    char line[100000];
    char lineBackup[100000], *data[3];
    char *lineParse;
    char type[8];
    while(fgets(line, sizeof(line), file)){
            if(line[0] == '\n'){
                continue;
            }
        strcpy(lineBackup, line);
        lineParse = strtok(lineBackup, "\n");
        lineParse = strtok(lineBackup, " ");
        while(lineParse != NULL){
            data[i] = lineParse;
            lineParse = strtok(NULL, " ");
            i++;
        }

        strcpy(type, data[1]);
        process(line, fileOut);
        if(strcmp(data[0], "print") == 0 || strcmp(data[0], "print\n") == 0){ /*If command is print, then not do any operation anymore read a new line  */
            i = 0;
            continue;
        }
        if(strcmp(getTop(StackOfOperand), "error") == 0){ /*If operation is failed, then add queue error */
            enqueue("error");
        }
        else{

            data[0][0] = '\0';
            sprintf(data[0], "%s %s", type, getTop(StackOfOperand));
                if(getTop(StackOfOperand)[0] == '-'){
                    pop(getTop(StackOfOperand)[1]);
                }
                else{
                    pop(getTop(StackOfOperand)[0]);
                }
            if(isEmpty(StackOfOperand) == 0){ /*If there is a situation like: "(8+5)(9*2)" */
                enqueue("error");

            } else{
                enqueue(data[0]);
            }
        }
        /*After adding queue, reset stacks and counter */
        i = 0;

        StackOfOperand = NULL;
        StackOfOperation = NULL;
        lineParse = NULL;

    }
    fclose(file);
    fclose(fileOut);
}

/*In this function it categorize the coming input line as integer or hex and print or calculate */
void process(char *token, FILE *file){
    char *piece; /*It needs to be categorize coming tokens  */
    char *data[3]; /*We have 2 spaces, 3 elements each line */
    short commandFlag; /*To hold categorized short variable */
    short calculationFlag; /*Hold categorized value of integer or hex  */
    int i = 0;
    int counter = 0; /*If there is a statement like "1 1". */
    piece = strtok(token, " ");


    while(piece != NULL){   /*It parse all line element and make them easy to categorize */
       data[i] = piece;
       piece = strtok(NULL, " ");
       i++;
       counter ++; /*It counts the spaces, if space are more than expected, then push error and return */
    }
    if(counter > 3){
        push("error");
        i = 0;
        return;
    }
    commandFlag = categorizeCommand(data[0]); /*return "print or calculate". */

    switch(commandFlag){
        case 0:
            calculationFlag = categorizeCalculation(data[1]);
            data[2] = strtok(data[2], "\"");
            if(calculationFlag == 0){
                intCalculator(data[2]);
            }
            else if(calculationFlag == 1){
                hexCalculator(data[2]);
                if(isHexControl(getTop(StackOfOperand)) == 1){
                    strcpy(data[2], getTop(StackOfOperand)); /*If input has just one value, should avoid from 0x part */
                    pop(getTop(StackOfOperand)[0]);
                    push(&data[2][2]); /*It sends stack a value which get rid of "0x" part of hex number */
                }
            }
            break;
        case 1:
            displayQueue(file);
            break;
        default:

            break;
    }
}

/*It categorize command as print or calculate*/
short categorizeCommand(char *token){
    /* calculate --> 0
       print     --> 1 */

       if(strcmp(token, "calculate") == 0){
            return 0;
       }
       else  if(strcmp(token, "print") == 0 || strcmp(token, "print\n") == 0){
            return 1;
       }
       else{
        return -1;
       }
}
/*It categorize command as hex or integer*/
short categorizeCalculation(char *token){
    /* integer --> 0
       hex     --> 1 */
       if(strcmp(token, "integer") == 0){
            return 0;
       }
       else if(strcmp(token, "hex") == 0){
            return 1;
       }
       else{
            return -1;
       }
}
/* ----------------------------------------------------------------------------------------*/
/*Calculation Functions */

/*It takes calculation part, split them
operator by operator and operand by operand.
All condition or situation controls is doing here. Necessary comments appear near of statements  */
void intCalculator(char *stmt){
    int i, j=0;
    char operand[25];
    char tmp[3];
    for(i = 0; i < strlen(stmt); i++){
        if(stmt[i] == ' '){
            continue;
        }
        else if(isOperand(stmt[i]) == 1 || isOperator(stmt[i]) == 1 || stmt[i] == '('  || stmt[i] == ')'){ /*Expected tokens in integer calculations */


            /*If there is a statement like: -(...), it pushes -1 and '*' first then continue to process */
            if(stmt[i] == '-' && i == 0 && stmt[i+1] == '('){
                strcpy(tmp, "-1");
                push(tmp);
                tmp[0] = '*';
                tmp[1] = '\0';
                push(tmp);
                i++;
            }
            /*If there is a statement like: +(...), it continues process after one step. */
            else if(stmt[i] == '+' && i == 0 && stmt[i+1] == '('){
                i++;
            }
            /* If there is a situation like "+2*8" or "+2-5*(+12+4): */
            if((stmt[i] == '(' && stmt[i+1] == '+') || (i == 0 && stmt[i] == '+')){ /*If "+" comes after an
                                                                                                                open parentheses
                                                                                                or beginning of the calculation */

                    if(stmt[i] == '('){
                        if(isOperand(stmt[i+2]) == 0){ /*If there is a statement like ==> (+-8... */
                            push("error");
                            return;
                    }
                    pushCharacter(stmt[i]); /*Push the parenthesis */
                    i++;
                    }
                    else{
                        if(isOperand(stmt[i+1]) == 0){ /*If there is a statement like ==> +-8... */
                            push("error");
                            return;
                        }
                        continue;
                    }
            }
            else if((stmt[i] == '(' && stmt[i+1] == '/') || (i == 0 && stmt[i] == '/')){ /*If there is a situation like /8+5..
                                                                                                or (/2+6...*/
                push("error");
                return;
            }
            else if((stmt[i] == '(' && stmt[i+1] == '*') || (i == 0 && stmt[i] == '*')){/*If there is a situation like *8+5..
                                                                                                or (*2+6...*/
                push("error");
                return;
            }
            else if((stmt[i] == '*' || stmt[i] == '/') && stmt[i+1] == '-'){/*If there is a statement like ==> ..*-8...
                                                                                                or             ../-8...*/
                        j = 1; /*should be started to add from second index to array for adding '-' first index. */
                        tmp[0] = stmt[i];
                        tmp[1] = '\0';
                        i += 2;
                        if(isOperand(stmt[i]) == 1){
                        while(isOperand(stmt[i]) == 1){ /*If the elements are more than one digit. */
                            operand[j] = stmt[i];
                            i++;
                            j++;
                        }
                        operand[j] = '\0';
                        j = 0;
                        operand[0] = '-';
                        push(operand);
                        push(tmp);
                        i--;/*for loop already will increase "i" by 1, therefore decrease it by 1 */
                }
            }
            else if((stmt[i] == '(' && stmt[i+1] == '-') || (i == 0 && stmt[i] == '-')){/*If there is a situation like ..(-4+5.. | -4+5.. */
                if(stmt[i] == '('){
                    if(stmt[i+2] == '('){ /*If there is a situation like ..(-(-5+2)) */
                        pushCharacter(stmt[i]);
                         strcpy(tmp, "-1");
                         push(tmp);
                         tmp[0] = '*';
                         tmp[1] = '\0';
                         push(tmp);
                         i += 1;
                         continue;

                    }
                    else if(isOperand(stmt[i+2]) == 0){ /*If there is a statement like ==> (+-8... */
                            push("error");
                            return;
                    }
                    pushCharacter(stmt[i]);
                    j = 1;
                    i += 2;
                    if(isOperand(stmt[i]) == 1){
                    while(isOperand(stmt[i]) == 1){/*If the elements are more than one digit. */
                        operand[j] = stmt[i];
                        i++;
                        j++;
                    }
                    operand[j] = '\0';
                    j = 0;
                    operand[0] = '-';
                    push(operand);
                    i --;
                    }
                }
                else{/*If there is a statement like ==> +-8... */
                    if(isOperand(stmt[i+1]) == 0){
                            push("error");
                            return;
                    }
                    j = 1;
                    i++;
                    if(isOperand(stmt[i]) == 1){
                    while(isOperand(stmt[i]) == 1){/*If the elements are more than one digit. */
                        operand[j] = stmt[i];
                        i++;
                        j++;
                    }
                    operand[j] = '\0';
                    j = 0;
                    operand[0] = '-';
                    push(operand);
                    i--;
                    }
                    else{
                        printf("error\n");
                        return;
                    }
                }
            }
            else if(isOperator(stmt[i]) == 1){ /*If there is a situation like "2--1", it makes "--" >>> "+"  */
                if(stmt[i] == '-' && stmt[i+1] == '-' && isOperator(stmt[i+2]) == 0) { /*If there are triple operation consecutively
                                                                                            it directly go last end if */
                        i++;    /* It replaces second '-' with '+' */
                        stmt[i] = '+';
                }
                else if(stmt[i] == '+' && stmt[i+1] == '+' && isOperator(stmt[i+2]) == 0) {/*If there are triple operation consecutively
                                                                                            it directly go last end if */
                        i++;
                        stmt[i] = '+';
                }
                else if(stmt[i] == '+' && stmt[i+1] == '-' && isOperator(stmt[i+2]) == 0) {/*If there are triple operation consecutively
                                                                                            it directly go last end if */
                        i++;
                        stmt[i] = '-';
                }
                else if(stmt[i] == '-' && stmt[i+1] == '+' && isOperator(stmt[i+2]) == 0) {/*If there are triple operation consecutively
                                                                                            it directly go last end if */
                        i++;
                        stmt[i] = '-';
                }
                else if(stmt[i] == '*' && stmt[i+1] == '+' && isOperator(stmt[i+2]) == 0){/*If there is a situation like
                                                                                                "8*+2"*/
                    i++;
                    stmt[i] = '*';
                }
                else if(stmt[i] == '/' && stmt[i+1] == '+' && isOperator(stmt[i+2]) == 0){/*If there is a situation like
                                                                                                "8/+2"*/
                    i++;
                    stmt[i] = '/';
                }
                else if(stmt[i+1] != '(' && stmt[i+1] != ')' && isOperand(stmt[i+1]) == 0){/* If there is operator overloading condition, it prints error */
                        push("error");
                        return;
                }
                while(isEmpty(StackOfOperation) == 0 && getTop(StackOfOperation)[0] != '(' && HasHigherPrecedence(getTop(StackOfOperation)[0], stmt[i]) == 1){
                            processOperation();
                            pop(getTop(StackOfOperation)[0]);
                      }
                       pushCharacter(stmt[i]);
            }
            else if(isOperand(stmt[i]) == 1){
                if(isOperand(stmt[i+1]) == 1){ /*If element has more than one digit: */
                    operand[j] = stmt[i];   /*Is one after is digit ? (add it to char array) : (jump else section) */
                    j++;
                }else{
                    if(operand[0] == '\0'){ /*If char array which hold more than one digit is empty, in other words
                                                    the element has only one element:*/
                        pushCharacter(stmt[i]);
                    }
                    else{/*If one after element not digit but this element has more than one digit, then add it to char array then push  */
                        operand[j++] = stmt[i];
                        operand[j] = '\0';
                        j = 0;
                        push(operand);
                        operand[0] = '\0';
                    }
                }
            }
            else if(stmt[i] == '('){
                pushCharacter(stmt[i]);
            }
            else if(stmt[i] == ')'){
                while(isEmpty(StackOfOperation) == 0 && getTop(StackOfOperation)[0] != '('){
                    processOperation();               /*If it time to calculation, it makes necessary calculations */
                    pop(getTop(StackOfOperation)[0]); /*When calculation is done, it needed to be iterated to next operation */
                }
                if(isEmpty(StackOfOperation) == 1){ /*If there is a closing parentheses that not opened before:  */
                    push("error");
                    return;
                }
                pop(getTop(StackOfOperation)[0]); /*When it is done with an open-closed parenthesis popped open parenthesis from stack */
            }
        }
        else{ /*If any character that is not expected: */
           push("error");
           return;
        }
    }
    while(isEmpty(StackOfOperation) == 0){
        processOperation();
        if(getTop(StackOfOperation)[0] == '('){ /*If an opening parentheses is not closed with closing parentheses. */
            push("error");
            return;
        }
        pop(getTop(StackOfOperation)[0]);
    }


}


void hexCalculator(char *stmt){
    int i, j = 0;
    char operand[25];
    char hex[25];
    char tmp[2];
    for(i = 0; i < strlen(stmt); i++){
        if(stmt[i] == ' '){
            continue;
        }

            /*If there is a statement like: -(...), it pushes -1 and '*' first then continue to process */
            if(stmt[i] == '-' && i == 0 && stmt[i+1] == '('){
                strcpy(tmp, "-1");
                push(tmp);
                tmp[0] = '*';
                tmp[1] = '\0';
                push(tmp);
                i++;
            }
            else if(stmt[i] == '+' && i == 0 && stmt[i+1] == '('){
                i++;
            }
            /* If there is a situation like "+2*8" or "+2-5*(+12+4): */
            if((stmt[i] == '(' && stmt[i+1] == '+') || (i == 0 && stmt[i] == '+')){ /*If "+" comes after an
                                                                                                                open parentheses
                                                                                                or beginning of the calculation */

                    if(stmt[i] == '('){
                        if(isHexOperand(stmt[i+2]) == 0){ /*If there is a statement like ==> (+-8... */
                            push("error");
                            return;
                    }
                        pushCharacter(stmt[i]);
                        i++;
                    }
                    else{
                        if(isHexOperand(stmt[i+1]) == 0){ /*If there is a statement like ==> +-8... */
                            push("error");
                            return;
                        }
                        continue;
                    }
            }
            else if((stmt[i] == '*' || stmt[i] == '/') && stmt[i+1] == '-'){/*If there is a statement like ==> ..*-8...
                                                                                                or             ../-8...*/
                        j = 1;
                        tmp[0] = stmt[i];
                        tmp[1] = '\0';
                        i += 2;
                        if(isOperand(stmt[i]) == 1){
                        while(isOperand(stmt[i]) == 1){
                            operand[j] = stmt[i];
                            i++;
                            j++;
                        }
                        operand[j] = '\0';
                        j = 0;
                        operand[0] = '-';
                        push(operand);
                        push(tmp);
                        i--;
                }
            }
            else if((stmt[i] == '(' && stmt[i+1] == '-') || (i == 0 && stmt[i] == '-')){
                if(stmt[i] == '('){
                    if(stmt[i+2] == '('){ /*If there is a situation like ..(-(-5+2)) */
                         pushCharacter(stmt[i]);
                         strcpy(tmp, "-1");
                         push(tmp);
                         tmp[0] = '*';
                         tmp[1] = '\0';
                         push(tmp);
                         i += 1;
                         continue;

                    }
                    else if(isHexOperand(stmt[i+2]) == 0){ /*If there is a statement like ==> (+-8... */
                            push("error");
                            return;
                    }
                    pushCharacter(stmt[i]); /*First push the opening parentheses */
                    j = 1;
                    i += 2;
                    if(isHexOperand(stmt[i]) == 1){
                        while(isHexOperand(stmt[i]) == 1){/*If number has more than one digit */
                            operand[j] = stmt[i];
                            i++;
                            j++;
                        }
                        operand[j] = '\0';
                        j = 0;
                        operand[0] = '-';
                        strcpy(hex, convertToHex(operand));
                        push(hex);
                        i --;
                    }
                }
                else{
                    if(isHexOperand(stmt[i+1]) == 0){
                            push("error");
                            return;
                    }
                    j = 1;
                    i++;
                    if(isHexOperand(stmt[i]) == 1){
                    while(isHexOperand(stmt[i]) == 1){/*If number has more than one digit */
                        operand[j] = stmt[i];
                        i++;
                        j++;
                    }
                    operand[j] = '\0';
                    j = 0;
                    operand[0] = '-';
                    strcpy(hex, convertToHex(operand));
                    push(hex);
                    i--;
                    }
                    else{
                        printf("error\n");
                        return;
                    }
                }
            }
            else if(isOperator(stmt[i]) == 1){ /*If there is a situation like "2--1", it makes "--" >>> "+"  */
                if(stmt[i] == '-' && stmt[i+1] == '-' && isOperator(stmt[i+2]) == 0) { /*If there are triple operation consecutively
                                                                                            it directly go last end if */
                        i++;    /* It replaces second '-' with '+' */
                        stmt[i] = '+';
                }
                else if(stmt[i] == '+' && stmt[i+1] == '+' && isOperator(stmt[i+2]) == 0) {/*If there are triple operation consecutively
                                                                                            it directly go last end if */
                        i++;
                        stmt[i] = '+';
                }
                else if(stmt[i] == '+' && stmt[i+1] == '-' && isOperator(stmt[i+2]) == 0) {/*If there are triple operation consecutively
                                                                                            it directly go last end if */
                        i++;
                        stmt[i] = '-';
                }
                else if(stmt[i] == '-' && stmt[i+1] == '+' && isOperator(stmt[i+2]) == 0) {/*If there are triple operation consecutively
                                                                                            it directly go last end if */
                        i++;
                        stmt[i] = '-';
                }
                else if(stmt[i] == '*' && stmt[i+1] == '+' && isOperator(stmt[i+2]) == 0){/* "8*+2" */
                    i++;
                    stmt[i] = '*';
                }
                else if(stmt[i] == '/' && stmt[i+1] == '+' && isOperator(stmt[i+2]) == 0){/* "8/+2" */
                    i++;
                    stmt[i] = '/';
                }
                else if(stmt[i+1] != '(' && stmt[i+1] != ')' && isHexOperand(stmt[i+1]) == 0){/* If there is operator overloading condition, it prints error */
                        push("error");
                        return;
                }
                while(isEmpty(StackOfOperation) == 0 && getTop(StackOfOperation)[0] != '(' && HasHigherPrecedence(getTop(StackOfOperation)[0], stmt[i]) == 1){
                            processHexOperation();
                            pop(getTop(StackOfOperation)[0]);
                      }
                       pushCharacter(stmt[i]);
            }
            else if(isHexOperand(stmt[i]) == 1){
                if(isHexOperand(stmt[i+1]) == 1){ /*If element has more than one digit: */
                    operand[j] = stmt[i];   /*Is one after is digit ? (add it to char array) : (jump else section) */
                    j++;
                }else{
                    if(operand[0] == '\0'){ /*If char array which hold more than one digit is empty, in other words
                                                    the element has only one element:*/
                        hex[0] = stmt[i];
                        hex[1] = '\0';
                        strcpy(hex, convertToHex(hex));
                        push(hex);
                    }
                    else{/*If one after element not digit but this element has more than one digit, then add it to char array then push  */
                        operand[j++] = stmt[i];
                        operand[j] = '\0';
                        j = 0;
                        strcpy(hex, convertToHex(operand));
                        push(hex);
                    }
                }
            }
            else if(stmt[i] == '('){
                pushCharacter(stmt[i]);
            }
            else if(stmt[i] == ')'){
                while(isEmpty(StackOfOperation) == 0 && getTop(StackOfOperation)[0] != '('){
                    processHexOperation();               /*If it time to calculation, it makes necessary calculations */
                    pop(getTop(StackOfOperation)[0]); /*When calculation is done, it needed to be iterated to next operation */
                }
                if(isEmpty(StackOfOperation) == 1){ /*If there is a closing parentheses that not opened before:  */
                    push("error");
                    return;
                }
                pop(getTop(StackOfOperation)[0]); /*When it is done with an open-closed parenthesis popped open parenthesis from stack */
            }

        else{ /*If any character that is not expected: */
           push("error");
           return;
        }
    }
    while(isEmpty(StackOfOperation) == 0){
        processHexOperation();
        if(getTop(StackOfOperation)[0] == '('){ /*If an opening parentheses is not closed with closing parentheses. */
            push("error");
            return;
        }
        pop(getTop(StackOfOperation)[0]);
    }

}

/*It decides which operand precedence are greater. */
short HasHigherPrecedence(char op1, char op2)
{
	int op1Prec = GetOperatorPrecedence(op1);
	int op2Prec = GetOperatorPrecedence(op2);

	if(op1Prec == op2Prec)
	{
	    return 1;
	}
	return op1Prec > op2Prec ?  1 : 0;
}

/*When it needed to be calculation as hex, it needs to be "0x2AD.." format. It converts the single form number to "0x2AD.. form */
char *convertToHex(char *number){
    int k, j;
    char *tmp = (char *)malloc(strlen(number + 3) * sizeof(char));
    if(number[0] == '-'){
        k = 1;
        j = 3;
        tmp[0] = '-';
        tmp[1] = '0';
        tmp[2] = 'x';
    }
    else{
        j = 2;
        k = 0;
        tmp[0] = '0';
        tmp[1] = 'x';
    }
    for(; k < strlen(number); k++, j++){
        tmp[j] = number[k];

    }
    tmp[j] = '\0';
    return tmp;
}

/*It determines precedence of operator */
short GetOperatorPrecedence(char op)
{
    short prec = -1;
    if(op == '+' || op == '-'){
        prec = 1;
    }
    else if(op == '*' || op == '/'){
        prec = 2;
    }
	return prec;
}


/*Controlling it is + - * / */
short isOperator(char operation){
    if(operation == '+' || operation == '-' || operation == '*' || operation == '/'){
        return 1;
    }
    return 0;
}

/*Controlling it is 0 1 2 3 4 5 6 7 8 9 */
short isOperand(char operand){
    if(operand >= '0' && operand <= '9'){
        return 1;
    }
    return 0;
}

/*Controlling it is A B C D E F 0 1 2 3 4 5 6 7 8 9 */
short isHexOperand(char operand){
    if((operand >= '0' && operand <= '9') || (operand >= 'a' && operand <= 'f') || (operand >= 'A' && operand <= 'F')){
        return 1;
    }
    return 0;
}
/*-----------------------------------------------------------------------------------------*/

/*Calculator*/
/*It decides operation according to top element of the operator stack*/
void processOperation(){
    if(getTop(StackOfOperation)[0] == '+'){
        add();
    }
    else if(getTop(StackOfOperation)[0] == '-'){
        subtract();
    }
    else if(getTop(StackOfOperation)[0] == '*'){
        multiply();
    }
    else if(getTop(StackOfOperation)[0] == '/'){
        divide();
    }
}

/*It decides Hex operation as appears above */
void processHexOperation(){
    if(getTop(StackOfOperation)[0] == '+'){
        addHex();
    }
    else if(getTop(StackOfOperation)[0] == '-'){
        subtractHex();
    }
    else if(getTop(StackOfOperation)[0] == '*'){
        multiplyHex();
    }
    else if(getTop(StackOfOperation)[0] == '/'){
        divideHex();
    }
}

/*Integers addition operation. Getting top of operand stack, pop it then adding  */
void add(){

    char s[25];
    int op1 = atoi(getTop(StackOfOperand));
    int op2, result;
    if(op1 < 0){/*Working mechanism of pop() function is char. Therefore when we sent a negative number's first character to it. */
       pop(getTop(StackOfOperand)[1]);
    }
    else{
        pop(getTop(StackOfOperand)[0]);
    }
    op2 = atoi(getTop(StackOfOperand));
    if(op2 < 0){/*Working mechanism of pop() function is char. Therefore when we sent a negative number's first character to it. */
       pop(getTop(StackOfOperand)[1]);
    }
    else{
        pop(getTop(StackOfOperand)[0]);
    }
    result = op1 + op2;
    sprintf(s, "%d", result);
    push(s);

}

void subtract(){
    char s[25];
    int op1 = atoi(getTop(StackOfOperand));
    int op2, result;
    if(op1 < 0){/*Working mechanism of pop() function is char. Therefore when we sent a negative number's first character to it. */
       pop(getTop(StackOfOperand)[1]);
    }
    else{
    pop(getTop(StackOfOperand)[0]);
    }
    op2 = atoi(getTop(StackOfOperand));
    if(op2 < 0){/*Working mechanism of pop() function is char. Therefore when we sent a negative number's first character to it. */
       pop(getTop(StackOfOperand)[1]);
    }
    else{
        pop(getTop(StackOfOperand)[0]);
    }
    result = op2 - op1;
    sprintf(s, "%d", result);
    push(s);
}

void multiply(){
    char s[25];
    int op1 = atoi(getTop(StackOfOperand));
    int op2, result;
    if(op1 < 0){/*Working mechanism of pop() function is char. Therefore when we sent a negative number's first character to it. */
       pop(getTop(StackOfOperand)[1]);
    }
    else{
    pop(getTop(StackOfOperand)[0]);
    }
    op2 = atoi(getTop(StackOfOperand));
    if(op2 < 0){/*Working mechanism of pop() function is char. Therefore when we sent a negative number's first character to it. */
       pop(getTop(StackOfOperand)[1]);
    }
    else{
        pop(getTop(StackOfOperand)[0]);
    }
    result = op1 * op2;
    sprintf(s, "%d", result);
    push(s);
}

void divide(){
    char s[25];
    int op1 = atoi(getTop(StackOfOperand));
    int result, op2;
    if(op1 < 0){/*Working mechanism of pop() function is char. Therefore when we sent a negative number's first character to it. */
       pop(getTop(StackOfOperand)[1]);
    }
    else{
    pop(getTop(StackOfOperand)[0]);
    }
   op2 = atoi(getTop(StackOfOperand));
    if(op2 < 0){/*Working mechanism of pop() function is char. Therefore when we sent a negative number's first character to it. */
       pop(getTop(StackOfOperand)[1]);
    }
    else{
        pop(getTop(StackOfOperand)[0]);
    }
    result = op2 / op1; /*While LIFO, op2 should be divided by op1 that first comer */
    sprintf(s, "%d", result);
    push(s);
}

void addHex(){
    char s[25];
    char tmp[25];
    long op1, op2, result;
    unsigned uInt;
        if(isHexControl(getTop(StackOfOperand)) == 0){ /*If coming number is not form of Hex, convert it to hex form */
            strcpy(tmp, convertToHex(getTop(StackOfOperand)));
            op1 = strtol(tmp, NULL, 0);
        }
        else{
            op1 = strtol(getTop(StackOfOperand), NULL, 0);
        }
    if(op1 < 0){/*Working mechanism of pop() function is char. Therefore when we sent a negative number's first character to it. */
       pop(getTop(StackOfOperand)[1]);
    }
    else{
        pop(getTop(StackOfOperand)[0]);
    }
        if(isHexControl(getTop(StackOfOperand)) == 0){/*If coming number is not form of Hex, convert it to hex form */
            strcpy(tmp, convertToHex(getTop(StackOfOperand)));
            op2 = strtol(tmp, NULL, 0);
        }
        else{
            op2 = strtol(getTop(StackOfOperand), NULL, 0);
        }
    if(op2 < 0){/*Working mechanism of pop() function is char. Therefore when we sent a negative number's first character to it. */
       pop(getTop(StackOfOperand)[1]);
    }
    else{
        pop(getTop(StackOfOperand)[0]);
    }
    result = op1 + op2; /*While LIFO, op2 should be divided by op1 that first comer */

    if(result < 0){ /*If result is negative, then we should prevent complement numbers 'FFFFF' therefore this is a way that
                        hold digit part of hexadecimal number and add a '-' beginning of it. */
        sprintf(tmp, "%ld", result);
        uInt = strtol(&tmp[1], NULL, 0);
        sprintf(s, "-%X", uInt);
    }
    else{
            uInt = result;
            sprintf(s, "%X", uInt);
    }
    push(s);
}

void subtractHex(){
    char s[25];
    char tmp[25];
    long op1, op2;
    unsigned uInt;
    long result;
        if(isHexControl(getTop(StackOfOperand)) == 0){/*If coming number is not form of Hex, convert it to hex form */
            strcpy(tmp, convertToHex(getTop(StackOfOperand)));
            op1 = strtol(tmp, NULL, 0);
        }
        else{
            op1 = strtol(getTop(StackOfOperand), NULL, 0);
        }
    if(op1 < 0){
       pop(getTop(StackOfOperand)[1]);
    }
    else{
        pop(getTop(StackOfOperand)[0]);
    }
        if(isHexControl(getTop(StackOfOperand)) == 0){/*If coming number is not form of Hex, convert it to hex form */
            strcpy(tmp, convertToHex(getTop(StackOfOperand)));
            op2 = strtol(tmp, NULL, 0);
        }
        else{
            op2 = strtol(getTop(StackOfOperand), NULL, 0);
        }
    if(op2 < 0){
       pop(getTop(StackOfOperand)[1]);
    }
    else{
        pop(getTop(StackOfOperand)[0]);
    }
    result = op2 - op1; /*While LIFO, op2 should be divided by op1 that first comer */
    if(result < 0){     /*If result is negative, then we should prevent complement numbers 'FFFFF' therefore this is a way that
                        hold digit part of hexadecimal number and add a '-' beginning of it. */
        sprintf(tmp, "%ld", result);
        uInt = strtol(&tmp[1], NULL, 0);
        sprintf(s, "-%X", uInt);
    }
    else{
            uInt = result;
            sprintf(s, "%X", uInt);
    }
    push(s);
}

void multiplyHex(){
    char s[25];
    char tmp[25];
    long op1, op2, result;
    unsigned uInt;
        if(isHexControl(getTop(StackOfOperand)) == 0){/*If coming number is not form of Hex, convert it to hex form */
            strcpy(tmp, convertToHex(getTop(StackOfOperand)));
            op1 = strtol(tmp, NULL, 0);
        }
        else{
            op1 = strtol(getTop(StackOfOperand), NULL, 0);
        }
    if(op1 < 0){
       pop(getTop(StackOfOperand)[1]);
    }
    else{
        pop(getTop(StackOfOperand)[0]);
    }
        if(isHexControl(getTop(StackOfOperand)) == 0){/*If coming number is not form of Hex, convert it to hex form */
            strcpy(tmp, convertToHex(getTop(StackOfOperand)));
            op2 = strtol(tmp, NULL, 0);
        }
        else{
            op2 = strtol(getTop(StackOfOperand), NULL, 0);
        }
    if(op2 < 0){
       pop(getTop(StackOfOperand)[1]);
    }
    else{
        pop(getTop(StackOfOperand)[0]);
    }
    result = op1 * op2; /*While LIFO, op2 should be divided by op1 that first comer */
    if(result < 0){     /*If result is negative, then we should prevent complement numbers 'FFFFF' therefore this is a way that
                        hold digit part of hexadecimal number and add a '-' beginning of it. */
        sprintf(tmp, "%ld", result);
        uInt = strtol(&tmp[1], NULL, 0);
        sprintf(s, "-%X", uInt);
    }
    else{
            uInt = result;
            sprintf(s, "%X", uInt);
    }
    push(s);
}

void divideHex(){
    char s[25];
    char tmp[25];
    long op1;
    long op2, result;
    unsigned uInt;
        if(isHexControl(getTop(StackOfOperand)) == 0){/*If coming number is not form of Hex, convert it to hex form */
            strcpy(tmp, convertToHex(getTop(StackOfOperand)));
            op1 = strtol(tmp, NULL, 0);
        }
        else{
            op1 = strtol(getTop(StackOfOperand), NULL, 0);
        }
    if(op1 < 0){
       pop(getTop(StackOfOperand)[1]);
    }
    else{
        pop(getTop(StackOfOperand)[0]);
    }
        if(isHexControl(getTop(StackOfOperand)) == 0){/*If coming number is not form of Hex, convert it to hex form */
            strcpy(tmp, convertToHex(getTop(StackOfOperand)));
            op2 = strtol(tmp, NULL, 0);
        }
        else{
            op2 = strtol(getTop(StackOfOperand), NULL, 0);
        }
    if(op2 < 0){
       pop(getTop(StackOfOperand)[1]);
    }
    else{
        pop(getTop(StackOfOperand)[0]);
    }
    result = op2 / op1; /*While LIFO, op2 should be divided by op1 that first comer */
    if(result < 0){     /*If result is negative, then we should prevent complement numbers 'FFFFF' therefore this is a way that
                        hold digit part of hexadecimal number and add a '-' beginning of it. */
        sprintf(tmp, "%ld", result);
        uInt = strtol(&tmp[1], NULL, 0);
        sprintf(s, "-%X", uInt);
    }
    else{
            uInt = result;
            sprintf(s, "%X", uInt);
    }
    push(s);
}
/*If coming number is already in form of hex it returns 1, else 0 */
short isHexControl(char *number){
    int i;

    for(i = 0; i < strlen(number); i++){
        if(number[i] == 'x' || number[i] == 'X'){
           return 1;
        }
    }
    return 0;
}
/*---------------------------------------------------------------------------   --------------*/
/*Stack Functions */
void push(char *expr){
    /*If it is error then reset stacks and add error  */
    if(strcmp(expr, "error") == 0){
        StackOfOperand = NULL;
        StackOfOperation = NULL;
        StackOfOperand = (struct Node*)malloc(sizeof(struct Node));
        StackOfOperand->token = (char *)malloc((strlen(expr)+1) * sizeof(char));
        strcpy(StackOfOperand->token, expr);
        StackOfOperand->next = NULL;
    }
    /*If it is operand or it is negative (-1) its length will be > 1  */
    else if(isHexOperand(expr[0]) == 1 || strlen(expr) > 1){
        if(StackOfOperand == NULL){
            StackOfOperand = (struct Node*)malloc(sizeof(struct Node));
            StackOfOperand->value = atoi(expr);
            StackOfOperand->token = (char *)malloc((strlen(expr)+1) * sizeof(char));
            strcpy(StackOfOperand->token, expr);
            StackOfOperand->next = NULL;
        }
        else{
            struct Node *tmp = (struct Node*)malloc(sizeof(struct Node));
            tmp->value = atoi(expr);
            tmp->token = (char*)malloc((strlen(expr)+1) * sizeof(char));
            strcpy(tmp->token, expr);
            tmp->next = StackOfOperand;
            StackOfOperand = tmp;
        }
    }
    else{
        if(StackOfOperation == NULL){
            StackOfOperation = (struct Node*)malloc(sizeof(struct Node));
            StackOfOperation->token = (char*)malloc((strlen(expr)+1) * sizeof(char));
            strcpy(StackOfOperation->token, expr);
            StackOfOperation->next = NULL;
        }
        else{
            struct Node *tmp = (struct Node*)malloc(sizeof(struct Node));
            tmp->token = (char*)malloc((strlen(expr)+1) * sizeof(char));
            strcpy(tmp->token, expr);
            tmp->next = StackOfOperation;
            StackOfOperation = tmp;
        }
    }
}

/*It pushes the character which call this function  */
void pushCharacter(char token){
     char tmp[2];
     tmp[0] = token;
     tmp[1] = '\0';
     push(tmp);
}

void pop(char expr){
    if(isHexOperand(expr) == 1){
        struct Node *topBackup = StackOfOperand;
        if (topBackup == NULL){
            printf("It can't be popped, it is empty\n");
        }
        else{
            StackOfOperand = StackOfOperand->next;
            free(topBackup);
        }
    }
    else{
        struct Node *topBackup = StackOfOperation;
        if (topBackup == NULL){
            printf("It can't be popped, it is empty\n");
        }
        else{
            StackOfOperation = StackOfOperation->next;
            free(topBackup);
        }
    }
}
char *getTop(struct Node *type){
       if(type != NULL){
            return type->token;
       }
       else{
            printf("Empty Stack\n");
            return NULL;
       }
}

short isEmpty(struct Node *type){
    return type == NULL ? 1 : 0;
}

void displayStack(struct Node *type){
    /*if(type != NULL && type->token[0] != '\0'){
       struct Node *tmp = type;
       printf("Operators\n");
       while(tmp != NULL){
            printf("%s\n", tmp->token);
            tmp = tmp->next;
       }
       printf("*******\n");
    }
    else if(type != NULL && type->token[0] == '\0'){
        struct Node *tmp = type;
       printf("Operands\n");
       while(tmp != NULL){
            printf("%d\n", tmp->value);
            tmp = tmp->next;
       }
       printf("*******\n");
    } */
}

/*--------------------------------- */
/*Queue Functions */

void enqueue(char *expr){
    struct Node *tmp = (struct Node*)malloc(sizeof(struct Node));
    tmp->token = (char *)malloc((strlen(expr) + 1) * sizeof(char));
    strcpy(tmp->token, expr);
    tmp->next = NULL;

    if(queueFront == NULL && queueRear == NULL){
        queueFront = queueRear = tmp;
        return;
    }
    queueRear->next = tmp;
    queueRear = tmp;
}

void displayQueue(FILE *file){

     struct Node *tmp = queueFront;
     if(queueFront == NULL){
        printf("Empty Queue\n");
        fprintf(file, "Empty Queue\n");
        return;
    }
    else{
        while(tmp != NULL){
            printf("%s\n", tmp->token);
            fprintf(file, "%s\n", tmp->token);
            tmp = tmp->next;
        }
        queueFront = queueRear = NULL; /*After printing everything in queue, reset front and rear */
    }

}

