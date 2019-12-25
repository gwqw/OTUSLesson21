#include "command_handler.h"

#include <fstream>
#include <sstream>

using namespace std;

std::atomic<int> CmdFileHandler::counter_ = 0;

void CmdStreamHandler::update(BulkCmdHolder bulk) {
    stringstream buf;
    buf << "bulk: ";
    bool is_first = true;
    for (const auto& c : bulk->data_) {
        if (is_first) {
            is_first = false;
        } else {
            buf << ", ";
        }
        buf << c.data;
    }
    buf << '\n';
    out_ << buf.str();
    out_.flush();
}

void CmdFileHandler::update(BulkCmdHolder bulk) {
    ofstream out(getFileName(*bulk));
    for (const auto& c : bulk->data_) {
        out << c.data << '\n';
    }
}

std::string CmdFileHandler::getFileName(const BulkCmd& bulk) {
    return "bulk" + to_string(bulk.time_)
        + "_" + to_string(counter_++)
        + ".log";
}
