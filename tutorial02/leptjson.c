#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <string.h>
#include <errno.h>
#include <math.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct {
    const char* json;
}lept_context;


//建议先去看看力扣上的状态机实现，再在那个骨架上实现
static int is_number(const char* s) {
    int size = strlen(s);
    int zero_start = 0;
    int num_flag = 0;
    int dot_flag = 0;
    int e_flag = 0;
    for (int i = 0; i < size && s[i] != ' ' && s[i] != '\t' && s[i] != '\n' && s[i] != '\r'; ++i) {
        if (!num_flag && s[i] == '0')
            zero_start = 1;
        else if (!zero_start && s[i] >= '0' && s[i] <= '9')
            num_flag = 1;
        else if (zero_start && s[i] != '.' && s[i] !=  'e' && s[i] != 'E')
            return 2;
        else if (s[i] == '.' && !dot_flag && !e_flag && (num_flag || zero_start)) {
            dot_flag = 1;
            num_flag = 0;
            zero_start = 0;
        }
        else if ((s[i] == 'e' || s[i] == 'E') && !e_flag && (num_flag || zero_start)) {
            num_flag = 0;
            zero_start = 0;
            e_flag = 1;
        }
        else if (s[i] == '+' && i > 0 && (s[i - 1] == 'e' || s[i - 1] == 'E')) {}
        else if (s[i] == '-' && (i == 0 || s[i - 1] == 'e' || s[i - 1] == 'E')) {}
        else    return 0;
    }
    return num_flag || zero_start;
}

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, const char* raw, int initial){
    int size0 = strlen(raw);
    int size1 = strlen(c->json);
    if (size0 > size1)   return LEPT_PARSE_INVALID_VALUE;
    for (int i = 0; i < size0; ++i){
        if(c->json[i] != raw[i])
            return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += size0;
    v->type = initial;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end;
    /* \TODO validate number */
    //微笑了属于是，记得上次写判断合法数字状态机差点没写死
    //太棒了，看了一位力扣扣友的状态机，我又觉得我行了
    int status = is_number(c->json);
    if (status == 0) {
        return LEPT_PARSE_INVALID_VALUE;
    }
    else if (status == 2) {
        return LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    v->n = strtod(c->json, &end);
    if ((v->n == HUGE_VAL || v->n == -HUGE_VALF) && errno == ERANGE)
        return LEPT_PARSE_NUMBER_TOO_BIG;
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
        default:   return lept_parse_number(c, v);
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
