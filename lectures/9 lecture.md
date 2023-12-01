# Иерархия процессов (группы/семьи и сеансы/кланы)

![Процессы и сеансы](/img/7.png)


## Иерархия процессов в ОС
- Все процессы связаны родственными отношениями и образуют `генеалогическое дерево` или лес из таких деревьев. Все эти деревья принято разделять на группы процессов, или семьи.
- Группа процессов включает в себя один или более процессов и существует, пока в группе присутствует хотя бы один процесс.
   - Процесс обязательно включен в какую-нибудь группу
   - При рождении процесс попадает в ту же группу, в которой находится его родитель
   - Процессы могут мигрировать из группы в группу
   - Многие системные вызовы могут быть применены ко всем процессам в некоторой группе.
   - `Лидер группы` – процесс с номером группы
- Группы процессов объединяются в `сеансы` («кланы семей»).
   - Понятие сеанса изначально было введено в UNIX для логического объединения групп процессов, созданных в результате каждого входа и последующей работы пользователя в системе.
   - С каждым сеансом может быть связан терминал, называемый `управляющим терминалом сеанса`, через который обычно и общаются процессы сеанса с пользователем.
   - Сеанс может иметь `0 или 1` управляющий терминал.
   - Терминал не может быть управляющим для нескольких сеансов.

## Управляющий терминал

- Управляющий терминал (если есть) обязательно приписывается к некоторой группе процессов в сеансе. Такая группа процессов называется `текущей группой процессов` для данного сеанса.
- Все процессы, входящие в текущую группу процессов, могут совершать операции ввода-вывода, используя управляющий терминал.
- Все остальные группы процессов сеанса называются фоновыми группами, а процессы, входящие в них – `фоновыми процессами`.
- При попытке ввода-вывода фонового процесса через управляющий терминал этот процесс получит сигналы, которые стандартно приводят к прекращению работы процесса.
- Передавать управляющий терминал от одной группы процессов к другой может только лидер сеанса.
- Процессы, входящие в текущую группу сеанса, могут получать сигналы, инициируемые нажатием определенных клавиш на терминале
- SIGINT при нажатии клавиш <ctrl> и <c>
- SIGQUIT при нажатии клавиш <ctrl> и <4>.
- Стандартная реакция на эти сигналы – завершение процесса (с образованием core файла для сигнала SIGQUIT ).

## Лидер сеанса

- При завершении работы лидера сеанса все процессы из текущей группы сеанса получают сигнал SIGHUP (hang up — отбой, прерывание линии).
- SIGHUP по умолчанию приводит к завершению процесса.
- После завершения лидера сеанса в нормальной ситуации работу продолжат только фоновые процессы.

## Номер группы процессов (вариант 1)

### вариант 1

```c
#include <sys/types.h>
#include <unistd.h>
// Системный вызов возвращает идентификатор группы процессов для процесса, задаваемого идентификатором.
pid_t getpgid(pid_t pid); // Возвращаемое значение: pgid > 0 – идентификатор группы / -1 – ошибка, расшифровка в errno
// pid – идентификатор процесса, номер группы которого требуется узнать допустим только pid себя самого или процесса из своего сеанса
```

### вариант 2

```c
#include <sys/types.h>
#include <unistd.h>
// Системный вызов возвращает идентификатор группы процессов, к которому относится текущий процесс.
pid_t getpgrp(void); // Возвращаемое значение: pgid > 0 – идентификатор группы / -1 – ошибка, расшифровка в errno
```

## Изменение группы

### вариант 1

```c
#include <sys/types.h>
#include <unistd.h>
// Системный вызов setpgid служит для перевода процесса из одной группы процессов в другую, а также для создания новой группы процессов.
int setpgid(pid_t pid, pid_t pgid); // Возвращаемое значение: 0 – успех; -1 – ошибка (расшифровка в errno)
// pid – идентификатором процесса, который нужно перевести в другую группу; только сам, либо прямой ребёнок
// pgid – идентификатором группы процессов, в которую предстоит перевести этот процесс.
// Если pid == pgid создаётся новая группа первоначально из одного процесса.
```

### вариант 2

```c
#include <sys/types.h>
#include <unistd.h>
// Системный вызов setpgrp создаёт новую группу процессов и переводит в неё текущий процесс, который становится лидером группы.
int setpgrp(void); // Возвращаемое значение: 0 – успех; -1 – ошибка (расшифровка в errno)
```

## Номер сеанса

```c
#include <sys/types.h>
#include <unistd.h>
// Возвращает идентификатор сеанса для указанного процесса.
pid_t getsid(pid_t pid); // Возвращаемое значение: sid > 0 – номер сеанса / -1 – ошибка, расшифровка в errno
// pid – идентификатор процесса, номер сеанса которого требуется узнать если pid==0, то предполагается текущий процесс.
```

```c
#include <sys/types.h>
#include <unistd.h>
// Cоздает: 
//         - новую группу, состоящую только из процесса, который его выполнил (он становится лидером новой группы )
//         - новый сеанс, идентификатор которого совпадает с идентификатором процесса, сделавшего вызов. Такой процесс называется лидером сеанса.
// Этот системный вызов может применять только процесс, не являющийся лидером группы.
int setsid(void); // Возвращаемое значение: 0 – успех, -1 – ошибка (расшифровка в errno)
```

# Механизм сигналов в ОС (управление процессами)

## Управление процессами
- Процессы независимы друг от друга и могут не общаться с «внешним» миром.
- Как управлять такими «замкнувшимися» процессами? Например, как уведомить об исключительной ситуации? Или просто приказать/попросить завершиться?
- …
- Сгенерировать внутри процесса псевдо прерывание! и тем самым заставить его приостановить текущую работу и обработать прерывание
- В Unix этот механизм называют «послать процессу сигнал»

## Идея классического механизма прерываний (interrupt)
- Процессор, после оправки запроса ввода-вывода к устройству, ожидает результата.
1. Процессор периодически сам опрашивает регистр состояния контроллера устройства на наличие бита занятости – `polling`
2. У процессора имеется специальный вход, на который устройство по готовности выставляет сигнал запроса прерывания interrupt request (само или через контроллер прерываний). При этом
   - Процессор по окончании текущей команды НЕ переходит к следующей
   - Сохраняет состояние
   - Выполняет команды по фиксированным адресам для обработки прерывания.
   - После обработки восстанавливает предыдущее состояние
   - Выполнение нормальных команд продолжается.

## Механизм исключений (exception)
- Процессор при возникновении в текущей команде исключительной ситуации (деление на ноль, защита памяти, неверный адрес, …) поступает так же как при прерывании:
   - По окончании текущей команды НЕ переходит к следующей.
   - Сохраняет состояние, которое было до выполнения ошибочной команды.
   - Выполняет команды по фиксированным адресам для обработки исключения.
   - После обработки исключения может восстановить предыдущее состояние и продолжить или не возвращается.

## Механизм программных прерываний (trap, software interrupt)
- При «алгоритмической» необходимости прервать текущую последовательность команд процессора (например, переключить режим выполнения с пользовательского на ядерный в системном вызове) используют механизм аналогичный прерываниям:
- Процессор по окончании текущей команды НЕ переходит к следующей
- Сохраняет состояние.
- Выполняет специальные команды по фиксированным адресам.
- После обработки может восстановить выполнение отложенной команды.

## Обработка прерываний и исключений – идея сигналов

- Аппаратные прерывания от устройств ввода-вывода производит сама операционная система, не доверяя работу с системными ресурсами процессам пользователя.
- Исключительные ситуации и некоторые программные прерывания возможно обрабатывать в пользовательском процессе через механизм сигналов.
- `Сигнал` — асинхронное уведомление пользовательского процесса (НЕ процессора) о каком-либо событии, один из основных способов взаимодействия между процессами.
- Получение процессом сигнала выглядит как возникновение прерывания. Процесс прекращает регулярное исполнение, и управление передается обработчику сигнала. По окончании обработки сигнала процесс может возобновить регулярное исполнение.

## Типы сигналов
- Типы сигналов и способы возникновения регламентированы в ОС
- Сигналы задаются номерами 1 – 31 и именуются как SIG* (SIGTERM, SIGKILL, …)
- Процесс может получить сигнал от:
   - hardware (при возникновении исключительной ситуации);
   - терминала (при нажатии определенной комбинации клавиш);
_______________________________________________________________________
   - другого процесса, выполнившего системный вызов передачи сигнала; |
   - операционной системы (при наступлении некоторых событий);        | - сигнальное средство связи между процессами
   - системы управления заданиями.                                    |
______________________________________________________________________|


## Некоторые сигналы POSIX

![Некоторые сигналы POSIX 1/2](/img/8.png)
![Некоторые сигналы POSIX 2/2](/img/9.png)

## Реакция процесса на сигнал

- Принудительно проигнорировать сигнал
- Произвести обработку по умолчанию:
   - проигнорировать
   - остановить процесс (перевести в состояние ожидания до получения другого специального сигнала)
   - завершить работу с образованием core файла или без него.
- Выполнить обработку сигнала, заданную пользователем:
   - пользовательская реакция на сигнал устанавливается специальным системным вызовом;
   - реакцию на некоторые сигналы запрещено изменять, они всегда обрабатываются по умолчанию (например 9 – SIGKILL).
   - при fork() все установленные реакции наследуются.

## Отправка сигналов процессам

```bash
# Посылает сигнал одному или нескольким процессам.
$ kill [-signal] pid
# -signal сигнал, который должен быть доставлен; задаётся в числовой или символьной форме, например -9 или - KILL. Если этот параметр опущен посылается сигнал SIGTERM.
# pid — процесс или процессы, которым будут доставляться сигналы. n > 0 — сигнал конкретному процессу 0 — сигнал всем процессам текущей группы в сеансе -1 — сигнал всем процессам с pid > 1 (т.е. почти ВСЕМ процессам) -n — сигнал всем процессам группы номер n.
$ kill -l
# -l выводится список сигналов, существующих в системе, никакой сигнал не посылается

# Без прав суперпользователя послать сигнал можно только процессу, у которого эффективный UID совпадает с ID текущего пользователя.
```

```c
#include <sys/types.h>
#include <signal.h>
// Передаёт сигнал одному или нескольким процессам в рамках полномочий пользователя.
int kill(pid_t pid, // pid – процесс или процессы, которым будут доставляться сигналы. > 0 – сигнал конкретному процессу / 0 – сигнал всем процессам текущей группы / 1 - сигнал всем процессам с идентификатором текущего пользователя / n < 0 - сигнал всем процессам группы номер abs(n).
    int signal); // sig – номер сигнала если sig == 0
```

## Установка обработчика сигнала

```c
#include <signal.h>
// Изменение реакции процесса на какой-либо сигнал.
void (* signal(int sig, // sig – номер сигнала, обработка которого меняется
void (*handler)(int)) // handler — новый способ обработки сигнала – указатель на пользовательскую функцию – SIG_DFL восстановления реакции процесса на этот сигнал по умолчанию – SIG_IGN игнорировать поступившие сигналы
)(int); // Возвращаемое значение: указатель на предыдущую функцию обработчика

typedef void (*handler_t)(int);
handler_t signal(int sig, handler_t h);
```

## Сигнал от процесса-потомка
- SIGCHLD. Посылается ядром автоматически при изменении статуса дочернего процесса (завершён, приостановлен или возобновлен).
- Типичная обработка SIGCHLD предполагает считывание кода завершения дочернего процесса с помощью системного вызова waitpid()
- Если родительский процесс не считывает информацию о завершившемся дочернем, то тот становится «зомби» (zombie или defunct process)
- В стандарте POSIX.1-2001 разрешается на сигнал SIGCHLD установить реакцию SIG_IGN, которая считает код завершения дочернего процесса и «выбросит» его, так что зомби не образуются

## Считывание информации о потомке

```c
#include <sys/types.h>
#include <sys/wait.h>
// Ожидание изменения состояния процесса-потомка И получение информации о таком потомке.
// Сменой состояния считается: прекращение работы потомка, останов потомка по сигналу, продолжение работы потомка по сигналу.
pid_t waitpid(pid_t pid, // pid — идентификатор процесса-потомка –n (n>1) — любой потомок из группы процессов с grpID = n / --1 — любой потомок / 0 — любой потомок из группы процессов с grpID = ppid, где ppid — номер «родителя», вызвавшего процесса / n > 0 — конкретный потомок с указанным ID
    int* status, // status — указатель на переменную, куда помещается информация о состоянии процесса-потомка в кодированном виде; для извлечения информации применяются спец. макросы (см. далее); может быть NULL, если информация не нужна
    int options // options — определяет поведение системного вызова, объединение битовым ИЛИ (“|”) констант: WNOHANG — не ждать изменения состояния, только считывать код завершения
); // Возвращаемое значение: n>0 — ID процесса-потомка, о котором получена информация / 0 — существуют незавершившиеся потомки, а ожидание запрещено опцией WNOHANG / -1 — ошибка + конкретная причина в errno, например, не существует процессов-потомков, соответствующих указанному PID

pid_t wait(int* status);

```

## Расшифровка кода завершения


```c
#include <sys/wait.h>

WIFEXITED(status) // возвращает TRUE, если потомок нормально завершился, то есть вызвал системный вызов exit(rc) или вернулся из функции main()
WEXITSTATUS(status) // возвращает код завершения retcode потомка, который тот указал в exit(retcode) или сделал `return retcode` из функции main()

WIFSIGNALED(status) // возвращает TRUE, если потомок завершился из-за сигнала
WTERMSIG(status) // возвращает номер сигнала, который привел к завершению потомка
WCOREDUMP(status) // возвращает TRUE, если потомок создал дамп памяти (core dump)
```