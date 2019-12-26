#pragma once
/**@file
    @brief command reader

    Command reader interface and implemetations
*/

#include <iostream>
#include <memory>
#include <deque>
#include <string>

#include "command.h"

/**
 *  @brief Command reader
 *
 *  Interface for command reader.
 */
class ICmdReader {
public:
    virtual ~ICmdReader() = default;
    virtual Command read_next_cmd() = 0;
    virtual bool hasCmd() = 0;
    virtual bool isCmdComplete() = 0;
protected:
    static Command getCmd(std::string cmd_line);
};
using CmdReaderHolder = std::unique_ptr<ICmdReader>;

/**
 *  @brief Command reader from input stream
 *
 *  Command reader implementation. Obtains stream in ctor and reads command, separated by eol
 */
class StreamCmdReader : public ICmdReader{
public:
    explicit StreamCmdReader(std::istream& in = std::cin) : in_(in) {}
    Command read_next_cmd() override;
    bool hasCmd() override;
    bool isCmdComplete() override {return true;}
private:
    std::istream& in_;
};

class QueueReader : public ICmdReader {
public:
    explicit QueueReader(std::deque<std::string>& buffer)
        : buffer_(buffer)
    {}
    Command read_next_cmd() override;
    bool hasCmd() override;
    bool isCmdComplete() override;
private:
    std::deque<std::string>& buffer_;
};


