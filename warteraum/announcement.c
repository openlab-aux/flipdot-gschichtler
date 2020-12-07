#include <stdlib.h>
#include <string.h>

#include "announcement.h"

void announcement_new(struct warteraum_announcement *announcement) {
  announcement->text.len = -1;
  announcement->text.buf = NULL;
  announcement->announcement_expires = 0;
  announcement->announcement_expiry = 0;
}

void announcement_delete(struct warteraum_announcement *announcement) {
  if(announcement->text.buf != NULL) {
    free((void *) announcement->text.buf);
  }
  announcement->text.len = -1;
  announcement->text.buf = NULL;

  announcement->announcement_expires = 0;
  announcement->announcement_expiry = 0;
}

bool announcement_set(struct warteraum_announcement *announcement, struct http_string_s s) {
  announcement_delete(announcement);

  char *new_buf = malloc(s.len);

  if(new_buf == NULL) {
    return false;
  }

  memcpy(new_buf, s.buf, s.len);

  announcement->text.len = s.len;
  announcement->text.buf = new_buf;

  announcement->announcement_expires = 0;
  announcement->announcement_expiry = 0;

  return true;
}

bool announcement_set_expiring(struct warteraum_announcement *announcement, struct http_string_s s, time_t t) {
  if(!announcement_set(announcement, s)) {
    return false;
  }

  announcement->announcement_expires = 1;
  announcement->announcement_expiry = t;

  return true;
}

bool announcement_expired(struct warteraum_announcement announcement) {
  if(announcement.announcement_expires) {
    time_t now = time(NULL);
    return now > announcement.announcement_expiry;
  } else {
    return false;
  }
}
