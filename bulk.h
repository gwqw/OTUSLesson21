#pragma once
/**@file
    @brief Bulk manager

    Bulk command manager can accumulate commands in inner buffer.
    If buffer size is equal its capacity, it will flush buffer to its subscribers.
*/

#include <vector>
#include <deque>
#include <string>
#include <memory>

#include "command.h"
#include "thread_pool.h"

#define MULTI_THREAD

/// Bulk of commands
struct BulkCmd {
    std::time_t time_ = 0;
    std::deque<Command> data_;
    BulkCmd() = default;
    BulkCmd(std::time_t time, std::deque<Command>&& data)
        : time_(time), data_(std::move(data))
    {}
    [[nodiscard]] bool empty() const {return data_.empty();}
    void clear() {data_.clear();}
    [[nodiscard]] const std::deque<Command>& getData() const {return data_;}
};

/// Command bulk holder
using BulkCmdHolder = std::shared_ptr<BulkCmd>;

class IObserver;
using ObserverHolder = std::shared_ptr<IObserver>;

/**
 *  @brief State machine for Bulk manager
 *
 * Interface for Bulk manager state machine
 */
class BulkStateHandler {
public:
    virtual ~BulkStateHandler() = default;
    virtual void handle_cmd(class BulkCmdManager* m, Command cmd) = 0;
};

/**
 *  @brief Bulk command manager
 *
 * Bulk command manager can accumulate commands in inner buffer (method add_cmd).
 * If buffer size is equal its capacity (max_size), it will flush buffer to its subscribers (notify them).
 * It obtains bulk capacity (max_size) in ctor
 */
class BulkCmdManager {
public:
    friend class GeneralStateHandler;
    friend class CustomStateHandler;
public:
    explicit BulkCmdManager(std::size_t bulk_max_size);
    void subscribe(ObserverHolder obs);
    void add_cmd(Command cmd);
private:
    static constexpr std::size_t THREADS_NUM = 2;
    std::size_t bulk_capacity_ = 0;
    int nesting_counter_ = 0;
    std::vector<ObserverHolder> subs_;
    BulkCmd cur_bulk_;
    std::unique_ptr<BulkStateHandler> bulk_handler_ = nullptr;
#ifdef MULTI_THREAD
    ThreadPool cout_tread_pool_;
    ThreadPool file_tread_pool_;
#endif
    // methods
    void flush_data();
    void notify(BulkCmdHolder bulk_cmd);
};
using BulkMgrHolder = std::unique_ptr<BulkCmdManager>;

/**
 *  @brief State machine for Bulk manager
 *
 * State machine implementation. General state: Bulk manager accumulate to buffer until size < buffer capacity
 */
class GeneralStateHandler : public BulkStateHandler {
public:
    void handle_cmd(BulkCmdManager* m, Command cmd) override;
};

/**
 *  @brief State machine for Bulk manager
 *
 * State machine implementation. Custom state: Bulk manager accumulate to buffer until stop command will arrive
 */
class CustomStateHandler : public BulkStateHandler {
public:
    void handle_cmd(BulkCmdManager* m, Command cmd) override;
};