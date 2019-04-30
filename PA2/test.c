#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
 
#define DEVICE "/dev/simple_char_driver"
#define BUFFER_SIZE 1024

int main() {
    char input;
    char sendbuffer[BUFFER_SIZE] = {'\0'};
    int feedback;
    int writefeed;
 
    int read_buffer_size = 0;
    int offset = 0;
    int whence = 0;
 
    int file = open(DEVICE, O_RDWR);
    if(file < 0) {
        printf("Failed open\n");
        return -1;
    }
 
    bool terminal = false;
 
    while(!terminal) {
        printf("Input options: r, w, s, e\n");
        printf("Your input:");
        scanf("%c", &input);

        switch(input) {
            case 'w': 
                printf("Enter String: ");
                scanf("%s", sendbuffer);
                while(getchar() != '\n');
                printf("Input: %s\n", sendbuffer);
                feedback = write(file, sendbuffer, strlen(sendbuffer));
                (feedback < 0) ? printf("Failed write\n") : printf("Wrote %d bytes\n", feedback);
                break;
            
            case 'r': 
                printf("Enter # bytes: ");
                scanf("%d", &read_buffer_size);
                while(getchar() != '\n');
                if (read_buffer_size > BUFFER_SIZE - 1){
                    printf("Too big!\n");
                    break;
                }
                char * bufftoreceive = (char *)malloc(read_buffer_size * sizeof(char));
                feedback = read(file, bufftoreceive, read_buffer_size);
                writefeed = feedback;
                (feedback < 0) ? printf("Failed read\n") : printf("Output: %s\n", bufftoreceive);
                free(bufftoreceive);
                break;

            case 's':
                printf("Enter offset number: ");
                scanf("%d", &offset);
                while(getchar() != '\n');
                if(offset < (-writefeed + 1) || (offset > writefeed - 1)) {
                    printf("You are seeking beyond the file size\n");
                    break;
                }
                printf("Enter whence value: ");
                scanf("%d", &whence);
                while(getchar() != '\n');
                if(!(whence >= 0 && whence < 3)) {
                    printf("Enter correct whence values\n");
                    break;
                }
                lseek(file, offset, whence);
                break;

            case 'e':
                printf("Exiting\n");
                terminal = true;
                break;
        }
    }
    close(file);
    return 0;
}