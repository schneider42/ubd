#include <config.h>
#include <stdio.h>
#include <glib.h>
#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <string.h>
#include <gio/gio.h>

#include "ubpacket.h"
#include "serial.h"
#include "debug.h"
#include "net6.h"
#include "packet.h"
#include "ubpacket.h"
#include "busmgt.h"
#include "address6.h"
#include "mgt.h"
#include "db.h"

gpointer reader(gpointer data)
{
    //struct queues * q = data;
    printf("reader thread started\n");
    while(1){
        data = NULL;
    /*    printf("waiting for packet...");
        struct ubpacket * p = g_async_queue_pop(packet_queues.packet_in); 
        printf("read packet from %u to %u flags: %x seq=%u len %u: ", 
                p->src, p->dest, p->flags, p->seq, p->len);
        debug_hexdump(p->data, p->len);
        printf("\n");*/
    }
    printf("reader thread stoped\n");
}


/*void main_inpacket(struct ubpacket* p)
{

}*/

int main (int argc, char *argv[])
{
    if (!g_thread_supported ()) g_thread_init (NULL);
    g_type_init();
    
    printf ("This is " PACKAGE_STRING ".\n");

    if( argc < 2 ){
        printf("Please specify an interface to use.\n");
        return 0;
    }

    if( argc < 3 ){
        printf("Please specify a base address to use.\n");
        return 0;
    }
    db_init("nodes.db", "groups.db");
    g_assert( net_init(argv[1], argv[2], 8) == 0);

    mgt_init();

    if( argc > 3 ){
        if( serial_open(argv[3]) == -1 ){
            printf("Failed to open serial device %s\nAborting.\n", argv[1]);
            return 0;
        }
        //activate bridge
        serial_switch();
        packet_init(); 
        busmgt_init();
    }else{
        printf("Please specify a serial port to use.\n");

        mgt_createNode(TYPE_NODE, "blubb.exmaple.com");
        mgt_createNode(TYPE_NODE, "fnord.barrr,example.com");
    }
   
    GMainLoop * mainloop = g_main_loop_new(NULL,TRUE);
    g_main_loop_run(mainloop);

    return 0;
}

