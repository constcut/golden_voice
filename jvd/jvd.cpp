#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "json.hpp"

namespace fs = std::filesystem;


// Подробная статистика по каждому слову
// # в череде глобальных слов, праат, длительность, диапазоны высоты и грокомости


class RecordsManager {
 public:
  RecordsManager() = default;
  
  void add_json_record(const fs::path& json_path) {
    paths_.push_back(json_path);
    std::ifstream file(json_path);
    auto data = nlohmann::json::parse(file);
    for (const auto& stat: stats_list_) {
        stats_[stat] += static_cast<double>(data[stat]);
    }
    records_.push_back(std::move(data));
    // by_date_[data["date"]].push_back(&data);
    // by_month_["october"], by_week_day_["monday"]...

    // cycle - each event: pause or word
    // part of speech
    //

    // letters freq, duration
    // stats - praat_pitch, intensiti -> word "range"
    // full praat info -> Jitter, Shimmer

    //
    //
    //by_word_["word"].push_back(events[17])

    // TODO calculate each word, to make top frequent
  }

  void print_stats() {
    for (const auto& [name, value]: stats_) {
        std::cout << name << " = " << value << ";" << std::endl;
    }
  }

 private:
  std::vector<fs::path> paths_;
  std::vector<nlohmann::json> records_; //  TODO make nice naming dictionay
  std::unordered_map<std::string, double> stats_;
  const std::vector<std::string> stats_list_ = {"duration", "words_count", "words_duration", "pauses_duration"};
};


int main() {
    RecordsManager man;
    using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
    for (const auto& it : recursive_directory_iterator("/home/constcut/life/jvd/files")) { // fs::current_path()
        const auto report_file = it.path();
        if (report_file.extension() == ".json") {
            //std::cout << report_file << std::endl;
            man.add_json_record(report_file);
        }
    }
    man.print_stats();
    return 0;
}

