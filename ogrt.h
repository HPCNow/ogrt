#ifndef OGRT_H_INCLUDED
#define OGRT_H_INCLUDED

#include "ogrt.pb-c.h"

#define OGRT_SECTION_NAME (".note.ogrt.info")

int ogrt_read_info(const char *filename);

#endif
