#ifndef __VRAFT_UTIL_H__
#define __VRAFT_UTIL_H__

#include <random>
#include <vector>
#include <string>

namespace vraft {

namespace util {

int RandomInt(int min, int max);

unsigned int RSHash(const char *str);
void Split(const std::string &s, char separator, std::vector<std::string> &sv, const std::string ignore = "");
std::string LocalTimeString(time_t t);
void ToLower(std::string &str);

bool RemoveDir(const std::string &path);
bool ChildrenOfDir(const std::string &path, std::vector<std::string> &children_paths, std::vector<std::string> &children_names);
bool DirOK(const std::string &path);
bool MakeDir(const std::string &path);
bool RecurMakeDir(const std::string &path);

} // namespace util

} // namespace vraft

#endif
