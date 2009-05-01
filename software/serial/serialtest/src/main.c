#include <config.h>
#include <stdio.h>
#include <gnet.h>
#include <glib.h>
#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <string.h>

uint8_t serialbuffer[255];

#define SERIAL_BUFFERLEN    100
#define SERIAL_ESCAPE   '\\'
#define SERIAL_START    '0'
#define SERIAL_END     '1'

void hexdump(uint8_t * data, uint16_t len)
{
    uint16_t i;
    for(i=0; i<len; i++){
        if( data[i] < 0x10 ){
            printf(" 0%X", data[i]);
        }else{
            printf(" %X", data[i]);
        }
    }
}

uint16_t serialin( uint8_t data )
{
    static int fill = 0;
    static uint8_t escaped = 0;

    if(data == SERIAL_ESCAPE){
        if(!escaped){
            escaped = 1;
            return 0;
        }
        escaped = 0;
    }else if(escaped){
        escaped = 0;
        if(data == SERIAL_START){
            fill = 0;
            return 0;
        }else if( data == SERIAL_END){
            return fill;
        }
    }
    serialbuffer[fill++] = data;
    if(fill >= SERIAL_BUFFERLEN)
        fill = SERIAL_BUFFERLEN - 1;
    return 0;
}

struct message{
    uint16_t    len;
    uint8_t     data[126];
};

struct queues{
    GAsyncQueue * packet_in;
    GAsyncQueue * status_in;
    GAsyncQueue * packet_out;
    GAsyncQueue * status_out;
};

gpointer reader(gpointer data)
{
    struct queues * q = data;
    printf("reader thread started\n");
    while(1){
        struct message * msg = g_async_queue_pop(q->packet_in); 
        printf("read a %u long message:", msg->len);
        hexdump(msg->data, msg->len);
        printf("\n");
    }
    printf("reader thread stoped\n");
}

gboolean readserial(GIOChannel *serial, GIOCondition condition, gpointer data)
{
    uint8_t gbuf[10];
    gsize gread;
    struct queues * q = data;

    int r = g_io_channel_read_chars(serial,gbuf,1,&gread,NULL);
    if( gread > 0 ){
        //printf("result: %u read %u bytes data=0x%2X.\n",r,gread,gbuf[0]);
        struct message * msg = g_new(struct message,1);
        msg->len = serialin(gbuf[0]);
        if(msg->len){
            //printf("got a %u long msg:",msg->len);
            if( serialbuffer[0] == 'P' ){
                memcpy(msg->data,serialbuffer+1,msg->len-1);
                g_async_queue_push(q->packet_in,msg);
            }else if( serialbuffer[0] == 'S'){ 
                memcpy(msg->data,serialbuffer,msg->len);
                g_async_queue_push(q->status_in,msg);
            }
            //printf("\n");
        }
    }else{
        printf("result: %u\n",r);
    }

    return 1;
}

int main (int argc, char *argv[])
{
    GIOChannel  * serial;
    struct queues q;

    if (!g_thread_supported ()) g_thread_init (NULL);
    printf ("This is " PACKAGE_STRING ".\n");
    int fd = open("/dev/ttyUSB0", O_RDWR|O_NOCTTY);//|O_NONBLOCK);
    struct termios tio;
    tcgetattr (fd, &tio);
    tio.c_cflag = CREAD | CLOCAL | B115200 | CS8;
    tio.c_iflag = IGNPAR | IGNBRK;
    tio.c_oflag = 0;
    tio.c_lflag = 0;
    tio.c_cc[VTIME] = 0;
    tio.c_cc[VMIN]  = 0;
    tcsetattr (fd, TCSANOW, &tio);
    tcflush (fd, TCIFLUSH);
    tcflush (fd, TCOFLUSH);

    serial = g_io_channel_unix_new(fd);
    g_io_channel_set_encoding(serial,NULL,NULL);

    q.packet_in = g_async_queue_new(); 
    q.status_in = g_async_queue_new();

    GThread * readerthread = g_thread_create(reader,&q,FALSE,NULL);
    
    g_io_add_watch(serial,G_IO_IN,readserial,&q);
    g_io_channel_write_chars(serial, "\\0P\x01\xFF\x00\x00\x02Mr\\1",12,NULL,NULL);
    
    GMainLoop * mainloop = g_main_loop_new(NULL,TRUE);
    g_main_loop_run(mainloop);

    printf("past mainloop\n");
    return 0;
}

