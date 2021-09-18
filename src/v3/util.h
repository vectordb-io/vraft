#ifndef __VRAFT_UTIL_H__
#define __VRAFT_UTIL_H__

#include <random>
#include <vector>
#include <string>

namespace vraft {

namespace util {


int random_int(int min, int max);

unsigned int RSHash(const char *str);
void Split(const std::string &s, char separator, std::vector<std::string> &sv, const std::string ignore = "");

void ToLower(std::string &str);
bool DirOK(const std::string &path);
void Mkdir(const std::string &path);

} // namespace util

} // namespace vraft

#endif
