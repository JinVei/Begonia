#pragma once

namespace logger {
    #define GENERATE_LOGGER(X) Logger(#X),
    #define GENERATE_ENUM(X) X,

    #define ALL_LOGGER_IDS(FUNCTION)   \
        FUNCTION(admin)                \
        FUNCTION(assert)               \
        FUNCTION(client)               \
        FUNCTION(config)               \
        FUNCTION(connection)           \
        FUNCTION(file)                 \
        FUNCTION(filter)               \
        FUNCTION(hc)                   \
        FUNCTION(http)                 \
        FUNCTION(http2)                \
        FUNCTION(main)                 \
        FUNCTION(mongo)                \
        FUNCTION(pool)                 \
        FUNCTION(router)               \
        FUNCTION(runtime)              \
        FUNCTION(testing)              \
        FUNCTION(upstream)

    enum class Id {
        ALL_LOGGER_IDS(GENERATE_ENUM)
    };

    class Registry;

    class Logger {
    public:
        std::string levelString() const ;
        std::string name() const ;
        void setLevel(spdlog::level::level_enum level) const;

    private:
        Logger(const std::string& name);

        std::shared_ptr<spdlog::logger> logger_; 
        friend class Registry;
    };

    class LockingStderrSink : public spdlog::sinks::sink {
    public:
        void setLock(std::mutex& lock) { lock_ = lock; }

        void log(const spdlog::details::log_msg& msg) override {
            std::lock_guard<std::mutex> guard(lock_);
            std::cerr << msg.formatted.str();
        }
        void flush() override {
            std::lock_guard<std::mutex> guard(lock_);
            std::cerr << std::flush;
        }

    private:
        std::mutex lock_;
    };

    class Registry {
    public:
        static spdlog::logger& getLog(Id id);

        static std::shared_ptr<LockingStderrSink> getSink();

        void Registry::initialize(uint64_t log_level);

        static const std::vector<Logger>& loggers() {return all_loggers_; }

    private:
        static std::vector<Logger> all_loggers_;
    };

    template <Id id> class Loggable {
    protected:
        static spdlog::logger& log() {
            static spdlog::logger& instance = Registry::getLog(id);
            return instance;
        }
    };
}
