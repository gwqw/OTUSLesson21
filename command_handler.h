#pragma once
/**@file
    @brief Command handler

    Command handler interface and its implementations
*/

#include <iostream>
#include <string>
#include <memory>
#include <atomic>

#include "bulk.h"

/**
 * @brief Subscriber interface.
 *
 * Command handlers subscribe to command manager and obtain data from it via update method
 */
class IObserver {
public:
    virtual void update(BulkCmdHolder bulk_holder) = 0;
    virtual ~IObserver() = default;
};


/**
 * @brief Command handler implementation. Outputs to stream.
 *
 * It ouputs commands to stream with "bulk: " header. All commands are ", " separated
 * All commands output ends with eol
 */
class CmdStreamHandler : public IObserver {
public:
    explicit CmdStreamHandler(std::ostream& out = std::cout)
        : out_(out) {}

    void update(BulkCmdHolder bulk_holder) override;
private:
    std::ostream& out_;
};

/**
 * @brief Command handler implementation. Outputs to file.
 *
 * It ouputs commands to file with name "bulk[TIME].log", where [TIME] -- time of the first command in UNIX format.
 * commands separated with '/n'
 */
class CmdFileHandler : public IObserver {
public:
    void update(BulkCmdHolder bulk_holder) override;
private:
    static std::string getFileName(const BulkCmd& bulk);
    static std::atomic<int> counter_;
};

template <typename T, typename ... Args>
ObserverHolder createObserverAndSubscribe(BulkCmdManager* bulkCmdManager, Args&& ... args) {
    auto res = std::make_shared<T>(std::forward<Args>(args)...);
    bulkCmdManager->subscribe(res);
    return res;
}