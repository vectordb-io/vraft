#ifndef __VRAFT_CONFIG_H__
#define __VRAFT_CONFIG_H__

#include <getopt.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "util.h"
#include "status.h"

namespace vraft {

class HostAndPort {
  public:
    HostAndPort() = default;
    HostAndPort(const std::string &host, int port)
        :host_(host), port_(port) {
    }

    HostAndPort(const HostAndPort&) = default;
    HostAndPort& operator=(const HostAndPort&) = default;
    ~HostAndPort() = default;

    std::string
    ToString() const {
        char buf[32];
        snprintf(buf, sizeof(buf), ":%d", port_);
        return host_ + std::string(buf);
    }

    const std::string& host() const {
        return host_;
    }

    void set_host(const std::string& host) {
        host_ = host;
    }

    int port() const {
        return port_;
    }

    void set_port(int port) {
        port_ = port;
    }

  private:
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

    Status Init(int argc, char **argv);
    std::string ToString() const;
    void PrintHelp() const;

    int Quorum() const {
        int n = addresses_.size();
        return n /  2 + 1;
    }

    const HostAndPort& me() const {
        return me_;
    }

    const std::vector<HostAndPort>& peers() const {
        return peers_;
    }

    const std::vector<HostAndPort>& addresses() const {
        return addresses_;
    }

    const std::string& exe_name() const {
        return exe_name_;
    }

    bool ping() const {
        return ping_;
    }

    const std::string& path() const {
        return path_;
    }

    int election_timeout() const {
        return election_timeout_;
    }

    int heartbeat_timeout() const {
        return heartbeat_timeout_;
    }

  private:
    Config();
    ~Config();

    void ParseHostPort(const std::string &s, std::string &host, int &port);
    void ParseOne(const std::string &s, vraft::HostAndPort &hp);
    void ParsePeers(const std::string &s, std::vector<vraft::HostAndPort> &v);
    void ParseMe(const std::string &s, vraft::HostAndPort &hp);

    HostAndPort me_;
    std::vector<HostAndPort> peers_;
    std::vector<HostAndPort> addresses_;

    std::string exe_name_;
    bool ping_;
    std::string path_;
    int election_timeout_;   // ms
    int heartbeat_timeout_;  // ms
};

} // namespace vraft

#endif
