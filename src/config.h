#ifndef __CONFIG_H_
#define __CONFIG_H_
#include <glib.h>

struct ubconfig {
    gchar *interface;
    gchar *base;
    gchar *multicastbase;
    gchar *device;
    gint rate;
    gint nodetimeout;
    gint sysloglevel;
    gint demo;
    gint dirserver;
    gint dirttl;
    gint dirserverport;
};

extern struct ubconfig config;
void config_init(void);
#endif
