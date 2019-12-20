#include "command_handler.h"

#include <fstream>

using namespace std;

#ifdef TEST
int CmdFileHandler::counter_ = 0;
#endif

void CmdStreamHandler::update(BulkCmdHolder bulk) {
    out_ << "bulk: ";
    bool is_first = true;
    for (const auto& c : bulk->data_) {
        if (is_first) {
            is_first = false;
        } else {
            out_ << ", ";
        }
        out_ << c.data;
    }
    out_ << std::endl;
}

void CmdFileHandler::update(BulkCmdHolder bulk) {
    ofstream out(getFileName(*bulk));
    for (const auto& c : bulk->data_) {
        out << c.data << '\n';
    }
}

std::string CmdFileHandler::getFileName(const BulkCmd& bulk) {
    return "bulk" + to_string(bulk.time_)
#ifdef TEST
        + "_" + to_string(counter_++)
#endif
        + ".log";
}
