#ifndef __VRAFT_CONFIG_H__
#define __VRAFT_CONFIG_H__

#include <getopt.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <memory>
#include "status.h"

namespace vraft {

class HostAndPort {
  public:
    HostAndPort() = default;
    HostAndPort(const HostAndPort&) = default;

    HostAndPort(std::string host, int port)
        :host_(host), port_(port) {
    }

    std::string
    ToString() const {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s:%d", host_.c_str(), port_);
        return std::string(buf);
    }

    std::string host_;
    int port_;
};

class Config {
  public:
    static Config&
    GetInstance() {
        static Config instance;
        return instance;
    }

    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    std::string DebugString() const {
        std::string s;
        s.append("\n[\n");
        s.append("address: \n");
        for (auto hp : address_) {
            s.append(hp->ToString());
            s.append("\n");
        }
        s.append("]\n");
        return s;
    }

    int Quorum() const {
        int n = address_.size();
        return n /  2 + 1;
    }

    Status Load(int argc, char **argv);

    std::shared_ptr<HostAndPort>
    MyAddress() const {
        auto it = address_.begin();
        if (it != address_.end()) {
            return *it;
        } else {
            return std::shared_ptr<HostAndPort>();
        }
    }

    bool ping() const {
        return ping_;
    }

    const std::string& storage_path() const {
        return storage_path_;
    }

    // address_[0] is mine
    std::vector<std::shared_ptr<HostAndPort>> address_;

    int election_timeout() const {
        return election_timeout_;
    }

    int heartbeat_timeout() const {
        return heartbeat_timeout_;
    }

  private:
    Config()
        :ping_(false) {
    }

    ~Config() {}

    // intput:
    // hp = 127.0.0.1:38000
    // output:
    // host = 127.0.0.1
    // port = 38000
    void ParseHostPort(std::string &hp, std::string &host, int &port) {
        char* psave = nullptr;
        const char *d = ":";
        char *p;
        p = strtok_r((char*)hp.c_str(), d, &psave);
        host = std::string(p);
        p = strtok_r(nullptr, d, &psave);
        sscanf(p, "%d", &port);
    }

    void ParseOne(std::string s, vraft::HostAndPort &hp) {
        ParseHostPort(s, hp.host_, hp.port_);
    }

    void ParsePeers(std::string s, std::vector<vraft::HostAndPort> &v) {
        char* psave = nullptr;
        const char *d = ",";
        char *p;
        p = strtok_r((char*)s.c_str(), d, &psave);
        while (p) {
            std::string tmp(p);
            vraft::HostAndPort hp;
            ParseOne(tmp, hp);
            v.push_back(hp);
            p = strtok_r(nullptr, d, &psave);
        }
    }

    void ParseMe(std::string s, vraft::HostAndPort &hp) {
        ParseOne(s, hp);
    }

    bool ping_;
    std::string storage_path_;

    int election_timeout_;   // ms
    int heartbeat_timeout_;  // ms
};

} // namespace vraft

#endif
