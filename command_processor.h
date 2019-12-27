#pragma once

#include <deque>
#include <utility>
#include <string>
#include <mutex>

#include "bulk.h"
#include "command_reader.h"

class CommandProcessor {
public:
    explicit CommandProcessor(std::size_t bulk_size)
    :   cmdReader_(std::make_unique<QueueReader>(buffer_)),
        bulkMgr_(std::make_unique<BulkCmdManager>(bulk_size))
    {}

    void pushToBuffer(const char* data, std::size_t data_size);

    // getters
    std::deque<std::string>& getBuffer() {return buffer_;}
    ICmdReader& getcmdReader() {return *cmdReader_;}
    BulkMgrHolder& getBulkMgr() {return bulkMgr_;}
    std::mutex& getMutex() {return mtx_;}
private:
    std::deque<std::string> buffer_;
    std::unique_ptr<ICmdReader> cmdReader_;
    BulkMgrHolder bulkMgr_;
    std::mutex mtx_;
};

void process_all_commands(ICmdReader& cmdReader, BulkCmdManager& bulkMgr);