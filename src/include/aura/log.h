#ifndef AURA_LOG_H
#define AURA_LOG_H

void log_init(void);
void log_info(const char *msg);
void log_printf(const char *fmt, ...);

#endif
