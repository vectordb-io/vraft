#include "jsonxx/json.hpp"
#include "config.h"

namespace vraft {

Config::Config()
    :ping_(false),
     election_timeout_(0),
     heartbeat_timeout_(0) {
}

Config::~Config() {
}

Status
Config::Init(int argc, char **argv) {
    int option_index, option_value;
    option_index = 0;
    static struct option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {"ping", no_argument, nullptr, 't'},
        {"peers", required_argument, nullptr, 'p'},
        {"path", required_argument, nullptr, 's'},
        {"me", required_argument, nullptr, 'm'},
        {nullptr, 0, nullptr, 0}
    };

    exe_name_ = std::string(argv[0]);
    while ((option_value = getopt_long(argc, argv, "hp:m:", long_options, &option_index)) != -1) {
        switch (option_value) {
        case 'm':
            ParseMe(std::string(optarg), me_);
            break;

        case 't':
            ping_ = true;
            break;

        case 'p':
            ParsePeers(std::string(optarg), peers_);
            break;

        case 's':
            path_ = std::string(optarg);
            break;

        case 'h':
            PrintHelp();
            exit(0);

        default:
            PrintHelp();
            exit(0);
        }
    }

    addresses_.push_back(me_);
    for (auto hp : peers_) {
        addresses_.push_back(hp);
    }

    election_timeout_ = 150;
    heartbeat_timeout_ = 30;

    return Status::OK();
}

std::string
Config::ToString() const {
    jsonxx::json64 j, jret;
    j["me"] = me_.ToString();
    for (size_t i = 0; i < peers_.size(); ++i) {
        j["peers"][i] = peers_[i].ToString();
    }

    for (size_t i = 0; i < addresses_.size(); ++i) {
        j["addresses"][i] = addresses_[i].ToString();
    }

    j["exe_name"] = exe_name_;
    j["ping"] = ping_;
    j["path"] = path_;
    j["election_timeout"] = election_timeout_;
    j["heartbeat_timeout"] = heartbeat_timeout_;

    jret["Config"] = j;

    return jret.dump(4, ' ');
}

void
Config::PrintHelp() const {
    std::cout << std::endl;
    std::cout << "Usage: " << std::endl;

    std::cout << std::endl;
    std::cout << "normal: " << std::endl;
    std::cout << exe_name_ << " --me=127.0.0.1:38000 --peers=127.0.0.1:38001,127.0.0.1:38002 --path=/tmp/vraft/127.0.0.1:38000" << std::endl;
    std::cout << exe_name_ << " --me=127.0.0.1:38001 --peers=127.0.0.1:38000,127.0.0.1:38002 --path=/tmp/vraft/127.0.0.1:38001" << std::endl;
    std::cout << exe_name_ << " --me=127.0.0.1:38002 --peers=127.0.0.1:38000,127.0.0.1:38001 --path=/tmp/vraft/127.0.0.1:38002" << std::endl;
    std::cout << exe_name_ << " -h" << std::endl;
    std::cout << exe_name_ << " --help" << std::endl;

    std::cout << std::endl;
    std::cout << "ping: " << std::endl;
    std::cout << exe_name_ << " --me=127.0.0.1:38000 --peers=127.0.0.1:38001,127.0.0.1:38002 --path=/tmp/vraft/127.0.0.1:38000 --ping" << std::endl;
    std::cout << exe_name_ << " --me=127.0.0.1:38001 --peers=127.0.0.1:38000,127.0.0.1:38002 --path=/tmp/vraft/127.0.0.1:38001 --ping" << std::endl;
    std::cout << exe_name_ << " --me=127.0.0.1:38002 --peers=127.0.0.1:38000,127.0.0.1:38001 --path=/tmp/vraft/127.0.0.1:38002 --ping" << std::endl;
    std::cout << std::endl;
}

void
Config::ParseHostPort(const std::string &s, std::string &host, int &port) {
    std::vector<std::string> sv;
    util::Split(s, ':', sv, " \t");
    assert(sv.size() == 2);
    host = sv[0];
    sscanf(sv[1].c_str(), "%d", &port);
}

void
Config::ParseOne(const std::string &s, vraft::HostAndPort &hp) {
    std::string host;
    int port;
    ParseHostPort(s, host, port);
    hp.set_host(host);
    hp.set_port(port);
}

void
Config::ParsePeers(const std::string &s, std::vector<vraft::HostAndPort> &v) {
    std::vector<std::string> sv;
    util::Split(s, ',', sv, " \t");
    for (auto &hp_str : sv) {
        vraft::HostAndPort hp;
        ParseOne(hp_str, hp);
        v.push_back(hp);
    }
}

void
Config::ParseMe(const std::string &s, vraft::HostAndPort &hp) {
    ParseOne(s, hp);
}

} // namespace vraft
