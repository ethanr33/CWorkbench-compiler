
#include <vector>
#include <cstdint>

template<typename T>
class Arena {
    using Id = uint32_t;

    private:
        std::vector<T> items;
    public:

        static constexpr Id invalid_id = -1;

        Id add(T&& item) {
            items.push_back(std::move(item));
            return static_cast<Id>(items.size() - 1);
        } 

        T& get(Id id) {
            return items.at(id);
        }

        const T& get(Id id) const {
            return items.at(id);
        }

        Id get_max_id() const {
            return static_cast<Id>(items.size());
        }
};