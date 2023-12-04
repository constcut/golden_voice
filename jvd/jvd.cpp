#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "simdjson.cpp"
#include "simdjson.h"

struct WordStats {
  std::map<std::string, double> stats;
  std::vector<unsigned int> idxes;

  void process_stat(const std::string& name, double value) {
    if (value == 0) {
      // TODO remove tmp fix for freq etc - should be done on level higher in report generator
      return;
    }
    if (stats.count(name + "_count") == 0) {
      stats[name + "_count"] = {};
      stats[name + "_sum"] = {};
      stats[name + "_max"] = std::numeric_limits<double>::min();
      stats[name + "_min"] = std::numeric_limits<double>::max();
    }
    stats[name + "_count"] += 1;
    stats[name + "_sum"] += value;
    stats[name + "_max"] = std::max(stats[name + "_max"], value);
    stats[name + "_min"] = std::min(stats[name + "_min"], value);
    stats[name + "_mean"] = stats[name + "_sum"] / stats[name + "_count"];
  }
};

using namespace simdjson;

class RecordsManager {
 public:
  RecordsManager()
      : global_word_idx_(0) {
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
        WordStats& word_stats = words_stats_[word];
        // word_stats.process_stat("letters_speed", event["letters_speed"].get_double());
        word_stats.process_stat("letters_freq", event["letters_freq"].get_double());
        if (event["stats"]["praat_pitch"].error() == SUCCESS) {
          if (event["stats"]["praat_pitch"].find_field("min").error() == SUCCESS) {
            word_stats.process_stat("pitch_min", event["stats"]["praat_pitch"]["min"].get_double());
            word_stats.process_stat("pitch_max", event["stats"]["praat_pitch"]["max"].get_double());
            word_stats.process_stat("intensity_min", event["stats"]["intensity"]["min"].get_double());
            word_stats.process_stat("intensity_max", event["stats"]["intensity"]["max"].get_double());
          }
        }
        if (!event["info"].is_null()) {
          if (event["info"].find_field("duration").error() == SUCCESS) {
            word_stats.process_stat("duration", event["info"]["duration"].get_double());
          }
          if (event["info"].find_field("Number of pulses").error() == SUCCESS) {
            // word_stats.process_stat("pulses", event["info"]["Number of pulses"].get_double());
          }
          if (event["info"].find_field("Number of voice breaks").error() == SUCCESS) {
            // word_stats.process_stat("voice_breaks", event["info"]["Number of voice breaks"].get_double());
          }
        }
        word_stats.idxes.push_back(global_word_idx_);  // TODO find min/max gap
        ++global_word_idx_;
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
        std::cout << records.size() << " times " << name << std::endl;
        for (const auto& [key, val] : words_stats_[name].stats) {
          if (key.find("count") != std::string::npos || key.find("sum") != std::string::npos)
            continue;
          std::cout << "\n    " << key << " = " << val << ";";
        }
        std::cout << std::endl;
      }
    }
  }

 private:
  unsigned int global_word_idx_;

  std::vector<std::filesystem::path> paths_;
  std::vector<ondemand::document> records_;
  std::unordered_map<std::string, double> stats_;

  std::map<std::string, std::vector<const simdjson_result<ondemand::value>*>> words_;
  std::map<std::string, WordStats> words_stats_;

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
      man.add_json_record(report_file);
    }
  }
  const auto end{std::chrono::steady_clock::now()};
  const std::chrono::duration<double> elapsed_seconds{end - start};
  std::cout << "Time spent: " << elapsed_seconds.count() << std::endl;
  man.print_stats();
  return 0;
}
