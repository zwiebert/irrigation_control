#pragma once

#include <misc/cstring_utils.hh>
#include <rv/rv.hh>
#include <stddef.h>
#include <stdlib.h>

class RvZone {
  int mLph = 0;
#ifdef USE_ZONE_NAME
  char *mName = NULL;
#endif
public:

  int getLph() const {
    return mLph ? mLph : ZONE_LPH_DEFAULT;
  }
  bool setLph(int lph) {
    mLph = lph;
    return true;
  }

#ifdef USE_ZONE_NAME
  const char* getName() const {
    return mName ? mName : "";
  }
  void setName(const char *name) {
    csu_assign(&mName, name);
  }

  ~RvZone() { csu_destroy(&mName); }
#endif
};
