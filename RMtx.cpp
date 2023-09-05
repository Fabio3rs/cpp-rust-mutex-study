#include "RustMutex.hpp"
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

struct SomeCommonStruct {
    int a;
    int b;

    auto &operator+=(int i) {
        a += i;
        return *this;
    }

    auto printA() const -> const SomeCommonStruct & {
        std::cout << a << std::endl;
        return *this;
    }

    auto printB() const -> const SomeCommonStruct & {
        std::cout << b << std::endl;
        return *this;
    }

    SomeCommonStruct(int inA, int inB) : a(inA), b(inB) {}
};

namespace {

void fn(Mutex<SomeCommonStruct> &someObj) {
    auto data = someObj.lock();

    data->printA();

    *data += 1;
    std::cout << "executing " << __func__ << std::endl;

    data->printA();
}

void readData(const SharedMutex<SomeCommonStruct> &someObj) {
    auto data = someObj.lock_for_read();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    data->printA();

    std::cout << "executing " << __func__ << std::endl;

    data->printA();
}

void writeData(SharedMutex<SomeCommonStruct> &someObj) {
    auto data = someObj.lock_for_rw();

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    data->printA();

    *data += 1;
    std::cout << "executing " << __func__ << std::endl;

    data->printA();

    *data += 1;

    data->printA();
}

} // namespace

int main() {
    SharedMutex<SomeCommonStruct> someShared(1, 2);
    Mutex<SomeCommonStruct> someObj(1, 2);

    fn(someObj);

    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&someShared]() { readData(someShared); });
    }

    threads.emplace_back([&someShared]() { writeData(someShared); });

    for (auto &t : threads) {
        t.join();
    }
}
