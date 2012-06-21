#ifndef _SFMVALIDATORS_H
#define _SFMVALIDATORS_H

#include <arpa/inet.h>

static bool ValidatePort(const char* flagname, int32_t value) {
  if (value > 0 && value < 32768)   // value is ok
    return true;
  printf("Invalid value for --%s: %d\n", flagname, (int)value);
  return false;
}

static bool ValidateAddress(const char* flagname, const string& value) {
  if (value.size() == 0)
    return true;
  struct sockaddr_in sa;
  int result = inet_pton(AF_INET, value.c_str(), &(sa.sin_addr));
  if (result > 0)
    return true;
  printf("Invalid value for --%s: %s\n", flagname, value.c_str());
  return false;
}

static bool ValidateThreads(const char* flagname, int32_t value){
  if (value>0 && value <=16 && (value%2)==0)
    return true;
  printf("Invalid value for --%s: %d\n", flagname, (int)value);
  return false;
}

static bool ValidateCache(const char* flagname, int32_t value){
  if (value>256 && value <=32768 && (value%16)==0)
    return true;
  printf("Invalid value for --%s: %d\nMust be between 256 and 32768 and divisible by 16.\n", flagname, (int)value);
  return false;
}

static bool ValidateHashWidth(const char* flagname, int32_t value){
  if (value>=16 && value <=32)
    return true;
  printf("Invalid value for --%s: %d\n", flagname, (int)value);
  return false;
}

static bool ValidateWindowSize(const char* flagname, int32_t value){
  if (value>10)
    return true;
  printf("Invalid value for --%s: %d\n Must be greater than 10.\n", flagname, (int)value);
  return false;
}

static bool ValidateWhiteSpaceThreshold(const char* flagname,double value){
  if (value>=0.0 && value <=1.0)
    return true;
  printf("Invalid value for --%s: %f\n", flagname, (double)value);
  return false;
}

static bool ValidateSlotCount(const char* flagname,int32_t value){
  if (value>0 && (uint32_t)value && !((uint32_t)value & ((uint32_t)value - 1)))
    return true;
  printf("Invalid value for --%s: %d\nMust be a power of 2\n", flagname, (int32_t)value);
  return false;
}
#endif
