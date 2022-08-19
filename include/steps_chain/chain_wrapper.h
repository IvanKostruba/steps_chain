#pragma once

#include <memory>
#include <string>
#include <tuple>

namespace steps_chain {

class ChainWrapper {
public:
    ChainWrapper() : _self{nullptr} {
    }

    template <typename T>
    ChainWrapper(T x) : _self{std::make_unique<model<T>>(std::move(x))} {
    }

    // Wrapper can hold context for ContextStepsChain, but then it will be copied (or moved)
    // in the ctor, and there are 2 more moves involved when creating polymorphic concept.
    // Use smart pointers to avoid copying heavy instances.
    template <typename T, typename C>
    ChainWrapper(T x, C c) :
        _self{std::make_unique<context_model<T, C>>(std::move(x), std::move(c))} {
    }

    ChainWrapper(const ChainWrapper& other) : _self{other._self->copy()} {
    }

    ChainWrapper(ChainWrapper&&) noexcept = default;

    ChainWrapper& operator=(const ChainWrapper& other) {
        return *this = ChainWrapper(other);
    }
    ChainWrapper& operator=(ChainWrapper&&) noexcept = default;

    bool run(std::string parameters, uint8_t begin = 0) {
        if(_self) { return _self->run(std::move(parameters), begin); }
        else { return false; }
    }

    bool initialize(std::string parameters, uint8_t begin = 0) {
        if(_self) { return _self->initialize(std::move(parameters), begin); }
        else { return false; }
    }

    bool advance() {
        if(_self) { return _self->advance(); }
        else { return false; }
    }

    bool resume() {
        if(_self) { return _self->resume(); }
        else { return false; }
    }

    std::tuple<uint8_t, std::string> get_current_state() const {
        if(_self) { return _self->get_current_state(); }
        return std::make_tuple(-1, "");
    }

    bool is_finished() const {
        if(_self) { return _self->is_finished(); }
        return false;
    }

private:
    struct chain_concept {
        virtual ~chain_concept() = default;
        virtual std::unique_ptr<chain_concept> copy() = 0;

        virtual bool run(std::string parameters, uint8_t begin) = 0;
        virtual bool initialize(std::string parameters, uint8_t begin) = 0;
        virtual bool advance() = 0;
        virtual bool resume() = 0;
        virtual std::tuple<uint8_t, std::string> get_current_state() const = 0;
        virtual bool is_finished() const = 0;
    };

    template <typename T>
    struct model final : chain_concept {
        model(T x) : _data{std::move(x)} {
        }

        std::unique_ptr<chain_concept> copy() override { 
            return std::make_unique<model>(*this);
        }

        bool run(std::string parameters, uint8_t begin) override {
            return _data.run(std::move(parameters), begin);
        }
        bool initialize(std::string parameters, uint8_t begin) override {
            return _data.initialize(std::move(parameters), begin);
        }
        bool advance() override {
            return _data.advance();
        }
        bool resume() override {
            return _data.resume();
        }
        std::tuple<uint8_t, std::string> get_current_state() const override {
            return _data.get_current_state();
        }
        bool is_finished() const override {
            return _data.is_finished();
        }

        T _data;
    };

    template <typename T, typename C>
    struct context_model final : chain_concept {
        context_model(T x, C ctx) : _data{std::move(x)}, _context{std::move(ctx)} {
        }

        std::unique_ptr<chain_concept> copy() override { 
            return std::make_unique<context_model>(*this);
        }

        bool run(std::string parameters, uint8_t begin) override {
            return _data.run(std::move(parameters), _context, begin);
        }
        bool initialize(std::string parameters, uint8_t begin) override {
            return _data.initialize(std::move(parameters), begin);
        }
        bool advance() override {
            return _data.advance(_context);
        }
        bool resume() override {
            return _data.resume(_context);
        }
        std::tuple<uint8_t, std::string> get_current_state() const override {
            return _data.get_current_state();
        }
        bool is_finished() const override {
            return _data.is_finished();
        }

        T _data;
        C _context;
    };

    std::unique_ptr<chain_concept> _self;
};

}; // namespace steps_chain
