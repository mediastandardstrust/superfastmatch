#ifndef _SFMCOMMON_H                       // duplication check
#define _SFMCOMMON_H

#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

#include <kcutil.h>

typedef uint32_t hash_t;

inline hash_t hashmurmur(const void* buf, size_t size) {
  const uint64_t mul = 0xc6a4a7935bd1e995ULL;
  const int32_t rtt = 47;
  uint64_t hash = 19780211ULL ^ (size * mul);
  const unsigned char* rp = (const unsigned char*)buf;
  while (size >= sizeof(uint64_t)) {
    uint64_t num = ((uint64_t)rp[0] << 0) | ((uint64_t)rp[1] << 8) |
        ((uint64_t)rp[2] << 16) | ((uint64_t)rp[3] << 24) |
        ((uint64_t)rp[4] << 32) | ((uint64_t)rp[5] << 40) |
        ((uint64_t)rp[6] << 48) | ((uint64_t)rp[7] << 56);
    num *= mul;
    num ^= num >> rtt;
    num *= mul;
    hash *= mul;
    hash ^= num;
    rp += sizeof(uint64_t);
    size -= sizeof(uint64_t);
  }
  switch (size) {
    case 7: hash ^= (uint64_t)rp[6] << 48;
    case 6: hash ^= (uint64_t)rp[5] << 40;
    case 5: hash ^= (uint64_t)rp[4] << 32;
    case 4: hash ^= (uint64_t)rp[3] << 24;
    case 3: hash ^= (uint64_t)rp[2] << 16;
    case 2: hash ^= (uint64_t)rp[1] << 8;
    case 1: hash ^= (uint64_t)rp[0];
      hash *= mul;
  };
  hash ^= hash >> rtt;
  hash *= mul;
  hash ^= hash >> rtt;
  return hash;
}

inline std::string unitnumstr(int64_t num) {
  if (num >= std::pow(1000.0, 6)) {
    return kc::strprintf("%.3Lf quintillion", (long double)num / std::pow(1000.0, 6));
  } else if (num >= std::pow(1000.0, 5)) {
    return kc::strprintf("%.3Lf quadrillion", (long double)num / std::pow(1000.0, 5));
  } else if (num >= std::pow(1000.0, 4)) {
    return kc::strprintf("%.3Lf trillion", (long double)num / std::pow(1000.0, 4));
  } else if (num >= std::pow(1000.0, 3)) {
    return kc::strprintf("%.3Lf billion", (long double)num / std::pow(1000.0, 3));
  } else if (num >= std::pow(1000.0, 2)) {
    return kc::strprintf("%.3Lf million", (long double)num / std::pow(1000.0, 2));
  } else if (num >= std::pow(1000.0, 1)) {
    return kc::strprintf("%.3Lf thousand", (long double)num / std::pow(1000.0, 1));
  }
  return kc::strprintf("%lld", (long long)num);
}


// convert a number into the string with the byte unit
inline std::string unitnumstrbyte(int64_t num) {
  if ((unsigned long long)num >= 1ULL << 60) {
    return kc::strprintf("%.3Lf EiB", (long double)num / (1ULL << 60));
  } else if ((unsigned long long)num >= 1ULL << 50) {
    return kc::strprintf("%.3Lf PiB", (long double)num / (1ULL << 50));
  } else if ((unsigned long long)num >= 1ULL << 40) {
    return kc::strprintf("%.3Lf TiB", (long double)num / (1ULL << 40));
  } else if ((unsigned long long)num >= 1ULL << 30) {
    return kc::strprintf("%.3Lf GiB", (long double)num / (1ULL << 30));
  } else if ((unsigned long long)num >= 1ULL << 20) {
    return kc::strprintf("%.3Lf MiB", (long double)num / (1ULL << 20));
  } else if ((unsigned long long)num >= 1ULL << 10) {
    return kc::strprintf("%.3Lf KiB", (long double)num / (1ULL << 10));
  }
  return kc::strprintf("%lld B", (long long)num);
}

template <class C> void FreeClear( C & cntr ) {
    for ( typename C::iterator it = cntr.begin(); it != cntr.end(); ++it ) {
        if (*it!=0){
			delete *it;
			*it=0;
		}
    }
    cntr.clear();
}

#endif