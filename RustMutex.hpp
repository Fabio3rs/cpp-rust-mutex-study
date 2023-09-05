#pragma once
#include <iostream>
#include <mutex>
#include <shared_mutex>

template <class Obj, class MtxT = std::mutex,
          class GuardT = std::scoped_lock<MtxT>>
class ObjScopeGuard {
    GuardT lck;

  public:
    Obj *const objPtr;

    Obj *operator->() { return objPtr; }
    const Obj *operator->() const { return objPtr; }

    Obj &operator*() { return *objPtr; }
    const Obj &operator*() const { return *objPtr; }

    ObjScopeGuard(MtxT &m, Obj &obj) : lck(m), objPtr(&obj) {}

    ObjScopeGuard(const ObjScopeGuard &) = delete;

    template <class T> auto &exec(T &&f) {
        f(*this);
        return *this;
    }

    ~ObjScopeGuard() = default;
};

template <class Obj, class MtxT = std::mutex> struct Mutex {
    auto lock() -> ObjScopeGuard<Obj, MtxT> { return {m, obj}; }

    template <class... Args>
    Mutex(Args &&...args) : obj(std::forward<Args>(args)...) {}

  protected:
    Obj obj;
    mutable MtxT m;
};

template <class Obj>
struct SharedMutex : protected Mutex<Obj, std::shared_mutex> {
    using parentType = Mutex<Obj, std::shared_mutex>;

    template <class... Args>
    SharedMutex(Args &&...args) : parentType(std::forward<Args>(args)...) {}

    auto lock_for_read() const
        -> const ObjScopeGuard<const Obj, std::shared_mutex,
                               std::shared_lock<std::shared_mutex>> {
        return {parentType::m, parentType::obj};
    }

    auto lock_for_rw() -> ObjScopeGuard<Obj, std::shared_mutex,
                                        std::shared_lock<std::shared_mutex>> {
        return {parentType::m, parentType::obj};
    }
};
