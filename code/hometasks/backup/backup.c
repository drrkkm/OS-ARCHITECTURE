/*Для того чтобы не допустить потерю информации при сбое диска,
обычно используют резервное копирование файлов (backup). Простей-
шей формой backup'а является копирование файлов из одного каталога в
другой. Этот способ требует много времени и места на диске. Напишите
программу, осуществляющую более интеллектуальный подход. Про-
грамма должна брать из командной строки два параметра: имена исход-
ного каталога и каталога назначения. Она должна рекурсивно сканиро-
вать исходный каталог, делать копии всех файлов, для которых ранее не
делались копии или которые были изменены с момента последнего
backup'а, размещая их в соответствующих местах директории назначе-
ния. После копирования каждого файла должна вызываться команда
сжатия gzip. Это уменьшит требуемое дисковое пространство, а файл
будет переименована с добавлением расширения .gz. Все возникающие
ошибки (нет исходного каталога, файл не доступен для чтения и т.д.)
должны корректно обрабатываться с выдачей соответствующего сооб-
щения.

command line -> args = 2, argv = ['from_directory_name', 'to_directory_name']

recursive down jump and copy files, which wasn't copy yeat or was changed
*/
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>

char* catToNewPath(char* str1, char* str2){ // creare a new string (path) from two strings  

    char * newstr = calloc(strlen(str1) + strlen(str2) + 2, sizeof(char));
    strcpy(newstr, str1);
    strcat(newstr, "/");
    strcat(newstr, str2);

    return newstr;
}

char* catToNewString(char* str1, char* str2){ // concatenates two strings to new (another) string

    char * newstr = calloc(strlen(str1) + strlen(str2) + 1, sizeof(char));
    strcpy(newstr, str1);
    strcat(newstr, str2);

    return newstr;
}

char* changeExtension(char * file){ // change etxtension of file to .gz extension

    char * resStr = calloc(strlen(file) + 4, sizeof(char));
    
    strncpy (resStr, file, strlen(file));
    strcat(resStr, ".gz\0");

    return resStr;
}

char* unchangeExtension(char * file){ // delete .gz etxtension of file

    char * resStr = calloc(strlen(file) - 2, sizeof(char));
    strncpy (resStr, file, strlen(file) - 3);
    strcat(resStr, "\0");

    return resStr;
}

int copyFile(char* file, char* backupDirectoryFile) { // copy file from directory to backupFirectoryFile in backupDirectory

    FILE *fd = fopen(file, "r");
    if (fd == NULL) { 
        perror("fopen");
        return -1;
    }
    
    FILE *fbd = fopen(backupDirectoryFile, "w");
    if (fbd == NULL) { 
        perror("fopen");
        return -1;
    }

    int flag_return = 0;
    char * inside = calloc(1, sizeof(char)); 

    while (fread(inside, sizeof(char), 1, fd)) { // character-by-character copying
        if (ferror(fd)) {
            perror("fread");
            flag_return =  -1;
            break;
        }

        if (!fwrite(inside, sizeof(char), 1, fbd)){
            perror("fwrite");
            flag_return = -1;
            break;
        }

    }
    fclose(fd);
    fclose(fbd);
    free(inside);
    return flag_return;
}

int compressFile(char * pathToFile){

    char* filegz = changeExtension(pathToFile);
    gzFile fgz = gzopen(filegz, "w");

    if (!fgz) {
        perror("gzopen");
        return -1;
    } 

    FILE *f = fopen(pathToFile, "r");
    if (f == NULL) { 
        perror("fopen");
        return -1;
    }

    char * inside = calloc(1, sizeof(char)); 
    int flag_return = 0;

    while (fread(inside, sizeof(char), 1, f)) { // character-by-character copying
        if (ferror(f)) {
            perror("fread");
            flag_return =  -1;
            break;
        }

        if (!gzwrite(fgz, inside, 1)){
            perror("gzwrite");
            flag_return = -1;
            break;
        }

    }
    free(inside);
    free(filegz);

    fclose(f);
    gzclose(fgz);
    remove(pathToFile);

    return flag_return;
}

int archiveFile(char * pathToFile, int key) { // key = 0  - archive gzip /  key = 1 - decomprese gzip (gzip - d) / key = 2 - compress file handly

    char* command;
    if (key == 0) command = catToNewString("gzip ", pathToFile);
    else if (key == 1) command = catToNewString("gzip -d ", pathToFile);
    else if (key == 2) compressFile(pathToFile);

    if (key == 0 || key == 1) {
        if (system (command) < 0){
            perror("system");
            free(command);
            return -1;
        }
        free(command);
    }

    return 0;
}

void backupFile(char* name, char* directory, char* pathToBackupFile, int key){ // copy and archive file

    char* file = catToNewPath(directory, name); // backup path/file

    if (copyFile(file, pathToBackupFile) != 0) {
        free(file);
        return;
    }

    if (archiveFile(pathToBackupFile, key)) return;

    free(file);
    return;
}

int shasumOfFile(char* pathToFile, char ** shaFile){ // get shasum256 of file

    char * command = catToNewString("shasum -a 256 ", pathToFile); // shasum pathTobackupFile -> 1e2...

    FILE* f = popen(command, "r");
    if (f == NULL) {
        perror("popen");
        free(command);
        return -1;
    }

    *shaFile = calloc(65, sizeof(char));

    if (fread(*shaFile, sizeof(char), 64, f) != 64) {
        perror("fread");
        free(command);
        free(*shaFile);
        return -1;
    }

    //fclose(f);
    free(command);
    return 0;
}

int checkUpdatesFile(char* pathToFile, char* pathToBackupFileGz){ // comparte shasums

    char* shasumFile;
    char* shasumBackupFile;
    if (shasumOfFile(pathToFile, &shasumFile)) return -1; // get shasum of file

    if (archiveFile(pathToBackupFileGz, 1)) return -1; // dearchivacte file for compare

    char* pathToBackupFile = unchangeExtension(pathToBackupFileGz); // get pathToBackupFileGz without extation

    if (shasumOfFile(pathToBackupFile, &shasumBackupFile)) return -1;  // get shasum of backupFile

    archiveFile(pathToBackupFile, 0); // archive file

    int flag = 1; // shasums is different

    if (strcmp(shasumFile, shasumBackupFile) == 0) flag = 1; // if shasums are same

    free(pathToBackupFile);
    free(shasumBackupFile);
    free(shasumFile);
    return flag;
}

void checkFilesInDirectory(DIR *d, char* directory, char* backup_directory, int key) {

    struct dirent *files;

    while ((files = readdir(d))){
        if (files->d_type == DT_REG){
            char* pathToBackupFile = catToNewPath(backup_directory, files->d_name); // backup_directory/file
            char* pathToFileGz = changeExtension(pathToBackupFile); // backup_directory/file.gz
            FILE *bdf = fopen(pathToFileGz, "r");
            if (bdf == NULL) {
                backupFile(files->d_name, directory, pathToBackupFile, key);

            }
            else {

                char* file = catToNewPath(directory, files->d_name); // directory/file
                int flag = checkUpdatesFile(file, pathToFileGz);
                if (flag == 1) {

                    remove(pathToFileGz);
                    backupFile(files->d_name, directory, pathToBackupFile, key);
                }
                free(file);
                fclose(bdf);

            }
            free(pathToBackupFile);
            free(pathToFileGz);
        }
    }
    return;
}    
    
int downShift(char* directory, char* backupDirectory, int key){ // recursive down jump to directories
    
    DIR *d = opendir(directory);
    
    if (!d) {
        perror("opendir");
        return -1;
    }

    DIR *bd = opendir(backupDirectory);

    if (!bd) { // if there isn't a backupDirectory/anotherDirectory we create this directory
        mkdir(backupDirectory, 0777);
        bd = opendir(backupDirectory);
        if (!bd) {
            closedir(d);
            return -1;
        }
    }
    closedir(bd);

    checkFilesInDirectory(d, directory, backupDirectory, key); // check files for their state ( copy yet or not copy)
    rewinddir(d); // return pointer to begin of file

    struct dirent *dir;
    
    while ((dir = readdir(d)) != NULL) {

        if (dir->d_type == DT_DIR){
            if (strcmp(dir->d_name, ".") == 0) continue;
            if (strcmp(dir->d_name, "..") == 0) continue;

            char* newDirectory = catToNewPath(directory, dir->d_name); // -> directory/d->name
            char* newBackupDirectory = catToNewPath(backupDirectory, dir->d_name); // backupDirectory/d->name
            downShift(newDirectory, newBackupDirectory, key); // move to next recurcieve directory

            free(newDirectory);
            free(newBackupDirectory);
        }
    }
    closedir(d);
    return 0;
}

void backup(char* directory, char* backupDirectory, int key){

    downShift(directory, backupDirectory, key); // just wrapper to architecture
    return;
}

// arguments: path path key
int main(int argc, char* argv[]){

    if (argc <= 2) {
        fprintf(stderr, "Too few arguments.\n");
    }
    else {
        int key = 0;
        if (argc == 4 && strcmp(argv[3], "2") == 0) key = 2;
        backup(argv[1], argv[2], key);
    }
    return 0;
}
