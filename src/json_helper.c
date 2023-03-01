
#include <json-c/json.h>
#include "json_helper.h"

char *get(struct json_object *new_obj, const char *key)
{
    struct json_object *o = NULL;
    if (!json_object_object_get_ex(new_obj, key, &o)) {
        return "";
    } else {
        return (char *)json_object_get_string(o);
    }
}

