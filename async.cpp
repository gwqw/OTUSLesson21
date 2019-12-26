#include "async.h"

#include <utility>
#include <mutex>

#include "bulk.h"
#include "command_handler.h"
#include "command_processor.h"
#include "ts_cont.h"

using namespace std;

namespace async {

static ThreadSafeUnorderedMap<unique_ptr<CommandProcessor>> bulkmgrs;

handle_t connect(std::size_t bulk_size) {
    auto dataProcessor = make_unique<CommandProcessor>(bulk_size);
    createObserverAndSubscribe<CmdStreamHandler>(dataProcessor->getBulkMgr().get());
    createObserverAndSubscribe<CmdFileHandler>(dataProcessor->getBulkMgr().get());
    return reinterpret_cast<void*>(
            bulkmgrs.push(move(dataProcessor))
    );
}

void receive(handle_t handle, const char *data, std::size_t size) {
    auto idx = reinterpret_cast<size_t>(handle);
    if (bulkmgrs.contains(idx)) {
        auto &dataProcessor = *bulkmgrs[idx];
        auto &cmdReader = dataProcessor.getcmdReader();
        auto &bulkMgr = dataProcessor.getBulkMgr();
        auto &mtx = dataProcessor.getMutex();
        lock_guard<mutex> lk(mtx);
        dataProcessor.pushToBuffer(data, size);
        process_all_commands(cmdReader, *bulkMgr);
    }
}

void disconnect(handle_t handle) {
    auto idx = reinterpret_cast<size_t>(handle);
    if (bulkmgrs.contains(idx)) {
        auto &dataProcessor = *bulkmgrs[idx];
        auto &buffer = dataProcessor.getBuffer();
        auto &cmdReader = dataProcessor.getcmdReader();
        auto &bulkMgr = dataProcessor.getBulkMgr();
        auto &mtx = dataProcessor.getMutex();
        {
            lock_guard<mutex> lk(mtx);
            if (!buffer.empty()) {
                auto cmd = cmdReader.read_next_cmd();
                bulkMgr->add_cmd(move(cmd));
            }
            Command cmd{CommandType::Terminator};
            bulkMgr->add_cmd(move(cmd));
        }
        bulkmgrs.erase(idx);
    }
}

}