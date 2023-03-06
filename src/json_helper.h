#ifndef __json_helper_h__
#define __json_helper_h__

#include <json-c/json.h>

#ifdef __cplusplus
extern "C" {
#endif

char *get(struct json_object *new_obj, const char *key);

#ifdef __cplusplus
}
#endif

#endif
