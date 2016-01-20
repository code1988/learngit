#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <fcntl.h> 
#include <unistd.h> 
 
#define TEST_FOPEN 
 
int main(int argc, char *argv[]) 
{ 
    /* l_type   l_whence  l_start  l_len  l_pid   */ 
    struct flock fl = {F_WRLCK, SEEK_SET,   0,      0,     0 }; 
    int fd; 
 
#ifdef TEST_FOPEN 
    FILE *file = NULL; 
#endif /* TEST_FOPEN */ 
 
    fl.l_pid = getpid(); 
 
    if (argc > 1) 
        fl.l_type = F_RDLCK; 
    while(1) 
    { 
#ifdef TEST_FOPEN 
    if ((file = fopen("lockdemo.c", "rw+")) == NULL) { 
        perror("fopen"); 
        exit(1); 
    } 
#else 
    if ((fd = open("lockdemo.c", O_RDWR)) == -1) { 
        perror("open"); 
        exit(1); 
    } 
#endif /* TEST_FOPEN */ 
        printf("Press <RETURN> to try to get lock: "); 
        getchar(); 
        printf("Trying to get lock..."); 
 
    #ifdef TEST_FOPEN 
        fd = fileno(file); 
    #endif /* TEST_FOPEN */ 
 
        fl.l_type = F_WRLCK;  /* set to lock same region */ 
        if (fcntl(fd, F_SETLKW, &fl) == -1) { 
            perror("fcntl"); 
            exit(1); 
        } 
        
        printf("got lock\n"); 
        printf("Press <RETURN> to release lock: "); 
        getchar(); 
 
        fl.l_type = F_UNLCK;  /* set to unlock same region */ 
 
        if (fcntl(fd, F_SETLK, &fl) == -1) { 
            perror("fcntl"); 
            exit(1); 
        } 
 
        printf("Unlocked.\n"); 
#ifdef TEST_FOPEN 
    fclose(file); 
#else 
    close(fd); 
#endif /* TEST_FOPEN */ 
    } 
 
    return 0; 
} 