#include <iostream>
#include <future>
#include <thread>
#include <chrono>
#include <vector>
#include <utility>

#include "async.h"

using namespace std::chrono_literals;

/*simplest test*/
int main(int argc, char* argv[]) {
    const std::size_t bulk_size = 1;
    auto h = async::connect(bulk_size);
    async::receive(h, "1\n", 2);
    async::disconnect(h);
    return 0;
}

/*also simple test*/
//int main(int argc, char* argv[]) {
//    const std::size_t bulk_size = 5;
//    auto h = async::connect(bulk_size);
//    async::receive(h, "1", 1);
//    async::receive(h, "\n2\n3\n4\n5\n6\n{\na\n", 15);
//    async::receive(h, "b\nc\nd\n}\n89\n", 11);
//    async::disconnect(h);
//    return 0;
//}


/*simple test*/
//int main(int argc, char* argv[]) {
//    const std::size_t bulk_size = 5;
//    auto h = async::connect(bulk_size);
//    auto h2 = async::connect(bulk_size);
//    async::receive(h, "1", 1);
//    async::receive(h2, "1\n", 2);
//    async::receive(h, "\n2\n3\n4\n5\n6\n{\na\n", 15);
//    async::receive(h, "b\nc\nd\n}\n89\n", 11);
//    async::disconnect(h);
//    async::disconnect(h2);
//    return 0;
//}

/*2 threads test. every context in own thread*/
//int main(int argc, char* argv[]) {
//    const std::size_t bulk_size = 5;
//    auto l1 = [bulk_size](){
//        auto h = async::connect(bulk_size);
//        std::this_thread::sleep_for(1ms);
//        async::receive(h, "1", 1);
//        async::receive(h, "\n2\n3\n4\n5\n6\n{\na\n", 15);
//
//        async::receive(h, "b\nc\nd\n}\n89\n", 11);
//        async::disconnect(h);
//    };
//    auto l2 = [bulk_size](){
//        auto h2 = async::connect(bulk_size);
//        std::this_thread::sleep_for(1ms);
//        async::receive(h2, "1\n", 2);
//        async::disconnect(h2);
//    };
//
//    auto f2 = std::async(l2);
//    auto f1 = std::async(l1);
//
//    f1.get();
//    f2.get();
//
//    return 0;
//}

// n parallel threads
//int main(int argc, char* argv[]) {
//    constexpr std::size_t bulk_size = 5;
//    constexpr int N = 20;
//    auto l = [](){
//        auto h = async::connect(bulk_size);
////        std::this_thread::sleep_for(1ms);
//        async::receive(h, "1", 1);
//        async::receive(h, "\n2\n3\n4\n5\n6\n{\na\n", 15);
//
//        async::receive(h, "b\nc\nd\n}\n89\n", 11);
//        async::disconnect(h);
//    };
//
//    std::vector<std::future<void>> futures;
//    for (int i = 0; i < N; ++i) {
//        futures.push_back(
//                std::async(l)
//        );
//    }
//
//    return 0;
//}

/*3 threads for receive*/
//int main(int argc, char* argv[]) {
//    constexpr std::size_t bulk_size = 5;
//    auto h = async::connect(bulk_size);
//
//    auto l = [h](const char* line, std::size_t size){
//        async::receive(h, line, size);
//    };
//
//    {
//        std::vector<std::future<void>> futures;
//
//        futures.push_back(
//                std::async(l, "1", 1)
//        );
//        //std::this_thread::sleep_for(1ms);
//        futures.push_back(
//                std::async(l, "\n2\n3\n4\n5\n6\n{\na\n", 15)
//        );
//        futures.push_back(
//                std::async(l, "b\nc\nd\n}\n89\n", 11)
//        );
//    }
//
//    async::disconnect(h);
//
//    return 0;
//}