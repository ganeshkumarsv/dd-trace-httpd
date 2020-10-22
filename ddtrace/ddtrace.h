#ifndef DDTRACE_H
#define DDTRACE_H

#include "httpd.h"

#ifdef __cplusplus
extern "C" {
#endif

void ddtrace_create_tracer();
void ddtrace_start_span(request_rec *r);
void ddtrace_finish_span(request_rec *r);

#ifdef __cplusplus
}
#endif

#endif /*DDTRACE_H*/
