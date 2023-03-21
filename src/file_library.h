#ifndef __FILE_LIBRARY_H__
#define __FILE_LIBRARY_H__

#ifdef __cplusplus
extern "C" {
#endif

void file_library(bool *p_open, th_db_t * db);
void file_library_init(void);
void file_library_free(void);

#ifdef __cplusplus
}
#endif

#endif

