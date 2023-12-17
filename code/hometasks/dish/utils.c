#include "utils.h"

void action(struct dish* config, char * type, int key){ // key = 0 - wash, key = 1 - wipe
    int time = 0;

    for (int i = 0; i < NCONFIG; i++){
        if (strcmp(config[i].type, type) == 0){
            time = config[i].time;
            break;
        }
    }
    if (key) printf("I WIPED");
    else {
        printf("I WASHED");
    }
    printf(" THE DISH TYPE: %s. IT TOOK %i seconds.\n", type, time);
    sleep(time);
    return;
}


struct dish* loadConfig(int nconfig, char* path){
    struct dish* config = calloc(nconfig, sizeof(struct dish));

    FILE *f = fopen(path, "r");
    if (f == NULL) return config;
    int number = 0;
    int count = 0;
    
    while (count < nconfig){
        config[count].type = calloc(100, sizeof(char));
        if(fscanf(f, "%s %i", config[count].type, &(config[count].time)) == 2) {
            count += 1;
            continue;
        }
        else break;
    }
    return config;
}

void getStr(char ** type, char** array){
    for (int i = 0; i < 100; i++){
        (*type)[i] = (*array)[i];
    }
} 