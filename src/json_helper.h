#ifndef __json_helper_h__
#define __json_helper_h__

#include <json-c/json.h>

char *get(struct json_object *new_obj, const char *key);

#endif
