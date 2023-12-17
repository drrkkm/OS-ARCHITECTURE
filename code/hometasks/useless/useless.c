/*Напишите программу useless (Unix System Extremely Late Execution
Software System), которая читает файл и запускает указанные в нём про-
граммы с указанной задержкой от времени старта программы useless.
Формат записи в файле: «время задержки в секундах» «программа для
выполнения».
Например:
5 ls
1 ./test
3 ps
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <fcntl.h>

int main(){

    char* file = calloc(255, sizeof(char));
    int flag = scanf("%s", file);
    if ( !flag ) return -1;
    FILE *fp = fopen(file, "r");
    if (fp == NULL) return -1;

    flag = 2;
    while (flag == 2) {
        int time = 0;
        char* command = calloc(255, sizeof(char));
        flag = fscanf(fp, "%i %s", &time, command);
        pid_t pid = fork(); // to run processes "in parallel"
        if ( pid < 0 ) return -1;  // error with fork
        if ( pid == 0 ) { // child

            sleep(time);
            if (execlp(command, command, NULL) == -1) return -1; // error when launch command
        }
        free(command);
    }
    fclose(fp);
    free(file);
    return 0;
}