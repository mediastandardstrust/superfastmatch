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
  struct sockaddr_in sa;
  int result = inet_pton(AF_INET, value.c_str(), &(sa.sin_addr));
  if (result != 0)
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

static bool ValidateHashWidth(const char* flagname, int32_t value){
  if (value>=16 && value <=32)
    return true;
  printf("Invalid value for --%s: %d\n", flagname, (int)value);
  return false;

}

static bool ValidateWindowSize(const char* flagname, int32_t value){
  if (value>0 && value <999)
    return true;
  printf("Invalid value for --%s: %d\n", flagname, (int)value);
  return false;

}

#endif
