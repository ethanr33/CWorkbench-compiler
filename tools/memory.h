#pragma once

#include <vector>
#include <cstdint>

namespace ID {
    using DefaultId = uint32_t;
    using ASTNodeId = uint32_t;
    using SymbolTableId = uint32_t;
};

template<typename T>
class AbstractArena {
    public:
        static constexpr ID::DefaultId invalid_id = -1;

        virtual ID::DefaultId add(T&&) = 0;

        virtual T& get(ID::DefaultId) = 0;

        virtual const T& get(ID::DefaultId id) const = 0;

        virtual ID::DefaultId get_max_id() const = 0;
};

template<typename T>
class Arena : public AbstractArena<T> {
    private:
        std::vector<T> items;
    public:

        ID::DefaultId add(T&& item) {
            items.push_back(std::move(item));
            return static_cast<ID::DefaultId>(items.size() - 1);
        } 

        T& get(ID::DefaultId id) {
            return items.at(id);
        }

        const T& get(ID::DefaultId id) const {
            return items.at(id);
        }

        ID::DefaultId get_max_id() const {
            return static_cast<ID::DefaultId>(items.size());
        }
};
