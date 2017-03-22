#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/limits.h> /*If you compile the program in windows please comment this line. */
/*
*   @author: Serhat YILMAZ - 21607858
*   @date:  25.12.2016
*
*       The program take command line argument. First argument (argv[1]) means directory will be scanned, second command (argv[2])
*   means command file that includes the command for cases.
*       It scans directories recursively. It determines a file a readable file or not then add words inside files add to
*   binary tree.
*       First it sort readable files in dictionary order then add them in dictionary order binary tree. Then read from them
*   one by one.
*
*/


struct Words{
    char *name;
    struct FileAndNumber *file;
    int totalNumberOfWord;
};

struct FileAndNumber{
    char *fileName;
    int numberOfWord;
    struct FileAndNumber *front;
    struct FileAndNumber *rear;
    struct FileAndNumber *next;
};

struct Tree{
    struct Words *word;
    struct Tree *left;
    struct Tree *right;

}*root;

struct DirectoryTree{
    char *name;
    char *path;
    struct DirectoryTree *left;
    struct DirectoryTree *right;
}*rootOfDirectoryTree;

/*INITIAL FUNCTIONS  */
void readText(char *, char *, int);
void readCommand(char *, FILE *);
int isDirectory(char *);
void proceed();
void proceedCommand(char *, FILE *);
void directoryList(char *);

/*BINARY TREE FUNCTIONS */
struct Tree * addTree(struct Tree *, char *, char *);
struct Tree *removeTree(struct Tree *, char *);
struct Tree *findMinTree(struct Tree *);
void printTreeASC(struct Tree *, FILE *);
void printTreeDSC(struct Tree *, FILE *);
int findDepth(struct Tree *, struct Tree *, int);
struct Tree *searchTree(char *);
int getDepth(char *);

int findMax(int, int);
char *toUpper(char *);

void addWordToFile(struct Tree *, char *);

/*To read directories and files in orderly. The functions construct a new tree */
void inOrderDirectoryTree(struct DirectoryTree *);
struct DirectoryTree *addToDirectoryTree(struct DirectoryTree *, char *, char *);
struct DirectoryTree *findMinDirectoryTree(struct DirectoryTree *);
struct DirectoryTree *deleteDirectoryTree(struct DirectoryTree *, char *);



int main(int argc, char *argv[])
{
    FILE *fi = fopen("output.txt", "w");
    root = NULL;
    rootOfDirectoryTree = NULL;

    directoryList(argv[1]);
    proceed();

    readCommand(argv[2], fi);
    fclose(fi);


    return 0;
}

/*To read entered directory and files and directories inside it. */
void directoryList(char *path){
    /* DECLARATIONS */
    DIR *directory;
    struct dirent *directoryFeatures; /*To hold directories' features. */
    char *directoryName;
    char newPath[PATH_MAX];
    int pathLength;
    /* PROCESSES */
    directory = opendir(path);
    if(directory == NULL){
        fprintf(stderr, "ERROR : Directory can't be opened.\n");
        exit(EXIT_FAILURE);
    }
    /*It is a loop for iterating directories */
    while(1){
        directoryFeatures = readdir(directory);
        if(directoryFeatures == NULL){ /*If there is no more directory to read, then exit from loop */
            break;
        }
        directoryName = directoryFeatures->d_name;
        sprintf(newPath, "%s/%s", path, directoryFeatures->d_name); /*The function runs recursively. Therefore
                                                                        it needs next directory or file if it exists.*/
        if(isDirectory(newPath) == 1){
            if(strcmp(directoryFeatures->d_name, ".") == 0 || strcmp(directoryFeatures->d_name, "..") == 0){
                continue;
            }
            sprintf(newPath, "%s/%s", path, directoryFeatures->d_name);
            pathLength = strlen(newPath) + 1;
            if(pathLength >= PATH_MAX){
                fprintf (stderr, "ERROR : Path length is too long.\n");
                exit (EXIT_FAILURE);
            }
            directoryList(newPath); /*Recursion part of function. */

        }
        else{/*If the read part of directory is not a directory then it is sent to function that proceed it  */
            sprintf(newPath, "%s/%s", path, directoryName);
            readText(path, directoryName, 1);
        }
    }
}

/*First program goes into directories, read their names and add to tree. Find first file and path of it and take them into readText() */
void proceed(){
    struct DirectoryTree *tmpT;
    char exactPath[PATH_MAX];
    while(rootOfDirectoryTree != NULL){
        tmpT = findMinDirectoryTree(rootOfDirectoryTree); /*Take the first file that is alphabetically comes first and will be read */
        sprintf(exactPath, "%s/%s", tmpT->path, tmpT->name);
        readText(exactPath, tmpT->name, 0);
        deleteDirectoryTree(rootOfDirectoryTree, tmpT->name); /*after reading a file it deletes the file from tree to obtain next minimum file. */
    }
}

/* In mode 1: program goes into directories, reads their names looks their first 100 Bytes and add them to tree. */
/* In mode 0: It starts to read inside of files depend on proceed() function.*/
void readText(char *path, char *fileName, int mode){
    FILE *file;
    char controlCharacter;
    int counter_for_100_BYTES = 0;
    int controlASCII = 0;
    int counterForWords = 0;
    char word[100];
    char pathOfFile[300];
    sprintf(pathOfFile, "%s/%s", path, fileName);
    if(mode == 1){
        int ASCII_control = open(pathOfFile, O_RDONLY);/*Reading first 100 Bytes to determine it is a readable file */
        while(counter_for_100_BYTES < 100 && read(ASCII_control, &controlCharacter, sizeof(controlCharacter)) != 0){ /*read() returns 0 when it comes end of file */
            if(controlCharacter > 127 || controlCharacter < 0){/*If there is a inconsistent situation in file in
                                                                the other words, the file contains a character that not an ASCII
                                                                then it marks this file as 1.(not read)*/
                controlASCII = 1;
                break;
            }
            counter_for_100_BYTES++;
        }
        close(ASCII_control);

        if(controlASCII == 0){
            addToDirectoryTree(rootOfDirectoryTree, path, fileName);
        }
    }
    else{
        file = fopen(path, "r");
            while((controlCharacter = fgetc(file)) != EOF){/*Reading file character by character. */
                /*If the character is a lowercase or uppercase or ' ' or '\n' or '-' character, then proceed them.
                If it is not then continue reading characters from file.  */
                if((controlCharacter >= 65 && controlCharacter <=90) || (controlCharacter >=97 && controlCharacter <=127) || (controlCharacter >= 48 && controlCharacter <= 57) ||controlCharacter == ' ' || controlCharacter == '\n'
                    || controlCharacter == '-' || controlCharacter == '\t'){

                    if(controlCharacter == ' ' || controlCharacter == '\n' || controlCharacter == '\t' || controlCharacter == '\r'){/*If there is a whitespace or new line(means end of word)
                                                                              it ends the process of creating word and send it to tree to add  */
                        word[counterForWords] = '\0';
                        counterForWords = 0;
                        addTree(root, word, fileName);
                    }
                    else{
                        word[counterForWords++] = controlCharacter;/*If characters are still coming, it adds them in a string. */
                    }
                }
            }
            if(searchTree(word) == NULL){ /*If a word is last word in a file, then it prevents lack of the word in output file. */
                word[counterForWords] = '\0';
                addTree(root, word, fileName);
            }
        }
}

/*Read the command txt file. */
void readCommand(char *path, FILE *out){
    FILE *file = fopen(path, "r");
    char comingCharacter;
    char command[300];
    int counter = 0;
    while((comingCharacter = fgetc(file)) != EOF){
        if(comingCharacter == '\n'){
            command[counter] = '\0';
            counter = 0;
            proceedCommand(command, out);
            for (; counter < 300; counter++) /*To reset char array. */{
                command[counter] = '\0';
            }
            counter = 0;
        }
        else{
            command[counter++] = comingCharacter;
        }
    }
    if(command[0] != '\0'){
        proceedCommand(command, out);
    }
}

/*After reading command the function proceed it.  */
void proceedCommand(char *command, FILE *out){
    char commandBackup[50];
    int counter = 0;
    struct Tree *tmp;
    struct FileAndNumber *tmpFile;
    char *token;
    char nameFile[50];
    char *data[3];
    char dataFile[50];
    if(command[0] != '\0'){
        strcpy(commandBackup, command);
        token = strtok(command, " ");
        while(token != NULL){
            data[counter++] = token;
            token = strtok(NULL, " ");
        }
        if(strcmp(data[0], "PRINT") == 0){
            if(strcmp(data[2], "DSC") == 0){
                fprintf(out, "%s\n", commandBackup);
                printTreeDSC(root, out);
            }
            else{
                fprintf(out, "%s\n", commandBackup);
                printTreeASC(root, out);
            }
        }
        else if(strcmp(data[0], "SEARCH") == 0){
            printf("%s\n", commandBackup);
            fprintf(out, "%s\n", commandBackup);
            tmp = searchTree(data[1]);
            if(tmp == NULL){
                fprintf(out, "ERROR : There is no named %s in tree\n", data[1]);
                printf("ERROR : There is no named %s in tree\n", data[1]);
                return ;
            }

            printf(" %s\n\t%d\n\t%d\n", tmp->word->name, tmp->word->totalNumberOfWord, getDepth(data[1]));
            fprintf(out, " %s\n\t%d\n\t%d\n", tmp->word->name, tmp->word->totalNumberOfWord, getDepth(data[1]));
            tmpFile = tmp->word->file->front;
            while(tmpFile != NULL){
                printf("\t%s %d\n", tmpFile->fileName, tmpFile->numberOfWord);
                fprintf(out, "\t%s %d\n", tmpFile->fileName, tmpFile->numberOfWord);
                tmpFile = tmpFile->next;
            }
        }
        else if(strcmp(data[0], "ADD") == 0){
            printf("%s\n", commandBackup);
            fprintf(out, "%s\n", commandBackup);
            if(isDirectory(data[1]) == 1){
                directoryList(data[1]);
                proceed();
            }
            else{
                    /*If it is just a file then take its file name, backup its path then read the file via readText(). */
                strcpy(nameFile, data[1]);
                token = strtok(data[1], "/");
                while(token != NULL){
                    strcpy(dataFile, token);
                    token = strtok(NULL, "/");
                    if(token == NULL){
                        break;
                    }
                }
                readText(nameFile, dataFile, 0);
            }
        }
        else if(strcmp(data[0], "REMOVE") == 0){
            printf("%s\n", commandBackup);
            fprintf(out, "%s\n", commandBackup);
            tmp = removeTree(root, data[1]);
            if(tmp == NULL){
                fprintf(out, "ERROR : There is no named %s in tree\n", data[1]);
                printf("ERROR : There is no named %s in tree\n", data[1]);
                return ;
            }
        }
    }
}

/*To add coming structure into tree. Runs recursively. */
struct Tree * addTree(struct Tree *currentTree, char *word, char *fileName){
    if(strcmp(word, "") == 0){/*When there are more than one '\n' character occurs "" characters. To prevent them: */
        return NULL;
    }
    if(root == NULL){/*If there is no any element in tree, then it creates root of tree*/
        root = (struct Tree*)malloc(sizeof(struct Tree));
        root->left = NULL;
        root->right = NULL;

        root->word = (struct Words *)malloc(sizeof(struct Words));/*Memory allocation for new structure inside Tree. */
        root->word->totalNumberOfWord = 1;
        root->word->file = NULL;
        root->word->name = (char *)malloc((strlen(word) + 1) * sizeof(char));

        strcpy(root->word->name, word);
        addWordToFile(root, fileName);
        return root;
    }
    else{/*If there is at least one element in tree then it creates new Tree structures then add them
                according to its value (smaller than or larger than parent value.*/
        if(currentTree == NULL){
            currentTree = (struct Tree*)malloc(sizeof(struct Tree));
            currentTree->left = NULL;
            currentTree->right = NULL;

            currentTree->word = (struct Words *)malloc(sizeof(struct Words));
            currentTree->word->totalNumberOfWord = 1;
            currentTree->word->file = NULL;
            currentTree->word->name = (char *)malloc((strlen(word) + 1) * sizeof(char));

            strcpy(currentTree->word->name, word);
            addWordToFile(currentTree, fileName);

            return currentTree;
        }
        else if( strcmp(toUpper(currentTree->word->name), toUpper(word)) == 0){
            addWordToFile(currentTree, fileName);
            currentTree->word->totalNumberOfWord++;
            return currentTree;
        }
        else if(strcmp(toUpper(word), toUpper(currentTree->word->name)) > 0 ){
            currentTree->right = addTree(currentTree->right, word, fileName);
        }
        else{
            currentTree->left =  addTree(currentTree->left, word, fileName);
        }
    }
    return currentTree;
}

/* Remove command */
struct Tree *removeTree(struct Tree *currentTree, char *name){
    struct Tree *tmp;
    if(currentTree == NULL){
        return NULL;
    }
    else if(strcmp(toUpper(currentTree->word->name), toUpper(name)) < 0){
        currentTree->right = removeTree(currentTree->right, name);
    }
    else if(strcmp(toUpper(currentTree->word->name), toUpper(name)) > 0){
        currentTree->left = removeTree(currentTree->left, name);
    }
    else if(currentTree->left != NULL && currentTree->right != NULL){
        tmp = findMinTree(currentTree->right);
        strcpy(currentTree->word->name, tmp->word->name);
        currentTree->word->file = tmp->word->file;
        currentTree->word->totalNumberOfWord = tmp->word->totalNumberOfWord;
        currentTree->right = removeTree(tmp, tmp->word->name);
    }
    else if(currentTree->left == NULL && currentTree->right == NULL){
        if(currentTree == root){
            root = NULL;
        }
        free(currentTree);
        currentTree = NULL;
    }
    else{
        tmp = currentTree;
        if(currentTree->left == NULL){
            if(currentTree == root){
                root = currentTree->right;
            }
            else{
                currentTree = currentTree->right;
            }
        }
        else{
            if(currentTree == root){
                root = currentTree->left;
            }
            else{
                currentTree = currentTree->left;
            }
        }
        free(tmp);
    }
    return currentTree;
}

/*To replace deleted tree, it is find the minimum tree on its right.*/
struct Tree *findMinTree(struct Tree *currentTree){
    if(currentTree == NULL){
        return NULL;
    }
    if(currentTree->left == NULL){
        return currentTree;
    }
    else{
        return findMinTree(currentTree->left);
    }
}

/*It is a sub function of addTree. There is structures to add them each other. Therefore program needs a function: */
void addWordToFile(struct Tree *currentTree, char *fileName){
    struct FileAndNumber *tmpFile;
    if(currentTree->word->file == NULL){/*If file is read, the word is come and there is no word similar to it then it creates new
                                            file (with name) and increase number of the word inside the file.*/
        currentTree->word->file = (struct FileAndNumber *)malloc(sizeof(struct FileAndNumber));
        tmpFile = (struct FileAndNumber *)malloc(sizeof(struct FileAndNumber));
        tmpFile->numberOfWord = 1;
        tmpFile->next = NULL;
        tmpFile->fileName = (char *)malloc((strlen(fileName) + 1) * sizeof(char));
        strcpy(tmpFile->fileName, fileName);

        currentTree->word->file->front = currentTree->word->file->rear = tmpFile;
    }
    else{/*If there is a word similar to it, just increases the number of the word. */
        if(strcmp(fileName, currentTree->word->file->rear->fileName) == 0){
            currentTree->word->file->rear->numberOfWord++;
        }
        else{/*If there is a word similar to it in another file, then it adds file name onto it and increase the value. */
            tmpFile = (struct FileAndNumber *)malloc(sizeof(struct FileAndNumber));
            tmpFile->fileName = (char *)malloc((strlen(fileName) + 1) * sizeof(char));
            strcpy(tmpFile->fileName, fileName);
            tmpFile->next = NULL;
            currentTree->word->file->rear->next = tmpFile;
            currentTree->word->file->rear = tmpFile;
            tmpFile->numberOfWord = 1;
        }
    }
}

/*It controls the path if it is directory or a file */
int isDirectory(char *path){
    struct stat status;
    stat(path, &status);
    return S_ISDIR(status.st_mode);
}

/*It is a function that search the coming value and return its node. It runs iteratively. */
struct Tree *searchTree(char *data){
    struct Tree *currentTree = root;

    while(strcmp(toUpper(data), toUpper(currentTree->word->name)) != 0){

        if(strcmp(toUpper(data), toUpper(currentTree->word->name)) < 0){

            currentTree = currentTree->left;
        }
        else{

            currentTree = currentTree->right;
        }
        if(currentTree == NULL){

            return NULL;
        }
    }
    return currentTree;
}

/*To write of all tree in ascending order: */
void printTreeASC(struct Tree *currentTree, FILE *fi){

    struct FileAndNumber *tmpFile;
    if(currentTree != NULL){

        printTreeASC(currentTree->left, fi);
        printf(" %s\n\t%d\n\t%d\n", currentTree->word->name, currentTree->word->totalNumberOfWord, getDepth(currentTree->word->name));
        fprintf(fi, " %s\n\t%d\n\t%d\n", currentTree->word->name, currentTree->word->totalNumberOfWord, getDepth(currentTree->word->name));
        tmpFile = currentTree->word->file->front;
        while(tmpFile != NULL){
            printf("\t%s %d\n", tmpFile->fileName, tmpFile->numberOfWord);
            fprintf(fi, "\t%s %d\n", tmpFile->fileName, tmpFile->numberOfWord);
            tmpFile = tmpFile->next;
        }

        printTreeASC(currentTree->right, fi);
    }

}

/*To find depth of total tree or specific node in the tree. If mode is 1, it search for depth of specific node.
If it is 0, then it looks for total depth. It runs recursively. */
int findDepth(struct Tree *currentTree, struct Tree *targetTree, int mode){
    if(currentTree == NULL){
        return 0;
    }
    else{
        if(mode == 1){
            if(strcmp(toUpper(targetTree->word->name), toUpper(currentTree->word->name)) > 0){
                return 1 + findDepth(currentTree->right, targetTree, 1);
            }
            else if(strcmp(toUpper(targetTree->word->name), toUpper(currentTree->word->name)) < 0){
                return 1 + findDepth(currentTree->left, targetTree, 1);
            }
            else{
                return 0;
            }
        }
        else{
            return 1 + (findMax(findDepth(currentTree->left, NULL, 0), findDepth(currentTree->right, NULL, 0)));
        }
    }
}

/*It cooperates with findDepth() function to find depth of desired node. */
int getDepth(char *data){
    struct Tree *treeTmpRoot = root;
    struct Tree *treeTmp = searchTree(data);
    return 1 + findDepth(treeTmpRoot, treeTmp, 1);
}

/*It returns value that is bigger than the other. */
int findMax(int element1, int element2){
    if(element1 > element2){
        return element1;
    }
    else{
        return element2;
    }
}

/*When comparing the strings, there is no importance of case sensitiveness. Therefore they are converted to uppercase strings
when two strings will be compared. */
char *toUpper(char *data){
    int i = 0;
    char *upperData = (char *)malloc((strlen(data) + 1)* sizeof(char));

    while(data[i]){
        upperData[i] = toupper(data[i]);
        i++;
    }
    upperData[i] = '\0';
    return upperData;
}

struct DirectoryTree *addToDirectoryTree(struct DirectoryTree *currentTree, char *path, char *name){
        if(rootOfDirectoryTree == NULL){
            rootOfDirectoryTree =(struct DirectoryTree *)malloc(sizeof(struct DirectoryTree));
            rootOfDirectoryTree->name = (char *)malloc((strlen(name) + 1) * sizeof(char));
            rootOfDirectoryTree->path = (char *)malloc((strlen(path) + 1) * sizeof(char));
            rootOfDirectoryTree->right = NULL;
            rootOfDirectoryTree->left = NULL;
            strcpy(rootOfDirectoryTree->name, name);
            strcpy(rootOfDirectoryTree->path, path);
            return rootOfDirectoryTree;
        }
        else{
            if(currentTree == NULL){
                currentTree =(struct DirectoryTree *)malloc(sizeof(struct DirectoryTree));
                currentTree->name = (char *)malloc((strlen(name) + 1) * sizeof(char));
                currentTree->path = (char *)malloc((strlen(path) + 1) * sizeof(char));
                currentTree->right = NULL;
                currentTree->left = NULL;
                strcpy(currentTree->name, name);
                strcpy(currentTree->path, path);
            }
            else if(strcmp(currentTree->name, name) < 0){
                currentTree->right = addToDirectoryTree(currentTree->right, path, name);
            }
            else{
                currentTree->left = addToDirectoryTree(currentTree->left, path, name);
            }
        }
        return currentTree;
}

void inOrderDirectoryTree(struct DirectoryTree *currentTree){
    if(currentTree != NULL){
        inOrderDirectoryTree(currentTree->left);
        printf("File Name: %s\nFile Path: %s\n", currentTree->name, currentTree->path);
        inOrderDirectoryTree(currentTree->right);
    }
}

struct DirectoryTree *deleteDirectoryTree(struct DirectoryTree *currentTree, char *name){
    struct DirectoryTree *tmp;
    if(currentTree == NULL){
        return NULL;
    }
    else if(strcmp(currentTree->name, name) < 0){
        currentTree->right = deleteDirectoryTree(currentTree->right, name);
    }
    else if(strcmp(currentTree->name, name) > 0){
        currentTree->left = deleteDirectoryTree(currentTree->left, name);
    }
    else if(currentTree->left != NULL && currentTree->right != NULL){
        tmp = findMinDirectoryTree(currentTree->right);
        strcpy(currentTree->name, tmp->name);
        strcpy(currentTree->path, tmp->path);
        currentTree->right = deleteDirectoryTree(tmp, tmp->name);
    }
    else if(currentTree->left == NULL && currentTree->right == NULL){
        if(currentTree == rootOfDirectoryTree){
            rootOfDirectoryTree = NULL;
        }
        free(currentTree);
        currentTree = NULL;
    }
    else{
        tmp = currentTree;
        if(currentTree->left == NULL){
            if(currentTree == rootOfDirectoryTree){
                rootOfDirectoryTree = currentTree->right;
            }
            else{
                currentTree = currentTree->right;
            }
        }
        else{
            if(currentTree == rootOfDirectoryTree){
                rootOfDirectoryTree = currentTree->left;
            }
            else{
                currentTree = currentTree->left;
            }
        }
        free(tmp);
    }
    return currentTree;
}

struct DirectoryTree *findMinDirectoryTree(struct DirectoryTree *currentTree){
    if(currentTree == NULL){
        return NULL;
    }
    else if(currentTree->left == NULL){
        return currentTree;
    }
    else{
        return findMinDirectoryTree(currentTree->left);
    }
}

void printTreeDSC(struct Tree *currentTree, FILE *fi){
    struct FileAndNumber *tmpFile;
    if(currentTree != NULL){

        printTreeDSC(currentTree->right, fi);
        printf(" %s\n\t%d\n\t%d\n", currentTree->word->name, currentTree->word->totalNumberOfWord, getDepth(currentTree->word->name));
        fprintf(fi, " %s\n\t%d\n\t%d\n", currentTree->word->name, currentTree->word->totalNumberOfWord, getDepth(currentTree->word->name));
        tmpFile = currentTree->word->file->front;
        while(tmpFile != NULL){
            printf("\t%s %d\n", tmpFile->fileName, tmpFile->numberOfWord);
            fprintf(fi, "\t%s %d\n", tmpFile->fileName, tmpFile->numberOfWord);
            tmpFile = tmpFile->next;
        }

        printTreeDSC(currentTree->left, fi);
    }
}
