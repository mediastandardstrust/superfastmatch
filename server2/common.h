#ifndef _SFMCOMMON_H                       // duplication check
#define _SFMCOMMON_H

#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

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

extern "C"
{
#ifdef WIN32
#include <Rpc.h>
#else
#include <uuid/uuid.h>
#endif
}

std::string newUUID()
{
#ifdef WIN32
    UUID uuid;
    UuidCreate ( &uuid );

    unsigned char * str;
    UuidToStringA ( &uuid, &str );

    std::string s( ( char* ) str );

    RpcStringFreeA ( &str );
#else
    uuid_t uuid;
    uuid_generate_random ( uuid );
    char s[37];
    uuid_unparse ( uuid, s );
#endif
    return s;
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