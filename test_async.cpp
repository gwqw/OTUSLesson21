#define BOOST_TEST_MODULE async_test_module
#include <boost/test/unit_test.hpp>

#include <sstream>
#include <chrono>

#include "bulk.h"
#include "command_reader.h"
#include "command_handler.h"
#include "command_processor.h"

using namespace std;
using namespace std::chrono_literals;

BOOST_AUTO_TEST_SUITE(first_bulk_test_suite)

    BOOST_AUTO_TEST_CASE(test_1) {
        constexpr int N = 3;
        stringstream in;
        in << "1\n" << "2\n" << "3\n";
        in << "4\n" << "5\n";
        stringstream out;
        {
            auto bulkMgr = make_unique<BulkCmdManager>(N);
            auto commandReader = make_unique<StreamCmdReader>(in);

            createObserverAndSubscribe<CmdStreamHandler>(bulkMgr.get(), out);
            process_all_commands(*commandReader, *bulkMgr);
        }
        string res = "bulk: 1, 2, 3\nbulk: 4, 5\n";
        BOOST_CHECK(out.str() == res);
    }

    BOOST_AUTO_TEST_CASE(test_2) {
        constexpr int N = 3;
        stringstream in;
        in << "1\n" << "2\n" << "{\n"
           << "3\n" << "4\n" << "5\n" << "6\n" << "7\n" << "}\n";
        stringstream out;
        {
            auto bulkMgr = make_unique<BulkCmdManager>(N);
            auto commandReader = make_unique<StreamCmdReader>(in);
            createObserverAndSubscribe<CmdStreamHandler>(bulkMgr.get(), out);

            // start commands cycle
            process_all_commands(*commandReader, *bulkMgr);
        }
        string res = "bulk: 1, 2\nbulk: 3, 4, 5, 6, 7\n";
        BOOST_CHECK(out.str() == res);
    }

    BOOST_AUTO_TEST_CASE(test_3) {
        constexpr int N = 3;
        stringstream in;
        in << "{\n" << "1\n" << "2\n"
           << "{\n" << "3\n" << "4\n" << "}\n"
           << "5\n" << "6\n" << "}\n";
        stringstream out;
        {
            auto bulkMgr = make_unique<BulkCmdManager>(N);
            auto commandReader = make_unique<StreamCmdReader>(in);
            createObserverAndSubscribe<CmdStreamHandler>(bulkMgr.get(), out);

            // start commands cycle
            process_all_commands(*commandReader, *bulkMgr);
        }
        string res = "bulk: 1, 2, 3, 4, 5, 6\n";
        BOOST_CHECK(out.str() == res);
    }

    BOOST_AUTO_TEST_CASE(test_4) {
        constexpr int N = 3;
        stringstream in;
        in << "1\n" << "2\n" << "3\n"
           << "{\n" << "3\n" << "4\n" << "5\n" << "6\n" << "7\n";
        stringstream out;
        {
            auto bulkMgr = make_unique<BulkCmdManager>(N);
            auto commandReader = make_unique<StreamCmdReader>(in);
            createObserverAndSubscribe<CmdStreamHandler>(bulkMgr.get(), out);

            // start commands cycle
            process_all_commands(*commandReader, *bulkMgr);
        }

        string res = "bulk: 1, 2, 3\n";
        BOOST_CHECK(out.str() == res);
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(stream_cmd_reader_test_suite)

    BOOST_AUTO_TEST_CASE(test_Base) {
        {
            stringstream in;
            in << "cmd1\n";
            StreamCmdReader commandReader(in);
            auto cmd = commandReader.read_next_cmd();
            BOOST_CHECK(cmd.cmd_type == CommandType::Base);
            BOOST_CHECK(cmd.data == "cmd1");
        }
        {
            stringstream in;
            in << "cmd1 cmd2\n";
            StreamCmdReader commandReader(in);
            auto cmd = commandReader.read_next_cmd();
            BOOST_CHECK(cmd.cmd_type == CommandType::Base);
            BOOST_CHECK(cmd.data == "cmd1 cmd2");
        }
        {
            stringstream in;
            in << "\n";
            StreamCmdReader commandReader(in);
            auto cmd = commandReader.read_next_cmd();
            BOOST_CHECK(cmd.cmd_type == CommandType::Base);
            BOOST_CHECK(cmd.data.empty());
        }
        {
            stringstream in;
            in << "cmd{\n";
            StreamCmdReader commandReader(in);
            auto cmd = commandReader.read_next_cmd();
            BOOST_CHECK(cmd.cmd_type == CommandType::Base);
            BOOST_CHECK(cmd.data == "cmd{");
        }
        {
            stringstream in;
            in << "cmd}\n";
            StreamCmdReader commandReader(in);
            auto cmd = commandReader.read_next_cmd();
            BOOST_CHECK(cmd.cmd_type == CommandType::Base);
            BOOST_CHECK(cmd.data == "cmd}");
        }
    }

    BOOST_AUTO_TEST_CASE(test_Custom) {
        {
            stringstream in;
            in << "{\n";
            StreamCmdReader commandReader(in);
            auto cmd = commandReader.read_next_cmd();
            BOOST_CHECK(cmd.cmd_type == CommandType::StartCustomBulk);
        }
        {
            stringstream in;
            in << "}\n";
            StreamCmdReader commandReader(in);
            auto cmd = commandReader.read_next_cmd();
            BOOST_CHECK(cmd.cmd_type == CommandType::StopCustomBulk);
        }
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(queue_cmd_reader_test_suite)

    BOOST_AUTO_TEST_CASE(test_Base) {
        {
            deque<string> in {"cmd1\n"};
            QueueReader commandReader(in);
            auto cmd = commandReader.read_next_cmd();
            BOOST_CHECK(cmd.cmd_type == CommandType::Base);
            BOOST_CHECK(cmd.data == "cmd1");
        }
        {
            deque<string> in {"cmd1 cmd2\n"};
            QueueReader commandReader(in);
            auto cmd = commandReader.read_next_cmd();
            BOOST_CHECK(cmd.cmd_type == CommandType::Base);
            BOOST_CHECK(cmd.data == "cmd1 cmd2");
        }
        {
            deque<string> in {"\n"};
            QueueReader commandReader(in);
            auto cmd = commandReader.read_next_cmd();
            BOOST_CHECK(cmd.cmd_type == CommandType::Base);
            BOOST_CHECK(cmd.data.empty());
        }
        {
            deque<string> in {"cmd{\n"};
            QueueReader commandReader(in);
            auto cmd = commandReader.read_next_cmd();
            BOOST_CHECK(cmd.cmd_type == CommandType::Base);
            BOOST_CHECK(cmd.data == "cmd{");
        }
        {
            deque<string> in {"cmd}\n"};
            QueueReader commandReader(in);
            auto cmd = commandReader.read_next_cmd();
            BOOST_CHECK(cmd.cmd_type == CommandType::Base);
            BOOST_CHECK(cmd.data == "cmd}");
        }
        {
            deque<string> in {"cmd1\n", "cmd2\n"};
            QueueReader commandReader(in);
            auto cmd = commandReader.read_next_cmd();
            BOOST_CHECK(cmd.cmd_type == CommandType::Base);
            BOOST_CHECK(cmd.data == "cmd1");
        }
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(async_bulk_test_suite)

    BOOST_AUTO_TEST_CASE(test_1) {
        constexpr int N = 3;
        deque<string> in {"1\n", "2\n", "3\n", "4\n", "5\n"};
        stringstream out;
        {
            auto bulkMgr = make_unique<BulkCmdManager>(N);
            auto commandReader = make_unique<QueueReader>(in);

            createObserverAndSubscribe<CmdStreamHandler>(bulkMgr.get(), out);
            process_all_commands(*commandReader, *bulkMgr);
            bulkMgr->add_cmd(Command{CommandType::Terminator});
        }
        string res = "bulk: 1, 2, 3\nbulk: 4, 5\n";
        BOOST_CHECK(out.str() == res);
    }

    BOOST_AUTO_TEST_CASE(test_2) {
        constexpr int N = 3;
        deque<string> in {"1\n", "2\n", "{\n", "3\n", "4\n", "5\n", "6\n", "7\n", "}\n"};
        stringstream out;
        {
            auto bulkMgr = make_unique<BulkCmdManager>(N);
            auto commandReader = make_unique<QueueReader>(in);
            createObserverAndSubscribe<CmdStreamHandler>(bulkMgr.get(), out);

            // start commands cycle
            process_all_commands(*commandReader, *bulkMgr);
            bulkMgr->add_cmd(Command{CommandType::Terminator});
        }
        string res = "bulk: 1, 2\nbulk: 3, 4, 5, 6, 7\n";
        BOOST_CHECK(out.str() == res);
    }

    BOOST_AUTO_TEST_CASE(test_3) {
        constexpr int N = 3;
        deque<string> in {"{\n", "1\n", "2\n", "{\n", "3\n", "4\n", "}\n", "5\n", "6\n", "}\n"};
        stringstream out;
        {
            auto bulkMgr = make_unique<BulkCmdManager>(N);
            auto commandReader = make_unique<QueueReader>(in);
            createObserverAndSubscribe<CmdStreamHandler>(bulkMgr.get(), out);

            // start commands cycle
            process_all_commands(*commandReader, *bulkMgr);
            bulkMgr->add_cmd(Command{CommandType::Terminator});
        }
        string res = "bulk: 1, 2, 3, 4, 5, 6\n";
        BOOST_CHECK(out.str() == res);
    }

    BOOST_AUTO_TEST_CASE(test_4) {
        constexpr int N = 3;
        deque<string> in {"1\n", "2\n", "3\n", "{\n", "4\n", "5\n", "6\n", "7\n"};
        stringstream out;
        {
            auto bulkMgr = make_unique<BulkCmdManager>(N);
            auto commandReader = make_unique<QueueReader>(in);
            createObserverAndSubscribe<CmdStreamHandler>(bulkMgr.get(), out);

            // start commands cycle
            process_all_commands(*commandReader, *bulkMgr);
            bulkMgr->add_cmd(Command{CommandType::Terminator});
        }

        string res = "bulk: 1, 2, 3\n";
        BOOST_CHECK(out.str() == res);
    }

    BOOST_AUTO_TEST_CASE(test_5) {
        constexpr int N = 5;

        // first receive
        deque<string> in {"1"};
        stringstream out;
        auto commandReader = make_unique<QueueReader>(in);
        auto bulkMgr = make_unique<BulkCmdManager>(N);
        createObserverAndSubscribe<CmdStreamHandler>(bulkMgr.get(), out);
        process_all_commands(*commandReader, *bulkMgr);
        BOOST_CHECK(out.str().empty());

        // second receive
        in.back() += "\n";
        in.emplace_back("2\n");
        in.emplace_back("3\n");
        in.emplace_back("4\n");
        in.emplace_back("5\n");
        in.emplace_back("6\n");
        in.emplace_back("{\n");
        in.emplace_back("a\n");
        process_all_commands(*commandReader, *bulkMgr);
        std::this_thread::sleep_for(100ms);
        string res = "bulk: 1, 2, 3, 4, 5\nbulk: 6\n";
        BOOST_CHECK(out.str() == res);
        out.str("");

        //third recieve
        in.emplace_back("b\n");
        in.emplace_back("c\n");
        in.emplace_back("d\n");
        in.emplace_back("}\n");
        in.emplace_back("89\n");
        process_all_commands(*commandReader, *bulkMgr);
        bulkMgr->add_cmd(Command{CommandType::Terminator});
        std::this_thread::sleep_for(100ms);
        res = "bulk: a, b, c, d\nbulk: 89\n";
        BOOST_CHECK(out.str() == res);
    }

    BOOST_AUTO_TEST_CASE(test_6_terminate_wo_end) {
        constexpr int N = 5;

        deque<string> in {"1\n", "2\n", "3\n", "4"};
        stringstream out;
        auto bulkMgr = make_unique<BulkCmdManager>(N);
        auto commandReader = make_unique<QueueReader>(in);
        createObserverAndSubscribe<CmdStreamHandler>(bulkMgr.get(), out);
        process_all_commands(*commandReader, *bulkMgr);
        if (!in.empty()) {
            auto cmd = commandReader->read_next_cmd();
            bulkMgr->add_cmd(move(cmd));
        }
        bulkMgr->add_cmd(Command{CommandType::Terminator});
        std::this_thread::sleep_for(100ms);
        string res = "bulk: 1, 2, 3, 4\n";
        BOOST_CHECK(out.str() == res);
    }

    BOOST_AUTO_TEST_CASE(test_7_caught_in_the_middle) {
        constexpr int N = 3;

        deque<string> in {"cmd1\n", "cmd"};
        stringstream out;
        auto bulkMgr = make_unique<BulkCmdManager>(N);
        auto commandReader = make_unique<QueueReader>(in);
        createObserverAndSubscribe<CmdStreamHandler>(bulkMgr.get(), out);
        process_all_commands(*commandReader, *bulkMgr);
        std::this_thread::sleep_for(100ms);
        BOOST_CHECK(out.str().empty());

        in.back() += "2\n";
        in.emplace_back("cmd3\n");
        process_all_commands(*commandReader, *bulkMgr);
        std::this_thread::sleep_for(100ms);
        string res = "bulk: cmd1, cmd2, cmd3\n";
        BOOST_CHECK(out.str() == res);
    }

BOOST_AUTO_TEST_SUITE_END()