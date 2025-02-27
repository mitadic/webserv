#pragma once

#define STATUS_CODES_N 41

enum e_status_codes {
    CODE_100 = 100,
    CODE_101,
    CODE_200 = 200,
    CODE_201,
    CODE_202,
    CODE_203,
    CODE_204,
    CODE_205,
    CODE_206,
    CODE_300 = 300,
    CODE_301,
    CODE_302,
    CODE_303,
    CODE_304,
    CODE_305,
    CODE_306,
    CODE_307,
    CODE_400 = 400,
    CODE_401,
    CODE_402,
    CODE_403,
    CODE_404,
    CODE_405,
    CODE_406,
    CODE_407,
    CODE_408,
    CODE_409,
    CODE_410,
    CODE_411,
    CODE_412,
    CODE_413,
    CODE_414,
    CODE_415,
    CODE_416,
    CODE_417,
    CODE_500 = 500,
    CODE_501,
    CODE_502,
    CODE_503,
    CODE_504,
    CODE_505
};

const char *status_messages[STATUS_CODES_N];