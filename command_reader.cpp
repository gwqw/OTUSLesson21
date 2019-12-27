#include "command_reader.h"

#include <utility>
#include <string>

using namespace std;

Command ICmdReader::getCmd(std::string cmd_line) {
    if (cmd_line == "{") {
        return Command{CommandType::StartCustomBulk};
    } else if (cmd_line == "}") {
        return Command{CommandType::StopCustomBulk};
    } else {
        return Command{CommandType::Base, move(cmd_line)};
    }
}

Command StreamCmdReader::read_next_cmd() {
    string cmd_line;
    if (getline(in_, cmd_line)) {
        return getCmd(move(cmd_line));
    } else {
        return Command{CommandType::Terminator};
    }
}

bool StreamCmdReader::hasCmd() {
    return static_cast<bool>(in_);
}

// QueueReader2
Command QueueReader::read_next_cmd() {
    auto cmd_line = move(buffer_.front());
    buffer_.pop_front();
    if (!cmd_line.empty() && cmd_line.back() == '\n') {
        cmd_line.pop_back();
    }
    return getCmd(move(cmd_line));
}

bool QueueReader::hasCmd() {
    return !buffer_.empty() && !buffer_.front().empty()
           && buffer_.front().back() == '\n';
}

bool QueueReader::isCmdComplete() {
    if (!buffer_.empty() && !buffer_.back().empty()) {
        return buffer_.back().back() == '\n';
    } else {
        return true;
    }
}
