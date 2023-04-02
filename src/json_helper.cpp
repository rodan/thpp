
#include <stdlib.h>
#include <stdio.h>
#include <json-c/json.h>
#include "json_helper.h"

char *json_get(struct json_object *new_obj, const char *key)
{
    struct json_object *o = NULL;
    if (!json_object_object_get_ex(new_obj, key, &o)) {
        return NULL;
    } else {
        return (char *)json_object_get_string(o);
    }
}

uint8_t json_getf(struct json_object *new_obj, const char *key, float *value)
{
    struct json_object *o = NULL;
    if (!json_object_object_get_ex(new_obj, key, &o)) {
        return EXIT_FAILURE;
    } else {
        *value = strtof((char *)json_object_get_string(o), NULL);
        return EXIT_SUCCESS;
    }
}

uint8_t json_getd(struct json_object *new_obj, const char *key, double *value)
{
    struct json_object *o = NULL;
    if (!json_object_object_get_ex(new_obj, key, &o)) {
        return EXIT_FAILURE;
    } else {
        *value = strtod((char *)json_object_get_string(o), NULL);
        return EXIT_SUCCESS;
    }
}

uint8_t json_getw(struct json_object *new_obj, const char *key, uint16_t *value)
{
    long ret;
    struct json_object *o = NULL;
    if (!json_object_object_get_ex(new_obj, key, &o)) {
        return EXIT_FAILURE;
    } else {
        ret = strtol((char *)json_object_get_string(o), NULL, 10);
        *value = ret;
        return EXIT_SUCCESS;
    }
}

