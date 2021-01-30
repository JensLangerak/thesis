//
// Created by jens on 28-11-20.
//

#include "logger.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
namespace simple_sat_solver::logger {
std::recursive_mutex Logger::mutex_;
Logger * Logger::instance_ = nullptr;
void Logger::StartNewLog(std::string file) {
  mutex_.lock();
  if (Logger::instance_ != nullptr) {
    instance_->EndLog();
    delete instance_;
  }
  instance_ = new Logger();
  std::ifstream infile(file);
  if (infile.good()) {
    throw "Log file already exists";
  }
  instance_->OpenFile(file);
  mutex_.unlock();
}
void Logger::StartNewLog(std::string dir, std::string file) {
  time_t seconds;
  seconds = time(NULL);
  std::stringstream ss;
  ss << seconds;
  std::string ts = ss.str();
  std::string file_name = dir + "/" + file + "_" + ts + ".log";
  StartNewLog(file_name);
}
void Logger::OpenFile(std::string file) {
  file_name_ = file;
  file_.open(file_name_);
}
void Logger::EndLog() {
  if (file_.is_open()) {
    file_.flush();
    file_.close();
  }
}
void Logger::Log(std::string &message) {
  LogItem item;
  item.time = std::chrono::system_clock::now();
  item.message = message;
  instance_->Log(item);
}
void Logger::Log(Logger::LogItem &log_item) {
  mutex_.lock();
//  log_queue_.push(log_item);
  LogToFile(log_item);
  mutex_.unlock();
}
void Logger::LogToFile(Logger::LogItem &log_item) {
  auto ms_all = std::chrono::duration_cast<std::chrono::milliseconds>(log_item.time.time_since_epoch());
  auto ms = ms_all %1000;
  std::time_t tt = std::chrono::system_clock::to_time_t(log_item.time);
  std::tm tm = *std::gmtime(&tt); //GMT (UTC)
  std::stringstream ss;
  ss << std::put_time( &tm, "UTC: %Y-%m-%d %H:%M:%S:" );
  file_ << ss.rdbuf() << ms.count() << " " << ms_all.count() << " " << log_item.message << "\n";
}
void Logger::End() {
  mutex_.lock();
  if (Logger::instance_ != nullptr) {
    instance_->EndLog();
    delete instance_;
    instance_ = nullptr;
  }
  mutex_.unlock();
}

} // namespace simple_sat_solver::logger
