#pragma once

#include <unordered_map>
#include <shared_mutex>

template <typename T>
class ThreadSafeUnorderedMap {
public:
    std::size_t push(T&& value);
    T& operator[](std::size_t idx);
    const T& operator[](std::size_t idx) const;
    void erase(std::size_t idx);
    [[nodiscard]] bool contains(std::size_t key) const {
        std::shared_lock<std::shared_mutex> lk(data_mtx_);
        return data_.count(key) > 0;
    }
private:
    std::unordered_map<std::size_t, T> data_;
    std::size_t max_idx_ = 0;
    mutable std::shared_mutex data_mtx_;
};

template<typename T>
std::size_t ThreadSafeUnorderedMap<T>::push(T &&value) {
    std::unique_lock<std::shared_mutex> lk(data_mtx_);
    data_.emplace(max_idx_, std::forward<T>(value));
    return max_idx_++;
}

template<typename T>
T &ThreadSafeUnorderedMap<T>::operator[](std::size_t idx) {
    std::shared_lock<std::shared_mutex> lk(data_mtx_);
    return data_[idx];
}

template<typename T>
const T &ThreadSafeUnorderedMap<T>::operator[](std::size_t idx) const {
    std::shared_lock<std::shared_mutex> lk(data_mtx_);
    return data_[idx];
}

template<typename T>
void ThreadSafeUnorderedMap<T>::erase(std::size_t idx) {
    std::unique_lock<std::shared_mutex> lk(data_mtx_);
    data_.erase(idx);
}
