#pragma once
/**@file
    @brief command.h -- command description
*/

#include <ctime>

enum class CommandType {Base, StartCustomBulk, StopCustomBulk, Terminator};

/**
 *  @brief Command structure
 *
 *  time -- command arrival time
 */
struct Command {
    CommandType cmd_type = CommandType::Base;
    std::string data;
};
