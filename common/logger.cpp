// #include <vector>
// #include <mutex>

// #include "logger.h"

// namespace logger {
//     std::vector<Logger> Registry::all_loggers_ = {ALL_LOGGER_IDS(GENERATE_LOGGER)};

//     std::string Logger::levelString() const {
//         return spdlog::level::level_names[logger_->level()];
//     }
//     std::string Logger::name() const {
//         return logger_->name();
//     }
//     void Logger::setLevel(spdlog::level::level_enum level) const {
//         logger_->set_level(level);
//     }

//     Logger::Logger(const std::string& name) {
//         logger_ = std::make_shared<spdlog::logger>(name, Registry::getSink());
//         logger_->set_pattern("[%Y-%m-%d %T.%e][%t][%l][%n] %v");
//         logger_->set_level(spdlog::level::trace);
//     }

//     static spdlog::logger& 
//     Registry::getLog(Id id) {
//         return *all_loggers_[static_cast<int>(id)].logger_; 
//     }

//     static std::shared_ptr<LockingStderrSink> 
//     Registry::getSink() {
//         static std::shared_ptr<LockingStderrSink> sink(new LockingStderrSink());
//         return sink;
//     }

//     void Registry::initialize(uint64_t log_level) {
//         for (Logger& logger : all_loggers_) {
//             logger.logger_->set_level(static_cast<spdlog::level::level_enum>(log_level));
//         }
//     }

// }
