#pragma once

#include <string>
#include <tuple>
#include <type_traits>

namespace steps_chain {

namespace _detail {

    struct vtable {
        bool (*run)(void* ptr, std::string parameters, uint8_t begin);
        bool (*initialize)(void* ptr, std::string parameters, uint8_t begin);
        bool (*advance)(void* ptr);
        bool (*resume)(void* ptr);
        std::tuple<uint8_t, std::string> (*get_current_state)(const void* ptr);
        bool (*is_finished)(const void* ptr);

        void (*destroy_)(void* ptr);
        void (*clone)(void* storage, const void* ptr);
        void (*move_clone)(void* storage, void* ptr);
    };

    template<typename Chain>
    constexpr vtable vtable_for {
        [](void* ptr, std::string parameters, uint8_t begin) {
            return static_cast<Chain*>(ptr)->run(std::move(parameters), begin);
        },
        [](void* ptr, std::string parameters, uint8_t begin) {
            return static_cast<Chain*>(ptr)->initialize(std::move(parameters), begin);
        },
        [](void* ptr) {
            return static_cast<Chain*>(ptr)->advance();
        },
        [](void* ptr) {
            return static_cast<Chain*>(ptr)->resume();
        },
        [](const void* ptr) -> std::tuple<uint8_t, std::string> {
            return static_cast<const Chain*>(ptr)->get_current_state();
        },
        [](const void* ptr) -> bool {
            return static_cast<const Chain*>(ptr)->is_finished();
        },

        [](void* ptr) {
            static_cast<Chain*>(ptr)->~Chain();
        },
        [](void* storage, const void* ptr) {
            new (storage) Chain{*static_cast<const Chain*>(ptr)};
        },
        [](void* storage, void* ptr) {
            new (storage) Chain{std::move(*static_cast<Chain*>(ptr))};
        }
    };

    template<typename Chain, typename Context>
    constexpr vtable vtable_ctx_for {
        [](void* ptr, std::string parameters, uint8_t begin) {
            auto* p = static_cast<std::pair<Chain, Context>*>(ptr);
            return p->first.run(std::move(parameters), p->second, begin);
        },
        [](void* ptr, std::string parameters, uint8_t begin) {;
            return static_cast<std::pair<Chain, Context>*>(ptr)->first
                .initialize(std::move(parameters), begin);
        },
        [](void* ptr) {
            auto* p = static_cast<std::pair<Chain, Context>*>(ptr);
            return p->first.advance(p->second);
        },
        [](void* ptr) {
            auto* p = static_cast<std::pair<Chain, Context>*>(ptr);
            return p->first.resume(p->second);
        },
        [](const void* ptr) -> std::tuple<uint8_t, std::string> {
            return static_cast<const std::pair<Chain, Context>*>(ptr)->first.get_current_state();
        },
        [](const void* ptr) -> bool {
            return static_cast<const std::pair<Chain, Context>*>(ptr)->first.is_finished();
        },

        [](void* ptr) {
            static_cast<std::pair<Chain, Context>*>(ptr)->~pair();
        },
        [](void* storage, const void* ptr) {
            new (storage) std::pair<Chain, Context>{
                *static_cast<const std::pair<Chain, Context>*>(ptr)};
        },
        [](void* storage, void* ptr) {
            new (storage) std::pair<Chain, Context>{
                std::move(*static_cast<std::pair<Chain, Context>*>(ptr))};
        }
    };

};  // namespace _detail

// This wrapper will store its content in local buffer, so no heap allocations will occur.
// Everything will be on the stack, so big containers with these wrappers may cause overflow.
// It is also slightly faster to dispatch calls.

class ChainWrapperLS {
public:
    ChainWrapperLS()
        : vtable_{nullptr} {
    }

    template<typename Chain>
    ChainWrapperLS(Chain chain)
        : vtable_{&_detail::vtable_for<Chain>}
    {
        static_assert(sizeof(Chain) <= sizeof(buf_), "Wrapper buffer is too small!");
        new(&buf_) Chain{std::move(chain)};
    }

    template<typename Chain, typename Context>
    ChainWrapperLS(Chain x, Context c)
        : vtable_{&_detail::vtable_ctx_for<Chain, Context>}
    {
        static_assert(sizeof(std::pair<Chain, Context>) <= sizeof(buf_),
            "Wrapper buffer is too small!");
        new(&buf_) std::pair<Chain, Context>{std::move(x), std::move(c)};
    }

    ~ChainWrapperLS() {
        if (vtable_) {
            vtable_->destroy_(&buf_);
        }
    }

    // Copy/move operations will fail if right operand will be a default-constructed wrapper.
    // For now I will not add handling for that, as that would be a pessimization. Such assignments
    // are unlikely and will lead to a crash, so handling will be added if necessary.
    ChainWrapperLS(const ChainWrapperLS& other) {
        other.vtable_->clone(&buf_, &other.buf_);
        vtable_ = other.vtable_;
    }

    ChainWrapperLS(ChainWrapperLS&& other) noexcept {
        other.vtable_->move_clone(&buf_, &other.buf_);
        vtable_ = other.vtable_;
        other.vtable_ = nullptr;
    }

    ChainWrapperLS& operator=(const ChainWrapperLS& other) {
        if (vtable_) {
            vtable_->destroy_(&buf_);
        }
        other.vtable_->clone(&buf_, &other.buf_);
        vtable_ = other.vtable_;
        return *this;
    }
    ChainWrapperLS& operator=(ChainWrapperLS&& other) noexcept {
        if (vtable_) {
            vtable_->destroy_(&buf_);
        }
        other.vtable_->move_clone(&buf_, &other.buf_);
        vtable_ = other.vtable_;
        other.vtable_ = nullptr;
        return *this;
    }

    bool run(std::string parameters, uint8_t begin = 0) {
        return vtable_->run(&buf_, std::move(parameters), begin);
    }

    bool initialize(std::string parameters, uint8_t begin = 0) {
        return vtable_->initialize(&buf_, std::move(parameters), begin);
    }

    bool advance() {
        return vtable_->advance(&buf_);
    }

    bool resume() {
        return vtable_->resume(&buf_);
    }

    std::tuple<uint8_t, std::string> get_current_state() const {
        return vtable_->get_current_state(&buf_);
    }

    bool is_finished() const {
        return vtable_->is_finished(&buf_);
    }

private:
    std::aligned_storage_t<64> buf_;
    const _detail::vtable* vtable_;
};

}; // namespace steps_chain
