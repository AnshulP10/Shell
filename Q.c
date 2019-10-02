#include<fcntl.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<errno.h>
#include<stdio.h>
#include<sys/types.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<limits.h>
#include<dirent.h>
#include<pwd.h>
#include<grp.h>
#include<time.h>
#include<sys/types.h>
#include<signal.h>

char *read_command(void){
    char *command; 
    ssize_t size = 0; 
    getline(&command, &size, stdin);
    return command;
}
int count;
int count1;
int idstack[100];
char pstack[1000][200];
int pidtop=0;
void procExit()
{
    int status;

    for (int i = 0; i < pidtop; i++)
    {
        if (waitpid(idstack[i], &status, WNOHANG) > 0)
        {
            if (WIFEXITED(status) > 0){
                printf("%s with pid %d exited normally\n", pstack[i], idstack[i]);
            }
            else if (WIFSIGNALED(status)){
                printf("%s with pid %d exited with signal\n",pstack[i], idstack[i]);
            }
            else{
                printf("%s with pid %d exited abnormally\n", pstack[i], idstack[i]);
            }
        }
    }
}

char **tokencmd(char *line){
    count = 0;
    char **commands = malloc(100000);
    char *inp=malloc(PATH_MAX);
    strcpy(inp, line);
    char *token=strtok(inp, ";");
    while(token!=NULL){
        commands[count]=malloc(strlen(token));
        strcpy(commands[count], token);
        token=strtok(NULL, ";");
        count++;
    }
    return commands;
}

char **tokenpipe(char *line){
    count1 = 0;
    char **commands = malloc(100000);
    char *inp=malloc(PATH_MAX);
    strcpy(inp, line);
    char *token=strtok(inp, "|");
    while(token!=NULL){
        commands[count1]=malloc(strlen(token));
        strcpy(commands[count1], token);
        token=strtok(NULL, "|");
        count1++;
    }
    return commands;
}

char **tokenize(char *command){
    char **args = malloc(1000000);
    char *pointer;
    char *inp=malloc(PATH_MAX);
    strcpy(inp, command);
    pointer = strtok(inp, " \n\t\r\a");
    int i = 0;
    while(pointer){
        args[i] = realloc(args[i], strlen(pointer));
        strcpy(args[i], pointer);
        pointer = strtok(NULL, " \n");
        i++;
    }
    return args;
}

void pwd(){
    char *PATH = malloc(PATH_MAX);
    getcwd(PATH, PATH_MAX);
    printf("%s\n", PATH);
    return;
}

void echo(char **input){
    int i = 1;
    while(input[i]){
        printf("%s ", input[i]);
        i++;
    }
    printf("\n");
    return;
}

void cd(char *location, char *dirname){
    if(!strcmp(location, "~"))
        chdir(dirname);
    else{
        DIR *dir=opendir(location);
        if(!dir)
            perror("Directory not found");
        chdir(location);
    }
}

int check_directory(char *dirname){
    struct stat dir;
    if (stat(dirname, &dir) == 0 && S_ISDIR(dir.st_mode)!=0)
        return 1;
    else
        return 0;
}

char* check_permissions(char *file){
    char *string = malloc(15*sizeof(char));
    int check=check_directory(file);
    if(check)
        string[0]='d';
    else
        string[0]='-';
    struct stat stats;
    if(stat(file, &stats) == 0){
        mode_t perm = stats.st_mode;
        if(perm & S_IRUSR)
            string[1]='r';
        else
            string[1]='-';
        if(perm & S_IWUSR)
            string[2]='w';
        else
            string[2]='-';
        if(perm & S_IXUSR)
            string[3]='x';
        else
            string[3]='-';
        if(perm & S_IRGRP)
            string[4]='r';
        else
            string[4]='-';
        if(perm & S_IWGRP)
            string[5]='w';
        else
            string[5]='-';
        if(perm & S_IXGRP)
            string[6]='x';
        else
            string[6]='-';
        if(perm & S_IROTH)
            string[7]='r';
        else
            string[7]='-';
        if(perm & S_IWOTH)
            string[8]='w';
        else
            string[8]='-';    
        if(perm & S_IXOTH)
            string[9]='x';
        else
            string[9]='-';
        return string;        
    }
    else{
        return strerror(errno);
    }
}

void ls(char **args, char *home){
    if(!args[1]){
        DIR *dirname; 
        struct dirent *info;
        char *PATH=malloc(PATH_MAX);
        getcwd(PATH, PATH_MAX);         
        dirname = opendir(PATH);
        while(info=readdir(dirname)){
            if(info->d_name[0]!='.')
                printf("%s\n", info->d_name);    
        }    
    }
    else if(!strcmp(args[1],"-l")){
        DIR *dirname;
        struct dirent *info;
        char *PATH = malloc(PATH_MAX);
        getcwd(PATH, PATH_MAX);
        dirname = opendir(PATH);
        while(info=readdir(dirname)){
            if(info->d_name[0]!='.'){
                char *permissions = check_permissions(info->d_name);
                for(int i=0; i<10; i++)
                    printf("%c", permissions[i]);
                printf(" ");
                struct stat stats;
                char date[20];
                if(stat(info->d_name, &stats) == 0){
                    struct passwd *user = getpwuid(stats.st_uid);
                    struct group *group = getgrgid(stats.st_gid);
                    int size = (int)stats.st_size;
                    printf("%ld ", stats.st_nlink);
                    printf("%s %s ", user->pw_name, group->gr_name);
                    printf("%d ", size);
                    strftime(date, 20, "%b %d %H:%M", localtime(&(stats.st_mtime)));
                    printf("%s ",date);
                    printf("%s", info->d_name);
                    printf("\n");
                }
            }
        }
    }
    else if(!strcmp(args[1], "-a")){
        DIR *dirname; 
        struct dirent *info;
        char *PATH=malloc(PATH_MAX);
        getcwd(PATH, PATH_MAX);
        dirname = opendir(PATH);
        while(info=readdir(dirname))
            printf("%s\n", info->d_name);
    }
    else if(!strcmp(args[1],"-la") || !strcmp(args[1], "-al") || (!strcmp(args[1],"-l") && !strcmp(args[2], "-a")) || (!strcmp(args[1],"-a") && !strcmp(args[1],"-l"))){
        DIR *dirname;
        struct dirent *info;
        char *PATH = malloc(PATH_MAX);
        getcwd(PATH, PATH_MAX);
        dirname = opendir(PATH);
        while(info=readdir(dirname)){
            char *permissions = check_permissions(info->d_name);
            for(int i=0; i<10; i++)
                printf("%c", permissions[i]);
            printf(" ");
            struct stat stats;
            char date[20];
            if(stat(info->d_name, &stats) == 0){ 
                struct passwd *user = getpwuid(stats.st_uid);
                struct group *group = getgrgid(stats.st_gid);
                int size = (int)stats.st_size;
                printf("%ld ", stats.st_nlink);
                printf("%s %s ", user->pw_name, group->gr_name);
                printf("%d ", size);
                strftime(date, 20, "%b %d %H:%M", localtime(&(stats.st_mtime)));
                printf("%s ",date);
                printf("%s", info->d_name);
                printf("\n");   
            }   
        }   
    } 
    else if(args[1]){
        DIR *dirname;
        struct dirent *info;
        if(!strcmp(args[1],"~"))
            dirname = opendir(home);
        else
            dirname = opendir(args[1]);
        if(!dirname && strcmp(args[1],"~")){
            perror("Directory not found");
        }
        else{
            while(info=readdir(dirname))
                if(info->d_name[0]!='.')
                    printf("%s\n", info->d_name);
        }
    }
    return;
}

void pinfo(char **args){
    int pid;
    char *str2 = malloc(PATH_MAX);
    if(!args[1]){
        pid = getpid();
        printf("pid -- %d\n", pid);
        sprintf(str2, "%d", pid);
    }
    else{
        strcpy(str2, args[1]);
        printf("pid -- %s\n", args[1]);
    }
    char str1[17] = "/proc/";
    char str1d[17];
    strcpy(str1d, str1);
    char *str3 = "/exe";
    char *str4 = "/stat";
    strcat(str1, str2);
    strcat(str1, str4);
    FILE *file = fopen(str1, "r");
    if(!file){
        perror("No such process");
        return;
    }
    char info[25][50];
    char ans[25][50];
    int i = 0;
    int j = 0;
    while(i<23){
        if(i == 2 || i==22){
            fscanf(file, "%s", ans[j]);
            i++;
            j++;
        }
        else{
            fscanf(file,"%s", info[i]);
            i++;
        }
    }
    printf("Process Status -- %s\nmemory -- %s\n", ans[0], ans[1]);
    fclose(file);
    strcat(str1d, str2);
    strcat(str1d, str3);
    char *buf = malloc(PATH_MAX);
    int check = readlink(str1d, buf, PATH_MAX);
    if(check)
        printf("Executable Path -- %s\n", buf);
    else
        perror("PID not found\n");
    return;
}    

void setenv1(char **args){
    if(!args || args[3]){
        perror("Give 1-2 arguments");
        return;
    }
    else if(args[2]){
        int check1 = setenv(args[1] ,args[2], 1);
        if(check1==-1)
            perror("unable to use setenv");
    }
    else{
        int check2 = setenv(args[1],"\0",1); 
        if(check2==-1)
            perror("unable to use setenv");
    }
    return;
}

void unsetenv1(char **args){
    if(!args || args[2]){
        perror("Give 1 argument");
        return;
    }
    int check = unsetenv(args[1]);
    if(check==-1)
        perror("unable to use unsetenv");
    return;
}

int c_pid = 0;
void els(char **args, char *command){
    int flag=0;
    int status;
    char *app=args[0];
    pid_t pid=fork();
    if(command[strlen(command)-2]!='&'){
        flag=1;
    }
    command[strlen(command)-2]='\0';
    if(pid==0){
        setpgid(0,0);
        if(execvp(app,args)<0){
            perror("Unable to execute");
        }
    }
    else{
        if(flag==1)
        {
            c_pid = (int)pid;
            waitpid(pid,&status,WUNTRACED);
            if(WIFSTOPPED(status)){
                idstack[pidtop]=(int)pid;
                strcpy(pstack[pidtop], app);
                pidtop++;
            }
        }
        else{
            idstack[pidtop]=(int)pid;
            strcpy(pstack[pidtop],app);
            pidtop++;
        }
    }
    return;
}

void sigHandler(int num){
    if(c_pid)
        kill(c_pid, num);
    return;
}

void Jobs(){
    char procfile[200], status[10];
    int num = 0;
    for(int k=0; k<pidtop; k++){
        sprintf(procfile, "/proc/%d/stat", idstack[k]);
        int fd  = open(procfile, O_RDONLY);
        if(fd == -1)
            strcpy(status, "UNKNOWN");
        else{
            char itr[100];
            read(fd, itr, 100);
            int j = 0;
            close(fd);
            for(int i = 0; i < 100 ; i++){
                if(j == 2){
                    if(itr[i] == 'S' || itr[i] == 'R')
                        strcpy(status, "Running");
                    else if(itr[i] == 'T')
                        strcpy(status, "Stopped");
                    else if(itr[i] == 'Z')
                        strcpy(status, "Killed");
                    else
                        strcpy(status, "Unknown");
                    break;
                }
                if(itr[i]==' ')
                    j++;
            }
        }
        if(strcmp(status, "Killed") && strcmp(status, "UNKNOWN")){
            num++;
            printf("[%d] %s %s [%d]\n",num, status, pstack[k], idstack[k]);
        }
    }
}

void kjob(char **args){
    char procfile[200], status[10];
    int pidx = 0;
    int num = 0;
    int index = atoi(args[1]);
    for(int k=0; k<pidtop; k++){
        sprintf(procfile, "/proc/%d/stat", idstack[k]);
        int fd  = open(procfile, O_RDONLY);
        if(fd == -1) 
            strcpy(status, "UNKNOWN");
        else{
            char itr[100];
            read(fd, itr, 100);
            int j = 0;
            close(fd);
            for(int i = 0; i < 100 ; i++){
                if(j == 2){ 
                    if(itr[i] == 'S' || itr[i] == 'R')
                        strcpy(status, "Running");
                    else if(itr[i] == 'T')
                        strcpy(status, "Stopped");
                    else if(itr[i] == 'Z')
                        strcpy(status, "Killed");
                    else
                        strcpy(status, "Unknown");
                    break;
                }
                if(itr[i]==' ')
                    j++;
            }
        }
        if(strcmp(status, "Killed") && strcmp(status, "UNKNOWN"))
            num++;
        if(num==index){
            pidx = idstack[k];
            break;
        }
    }
    int signal = atoi(args[2]);
    kill(pidx, signal);
    return;
}

void fg(char **args){
    char procfile[200], status[10];
    int index = atoi(args[1]), num = 0;
    pid_t pidx = 0;
    for(int k=0; k<pidtop; k++){
        sprintf(procfile, "/proc/%d/stat", idstack[k]);
        int fd  = open(procfile, O_RDONLY);
        if(fd == -1)
            strcpy(status, "UNKNOWN");
        else{
            char itr[100];
            read(fd, itr, 100);
            int j = 0;
            close(fd);
            for(int i = 0; i < 100 ; i++){
                if(j == 2){
                    if(itr[i] == 'S' || itr[i] == 'R')
                        strcpy(status, "Running");
                    else if(itr[i] == 'T')
                        strcpy(status, "Stopped");
                    else if(itr[i] == 'Z')
                        strcpy(status, "Killed");
                    else
                        strcpy(status, "Unknown");
                    break;
                } 
                if(itr[i]==' ')
                    j++;
            }
            if(strcmp(status, "Killed") && strcmp(status, "UNKNOWN"))
                num++;
            if(num==index){
                pidx = idstack[k];
                break;
            }
        }
    }
    kill(pidx, SIGCONT);
    c_pid=pidx;
    int stat,wpid;
    wpid=waitpid(pidx,&stat,WUNTRACED);
    c_pid=0;
    return;
}

void bg(char **args){
    char procfile[200], status[20];
    int index = atoi(args[1]), num = 0, pidx = 0;
    for(int k=0; k<pidtop; k++){
        sprintf(procfile, "/proc/%d/stat", idstack[k]);
        int fd  = open(procfile, O_RDONLY);
        if(fd == -1)
            strcpy(status, "UNKNOWN");
        else{
            char itr[100];
            read(fd, itr, 100);
            int j = 0;
            close(fd);
            for(int i = 0; i < 100 ; i++){
                if(j == 2){
                    if(itr[i] == 'S' || itr[i] == 'R')
                        strcpy(status, "Running");
                    else if(itr[i] == 'T')
                        strcpy(status, "Stopped");
                    else if(itr[i] == 'Z')
                        strcpy(status, "Killed");
                    else
                        strcpy(status, "Unknown");
                    break;
                }
                if(itr[i]==' ')
                    j++;
            }
            if(strcmp(status, "Killed") && strcmp(status, "UNKNOWN"))
                num++;
            if(num==index){
                pidx = idstack[k];
                break;
            }
        }
    }

    kill(pidx, SIGCONT);
    return;
}

void overkill(){
    for(int i=0; i<pidtop; i++)
        kill(idstack[i], 9);
    return;
}

void execute(char **args, char *dirname, char *command){
    signal(SIGCHLD,procExit);
    c_pid = 0;
    if(!strcmp(args[0], "pwd"))
        pwd();
    else if(!strcmp(args[0], "echo"))
        echo(args);
    else if(!strcmp(args[0], "cd")){
        if(args[1])
            cd(args[1], dirname);
        else
            chdir(dirname);
    }
    else if(!strcmp(args[0], "ls"))
        ls(args, dirname);
    else if(!strcmp(args[0], "pinfo"))
        pinfo(args);
    else if(!strcmp(args[0], "setenv"))
        setenv1(args); 
    else if(!strcmp(args[0], "unsetenv"))
        unsetenv1(args); 
    else if(!strcmp(args[0], "jobs"))
        Jobs();
    else if(!strcmp(args[0], "kjob"))
        kjob(args);     
    else if(!strcmp(args[0], "fg"))
        fg(args); 
    else if(!strcmp(args[0], "bg"))
        bg(args);
    else if(!strcmp(args[0], "overkill"))
        overkill();
    else
        els(args, command);
    return;
}

void leftdir(char **args){
    int i = 0;
    while(args[i+1]){
        if(!strcmp(args[i],"<")){
            while(args[i+1]){
                args[i]=args[i+1];
                i++;
            }
            args[i]=NULL;
            i--;
            return;
        }
        i++;
    }
    return;
}

int out;
void rightdir(char **args){
    int i = 0;
    out = dup(1);
    while(args[i+1]){
        if(!strcmp(args[i],">")){
            char *file;
            file = args[i+1];
            args[i+1] = NULL;
            args[i] = NULL;
            i--;
            int fd1 = open(file, O_WRONLY | O_CREAT | O_TRUNC ,0644);
            if(fd1==-1)
                perror("Unable to open file");
            dup2(fd1, 1);
        }
        if(!strcmp(args[i],">>")){
            char *file;
            file = args[i+1];
            args[i+1] = NULL;
            args[i] = NULL;
            i--;
            int fd1 = open(file, O_WRONLY | O_CREAT | O_APPEND ,0644);
            if(fd1==-1)
                perror("Unable to open file");
            dup2(fd1, 1); 
        }

        i++;
    }
    return;
}

void checkdir(char **args){
    leftdir(args);
    rightdir(args);
    return;
}

void run(void){
    int savestdin=dup(0),savestdout=dup(1);
    char *username = getlogin();
    char *hostname = malloc(1000000);
    gethostname(hostname, sizeof(hostname));
    char *dirname = malloc(PATH_MAX);
    getcwd(dirname, PATH_MAX);
    printf("<%s@%s:~> ", username, hostname);
    int status = 1;
    while(status){
        signal(SIGINT, sigHandler);
        signal(SIGTSTP, sigHandler);
        int flag=0;
        char *command = read_command();
        char **commands=tokencmd(command);
        for(int i =0 ; i < count ; i++){
            signal(SIGINT, sigHandler);
            signal(SIGTSTP, sigHandler);
            char **pipe_commands = tokenpipe(commands[i]);
            for(int j = 0; j < count1-1; j++){
                signal(SIGINT, sigHandler);
                signal(SIGTSTP, sigHandler);
                int c_pipe[2];
                pipe(c_pipe);
                pid_t pid69;
                pid69=fork();
                if(pid69==0){
                    dup2(c_pipe[1], 1);
                    char **args;
                    if(strlen(pipe_commands[j])>1){
                        args = tokenize(pipe_commands[j]);
                        if(!strcmp(args[0],"quit") || !args)
                            return;
                    }                        
                    if(strlen(pipe_commands[j])>1){
                        checkdir(args);
                        execute(args, dirname, pipe_commands[j]);
                    }
                    abort();
                }
                else{
                    int status;
                    dup2(c_pipe[0],0);
                    close(c_pipe[1]);
                    waitpid(pid69,&status,0);
                }
                close(c_pipe[0]);
                close(c_pipe[1]);
            }
            dup2(savestdout,1);
            char **args;
            if(strlen(pipe_commands[count1-1])>1){
                args = tokenize(pipe_commands[count1-1]);
                if(!strcmp(args[0],"quit") || !args)
                    return;
            }                            
            if(strlen(pipe_commands[count1-1])>1){
                checkdir(args);
                execute(args, dirname, pipe_commands[count1-1]);
            }
            dup2(savestdout, 1);
            dup2(savestdin,0);
            char *current_dirname = malloc(PATH_MAX);
            getcwd(current_dirname, PATH_MAX);
            if(strstr(current_dirname, dirname)){
                int len = strlen(dirname);
                printf("<%s@%s:~/%s> ", username, hostname, &current_dirname[len+1]);
            }
            else{
                printf("<%s@%s:%s> ", username, hostname, current_dirname);
            }
        }
    }
    return;
}

int main(void){
    signal(SIGINT, sigHandler);
    signal(SIGTSTP, sigHandler);
    run();
    return 0;
}
