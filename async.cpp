#include "async.h"

#include <vector>
#include <utility>
//#include <list>

#include "bulk.h"
#include "command_handler.h"
#include "command_processor.h"

using namespace std;

namespace async {

//static list<unique_ptr<CommandProcessor>> bulkmgrs;
static vector<unique_ptr<CommandProcessor>> bulkmgrs;

handle_t connect(std::size_t bulk_size) {
    auto dataProcessor = make_unique<CommandProcessor>(bulk_size);
    createObserverAndSubscribe<CmdStreamHandler>(dataProcessor->getBulkMgr().get());
    createObserverAndSubscribe<CmdFileHandler>(dataProcessor->getBulkMgr().get());
    bulkmgrs.push_back(move(dataProcessor));
//    return reinterpret_cast<void*>(bulkmgrs.back().get());
    return reinterpret_cast<void*>(bulkmgrs.size()-1);
}

void receive(handle_t handle, const char *data, std::size_t size) {
    auto idx = reinterpret_cast<size_t>(handle);
    auto& dataProcessor = *bulkmgrs[idx];
//    auto& dataProcessor = *reinterpret_cast<CommandProcessor*>(handle);
    auto& cmdReader = dataProcessor.getcmdReader();
    auto& bulkMgr = dataProcessor.getBulkMgr();
    dataProcessor.pushToBuffer(data, size);
    process_all_commands(cmdReader, *bulkMgr);
}

void disconnect(handle_t handle) {
//    auto& dataProcessor = *reinterpret_cast<CommandProcessor*>(handle);
    auto idx = reinterpret_cast<size_t>(handle);
    auto& dataProcessor = *bulkmgrs[idx];
    auto& buffer = dataProcessor.getBuffer();
    auto& cmdReader = dataProcessor.getcmdReader();
    auto& bulkMgr = dataProcessor.getBulkMgr();
    if (!buffer.empty()) {
        auto cmd = cmdReader.read_next_cmd();
        bulkMgr->add_cmd(move(cmd));
    }
    Command cmd{CommandType::Terminator};
    bulkMgr->add_cmd(move(cmd));
    bulkmgrs[idx] = nullptr;
}

}