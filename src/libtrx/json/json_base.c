#include "json.h"

#include "memory.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static JSON_NUMBER *M_NumberNewInt(int number);
static JSON_NUMBER *M_NumberNewInt64(int64_t number);
static JSON_NUMBER *M_NumberNewDouble(double number);
static void M_NumberFree(JSON_NUMBER *num);

static JSON_STRING *M_StringNew(const char *string);
static void M_StringFree(JSON_STRING *str);

static JSON_VALUE *M_ValueFromNumber(JSON_NUMBER *const num);
static const JSON_NUMBER *M_ValueAsNumber(const JSON_VALUE *value);
static const JSON_STRING *M_ValueAsString(const JSON_VALUE *value);
static JSON_OBJECT *M_ValueAsObject(JSON_VALUE *const value);
static JSON_ARRAY *M_ValueAsArray(JSON_VALUE *const value);

static void M_ArrayElementFree(JSON_ARRAY_ELEMENT *element);
static void M_ObjectElementFree(JSON_OBJECT_ELEMENT *element);

static JSON_NUMBER *M_NumberNewInt(const int number)
{
    const size_t size = snprintf(NULL, 0, "%d", number) + 1;
    char *const buf = Memory_Alloc(size);
    sprintf(buf, "%d", number);
    JSON_NUMBER *const elem = Memory_Alloc(sizeof(JSON_NUMBER));
    elem->number = buf;
    elem->number_size = strlen(buf);
    return elem;
}

static JSON_NUMBER *M_NumberNewInt64(const int64_t number)
{
    const size_t size = snprintf(NULL, 0, "%" PRId64, number) + 1;
    char *const buf = Memory_Alloc(size);
    sprintf(buf, "%" PRId64, number);
    JSON_NUMBER *const elem = Memory_Alloc(sizeof(JSON_NUMBER));
    elem->number = buf;
    elem->number_size = strlen(buf);
    return elem;
}

static JSON_NUMBER *M_NumberNewDouble(const double number)
{
    const size_t size = snprintf(NULL, 0, "%f", number) + 1;
    char *const buf = Memory_Alloc(size);
    sprintf(buf, "%f", number);
    JSON_NUMBER *const elem = Memory_Alloc(sizeof(JSON_NUMBER));
    elem->number = buf;
    elem->number_size = strlen(buf);
    return elem;
}

static void M_NumberFree(JSON_NUMBER *const num)
{
    if (num->ref_count == 0) {
        Memory_Free(num->number);
        Memory_Free(num);
    }
}

static JSON_STRING *M_StringNew(const char *const string)
{
    JSON_STRING *const str = Memory_Alloc(sizeof(JSON_STRING));
    str->string = Memory_DupStr(string);
    str->string_size = strlen(string);
    return str;
}

static void M_StringFree(JSON_STRING *const str)
{
    if (str->ref_count == 0) {
        Memory_Free(str->string);
        Memory_Free(str);
    }
}

static JSON_VALUE *M_ValueFromNumber(JSON_NUMBER *const num)
{
    JSON_VALUE *const value = Memory_Alloc(sizeof(JSON_VALUE));
    value->type = JSON_TYPE_NUMBER;
    value->payload = num;
    return value;
}

static const JSON_NUMBER *M_ValueAsNumber(const JSON_VALUE *const value)
{
    if (value == NULL || value->type != JSON_TYPE_NUMBER) {
        return NULL;
    }
    return (const JSON_NUMBER *)value->payload;
}

static const JSON_STRING *M_ValueAsString(const JSON_VALUE *const value)
{
    if (value == NULL || value->type != JSON_TYPE_STRING) {
        return NULL;
    }
    return (const JSON_STRING *)value->payload;
}

static JSON_OBJECT *M_ValueAsObject(JSON_VALUE *const value)
{
    if (value == NULL || value->type != JSON_TYPE_OBJECT) {
        return NULL;
    }
    return (JSON_OBJECT *)value->payload;
}

static JSON_ARRAY *M_ValueAsArray(JSON_VALUE *const value)
{
    if (value == NULL || value->type != JSON_TYPE_ARRAY) {
        return NULL;
    }
    return (JSON_ARRAY *)value->payload;
}

static void M_ArrayElementFree(JSON_ARRAY_ELEMENT *const element)
{
    if (element->ref_count == 0) {
        Memory_Free(element);
    }
}

static void M_ObjectElementFree(JSON_OBJECT_ELEMENT *element)
{
    if (element->ref_count == 0) {
        Memory_FreePointer(&element);
    }
}

JSON_VALUE *JSON_ValueFromBool(const int b)
{
    JSON_VALUE *const value = Memory_Alloc(sizeof(JSON_VALUE));
    value->type = b ? JSON_TYPE_TRUE : JSON_TYPE_FALSE;
    value->payload = NULL;
    return value;
}

JSON_VALUE *JSON_ValueFromInt(const int number)
{
    return M_ValueFromNumber(M_NumberNewInt(number));
}

JSON_VALUE *JSON_ValueFromInt64(const int64_t number)
{
    return M_ValueFromNumber(M_NumberNewInt64(number));
}

JSON_VALUE *JSON_ValueFromDouble(const double number)
{
    return M_ValueFromNumber(M_NumberNewDouble(number));
}

JSON_VALUE *JSON_ValueFromString(const char *const string)
{
    JSON_VALUE *const value = Memory_Alloc(sizeof(JSON_VALUE));
    value->type = JSON_TYPE_STRING;
    value->payload = M_StringNew(string);
    return value;
}

JSON_VALUE *JSON_ValueFromArray(JSON_ARRAY *const arr)
{
    JSON_VALUE *const value = Memory_Alloc(sizeof(JSON_VALUE));
    value->type = JSON_TYPE_ARRAY;
    value->payload = arr;
    return value;
}

JSON_VALUE *JSON_ValueFromObject(JSON_OBJECT *const obj)
{
    JSON_VALUE *const value = Memory_Alloc(sizeof(JSON_VALUE));
    value->type = JSON_TYPE_OBJECT;
    value->payload = obj;
    return value;
}

void JSON_ValueFree(JSON_VALUE *const value)
{
    if (value == NULL || value->ref_count != 0) {
        return;
    }

    switch (value->type) {
    case JSON_TYPE_NUMBER:
        M_NumberFree((JSON_NUMBER *)value->payload);
        break;
    case JSON_TYPE_STRING:
        M_StringFree((JSON_STRING *)value->payload);
        break;
    case JSON_TYPE_ARRAY:
        JSON_ArrayFree((JSON_ARRAY *)value->payload);
        break;
    case JSON_TYPE_OBJECT:
        JSON_ObjectFree((JSON_OBJECT *)value->payload);
        break;
    case JSON_TYPE_TRUE:
    case JSON_TYPE_NULL:
    case JSON_TYPE_FALSE:
        break;
    }

    Memory_Free(value);
}

bool JSON_ValueIsNull(const JSON_VALUE *const value)
{
    return value != NULL && value->type == JSON_TYPE_NULL;
}

bool JSON_ValueIsTrue(const JSON_VALUE *const value)
{
    return value != NULL && value->type == JSON_TYPE_TRUE;
}

bool JSON_ValueIsFalse(const JSON_VALUE *const value)
{
    return value != NULL && value->type == JSON_TYPE_FALSE;
}

int JSON_ValueGetBool(const JSON_VALUE *const value, const int d)
{
    if (JSON_ValueIsTrue(value)) {
        return 1;
    } else if (JSON_ValueIsFalse(value)) {
        return 0;
    }
    return d;
}

int JSON_ValueGetInt(const JSON_VALUE *const value, const int d)
{
    const JSON_NUMBER *const num = M_ValueAsNumber(value);
    return num != NULL ? atoi(num->number) : d;
}

int64_t JSON_ValueGetInt64(const JSON_VALUE *const value, const int64_t d)
{
    const JSON_NUMBER *const num = M_ValueAsNumber(value);
    return num != NULL ? strtoll(num->number, NULL, 10) : d;
}

double JSON_ValueGetDouble(const JSON_VALUE *const value, const double d)
{
    const JSON_NUMBER *const num = M_ValueAsNumber(value);
    return num != NULL ? atof(num->number) : d;
}

const char *JSON_ValueGetString(
    const JSON_VALUE *const value, const char *const d)
{
    const JSON_STRING *const str = M_ValueAsString(value);
    return str != NULL ? str->string : d;
}

JSON_ARRAY *JSON_ValueAsArray(JSON_VALUE *const value)
{
    return M_ValueAsArray(value);
}

JSON_OBJECT *JSON_ValueAsObject(JSON_VALUE *const value)
{
    return M_ValueAsObject(value);
}

JSON_ARRAY *JSON_ArrayNew(void)
{
    JSON_ARRAY *const arr = Memory_Alloc(sizeof(JSON_ARRAY));
    arr->start = NULL;
    arr->length = 0;
    return arr;
}

void JSON_ArrayFree(JSON_ARRAY *const arr)
{
    JSON_ARRAY_ELEMENT *elem = arr->start;
    while (elem) {
        JSON_ARRAY_ELEMENT *const next = elem->next;
        JSON_ValueFree(elem->value);
        M_ArrayElementFree(elem);
        elem = next;
    }
    if (arr->ref_count == 0) {
        Memory_Free(arr);
    }
}

void JSON_ArrayAppend(JSON_ARRAY *const arr, JSON_VALUE *const value)
{
    JSON_ARRAY_ELEMENT *elem = Memory_Alloc(sizeof(JSON_ARRAY_ELEMENT));
    elem->value = value;
    elem->next = NULL;
    if (arr->start) {
        JSON_ARRAY_ELEMENT *target = arr->start;
        while (target->next) {
            target = target->next;
        }
        target->next = elem;
    } else {
        arr->start = elem;
    }
    arr->length++;
}

void JSON_ArrayAppendBool(JSON_ARRAY *arr, int b)
{
    JSON_ArrayAppend(arr, JSON_ValueFromBool(b));
}

void JSON_ArrayAppendInt(JSON_ARRAY *arr, int number)
{
    JSON_ArrayAppend(arr, JSON_ValueFromInt(number));
}

void JSON_ArrayAppendDouble(JSON_ARRAY *arr, double number)
{
    JSON_ArrayAppend(arr, JSON_ValueFromDouble(number));
}

void JSON_ArrayAppendString(JSON_ARRAY *arr, const char *string)
{
    JSON_ArrayAppend(arr, JSON_ValueFromString(string));
}

void JSON_ArrayAppendArray(JSON_ARRAY *arr, JSON_ARRAY *arr2)
{
    JSON_ArrayAppend(arr, JSON_ValueFromArray(arr2));
}

void JSON_ArrayAppendObject(JSON_ARRAY *arr, JSON_OBJECT *obj)
{
    JSON_ArrayAppend(arr, JSON_ValueFromObject(obj));
}

JSON_VALUE *JSON_ArrayGetValue(JSON_ARRAY *const arr, const size_t idx)
{
    if (arr == NULL || idx >= arr->length) {
        return NULL;
    }
    JSON_ARRAY_ELEMENT *elem = arr->start;
    for (size_t i = 0; i < idx; i++) {
        elem = elem->next;
    }
    return elem->value;
}

int JSON_ArrayGetBool(
    const JSON_ARRAY *const arr, const size_t idx, const int d)
{
    const JSON_VALUE *const value = JSON_ArrayGetValue((JSON_ARRAY *)arr, idx);
    return JSON_ValueGetBool(value, d);
}

int JSON_ArrayGetInt(const JSON_ARRAY *const arr, const size_t idx, const int d)
{
    const JSON_VALUE *const value = JSON_ArrayGetValue((JSON_ARRAY *)arr, idx);
    return JSON_ValueGetInt(value, d);
}

double JSON_ArrayGetDouble(
    const JSON_ARRAY *const arr, const size_t idx, const double d)
{
    const JSON_VALUE *const value = JSON_ArrayGetValue((JSON_ARRAY *)arr, idx);
    return JSON_ValueGetDouble(value, d);
}

const char *JSON_ArrayGetString(
    const JSON_ARRAY *const arr, const size_t idx, const char *const d)
{
    const JSON_VALUE *const value = JSON_ArrayGetValue((JSON_ARRAY *)arr, idx);
    return JSON_ValueGetString(value, d);
}

JSON_ARRAY *JSON_ArrayGetArray(JSON_ARRAY *arr, const size_t idx)
{
    JSON_VALUE *const value = JSON_ArrayGetValue(arr, idx);
    return JSON_ValueAsArray(value);
}

JSON_OBJECT *JSON_ArrayGetObject(JSON_ARRAY *arr, const size_t idx)
{
    JSON_VALUE *const value = JSON_ArrayGetValue(arr, idx);
    return JSON_ValueAsObject(value);
}

JSON_OBJECT *JSON_ObjectNew(void)
{
    JSON_OBJECT *obj = Memory_Alloc(sizeof(JSON_OBJECT));
    obj->start = NULL;
    obj->length = 0;
    return obj;
}

void JSON_ObjectFree(JSON_OBJECT *obj)
{
    JSON_OBJECT_ELEMENT *elem = obj->start;
    while (elem) {
        JSON_OBJECT_ELEMENT *next = elem->next;
        M_StringFree(elem->name);
        JSON_ValueFree(elem->value);
        M_ObjectElementFree(elem);
        elem = next;
    }
    if (obj->ref_count == 0) {
        Memory_Free(obj);
    }
}

void JSON_ObjectAppend(
    JSON_OBJECT *const obj, const char *const key, JSON_VALUE *const value)
{
    JSON_OBJECT_ELEMENT *elem = Memory_Alloc(sizeof(JSON_OBJECT_ELEMENT));
    elem->name = M_StringNew(key);
    elem->value = value;
    elem->next = NULL;
    if (obj->start) {
        JSON_OBJECT_ELEMENT *target = obj->start;
        while (target->next) {
            target = target->next;
        }
        target->next = elem;
    } else {
        obj->start = elem;
    }
    obj->length++;
}

void JSON_ObjectAppendBool(JSON_OBJECT *obj, const char *key, int b)
{
    JSON_ObjectAppend(obj, key, JSON_ValueFromBool(b));
}

void JSON_ObjectAppendInt(JSON_OBJECT *obj, const char *key, int number)
{
    JSON_ObjectAppend(obj, key, JSON_ValueFromInt(number));
}

void JSON_ObjectAppendInt64(JSON_OBJECT *obj, const char *key, int64_t number)
{
    JSON_ObjectAppend(obj, key, JSON_ValueFromInt64(number));
}

void JSON_ObjectAppendDouble(JSON_OBJECT *obj, const char *key, double number)
{
    JSON_ObjectAppend(obj, key, JSON_ValueFromDouble(number));
}

void JSON_ObjectAppendString(
    JSON_OBJECT *obj, const char *key, const char *string)
{
    JSON_ObjectAppend(obj, key, JSON_ValueFromString(string));
}

void JSON_ObjectAppendArray(JSON_OBJECT *obj, const char *key, JSON_ARRAY *arr)
{
    JSON_ObjectAppend(obj, key, JSON_ValueFromArray(arr));
}

void JSON_ObjectAppendObject(
    JSON_OBJECT *obj, const char *key, JSON_OBJECT *obj2)
{
    JSON_ObjectAppend(obj, key, JSON_ValueFromObject(obj2));
}

bool JSON_ObjectContainsKey(JSON_OBJECT *const obj, const char *const key)
{
    JSON_OBJECT_ELEMENT *elem = obj->start;
    while (elem != NULL) {
        if (!strcmp(elem->name->string, key)) {
            return true;
        }

        elem = elem->next;
    }

    return false;
}

void JSON_ObjectEvictKey(JSON_OBJECT *const obj, const char *const key)
{
    if (obj == NULL) {
        return;
    }
    JSON_OBJECT_ELEMENT *elem = obj->start;
    JSON_OBJECT_ELEMENT *prev = NULL;
    while (elem) {
        if (!strcmp(elem->name->string, key)) {
            if (prev == NULL) {
                obj->start = elem->next;
            } else {
                prev->next = elem->next;
            }
            M_ObjectElementFree(elem);
            return;
        }
        prev = elem;
        elem = elem->next;
    }
}

void JSON_ObjectMerge(JSON_OBJECT *const root, const JSON_OBJECT *const obj)
{
    JSON_OBJECT_ELEMENT *elem = obj->start;
    while (elem != NULL) {
        JSON_ObjectEvictKey(root, elem->name->string);
        JSON_ObjectAppend(root, elem->name->string, elem->value);
        elem = elem->next;
    }
}

JSON_VALUE *JSON_ObjectGetValue(JSON_OBJECT *const obj, const char *const key)
{
    if (obj == NULL) {
        return NULL;
    }
    JSON_OBJECT_ELEMENT *elem = obj->start;
    while (elem) {
        if (!strcmp(elem->name->string, key)) {
            return elem->value;
        }
        elem = elem->next;
    }
    return NULL;
}

int JSON_ObjectGetBool(
    const JSON_OBJECT *const obj, const char *const key, const int d)
{
    const JSON_VALUE *const value =
        JSON_ObjectGetValue((JSON_OBJECT *)obj, key);
    return JSON_ValueGetBool(value, d);
}

int JSON_ObjectGetInt(
    const JSON_OBJECT *const obj, const char *const key, const int d)
{
    const JSON_VALUE *const value =
        JSON_ObjectGetValue((JSON_OBJECT *)obj, key);
    return JSON_ValueGetInt(value, d);
}

int64_t JSON_ObjectGetInt64(
    const JSON_OBJECT *const obj, const char *const key, const int64_t d)
{
    const JSON_VALUE *const value =
        JSON_ObjectGetValue((JSON_OBJECT *)obj, key);
    return JSON_ValueGetInt64(value, d);
}

double JSON_ObjectGetDouble(
    const JSON_OBJECT *const obj, const char *const key, const double d)
{
    const JSON_VALUE *const value =
        JSON_ObjectGetValue((JSON_OBJECT *)obj, key);
    return JSON_ValueGetDouble(value, d);
}

const char *JSON_ObjectGetString(
    const JSON_OBJECT *const obj, const char *const key, const char *const d)
{
    const JSON_VALUE *const value =
        JSON_ObjectGetValue((JSON_OBJECT *)obj, key);
    return JSON_ValueGetString(value, d);
}

JSON_ARRAY *JSON_ObjectGetArray(JSON_OBJECT *obj, const char *key)
{
    JSON_VALUE *const value = JSON_ObjectGetValue(obj, key);
    return JSON_ValueAsArray(value);
}

JSON_OBJECT *JSON_ObjectGetObject(JSON_OBJECT *obj, const char *key)
{
    JSON_VALUE *const value = JSON_ObjectGetValue(obj, key);
    return JSON_ValueAsObject(value);
}
