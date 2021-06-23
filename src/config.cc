#include "config.h"

namespace vraft {

Status
Config::Load(int argc, char **argv) {
    int option_index, option_value;
    option_index = 0;
    static struct option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {"ping", no_argument, nullptr, 't'},  // t means test
        {"peers", required_argument, nullptr, 'p'},
        {"path", required_argument, nullptr, 's'},  // s means storage path
        {"me", required_argument, nullptr, 'm'},
        {nullptr, 0, nullptr, 0}
    };

    vraft::HostAndPort me;
    std::vector<vraft::HostAndPort> peers;
    while ((option_value = getopt_long(argc, argv, "hp:m:", long_options, &option_index)) != -1) {
        switch (option_value) {
        case 'm':
            ParseMe(std::string(optarg), me);
            break;

        case 't':
            ping_ = true;
            break;

        case 'p':
            ParsePeers(std::string(optarg), peers);
            break;

        case 's':
            storage_path_ = std::string(optarg);
            break;

        case 'h':
            return Status::InvalidArgument("-h", "help");
            exit(0);

        default:
            return Status::InvalidArgument("-h", "help");
            exit(0);
        }
    }

    vraft::Config::GetInstance().address_.push_back(std::make_shared<vraft::HostAndPort>(me));
    for (auto hp : peers) {
        vraft::Config::GetInstance().address_.push_back(std::make_shared<vraft::HostAndPort>(hp));
    }

    election_timeout_ = 150;
    heartbeat_timeout_ = 30;

    return Status::OK();
}

} // namespace vraft
