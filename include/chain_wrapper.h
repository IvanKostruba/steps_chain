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

    ChainWrapper(const ChainWrapper& other) : _self{other._self->copy()} {
    }

    ChainWrapper(ChainWrapper&&) noexcept = default;

    ChainWrapper& operator=(const ChainWrapper& other) {
        return *this = ChainWrapper(other);
    }
    ChainWrapper& operator=(ChainWrapper&&) noexcept = default;

    void run(std::string parameters, size_t begin = 0) {
        if(_self) { _self->run(std::move(parameters), begin); }
    }

    void initialize(std::string parameters, size_t begin = 0) {
        if(_self) { _self->initialize(std::move(parameters), begin); }
    }

    void advance() {
        if(_self) { _self->advance(); }
    }

    void resume() {
        if(_self) { _self->resume(); }
    }

    std::tuple<size_t, std::string> get_current_state() const {
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

        virtual void run(std::string parameters, size_t begin) = 0;
        virtual void initialize(std::string parameters, size_t begin) = 0;
        virtual void advance() = 0;
        virtual void resume() = 0;
        virtual std::tuple<size_t, std::string> get_current_state() const = 0;
        virtual bool is_finished() const = 0;
    };

    template <typename T>
    struct model final : chain_concept {
        model(T x) : _data{std::move(x)} {
        }

        std::unique_ptr<chain_concept> copy() { 
            return std::make_unique<model>(*this);
        }

        void run(std::string parameters, size_t begin) override {
            _data.run(std::move(parameters), begin);
        }
        void initialize(std::string parameters, size_t begin) override {
            _data.initialize(std::move(parameters), begin);
        }
        void advance() override {
            _data.advance();
        }
        void resume() override {
            _data.resume();
        }
        std::tuple<size_t, std::string> get_current_state() const override {
            return _data.get_current_state();
        }
        bool is_finished() const override {
            return _data.is_finished();
        }

        T _data;
    };

    std::unique_ptr<chain_concept> _self;
};

}; // namespace steps_chain
