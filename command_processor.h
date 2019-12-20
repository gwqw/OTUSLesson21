#pragma once

#include <deque>
#include <utility>
#include <string>

#include "bulk.h"
#include "command_reader.h"

struct CommandProcessor {
    std::deque<std::string> buffer_;
    CmdReaderHolder cmdReader_;
    BulkMgrHolder bulkMgr_;
    explicit CommandProcessor(std::size_t bulk_size)
    : cmdReader_(std::make_unique<QueueReader>(buffer_)),
      bulkMgr_(std::make_unique<BulkCmdManager>(bulk_size))
    {}
    void pushToBuffer(const char* data, std::size_t data_size);
};

void process_all_commands(ICmdReader* cmdReader, BulkCmdManager* bulkMgr);