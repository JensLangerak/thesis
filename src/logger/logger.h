//
// Created by jens on 28-11-20.
//

#ifndef SIMPLESATSOLVER_SRC_LOGGER_LOGGER_H_
#define SIMPLESATSOLVER_SRC_LOGGER_LOGGER_H_

#include <chrono>
#include <fstream>
#include <mutex>
#include <queue>
#include <string>

namespace simple_sat_solver::logger {
class Logger {
public:
  static void StartNewLog(std::string dir, std::string file);
  static void StartNewLog(std::string file);
  static void End();
  static void Log(std::string &message);
  static void Log2(std::string message) { Log(message);};
  static void Log(char* message) {
    std::string s(message);
    Log(s);
  };
  void EndLog();
protected:
  Logger() = default;
  ~Logger() = default;
  static Logger *instance_ ;
  static std::recursive_mutex mutex_;

  std::string file_name_;
  std::ofstream file_;

  struct LogItem {
    std::chrono::time_point<std::chrono::system_clock> time;
    std::string message;
  };
  void Log(LogItem &log_item);
  void LogToFile(LogItem &log_item);
  std::queue<LogItem> log_queue_;
  void OpenFile(std::string file);

};
}

#endif // SIMPLESATSOLVER_SRC_LOGGER_LOGGER_H_
