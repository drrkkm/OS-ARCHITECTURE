## Очереди сообщений

## System V IPC
### Средства связи System V IPC:
- совместно используемая (разделяемая) память – shared memory;
- семафоры – semaphores;
- очереди сообщений – message queues.
  
## Модель сообщений для взаимодействия процессов
1. На передаваемую информацию накладывается определённая структура: (известно где заканчивается одна порция данных, и начинается другая).
2. Возможно использовать одну линию связи для двунаправленной передачи между несколькими процессами.
3. Возможно использовать для синхронизации процессов (встроенные механизмы блокировки и взаимоисключения).

## Модель сообщений для взаимодействия процессов

![Модель сообщений для взаимодействия процессов](/img/2.png)

## Очереди сообщений System V IPC: message queues
1. Размещаются в адресном пространстве ядра
   - возможно взаимодействие процессов, работающих не одновременно
   - размер ограничен, задаётся администратором
2. Пространство имён – множество значений ключа, генерируемого ftok()
3. Реализованы в виде однонаправленных списков. Элемент списка –
сообщение.
4. Сообщения имеют атрибут «тип сообщения», размер не фиксирован.
5. Операции помещения (send) и получения (receive) сообщения в/из очереди
   - атомарные, взаимоисключающие
   - блокирующие при пустой или переполненной очереди.

## Очереди сообщений System V IPC: message queues
- Способы выборки сообщения из очереди (receive)
  1. В порядке FIFO независимо от типа сообщения
  2. В порядке FIFO для сообщений конкретного типа
  3. В порядке FIFO для сообщений с типом не превышающим заданный

![Пример выбора сообщений из очереди](/img/3.png)


## Консольная команда для очередей сообщений SysV IPC
```bash
$ ipcs –t|-p|-c|-l|-u … # Информация об объектах SysV IPC
# -t – показать время последней операции
# -p – PID процесса создателя и совершившего  последнюю операцию
# -l – системные лимиты для объектов SysV IPC, в частности размер очереди сообщений
# -u – суммарная информация (summary)
```


## Получение доступа / создание очереди сообщений SysV IPC
```C
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
// Получает доступ или создаёт очередь сообщений
int msgget(key_t key, int msgflg); //Возвращаемое значение >0 – дескриптор SysV IPC / -1 – ошибка, код ошибки в errno
// key – уникальное имя (ключ), созданное ftok() или IPC_PRIVATE
// msgflg – флаги, комбинируются с помощью | «битовое или»
// • IPC_CREAT - создать если не существует
// • IPC_EXCL - вместе с предыдущим, создавать эксклюзивно,  ошибка если сущуствует
// • Права доступа
// • 0400 – только чтение для владельца
// • 0200 – только запись для владельца
// • 0100 – только исполнение для владельца
// • 0040 – только чтение для группы
```

## Отправка сообщения SysV IPC

```c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
// Помещает сообщение в очередь
// Возвращаемое значение 0 – успех / -1 – ошибка, код ошибки в errno
int msgsnd(int msqid, // msqid – идентификатор SysV IPC очереди
    struct msgbuf* msgp, // msgp – указатель на сообщение, данные по шаблону `struct msgbuf`
    size_t msgsz, // msgsz – размер полезной части сообщения, сообщение МИНУС атрибут типа, может =0
    int msgflg // msgflg – флаг 0 – по-умолчанию / IPC_NOWAIT – не ждать освобождения очереди
);
```

## Шаблон сообщения SysV IPC

```c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
//Шаблон структуры сообщения
struct msgbuf {
    long mtype; // mtype –тип сообщения обязательный атрибут, > 0
    char mtext[1]; //mtext – заготовка полезной части сообщения в Linux размер полезной части <= 4080
};
```

## Примеры сообщений SysV IPC
```c
struct mymsg1 {
long mtype; // mtype – обязательное поле >0
char text[512]; // все данные после mtype – полезная часть
sizeof( struct mymsg ) – sizeof(long)
};
// размер полезной части sizeof( struct mymsg ) – sizeof(long)
struct mymsg2 {
    long mtype;
    struct {
        int n;
        float x;
    } info;
};
```

## Пример отправки сообщения

```c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

struct mymsg {
    long mtype;
    char text[512];
};

int main(int argc, char* argv[], char* envp[]) {
    key_t k = ftok(“mylabel”, 0); if( k < 0 ){ … }
    int msqid = msgget(k, IPC_CREAT|0664); if( msqid < 0 ){ … } 
    struct mymsg msg; msg.mtype = 1; strcpy(msg.text, “hello”);
    if( msgsnd(msqid, &msg, sizeof(msg)-sizeof(msg.mtype), 0) < 0 ){ … }
    // …
    return 0;
}
```

## Получение сообщения SysV IPC

```c

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
// msgtype – способ выборки сообщения
int msgrcv(int msqid,
    struct msgbuf* msgp,
    size_t msgsz,
    long msgtype,
    int msgflg
);

// 0 – в порядке FIFO независимо от типа
// +n – в порядке FIFO только сообщения с типом n
// -n – в порядке FIFO для сообщений с типом <=n: сначала друг за другом все сообщения с минимальным типом, потом все с большим типом, вплоть до n Первым выбирается сообщение с минимальным типом, не превышающим некоторого заданного значения, пришедшее раньше других сообщений с тем же типом.
```
## Пример получения сообщения

```c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct mymsgbuf { long mtype; char bytes[512]; } buf;
struct mymsg1 { long mtype;  struct { int n; float x; } info; };

int main(int argc, char* argv[], char* envp[]) {
    key_t k = ftok(“mylabel”, 0); if( k < 0 ){ … }
    int msqid = msgget(k, IPC_CREAT|0664); if( msqid < 0 ){ … }

    if( msgrcv(msqid, &buf, sizeof(buf.bytes), 0/*type*/, 0) < 0 ){ … }
    if( buf.mtype == 1 ) { 
        struct mymsg1* msg = (struct mymsg1*)&buf;
        // msg->info.n
    }
return 0;
}
```

## Удаление очереди сообщений SysV IPC

```c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
// Получает информацию об очереди сообщений, изменяет атрибуты, удаляет
// Возвращаемое значение 0 – успех / -1 – ошибка, код ошибки в errno
int msgctl(int msqid, // msqid – идентификатор SysV IPC очереди
    int cmd, // cmd – код команды IPC_RMID – для удаления
    struct msqid_ds* buf); // buf – выходные/выходные данные для команд NULL в случае команды удаления
```

## Консольная команда для удаления очереди сообщений SysV IPC

```bash 
$ ipcrm -q|-Q # Удаление очереди сообщений SysV IPC
# -q id – удалить очередь по идентификатору
# -Q key – удалить очередь по ключу
```

## Мультиплексирование информации

1. Возможность одновременного обмена информацией с несколькими партнёрами через одно средство связи
2. Очередь сообщений SysV IPC возможно использовать для мультиплексирования
   - каждому процессу приписать свой собственный тип сообщений: mtype == pid
   - один процесс может получать сообщения от множества других процессов через одну очередь сообщений
   - процесс может отправлять ответы через ту же очередь сообщений
3. Мультиплексирование часто применяется в модели взаимодействия клиент - сервер

## Модель клиент – сервер
1. Взаимодействующие процессы неравноправны
2. Сервер работает постоянно, а клиенты могут работать эпизодически.
3. Сервер ждет запроса от клиентов, инициатором же взаимодействия является клиент.
4. Клиент обращается к одному серверу за раз, в то время как к серверу могут одновременно поступать запросы от нескольких клиентов.
5. Клиент должен знать, как обратиться к серверу (например, какого типа сообщения он воспринимает) перед началом организации запроса к серверу, в то время как сервер может получить недостающую информацию о клиенте из пришедшего запроса.