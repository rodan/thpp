#ifndef __TOOL_PREFERENCES_H__
#define __TOOL_PREFERENCES_H__

#ifdef __cplusplus
extern "C" {
#endif

void tool_preferences(bool *p_open, th_db_t * db);

style_t *style_get_ptr(void);

#ifdef __cplusplus
}
#endif

#endif

