#ifndef __TOOL_EXPORT_H__
#define __TOOL_EXPORT_H__

#ifdef __cplusplus
extern "C" {
#endif

void tool_export(bool *p_open, th_db_t * db);
char *tool_export_get_buf_highlight(void);

#ifdef __cplusplus
}
#endif

#endif

