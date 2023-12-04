#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "simdjson.cpp"
#include "simdjson.h"

// Подробная статистика по каждому слову
// Отдельный класс
// # в череде глобальных слов, праат, длительность, диапазоны высоты и грокомости

using namespace simdjson;

class RecordsManager {
 public:
  RecordsManager() {
    paths_.reserve(500);
    records_.reserve(500);
  }

  void add_json_record(const std::filesystem::path& json_path) {
    auto str = padded_string::load(json_path.c_str());
    ondemand::parser parser;
    ondemand::document data = parser.iterate(str);
    paths_.push_back(json_path);

    for (const auto& stat : stats_list_) {
      stats_[stat] += data[stat].get_double();
      const std::string date = std::string(data["date"].get_string().take_value());
      dates_[date].push_back(&data);
      // std::time etc // weeks_; // mothes_; // years_;
    }

    for (auto event : data["events"]) {
      const std::string type = std::string(event["type"].get_string().take_value());
      if (type == "word") {
        std::string word = std::string(event["word"].get_string().take_value());
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        words_[word].push_back(&event);
        // TODO we don't have here exact time of each word!!!
        // TODO total stats:
            // letters freq, duration
            // stats - praat_pitch, intensiti -> word "range"
            // full praat info -> Jitter, Shimmer
      }
    }

    records_.push_back(std::move(data));
  }

  void print_stats() {
    for (const auto& [name, value] : stats_) {
      std::cout << name << " = " << value << ";" << std::endl;
    }
    for (const auto& [name, records] : words_) {
      if (records.size() > 50) {
        std::cout << records.size() << " times " << name  << std::endl;
      }
    }
  }

 private:
  std::vector<std::filesystem::path> paths_;
  std::vector<ondemand::document> records_;
  std::unordered_map<std::string, double> stats_;

  std::map<std::string, std::vector<const simdjson_result<ondemand::value>*>> words_;
  std::map<std::string, std::vector<const ondemand::document*>> dates_;
  std::vector<std::vector<const ondemand::document*>> weeks_;

  const std::vector<std::string> stats_list_ = {"duration", "words_count", "words_duration", "pauses_duration"};
};

int main() {
  RecordsManager man;
  const auto start{std::chrono::steady_clock::now()};
  using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
  for (const auto& it : recursive_directory_iterator("/home/constcut/life/jvd/files")) {  // fs::current_path()
    const auto report_file = it.path();
    if (report_file.extension() == ".json") {
      // std::cout << report_file << std::endl;
      man.add_json_record(report_file);
    }
  }
  const auto end{std::chrono::steady_clock::now()};
  const std::chrono::duration<double> elapsed_seconds{end - start};
  std::cout << "Time spent: " << elapsed_seconds.count() << std::endl;
  man.print_stats();
  return 0;
}
