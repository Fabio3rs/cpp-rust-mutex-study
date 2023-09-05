#include "RustMutex.hpp"
#include <iostream>

struct ObjetoComum {
    int a;
    int b;

    auto &operator+=(int i) {
        a += i;
        return *this;
    }

    auto printA() const -> const ObjetoComum & {
        std::cout << a << std::endl;
        return *this;
    }

    auto printB() const -> const ObjetoComum & {
        std::cout << b << std::endl;
        return *this;
    }

    auto printA() -> ObjetoComum & {
        const_cast<const ObjetoComum *>(this)->printA();
        return *this;
    }

    auto printB() -> ObjetoComum & {
        const_cast<const ObjetoComum *>(this)->printB();
        return *this;
    }

    ObjetoComum(int inA, int inB) : a(inA), b(inB) {}
};

namespace {

void fn(Mutex<ObjetoComum> &someObj) {
    auto data = someObj.lock();

    data->printA();

    *data += 1;
    std::cout << "executing " << __func__ << std::endl;

    data->printA();
}

void fn2(const SharedMutex<ObjetoComum> &someObj) {
    auto data = someObj.lock_for_read();

    data->printA();

    std::cout << "executing " << __func__ << std::endl;

    data->printA();
}

void fn3(SharedMutex<ObjetoComum> &someObj) {
    auto data = someObj.lock_for_rw();

    data->printA();

    *data += 1;
    std::cout << "executing " << __func__ << std::endl;

    data->printA();

    *data += 1;
}

} // namespace

int main() {
    SharedMutex<ObjetoComum> someShared(1, 2);
    Mutex<ObjetoComum> someObj(1, 2);

    fn(someObj);

    fn2(someShared);

    fn3(someShared);
}
