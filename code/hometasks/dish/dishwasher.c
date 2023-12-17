#include "utils.h"

int washSemaphore(struct dish* config, int table_max){

    // first semaphore use for flag the access for table, second flag the finish job
    int semid = semget(ftok("/dev/null", 'a'), 2, IPC_CREAT|0664);
    if( semid < 0 ) return -1;

    int count = 0;
    char * type = calloc(100, sizeof(char));

    FILE *f = fopen("./dish/dirty_dishes.txt", "r"); if (f == NULL) return -1;

    // delete info in file
    FILE *ftable = fopen("./dish/washed_dishes.txt", "w");  if (ftable == NULL) return -1; 
    fclose(ftable);
     
    // first second - count of dishes
    struct sembuf op0 = { .sem_num = 0, .sem_op = 0, .sem_flg = 0 }; 
    struct sembuf op1 = { .sem_num = 0, .sem_op = +1, .sem_flg = 0 };
    // second semaphore: 0 - washer wait for wiper, 1 - washer work, 2- washer finished
    struct sembuf op2 = { .sem_num = 1, .sem_op = +1, .sem_flg = 0 };
    struct sembuf op3 = { .sem_num = 1, .sem_op = -1, .sem_flg = 0 };

    if( semop(semid, &op2, 1) < 0 ) return -1; // start wash

    while (fscanf(f, "%s %i", type, &count) == 2){
        for (int i = 0; i < count; i++){

            int semvalue = semctl(semid, 0, GETVAL);
            if (semvalue >= table_max) { // table is full
                if( semop(semid, &op3, 1) < 0 ) return -1; // sign for wipper
                if( semop(semid, &op0, 1) < 0 ) return -1; // wait for free table
            }

            if( semop(semid, &op1, 1) < 0 ) return -1; // wash one dish
            action(config, type, 0);
            ftable = fopen("./dish/washed_dishes.txt", "a+");
            fprintf(ftable, "%s\n", type); // write to file about type of washed dish
            fclose(ftable);
        }
    }
    
    // in case there are still plates left
    if( semop(semid, &op3, 1) < 0 ) return -1; // sign for wipper
    if( semop(semid, &op0, 1) < 0 ) return -1; // wait for free table

    if( semop(semid, &op2, 1) < 0 ) return -1;  // finish wash and sign for wiper finish
    fclose(f);
    free(type);
    return 0;
}
 
int washPipe(struct dish* config, int table_max){

    int count = 0;
    char * type = calloc(100, sizeof(char));

    FILE *f = fopen("./dish/dirty_dishes.txt", "r");

    // Pipe for dishes
    int fd = open("./dish/dish_label.fifo", O_WRONLY);
    if(fd < 0) return -1;

    // Pipe for statement
    int fd_table = open("./dish/table_label.fifo", O_RDONLY);
    if(fd_table < 0) return -1;

    // 1 - table has free place
    // 0 - table hasn't free place
    char flag = '1';
    int count_dish = 0;

    char * symbol_dot = calloc(100, sizeof(char));
    strcpy(symbol_dot, ".");

    while (fscanf(f, "%s %i", type, &count) == 2){
        for (int i = 0; i < count; i++){
            if (count_dish >= table_max) flag = '0';
                        
            if (flag == '1'){
                action(config, type, 0);
                count_dish += 1; 
                write(fd, type, 100);
            }
            else{
                // wait for free place
                while ( !read(fd_table, &flag, 1) || flag == '0') {
                    continue;
                }
                count_dish -= 1;
                i--;
            }
            
        }  
        memset(type, 0, strlen(type));
    }
    // gives a sign that he has finished work
    write(fd, symbol_dot, 100);
    // wait for dishwiper's finish
    while ( !read(fd_table, &flag, 1) || flag != '2') {
        continue;
    }
    close(fd_table);
    close(fd);
    fclose(f);
    free(type);
    return 0;
}

int washMessage(struct dish* config, int table_max){

    FILE *f = fopen("./dish/dirty_dishes.txt", "r");

    key_t k = ftok("./dish/dish_label.fifo", 0); if( k < 0 ) return -1;
    int msqid = msgget(k, IPC_CREAT|0664); if( msqid < 0 ) return -1;

    // type: 1 - about dish
    // type: 2 - about table
    struct msg message; 
    int global_count = 0; // count dishes in the table

    int count = 0;
    char * type = calloc(100, sizeof(char));

    while (fscanf(f, "%s %i", type, &count) == 2){
        for (int i = 0; i < count; i++){

            if (global_count < table_max){
                message.mtype = 1; strcpy(message.type, type); message.ability = 1;
                if( msgsnd(msqid, &message, sizeof(message)-sizeof(message.mtype), 0) < 0 ) return -1;
                action(config, type, 0);
                global_count += 1;
            }
            else {
                // wait for free place
                while ( msgrcv(msqid, &message, sizeof(message) - sizeof(message.mtype), 2, 0) < 0 ){
                    printf("fdsfsdf");
                    continue;
                }
                global_count -= 1;
                i--;
            }
        }
    }

    // finish work
    message.mtype = 1; strcpy(message.type, type); message.ability = -1;
    if( msgsnd(msqid, &message, sizeof(message)-sizeof(message.mtype), 0) < 0 ) return -1;
            
    fclose(f);
    return 0;
}

int washSharedMemory(struct dish* config, int table_max){

    FILE *f = fopen("./dish/dirty_dishes.txt", "r"); if (f == NULL) return -1;

    int count = 0;
    char * type = calloc(100, sizeof(char));

    key_t kflag = ftok("./dish/label.fifo", 0); if( kflag < 0 ) return -1;
    int shmidflag = shmget(kflag, table_max * 100 *(sizeof(char)), IPC_CREAT | 0664);
    if( shmidflag < 0 ) return -1;    

    key_t k = ftok("./dish/dish_label.fifo", 0); if( k < 0 ) return -1;
    int shmid = shmget(k, table_max * 100 *(sizeof(char)), IPC_CREAT | 0664);
    if( shmid < 0 ) return -1;

    key_t kjob = ftok("./dish/dishwasher_config.txt", 0); if( kjob < 0 ) return -1;
    int shmidjob = shmget(kjob, sizeof(int), IPC_CREAT | 0664);
    if( shmidjob < 0 ) return -1;

    // count of dishes on the table
    int *job = (int*)shmat(shmidjob, NULL, 0);
    (*job) = 0; 

    // flag for statement of dishwasher
    int * flagjob = (int*)shmat(shmidflag, NULL, 0);
    (*flagjob) = 1; // i start work

    // table
    char *array = (char *)shmat(shmid, NULL, 0);

    for (int i = 0; i < table_max * 100; i++) {
        array[i] = '\0';
    }

    int index = 0;
    while (fscanf(f, "%s %i", type, &count) == 2){
        for (int i = 0; i < count; i++){
            
            while ((*job) > table_max - 1) continue;

            for (int i = 0; i < 100; i++){
                array[index * 100 + i] = type[i];
            }

            index = (index + 1) % table_max;
            action(config, type, 0);
            (*job) += 1;
        }
    }

    // wait for wiper's finish
    while ((*job) > 0) continue;

    // i finish
    (*flagjob) = 0; 

    fclose(f);
    free(type);
    if(shmdt(job) < 0) return -1;
    if(shmdt(flagjob) < 0) return -1;
    if(shmdt(array) < 0) return -1; 
    return 0;
}

int washSocket(struct dish* config, int table_max){

    int count = 0;
    char * type = calloc(100, sizeof(char));
    int global_count = 0;
    char flag = '0';

    // create socket
    int sockd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockd < 0) return -1;

    struct sockaddr_in addrsrv = {};
    addrsrv.sin_family = AF_INET;
    addrsrv.sin_addr.s_addr = INADDR_ANY;
    addrsrv.sin_port = htons(1616);    

    // connect socket to full adress
    if (bind(sockd, (struct sockaddr *)&addrsrv, sizeof(addrsrv)) < 0) return -1;
    listen(sockd, 1);

    FILE *f = fopen("./dish/dirty_dishes.txt", "r");

    int size = sizeof(addrsrv);
    // get connect from client
    int mgsock = accept(sockd, (struct sockaddr *)&addrsrv, &size); 

    while (fscanf(f, "%s %i", type, &count) == 2){
        for (int i = 0; i < count; i++){      

            if (global_count < table_max) {
                action(config, type, 0);
                write(mgsock, type, 100);
                global_count += 1;
            } else {
                // wait for free place
                while (!read(mgsock, &flag, 1) || flag != '1') {
                    continue;
                }
                global_count--;
                i--;
            }
        }
    }
    // gives a sign that he has finished work
    char * symbol_dot = calloc(100, sizeof(char));
    strcpy(symbol_dot, ".");
    write(mgsock, symbol_dot, 100);

    // wait for wiper's finish
    while (!read(mgsock, &flag, 1) || flag != '2') {
        continue;
    }

    free(type);
    fclose(f);
    return 0;
}

void dialogue(struct dish* config, int key, int table_max){
    switch (key){
        case 1: // through a file, using semaphores for synchronization
            // this implementation requires the washer to run first
            washSemaphore(config, table_max);
            break;
        case 2: // using Pipe
            // wiper and washer need start together
            // do not use '.' chars in type's name
            washPipe(config, table_max);
            break;
        case 3: // using Message
            washMessage(config, table_max);
            break;
        case 4: // using Shared Memory
            // washer need start first
            washSharedMemory(config, table_max);
            break;
        case 5: // using Socket
            // do not use '.' chars in type's name
            // server
            washSocket(config, table_max);
            break;
        default:
            break;
    }
    return;            
}


int main(){
    int TABLE_LIMIT = atoi(getenv("TABLE_LIMIT"));
    struct dish* config = loadConfig(NCONFIG, "./dish/dishwasher_config.txt");

    int semid = semget(ftok("/dev/null", 'a'), 1, IPC_CREAT|0664);
    if( semid < 0 ) return -1;
    semctl(semid, 0, IPC_RMID); // destroy last semaphore 


    //int fd = open("./dish/dish_label.fifo",  O_RDONLY | O_TRUNC);
    //close(fd);
    //fd = open("./dish/table_label.fifo",  O_RDONLY | O_TRUNC); // destroy last dishes
    //close(fd);

    //key_t k = ftok("./dish/dish_label.fifo", 0);
    //int msqid = msgget(k, IPC_CREAT|0664);
    //msgctl(msqid, IPC_RMID, NULL);
    dialogue(config, 5, TABLE_LIMIT);
    return 0;
}