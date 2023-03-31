#ifndef __json_helper_h__
#define __json_helper_h__

#include <json-c/json.h>

#ifdef __cplusplus
extern "C" {
#endif

char *json_get(struct json_object *new_obj, const char *key);
uint8_t json_getf(struct json_object *new_obj, const char *key, float *value);
uint8_t json_getd(struct json_object *new_obj, const char *key, double *value);
uint8_t json_getw(struct json_object *new_obj, const char *key, uint16_t *value);

#ifdef __cplusplus
}
#endif

#endif
