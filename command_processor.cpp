#include "command_processor.h"

#include <string_view>

using namespace std;

void process_all_commands(ICmdReader& cmdReader, BulkCmdManager& bulkMgr) {
    while (cmdReader.hasCmd()) {
        auto cmd = cmdReader.read_next_cmd();
        bool to_break = cmd.cmd_type == CommandType::Terminator;
        bulkMgr.add_cmd(move(cmd));
        if (to_break) break;
    }
}

void CommandProcessor::pushToBuffer(const char *data, std::size_t data_size) {
    string_view sv(data, data_size);
    auto& buffer = buffer_;
    while (!sv.empty()) {
        auto pos = sv.find('\n');

        if (pos != std::string_view::npos) {
            ++pos;
        }
        if (cmdReader_->isCmdComplete()) {
            buffer.emplace_back(sv.substr(0, pos));
        } else {
            buffer.back() += string(sv.substr(0, pos));
        }
        sv.remove_prefix(min(pos, sv.size()));
    }
}
