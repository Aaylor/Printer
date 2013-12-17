#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "error.h"
#include "message.h"

void 
create_random_tube_name(char tube[64], char *seed)
{
    int left;
    size_t current_length;

    srand(time(NULL));

    left = 54;
    strcpy(tube, "/tmp/tAsw_");
    current_length = strlen(tube);

    if (seed != NULL)
    {
        while (*seed && left > 23)
        {
            if (!isalnum(*seed))
            {
                seed += 1;
                continue;
            }

            if (rand() % 2 == 0)
                tube[current_length++] = toupper(*seed);
            else
                tube[current_length++] = tolower(*seed);
            seed += 3;
            tube[current_length] = '\0';
            --left;
        }
    }

    while (left-- > 1)
        tube[current_length++] = (rand() % 10) + '0';
    tube[current_length] = '\0';
}

int
create_tube(const char *name)
{
    return mkfifo(name, S_IRWXU | S_IRWXG | S_IRWXO);
}

void *
create_message(struct sending_message m)
{
    char *buf;
    size_t pos, length;
    unsigned int msg_length;

    printf("%d\n", m.buf_size);
    msg_length = sizeof(char) + sizeof(uid_t) + sizeof(gid_t)
        + ANSWERING_TUBE_SIZE + m.buf_size;

    buf = malloc(msg_length);
    if (buf == NULL)
        ERROR_EXIT(14);

    pos = 0;
    memcpy(buf, &msg_length, sizeof(unsigned int));
    pos += sizeof(unsigned int);

    memcpy(buf + pos, &(m.type), sizeof(char));
    pos += sizeof(char);

    memcpy(buf + pos, &(m.uid), sizeof(uid_t));
    pos += sizeof(uid_t);

    memcpy(buf + pos, &(m.gid), sizeof(gid_t));
    pos += sizeof(gid_t);

    memcpy(buf + pos, m.answering_tube, ANSWERING_TUBE_SIZE);
    pos += ANSWERING_TUBE_SIZE;

    if (m.buf_size != 0)
    {
        printf("buffer not empty\n");
        memcpy(buf + pos, m.buf, m.buf_size);
        pos += m.buf_size;
    }

    return buf;
}

void
send_message(const char * const tube, const void * const message)
{
    int fd;
    unsigned int size;

    memcpy(&size, message, sizeof(unsigned int));

    fd = open(tube, O_WRONLY);
    if (fd == -1)
        ERROR_EXIT(10);   
    
    write(fd, message, (size + sizeof(unsigned int)));
    
    close(fd);
}

void
free_message(struct sending_message m)
{

}

int
get_answer(const char * const answer_tube) 
{
    int fd, answer;
    size_t bytes_read;

    fd = open(answer_tube, O_RDONLY);
    if (fd == -1)
        ERROR_EXIT(10);

    bytes_read = read(fd, &answer, sizeof(int));
    if (bytes_read < sizeof(int))
        fprintf(stderr, "Warning : nombre de bytes lu inférieur");

    unlink(answer_tube);
    close(fd);

    return answer;
}

void
print_answer(char answer_tube[ANSWERING_TUBE_SIZE])
{
    int fd;
    size_t bytes_read;
    char buffer[1024];

    fd = open(answer_tube, O_RDONLY);
    if (fd == -1)
        ERROR_EXIT(1244);

    while((bytes_read = read(fd, buffer, 1024)) > 0)
        write(STDOUT_FILENO, buffer, bytes_read);
}

