#include <iostream>

#include "async.h"

int main(int argc, char* argv[]) {
    const std::size_t bulk_size = 5;
    auto h = async::connect(bulk_size);
    auto h2 = async::connect(bulk_size);
    async::receive(h, "1", 1);
    async::receive(h2, "1\n", 2);
    async::receive(h, "\n2\n3\n4\n5\n6\n{\na\n", 15);
    async::receive(h, "b\nc\nd\n}\n89\n", 11);
    async::disconnect(h);
    async::disconnect(h2);

    return 0;
}