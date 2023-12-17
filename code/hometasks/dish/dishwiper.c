#include "utils.h"

int wipeSemaphore(struct dish* config, int table_max){

    FILE *f;
    char * type = calloc(100, sizeof(char));
    // first second - 0 - table is empty
    struct sembuf op = { .sem_num = 0, .sem_op = -1, .sem_flg = 0 };
    // second semaphore: 1 - wipper finished and washer work,
    struct sembuf op1 = { .sem_num = 1, .sem_op = +1, .sem_flg = 0 };

    int semid = semget(ftok("/dev/null", 'a'), 1, IPC_CREAT|0664);
    if( semid < 0 ) return -1;

    int count = 0;

    while (semctl(semid, 1, GETVAL) != 2){
        if (semctl(semid, 1, GETVAL) == 0) {

            f = fopen("./dish/washed_dishes.txt", "r");
            while(fscanf(f, "%s", type) == 1){
                action(config, type, 1);
                count += 1;
            }
            fclose(f);

            // delete info in file
            f = fopen("./dish/washed_dishes.txt", "w");
            fclose(f);

            // subtract the number of dishes washed
            for (int i=0; i < count; i++){
                if( semop(semid, &op, 1) < 0 ) return -1;
            }
            // set the semaphore to the finish position for the wiper
            if( semop(semid, &op1, 1) < 0 ) return -1;
            count = 0;
        }
    }
    free(type);
    return 0;
}

int wipePipe(struct dish* config, int table_max){

    char * type = calloc(100, sizeof(char));
    strcpy(type, "");

    int fd = open("./dish/dish_label.fifo",  O_RDONLY);
    if(fd < 0) return -1;

    int fd_table = open("./dish/table_label.fifo", O_WRONLY);
    if(fd_table < 0) return -1;

    int flag = 0;

    while (strcmp(type, ".") != 0){

        // wait for dirty dishes
        while (!read(fd, type, 100) || strcmp(type, "") == 0) {
            continue;
        }

        // check dishwasher hasn't dishes
        if (strcmp(type, ".") == 0) break;

        action(config, type, 1);

        // table has 1 free place
        write(fd_table, "1", sizeof("1"));
        memset(type, 0, strlen(type));
    }

    // finish work
    write(fd_table, "2", sizeof("2"));

    close(fd_table);
    close(fd);
    free(type);
    return 0;
}

int wipeMessage(struct dish* config, int table_max){

    key_t k = ftok("./dish/dish_label.fifo", 0); if( k < 0 ) return -1;
    int msqid = msgget(k, IPC_CREAT|0664); if( msqid < 0 ) return -1;

    struct msg message; 
    // message.ability = 1;

    while (1){

        // wait for dirty dishes
        while (msgrcv(msqid, &message, sizeof(message) - sizeof(message.mtype), 1, 0) < 0 ) {
            continue;
        }
        // check dishwasher hasn't dishes
        if (message.ability == -1) break;

        action(config, message.type, 1);

        // table has 1 free place
        message.mtype = 2; strcpy(message.type, ""); message.ability = -1;
        if( msgsnd(msqid, &message, sizeof(message)-sizeof(message.mtype), 0) < 0 ) return -1;
    }

    return 0;
}

int wipeSharedMemory(struct dish* config, int table_max){

    int index = 0;
    int count = 0;
    char * type = calloc(100, sizeof(char));

    key_t kflag = ftok("./dish/label.fifo", 0); if( kflag < 0 ) return -1;
    int shmidflag = shmget(kflag, table_max * 100 *(sizeof(char)), IPC_CREAT | 0664);
    if( shmidflag < 0 ) return -1;    
    
    key_t k = ftok("./dish/dish_label.fifo", 0); if( k < 0 ) return -1;
    int shmid = shmget(k, 100 * table_max * sizeof(char), IPC_CREAT | 0664);
    if( shmid < 0 ) return -1;

    key_t kjob = ftok("./dish/dishwasher_config.txt", 0); if( k < 0 ) return -1;
    int shmidjob = shmget(kjob, (sizeof(int)), IPC_CREAT | 0664);
    if( shmidjob < 0 ) return -1;

    // count of dishes on the table
    int *job = (int*)shmat(shmidjob, NULL, 0);

    // flag for statement of dishwasher
    char *array = (char *)shmat(shmid, NULL, 0);

    // table
    int * flagjob = (int*)shmat(shmidflag, NULL, 0);

    char * emptystr = "";

    // while washer works
    while ((*flagjob)){
        while ((*job) > 0){
            char * g = array + index * 100;
            getStr(&type, &g);
            if (strcmp(type, "") != 0){
                action(config, type, 1);
                (*job) -= 1;
                getStr(&g, &emptystr);
            }
            index = (index + 1) % table_max;
        }
    }

    free(type);
    if(shmdt(job) < 0) return -1;
    if(shmdt(array) < 0) return -1;
    if(shmdt(flagjob) < 0) return -1;
    return 0;
}

int wipeSocket(struct dish* config, int table_max){
    
    // create socket
    int sockd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockd < 0) return -1;

    char * type = calloc(100, sizeof(char));

    struct sockaddr_in addrsrv = {};
    addrsrv.sin_family = AF_INET;
    addrsrv.sin_addr.s_addr = inet_addr("127.0.0.1");
    addrsrv.sin_port = htons(1616);    

    // connect to server
    if (connect(sockd, (struct sockaddr *)&addrsrv, sizeof(addrsrv)) < 0) {
        close(sockd);
        return -1;
    }

    while (strcmp(type, ".") != 0){
        // works
        while (read(sockd, type, 100) && strcmp(type, ".") != 0){
            action(config, type, 1);
            write(sockd, "1", sizeof("1"));
        }
    }
    // gives a sign that he has finished work
    write(sockd, "2", sizeof("2"));

    close(sockd);
    free(type);
    return 0;
}

void dialoge(struct dish* config, int key, int table_max){
    switch (key) {
    case 1: // through a file, using semaphores for synchronization
        // this implementation requires the washer to run first
        wipeSemaphore(config, table_max);
        break; 
    case 2: // using Pipe
        // wiper and washer need start together
        // do not use '.' chars in type's name
        wipePipe(config, table_max);
        break;
    case 3: // using Message
        wipeMessage(config, table_max);
        break;
    case 4: // using Shared Memory
        // washer need start first
        wipeSharedMemory(config, table_max);
        break;
    case 5: // using Socket
        // do not use '.' chars in type's name
        // client
        wipeSocket(config, table_max);
        break;
    default:
        break;
    }
    return;
}



int main(){
    int TABLE_LIMIT = atoi(getenv("TABLE_LIMIT"));
    struct dish* config = loadConfig(NCONFIG, "./dish/dishwiper_config.txt");
    
    dialoge(config, 5, TABLE_LIMIT);

    return 0;
}