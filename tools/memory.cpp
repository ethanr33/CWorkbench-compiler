
#include "memory.h"

template<typename T>
ID::DefaultId Arena<T>::add(T&& item) {
    items.push_back(std::move(item));
    return static_cast<ID::DefaultId>(items.size() - 1);
}

template<typename T>
T& Arena<T>::get(ID::DefaultId id) {
    return items.at(id);
}

template<typename T>
const T& Arena<T>::get(ID::DefaultId id) const {
    return items.at(id);
}

template<typename T>
ID::DefaultId Arena<T>::get_max_id() const {
    return static_cast<ID::DefaultId>(items.size());
}