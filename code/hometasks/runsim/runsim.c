/*Напишите программу runsim, которая осуществляет контроль количе-
ства одновременно работающих Unix-приложений, запущенных с её по-
мощью. Программа читает имя Unix-команды со стандартного ввода и
запускает её на выполнение. Количество одновременно работающих ко-
манд не должно превышать N, где N – параметр командной строки при
запуске runsim. При попытке запустить более чем N приложений выдай-
те сообщение об ошибке и продолжите ожидание ввода команд на ис-
полнение. Программа runsim должна прекращать свою работу при воз-
никновении признака конца файла на стандартном вводе.*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <sys/wait.h>
#include <sys/signal.h>

int global_counter = 0;

void check(int signal)
{
    while (waitpid(0, 0, WNOHANG) > 0) {
        global_counter -= 1;
    }
    return;
    signal -= 1;
}

int main(int argc, char* argv[]){
    signal(SIGCHLD, check);

    char *command = calloc(256, sizeof(char));

    if (argc < 2) return -1;
    int n = atoi(argv[1]);

    while (scanf("%s", command) == 1) {
        if (global_counter + 1 > n) {
            printf("No avalible process to launch\n");
            continue;
        }
        pid_t pid = fork();
        if ( pid < 0 ) return -1;
        if ( pid == 0 ) {
            if (execlp(command, command, NULL) == -1) {
                global_counter -= 1;
                return -1;
            }
        } if (pid > 0) {
            global_counter += 1;
        }
    }
    free(command);
    return 0;
}
