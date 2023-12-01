# Нити исполнения. Средства IPC: Семафоры

## Нити исполнения / потоки выполнения / threads
- Процесс = совокупность выделенных ресурсов + набор нитей исполнения
- Нить – «подпроцесс», «лёгкий процесс»
- Нити процесса совместно используют
  1. программный код;
  2. глобальные переменные;
  3. системные ресурсы
- Каждая нить имеет отдельные:
  1. программный счётчик; 
  2. содержимое регистров;
  3. стек

## Реализации потоков выполнения
1. «Лёгкие» единицы планирования ядра – потоки выполнения ядра
   - BSD: Light Weight Kernel Threads (LWKT);
   - Linux: Native POSIX Thread Library (NPTL);
   - MacOS: Apple Multiprocessing Services
   - Windows: WinAPI
1. Потоки в режиме пользователя на уровне библиотек - пользовательские потоки выполнения
   - GNU Portable Threads;
   - Thread Manager компании Apple;
   - Windows 7 user-mode scheduling;

## Стандарт POSIX для потоков выполнения - pthreads
1. POSIX threads -> PThreads.
2. Стандарт описывает API, реализация не ограничивается. Реализован во всех Unix-подобных ОС и Windows
3. Функции POSIX API:
   - pthread_create(), pthread_exit(), pthread_cancel(), pthread_self() – создание, удаление
   - pthread_join(), pthread_detach() – ожидание
   - pthread_attr_init(), pthread_attr_setdetachstate(), pthread_attr_destroy() – аттрибуты;
   - pthread_mutex_init(), pthread_mutex_destroy(), pthread_mutex_lock(), pthread_mutex_trylock(), pthread_mutex_unlock() – синхронизация.
   - pthread_cond_init(), pthread_cond_signal(), pthread_cond_wait() – условные переменные

## Создание потока выполнения

```c
#include <pthread.h>
int pthread_create( // Возвращаемое значение =0 – успех / >0 – (!) код ошибки, как в errno, но которая не изменяется
    pthread_t* thrid,  //thrid – идентификатор созданной нити.
    const pthread_attr_t* attr, // attr – атрибуты нити NULL – по-умолчанию
    void* (*start)(void*), //void* start(void*) – функция, которая будет выполняться внутри нити
    void* arg //arg – аргумент для функции
);
```

## Завершение потока выполнения

```c
#include <pthread.h>

void pthread_exit(void* status);
```

### Варианты завершения потока:
- Возврат из функции нити
- Вызов функции pthread_exit()
- Завершение всего процесса по exit()

## Ожидание потока выполнения


```c
#include <pthread.h>

int pthread_join( // Блокирует вызывающий поток до завершения указанной нити
    pthread_t thrid, // Возвращаемое значение =0 – успех / >0 – (!) код ошибки, как в errno, но которая не изменяется
    void** status //status – указатель, возвращаемый нитью, NULL, если результат не интересует
);
```

## Пример с потоком выполнения

```c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

void pause() {
    time_t start_time = time(NULL);
    while(time(NULL) == start_time) {} }

void* thread_func(void*) {
    for (int i = 0; i < 20; i++) {
        fputs("b\n",stderr);
        pause();
    }
}

int main (int argc, char *argv[], char *envp[]) {
    pthread_t thread;
    if( pthread_create(&thread, NULL, thread_func, NULL) ) return EXIT_FAILURE;
    for(int i = 0; i < 20; i++) {
        puts("a");
        pause();
    }
    if ( pthread_join(thread, NULL)) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
```

## Особенности доступа к совместно используемой памяти

1. Совместно используемая память:
   - для потоков исполнения – все глобальные переменные программы
   - для процессов – сегменты SysV IPC shared memory
2. Одновременное чтение из общей памяти: OK
3. Одновременная запись в общую память:
   - вероятность повреждения
   - необходимость синхронизации


## Синхронизация с помощью семафоров (semaphores)
1. Semaphore – примитив синхронизации на основе счётчика
2. Предложен Edsger Wybe Dijkstra в 1965.
3. Действия над семафором S, атомарные:
   - P(S) – ждать, если S==0, затем S = S – 1 (proberen – датск. проверить)
   - V(S) – увеличить S = S + 1 (verhogen – датск. увеличивать).

```c
______________________       _____________        _____________________
|                    |       |           |        |                   |
|     попробовать    |       |  работа с |        |  поднять (отдать) |
|опустить (захватить)| ----> |критическим| -----> |    семафор V(S)   |
|     семафор P(S)   |       | ресурсом  |        |                   |
|____________________|       |___________|        |___________________| 
```

## Семафоры System V IPC
11. Расширенный набор операций относительно Dijkstra.
2. Действия над семафором SysV IPC, атомарные:
   - A(S, n) – увеличить S = S + n
   - D(S, n) – попытаться уменьшить: ждать пока S < n, затем S = S – n
   - Z(S) – ждать, если S != 0

## Получение доступа / создание семафоров SysV IPC

```c
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/sem.h>
// Получает доступ или создаёт массив семафоров
int semget(key_t key, int nsems, int semflg); // Возвращаемое значение >0 – дескриптор SysV IPC -1 – ошибка, код ошибки в errno
// key – уникальное имя (ключ), созданное ftok() или IPC_PRIVATE
// nsems – количество семафоров в массиве
// semflg – флаги, комбинируются с помощью | «битовое или»
// • IPC_CREAT - создать если не существует
// • IPC_EXCL - вместе с предыдущим, создавать эксклюзивно, ошибка если сущуствует
// • Права доступа
// • 0400 – только чтение для владельца
// • 0200 – только запись для владельца
// • 0100 – только исполнение для владельца
// • 0040 – только чтение для группы
```

## Операции над семафором SysV IPC

```c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h> 
//Выполняет операцию над одним или несколькими семафорами
int semop(int semid, struct sembuf* sops, size_t nsops); //Возвращаемое значение 0 – успех / -1 – ошибка, код ошибки в errno
// semid – идентификатор массива семафоров
// sops – массив операций над несколькими семафорами, не обязательно всеми(!)
// nsops – количество операций в массиве sops

struct sembuf {
    unsigned short sem_num; // sem_num – номер (индекс) семафора в массиве семафоров
    short sem_op;
    short sem_flg; //sem_flg – флаги операции, 0 – по-умолчанию
}
// Кодирование операции 
// sem_op – код операции над семафором: 
// • +n – операция A(S,n)
// • –n – операция D(S, n)
// • 0 – операция Z(S)
```

## Порядок действий для работы с семафорами
1. Определить с какими семафорами (их индексы) из массива будем работать.
Пусть массив из 10 семафоров, а операции будет делать над 2мя
2. Подготовить массив `sops` из 2 элементов `struct sembuf`.
3. Заполнить элементы массива `sops` кодами нужной операции
4. Выполнить системный вызов semop

5. semop возвращается, только после выполнения всех операций
   - порядок не определён
   - если одни операции блокирующие, то неблокирующие могут ожидать, а могут выполнится до

## Примеры работы с семафорами

```c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int main(int argc, char* argv[], char* envp[]) {
    key_t k = ftok(“mylabel”, 0); if( k < 0 ){ … }
    int semid = semget(k, 1, IPC_CREAT|0664); if( semid < 0 ){ … }

    struct sembuf sops[1];
    sops[0].sem_num = 0;
    sops[0].sem_op = +1;
    sops[0].sem_flg = 0;
    if( semop(semid, sops, 1) < 0 ){ … }
        // …
    return 0;
}
```

```c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int main(int argc, char* argv[], char* envp[]){
    key_t k = ftok(“mylabel”, 0); if( k < 0 ){ … }
    int semid = semget(k, 1, IPC_CREAT|0664); if( semid < 0 ){ … }
    struct sembuf opAs0 = { .sem_num = 0, .sem_op = +1, .sem_flg = 0 }, opDs0 = { 0, -1, 0 };
    if( semop(semid, &opAs0, 1) < 0 ){ … }
    if( semop(semid, &opDs0, 1) < 0 ){ … }
    // …
    return 0;
}
```
