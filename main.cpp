#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>
#include <shared_mutex>

template <class Obj, class MtxT = std::mutex,
          class GuardT = std::scoped_lock<MtxT>>
class enclosedLckGuard {
    GuardT lck;

  public:
    Obj *const objPtr;

    Obj *operator->() { return objPtr; }
    const Obj *operator->() const { return objPtr; }

    Obj &operator*() { return *objPtr; }
    const Obj &operator*() const { return *objPtr; }

    enclosedLckGuard(MtxT &m, Obj &obj) : lck(m), objPtr(&obj) {
        std::cout << "lock acquired" << std::endl;
    }

    enclosedLckGuard(const enclosedLckGuard &) = delete;

    template <class T> auto &exec(T &&f) {
        f(*this);
        return *this;
    }

    ~enclosedLckGuard() { std::cout << "lock released" << std::endl; }

    auto &operator++() {
        ++(*objPtr);
        return *this;
    }

    auto &operator--() {
        --(*objPtr);
        return *this;
    }

    auto &operator++(int) {
        ++(*objPtr);
        return *this;
    }

    auto &operator--(int) {
        --(*objPtr);
        return *this;
    }

    template <class T> auto &operator+=(T i) {
        (*objPtr) += i;
        return *this;
    }

    template <class T> auto &operator-=(T i) {
        (*objPtr) -= i;
        return *this;
    }

    template <class T> auto &operator*=(T i) {
        (*objPtr) *= i;
        return *this;
    }

    template <class T> auto &operator/=(T i) {
        (*objPtr) /= i;
        return *this;
    }

    template <class T> auto &operator%=(T i) {
        (*objPtr) %= i;
        return *this;
    }

    template <class T> auto &operator&=(T i) {
        (*objPtr) &= i;
        return *this;
    }

    template <class T> auto &operator|=(T i) {
        (*objPtr) |= i;
        return *this;
    }

    template <class T> auto &operator^=(T i) {
        (*objPtr) ^= i;
        return *this;
    }

    template <class T> auto &operator<<=(T i) {
        (*objPtr) <<= i;
        return *this;
    }

    template <class T> auto &operator>>=(T i) {
        (*objPtr) >>= i;
        return *this;
    }

    template <class T> auto &operator=(T i) {
        (*objPtr) = i;
        return *this;
    }

    template <class T> auto &operator[](T i) { return (*objPtr)[i]; }
};

template <class T> struct Lockable {
    std::mutex m;

    auto lock() -> enclosedLckGuard<T> {
        return enclosedLckGuard<T>(m, *static_cast<T *>(this));
    }

    template <class Fun> auto exec(Fun &&f) -> T & {
        f();
        return *static_cast<T *>(this);
    }
};

#define LOCKABLE()                                                             \
    std::mutex m;                                                              \
                                                                               \
    auto lock() {                                                              \
        return enclosedLckGuard<std::decay_t<decltype(*this)>>(m, *this);      \
    }                                                                          \
                                                                               \
    template <class Fun> auto &exec(Fun &&f) {                                 \
        f();                                                                   \
        return *this;                                                          \
    }

struct SomeObjUsingMacro {
    LOCKABLE()

    int a;
    int b;

    auto printA() -> SomeObjUsingMacro & {
        std::cout << a << std::endl;
        return *this;
    }

    auto printB() -> SomeObjUsingMacro & {
        std::cout << b << std::endl;
        return *this;
    }

    SomeObjUsingMacro(int a, int b) : a(a), b(b) {}
};

struct SomeObjInheritance : public Lockable<SomeObjInheritance> {
    int a;
    int b;

    auto printA() -> SomeObjInheritance & {
        std::cout << a << std::endl;
        return *this;
    }

    auto printB() -> SomeObjInheritance & {
        std::cout << b << std::endl;
        return *this;
    }

    SomeObjInheritance(int a, int b) : a(a), b(b) {}
};

template <class Obj, class MtxT = std::mutex> struct lckEnclosure {
    auto lock() -> enclosedLckGuard<Obj, MtxT> { return {m, obj}; }

    auto lock_for_read() const
        -> const enclosedLckGuard<const Obj, MtxT, std::shared_lock<MtxT>> {
        return {m, obj};
    }

    auto lock_for_rw() -> enclosedLckGuard<Obj, MtxT, std::unique_lock<MtxT>> {
        return {m, obj};
    }

    // constructor with parameters
    template <class... Args>
    lckEnclosure(Args &&...args) : obj(std::forward<Args>(args)...) {}

  private:
    Obj obj;
    mutable MtxT m;
};

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

    ObjetoComum(int a, int b) : a(a), b(b) {}
};

namespace {

void fn(lckEnclosure<ObjetoComum> &someObj) {
    auto data = someObj.lock();

    data->printA();

    data += 1;
    std::cout << "executing " << __func__ << std::endl;

    data->printA();
}

void fn2(const lckEnclosure<ObjetoComum, std::shared_mutex> &someObj) {
    auto data = someObj.lock_for_read();

    data->printA();

    std::cout << "executing " << __func__ << std::endl;

    data->printA();
}

void fn3(lckEnclosure<ObjetoComum, std::shared_mutex> &someObj) {
    auto data = someObj.lock_for_rw();

    data->printA();

    data += 1;
    std::cout << "executing " << __func__ << std::endl;

    data->printA();

    *data += 1;
}

} // namespace

int main() {
    lckEnclosure<ObjetoComum> lckObj(1, 2);

    fn(lckObj);

    lckEnclosure<ObjetoComum, std::shared_mutex> lckObjShr(1, 2);

    fn3(lckObjShr);
    fn2(lckObjShr);

    lckObj.lock()
        .exec([](auto &) { std::cout << "executing" << std::endl; })
        ->printA()
        .printB();

    SomeObjUsingMacro a{1, 2};

    a.lock()->printA().printB().exec(
        []() { std::cout << "executing" << std::endl; });

    SomeObjInheritance b{1, 2};

    b.lock()->printA().printB().exec(
        []() { std::cout << "executing" << std::endl; });
}
