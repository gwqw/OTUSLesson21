#include "async.h"

#include <vector>
#include <utility>

#include "command_reader.h"
#include "bulk.h"
#include "command_handler.h"
#include "command_processor.h"

using namespace std;

namespace async {

static vector<CommandProcessor> bulkmgrs;

handle_t connect(std::size_t bulk_size) {
    auto cmdProc = CommandProcessor(bulk_size);
    createObserverAndSubscribe<CmdStreamHandler>(cmdProc.bulkMgr_.get());
    createObserverAndSubscribe<CmdFileHandler>(cmdProc.bulkMgr_.get());
    bulkmgrs.emplace_back(move(cmdProc));
    return reinterpret_cast<void*>(bulkmgrs.size()-1);
}

void receive(handle_t handle, const char *data, std::size_t size) {
    auto idx = reinterpret_cast<size_t>(handle);
    auto& dataProcessor = bulkmgrs[idx];
    auto& [buffer, cmdReader, bulkMgr] = dataProcessor;
    dataProcessor.pushToBuffer(data, size);
    while (cmdReader->hasCmd()) {
        auto cmd = cmdReader->read_next_cmd();
        bool to_break = cmd.cmd_type == CommandType::Terminator;
        bulkMgr->add_cmd(move(cmd));
        if (to_break) break;
    }
}

void disconnect(handle_t handle) {
    auto idx = reinterpret_cast<size_t>(handle);
    auto& dataProcessor = bulkmgrs[idx];
    auto& [buffer, cmdReader, bulkMgr] = dataProcessor;
    if (!buffer.empty()) {
        auto cmd = cmdReader->read_next_cmd();
        bulkMgr->add_cmd(move(cmd));
    }
    Command cmd{CommandType::Terminator};
    bulkMgr->add_cmd(move(cmd));
}

}