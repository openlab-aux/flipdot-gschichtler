#include "../third_party/httpserver.h/httpserver.h"

int split_segments(struct http_string_s, struct http_string_s **);

bool segment_match(int, char *, struct http_string_s[], int);

bool segment_match_last(int, char *, struct http_string_s[], int);
