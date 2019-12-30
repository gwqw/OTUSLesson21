#include "bulk.h"

#include <utility>

#include "command_handler.h"

using namespace std;

BulkCmdManager::BulkCmdManager(std::size_t bulk_max_size)
    : bulk_capacity_(bulk_max_size),
      bulk_handler_(std::make_unique<GeneralStateHandler>())
#ifdef MULTI_THREAD
    , cout_tread_pool_(1),
      file_tread_pool_(THREADS_NUM)
#endif
{
}

void BulkCmdManager::subscribe(ObserverHolder obs) {
    subs_.push_back(move(obs));
}

void BulkCmdManager::add_cmd(Command cmd) {
    bulk_handler_->handle_cmd(this, std::move(cmd));
}

void BulkCmdManager::flush_data() {
    if (cur_bulk_.empty()) return;
    auto data_ptr = make_shared<BulkCmd>(std::move(cur_bulk_));
    cur_bulk_.clear();
    notify(std::move(data_ptr));
}

void BulkCmdManager::notify(BulkCmdHolder bulk_cmd) {
#ifdef MULTI_THREAD
    try {
        auto update_func = [](const shared_ptr<IObserver>& subscriber,
                              BulkCmdHolder cmd) {
            subscriber->update(move(cmd));
        };

        // console output
        const auto &log_subs = subs_.at(0);
        cout_tread_pool_.addTask(update_func, log_subs, bulk_cmd);

        // file output
        if (subs_.size() > 1) {
            const auto &file_subs = subs_[1];
            file_tread_pool_.addTask(update_func, file_subs, bulk_cmd);
        }

        // other subscribers in the same thread
        for (size_t i = 2; i < subs_.size(); ++i) {
            subs_[i]->update(bulk_cmd);
        }
    } catch (const out_of_range& e) {
        cerr << "There is no subscribers" << endl;
    }
#else
    for (const auto& s : subs_) {
        s->update(bulk_cmd);
    }
#endif
}

void GeneralStateHandler::handle_cmd(BulkCmdManager *m, Command cmd) {
    switch (cmd.cmd_type) {
        case CommandType::Base:
            if (m->cur_bulk_.empty()) {m->cur_bulk_.time_ = time(nullptr);}
            m->cur_bulk_.data_.push_back(std::move(cmd));
            if (m->cur_bulk_.data_.size() == m->bulk_capacity_) {m->flush_data();}
            break;
        case CommandType::StartCustomBulk:
            m->flush_data();
            m->bulk_handler_ = make_unique<CustomStateHandler>();
            ++m->nesting_counter_;
            break;
        case CommandType::StopCustomBulk:
            break;
        case CommandType::Terminator:
            m->flush_data();
            break;
    }
}

void CustomStateHandler::handle_cmd(BulkCmdManager *m, Command cmd) {
    switch (cmd.cmd_type) {
        case CommandType::Base:
            if (m->cur_bulk_.empty()) {m->cur_bulk_.time_ = time(nullptr);}
            m->cur_bulk_.data_.push_back(std::move(cmd));
            break;
        case CommandType::StartCustomBulk:
            ++m->nesting_counter_;
            break;
        case CommandType::StopCustomBulk:
            --m->nesting_counter_;
            if (m->nesting_counter_ == 0) {
                m->flush_data();
                m->bulk_handler_ = make_unique<GeneralStateHandler>();
            }
            break;
        case CommandType::Terminator:
            //empty buffer
            m->cur_bulk_.clear();
            break;
    }
}
