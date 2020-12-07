#ifndef WARTERAUM_ANNOUNCEMENT_H
#define WARTERAUM_ANNOUNCEMENT_H

#include "../third_party/httpserver.h/httpserver.h"

#include <stdbool.h>
#include <time.h>

struct warteraum_announcement {
  struct http_string_s text;

  bool   announcement_expires;
  time_t announcement_expiry;
};

void announcement_new(struct warteraum_announcement *);

void announcement_delete(struct warteraum_announcement *);

bool announcement_set(struct warteraum_announcement *, struct http_string_s);

bool announcement_set_expiring(struct warteraum_announcement *, struct http_string_s, time_t);

bool announcement_expired(struct warteraum_announcement);
#endif
