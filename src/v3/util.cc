#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <cassert>
#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <glog/logging.h>
#include "util.h"

namespace vraft {

namespace util {

static std::default_random_engine random_(time(nullptr));

int RandomInt(int min, int max) {
    std::uniform_int_distribution<int> random_range(min, max);
    int n = random_range(random_);
    return n;
}

unsigned int RSHash(const char *str) {
    unsigned int b = 378551;
    unsigned int a = 63689;
    unsigned int hash = 0;

    while (*str) {
        hash = hash * a + (*str++);
        a *= b;
    }
    return (hash & 0x7FFFFFFF);
}

void
Split(const std::string &s, char separator, std::vector<std::string> &sv, const std::string ignore) {
    sv.clear();

    std::set<char> ignore_chars;
    for (auto c : ignore) {
        ignore_chars.insert(c);
    }

    std::string sub_str;
    for (auto c : s) {
        auto it = ignore_chars.find(c);
        if (it != ignore_chars.end()) {
            continue;
        }

        if (c != separator) {
            sub_str.push_back(c);
        } else {
            if (!sub_str.empty()) {
                sv.push_back(sub_str);
            }
            sub_str.clear();
        }
    }

    if (!sub_str.empty()) {
        sv.push_back(sub_str);
    }
}

bool
RemoveDir(const std::string &path) {
    std::string cmd = "rm -rf ";
    cmd.append(path);
    int ret = system(cmd.c_str());
    return ret == 0;
}

bool
ChildrenOfDir(const std::string &path, std::vector<std::string> &children_paths, std::vector<std::string> &children_names) {
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        return false;
    }

    children_paths.clear();
    children_names.clear();
    dirent *direntp;
    while ((direntp = readdir(dir)) != nullptr) {
        if (std::string(direntp->d_name) == "." || std::string(direntp->d_name) == "..") {
            continue;
        }

        std::string temp_path = path;
        temp_path.append("/").append(direntp->d_name);
        children_paths.push_back(temp_path);
        children_names.push_back(std::string(direntp->d_name));
        //std::cout << temp_path << std::endl;
    }

    closedir(dir);
    return true;
}

std::string
LocalTimeString(time_t t) {
    tm* local = localtime(&t); // to loal time
    char buf[128];
    memset(buf, 0, sizeof(buf));
    strftime(buf, 64, "%Y-%m-%d %H:%M:%S", local);
    return std::string(buf);
}

bool
DirOK(const std::string &path) {
    DIR* dir = opendir(path.c_str());
    if (dir != nullptr) {
        closedir(dir);
        return true;
    } else {
        return false;
    }
}

bool
MakeDir(const std::string &path) {
    int ret = mkdir(path.c_str(), 0775);
    return ret == 0;
}

bool
RecurMakeDir(const std::string &path) {
    std::vector<std::string> sv;
    Split(path, '/', sv, "");

    bool b;
    std::string s = "/";
    for (auto &level : sv) {
        s.append(level);
        b = MakeDir(s);
        s.append("/");
    }
    return b;
}

void
ToLower(std::string &str) {
    for (size_t i = 0; i < str.size(); i++)
        str[i] = tolower(str[i]);
}

} // namespace util

} // namespace vraft
