/**
 * \file    misc/cstring_utils.hh
 * \brief   Helpers for null terminated strings
 */

#pragma once

#include <cstring>
#include <rv_allocator.hh>

bool csu_assign(char **dst, const char *src);
void csu_delete(char *p);
void csu_destroy(char **p);
inline const char* csu_get(char *p) {
  return p ? p : "";
}

using CsuAllocator = RvAllocator<char>;

#ifdef HAS_ALLOCATOR_REALLOC
#define USE_REALLOCATE 1
#else
#define USE_REALLOCATE 0
#endif


template<class Allocator = CsuAllocator>
class csut {
  char *mStr = 0;
private:
  bool csu_assign(char **dst, const char *src) {
    if (*dst && std::strcmp(*dst, src) == 0)
      return true;

    size_t src_size = std::strlen(src) + 1;

    if (src_size == 1) {
      csu_destroy(dst);
      return true;
    }

    if constexpr (USE_REALLOCATE) {
      if (char *ptr = Allocator().reallocate(*dst, src_size)) {
        *dst = std::strcpy(ptr, src);
        return true;
      }
    } else {
      csu_destroy(dst);
      if ((*dst = Allocator().allocate(src_size))) {
        std::strcpy(*dst, src);
        return true;
      }
    }

    return false;
  }

  void csu_delete(char *p) {
    Allocator().deallocate(p, 0);
  }

  void csu_destroy(char **p) {
    csu_delete(*p);
    *p = nullptr;
  }


public:
  csut() {
  }
  ~csut() {
    csu_delete(mStr);
  }
  explicit csut(const char *s) {
    csu_assign(&mStr, s);
  }
  csut(csut &other) {
    csu_assign(&mStr, other.mStr);
  }
  csut(csut &&other) noexcept {
    mStr = other.mStr;
    other.mStr = 0;
  }
  csut& operator=(const char *rhs) {
    csu_assign(&mStr, rhs);
    return *this;
  }
  csut& operator=(csut &&rhs) noexcept {
    if (this != &rhs) {
      mStr = rhs.mStr;
      rhs.mStr = 0;
    }
    return *this;
  }
  operator const char *() const {
    return csu_get(mStr);
  }
  bool operator==(csut &rhs) const {
    return std::strcmp(*this, rhs) == 0;
  }
  bool operator!=(csut &rhs) const {
    return !(*this == rhs);
  }
};

using csu = csut<>;
static_assert(sizeof(csu) == sizeof(char *));



