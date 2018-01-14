#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include "error.h"
#include <string.h>
#define MAX 1024
#define MAXLENGTH 10000

typedef struct dirent Dirent;

typedef struct meta{
        char* ino;
        char* mode;
        char* nlink;
        char* uid;
        char* gid;
        char* size;
        char* date;
        char* name;
        char* color_name;
        char* category;
}Meta;

int open_read_dir(char* directory, int fd);
void sort_meta(int fd, Meta* metalist[], int mIndex);
void display_meta(int fd, int undi, int di, Meta* undirectory[], Meta* directory[]);

int getSize(char array[]){
        int i, length = 0;
        for(i=0; array[i] != '\0'; i++){
                length++;
        }
        return length;
}

char* ino(int fd, ino_t st_ino){
        char *ino = (char*)malloc(sizeof(char)*MAX);
        sprintf(ino, "%ld", st_ino);
        return ino;
}

char* mode(int fd, mode_t st_mode){
        char *str = (char*)malloc(sizeof(char)*11);

        if(S_ISDIR(st_mode)) str[0]='d';
                else if(S_ISLNK(st_mode)) str[0]='l';
                else if(S_ISCHR(st_mode)) str[0]='c';
                else if(S_ISBLK(st_mode)) str[0]='b';
                else if(S_ISREG(st_mode)) str[0]='-';

        if(st_mode & S_IRUSR) str[1]='r';       else str[1] = '-';
        if(st_mode & S_IWUSR) str[2]='w';       else str[2] = '-';
        if(st_mode & S_IXUSR) str[3]='x';       else str[3] = '-';
        if(st_mode & S_IRGRP) str[4]='r';       else str[4] = '-';
        if(st_mode & S_IWGRP) str[5]='w';       else str[5] = '-';
        if(st_mode & S_IXGRP) str[6]='x';       else str[6] = '-';
        if(st_mode & S_IROTH) str[7]='r';       else str[7] = '-';
        if(st_mode & S_IWOTH) str[8]='w';       else str[8] = '-';
        if(st_mode & S_IXOTH) str[9]='x';       else str[9] = '-';
                return str;
}

char* nlink(int fd, nlink_t st_nlink){
        char *nlink = (char*)malloc(sizeof(char)*MAX);
        sprintf(nlink, "%ld", st_nlink);
                return nlink;
}

char* uid(int fd, uid_t st_uid){
        char *uid = (char*)malloc(sizeof(char)*MAX);
        sprintf(uid, "%ld", st_uid);
                return uid;
}

char* gid(int fd, gid_t st_gid){
        char *gid = (char*)malloc(sizeof(char)*MAX);
        sprintf(gid, "%ld", st_gid);
                return gid;
}

char* size(int fd, off_t st_size){
        char *size = (char*)malloc(sizeof(char)*MAX);
        sprintf(size, "%ld", st_size);
                return size;
}

char* date(int fd, time_t atime){
        struct tm *t = localtime(&atime);
        int year, month, day, hour, minute, second;
        char *temp = (char*)malloc(sizeof(char)*5);
        char *date = (char*)malloc(sizeof(char)*MAX);
        year = t->tm_year+1900;
        month = t->tm_mon+1;
        day = t->tm_mday;
        hour = t->tm_hour;
        minute = t->tm_min;
        second = t->tm_sec;
        int i = 0, j;

        sprintf(temp, "%d\0", year);
        for(j=0; temp[j]!='\0'; j++, i++){
                date[i] = temp[j];
        }
        date[i] = '/'; i++;
        sprintf(temp, "%d\0", month);
        for(j=0; temp[j] != '\0'; j++, i++){
                date[i] = temp[j];
        }
        date[i] = '/'; i++;
        sprintf(temp, "%d\0", day);
        for(j=0; temp[j] != '\0'; j++, i++){
                date[i] = temp[j];
        }
        date[i] = ' '; i++;
        sprintf(temp, "%d\0", hour);
        for(j=0; temp[j] != '\0'; j++, i++){
                date[i] = temp[j];
        }
        date[i] = ':'; i++;
        sprintf(temp, "%d\0", minute);
        for(j=0; temp[j] != '\0'; j++, i++){
                date[i] = temp[j];
        }
        date[i] = ':'; i++;
        sprintf(temp, "%d\0", second);
        for(j=0; temp[j] != '\0'; j++, i++){
                date[i] = temp[j];
        }
        date[i] = ' '; i++;
                return date;
}

char* get_color_name(char *name){
                char *blue = "\033[34m";
                char *end = "\033[0m";
                int length = getSize(name)+getSize(blue)+getSize(end);
                char *colored_name = (char*)malloc(sizeof(name)*length)+1;
                int i, j;
                for(i=0, j=0; i<getSize(blue); i++, j++){
                        colored_name[i] = blue[j];
                }
                for(i=getSize(blue), j=0; i<getSize(blue)+getSize(name); i++, j++){
                        colored_name[i] = name[j];
                }
                for(i=getSize(blue)+getSize(name),j=0; i<length; i++, j++){
                        colored_name[i] = end[j];
                }
                colored_name[i] = '\0';
                return colored_name;
}

char* name(int fd, char d_name[]){
        return d_name;
}

int blocks = 0;
Meta* read_stat(int fd, char* file, struct stat *statbuf){
                Meta *meta = (Meta*)malloc(sizeof(Meta));
                meta->ino = ino(fd, statbuf->st_ino);
        	meta->mode = mode(fd, statbuf->st_mode);
                char fm = meta->mode[0];
                meta->mode[10] = '\0';
                meta->nlink = nlink(fd, statbuf->st_nlink);
                meta->uid = uid(fd, statbuf->st_uid);
                meta->gid = gid(fd, statbuf->st_gid);
                meta->size = size(fd, statbuf->st_size);
               meta->date = date(fd, statbuf->st_atime);
                meta->name = name(fd, file);

                if(fm == '-')   meta->category="-";
                else if(fm == 'd'){
                        meta->category="d";
                        meta->color_name = get_color_name(meta->name);
                }
                else if(fm == 'l') meta->category="l";
                else meta->category="whatever";

        blocks += (statbuf->st_blocks)/2;

                return meta;
}

int toggle = 0;
int open_read_dir(char* directory, int fd){
        Dirent *dp;
        DIR *dir_fd;
        struct stat *statbuf = (struct stat*)malloc(sizeof(struct stat));
        char *file;
        Meta* metalist[MAXLENGTH];
        int mIndex = 0;
        char *alert1 = "Can't open this directory: ";
        char *alert2 = "Can't use stat() on this file: ";

        if((dir_fd = opendir(directory)) == NULL){
                        //write(fd, alert1, getSize(alert1));
                        //write(fd, directory, getSize(directory));write(fd, "\n", 1);
                        return 0;
        }else{
                        chdir(directory);
                        toggle += 1;
            while((dp=readdir(dir_fd)) != NULL){
                           Meta *meta = (Meta*)malloc(sizeof(Meta));
               	          file = dp->d_name;
              	          if(lstat(file, statbuf)<0){
                                   write(fd, alert2, getSize(alert2));
                                   write(fd, file, getSize(file)); write(fd, "\n", 1);
                          }else{
                          meta = read_stat(fd, file, statbuf); //read meta per one file
                                        metalist[mIndex++] = meta;
                          }
            }
        }
                sort_meta(fd, metalist, mIndex);
                return 1;
}

void swap(Meta* list[], int a, int b){
        Meta* temp = list[a];
        list[a] = list[b];
        list[b] = temp;
}
void bubble_sort(Meta* list[], int n){
        int i, j;
        for(i=n-1; i>=1; i--){
                for(j=0; j<i; j++){
                        if(strcmp(list[j]->name, list[j+1]->name)>0)
                                swap(list, j, j+1);
                }
        }
}

void sort_meta(int fd, Meta* metalist[], int mIndex){
        Meta* undirectory[MAXLENGTH];
        int undi=2;
        Meta* directory[MAXLENGTH];
        int di=0;
        int i;
        for(i=0; i<mIndex; i++){
                if(strcmp(metalist[i]->name, ".") == 0){
                        undirectory[0] = metalist[i];
                }else if(strcmp(metalist[i]->name, "..") == 0){
                        undirectory[1] = metalist[i];
                }else if(strcmp(metalist[i]->category, "d") == 0){
                        directory[di++] = metalist[i];
                }else{
                        undirectory[undi++] = metalist[i];
                }
        }
        bubble_sort(undirectory, undi);
        bubble_sort(directory, di);
        display_meta(fd, undi, di, undirectory, directory);
}

void display_meta(int fd, int undi, int di, Meta* undirectory[], Meta* directory[]){
        int t;
        for(t=0; t<toggle-1; t++){
                write(fd, "  ", 2);
        }
        // print total
        char *total = (char*)malloc(sizeof(char)*MAX);
        sprintf(total, "%d", blocks);
        write(fd, "total ", 6);
        write(fd, total, getSize(total));
        write(fd, "\n", 1);

        int i;
        // undirectory print first
        for(i=0; i<undi; i++){
                int t;
                for(t=0; t<toggle-1; t++){
                        write(fd, "  ", 2);
                }
                write(fd, undirectory[i]->ino, getSize(undirectory[i]->ino)); write(fd, "  ", 2);
                write(fd, undirectory[i]->mode, getSize(undirectory[i]->mode)); write(fd, "  ", 2);
                write(fd, undirectory[i]->nlink, getSize(undirectory[i]->nlink)); write(fd, "  ", 2);
                write(fd, undirectory[i]->uid, getSize(undirectory[i]->uid)); write(fd, "  ", 2);
                write(fd, undirectory[i]->gid, getSize(undirectory[i]->gid)); write(fd, "  ", 2);
                write(fd, undirectory[i]->size, getSize(undirectory[i]->size)) ;write(fd, "  ", 2);
                write(fd, undirectory[i]->date, getSize(undirectory[i]->date)); write(fd, "  ", 2);
                if(i==0 || i== 1){
                        write(fd, undirectory[i]->color_name, getSize(undirectory[i]->color_name));
                }else{
                        write(fd, undirectory[i]->name, getSize(undirectory[i]->name));
                }
                write(fd, "\n", 1);
        }
        // directory print
        int j;
        for(j=0; j<di; j++){
                int t;
                for(t=0; t<toggle-1; t++){
                        write(fd, "  ", 2);
                }
                write(fd, directory[j]->ino, getSize(directory[j]->ino)); write(fd, "  ", 2);
                write(fd, directory[j]->mode, getSize(directory[j]->mode)); write(fd, "  ", 2);
                write(fd, directory[j]->nlink, getSize(directory[j]->nlink)); write(fd, "  ", 2);
                write(fd, directory[j]->uid, getSize(directory[j]->uid)); write(fd, "  ", 2);
                write(fd, directory[j]->gid, getSize(directory[j]->gid)); write(fd, "  ", 2);
                write(fd, directory[j]->size, getSize(directory[j]->size)); write(fd, "  ", 2);
                write(fd, directory[j]->date, getSize(directory[j]->date)); write(fd, "  ", 2);
                write(fd, directory[j]->color_name, getSize(directory[j]->color_name));
                write(fd, "\n", 1);

                // read child directory;
                int result = open_read_dir(directory[j]->name, fd);
                // -> sort_meta call -> display_meta call (Recursive);
                chdir("../");
                if(result == 1) toggle -= 1;
        }
}

void error_handle(){
        char *other = "Usage:\n./a.out -o [output file]\n./a.out -d [directory]\n./a.out -d [directory] -o [output file] | ./a.out -o [output file] -d [directory]\n[directory] can be relative or absoulte.\n";

        write(STDOUT_FILENO, other, strlen(other));
                return;
}


int main(int argc, char *argv[]){
        char *cwd = (char*)malloc(sizeof(char)*MAX);
        char *file;
        if(argc == 1){
                if(getcwd(cwd, MAX) == NULL){
                        perror("Fail to open current working directory");
                }
                open_read_dir(cwd, 1);
        }

        if(argc == 2){ // Handling with unallowed input.
                error_handle();
        }

        if(argc == 3){ // ./a.out -d directory
                int fd;
                char *option = argv[1]; // option : -d or -o
                char *parameter = argv[2];

                if((strcmp(argv[2],"-d")==0)){  // This can make "-d" file, which is troublesome to delete.
                        write(STDOUT_FILENO, "Warning: Unallowed argument\n", 28);
                                                error_handle();
                }

                if(strcmp(option, "-o") == 0){
                        if((fd = open(parameter, O_RDWR|O_TRUNC|O_CREAT, 0777 )) == -1){
                                perror("open error");
                        }
                        if(getcwd(cwd, MAX) == NULL)
                                perror("Fail to get current working directory");

                        open_read_dir(cwd, fd);
                        close(fd);
                }else if(strcmp(option, "-d") == 0){
                                        open_read_dir(parameter, 1);
                                }
        }
        if(argc == 4){
                error_handle();
        }
        if(argc == 5){
                int fd;
                char *directory;
                char *outputfile;
                if(strcmp(argv[1], "-d")==0 && strcmp(argv[3], "-o")==0){
                        directory = argv[2];
                        outputfile = argv[4];   // argv[3] should be -o. Otherwise, it should be handled.
                }else if(strcmp(argv[1], "-o") == 0 && strcmp(argv[3], "-d")==0){
                        directory = argv[4];
                        outputfile = argv[2];
                }else if(strcmp(argv[2], "-d") == 0 || strcmp(argv[4], "-d") ==0){
                        write(STDOUT_FILENO, "Warning: Unallowed argument\n", 28);
                                                error_handle();
                                                return ;
                                }

                if((fd = open(outputfile, O_RDWR|O_TRUNC|O_CREAT, 0777)) == -1)
                        perror("Fail to open output file");
                open_read_dir(directory, fd);
                close(fd);
        }

        return 0;
}
