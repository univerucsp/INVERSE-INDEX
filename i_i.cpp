
// g++ -std=c++17 -o inverted_i i_i.cpp -lpthread
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <thread>
#include <mutex>
#include <chrono>

namespace fs = std::filesystem;

using InvertedIndex = std::unordered_map<std::string, std::unordered_set<std::string>>;

std::mutex index_mutex;

InvertedIndex processSegment(const std::string& segment, const std::string& filename) {
    InvertedIndex local_index;
    std::istringstream iss(segment);
    std::string word;
    while (iss >> word) {
        word.erase(std::remove_if(word.begin(), word.end(), ::ispunct), word.end());
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);

        local_index[word].insert(filename);
    }
    return local_index;
}

void updateIndex(const std::string& filename, InvertedIndex& index) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    const unsigned int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    std::vector<std::string> segments(num_threads);
    std::vector<InvertedIndex> local_indices(num_threads);

    std::vector<std::ifstream> segment_files(num_threads);
    std::vector<std::streampos> segment_offsets(num_threads);
    std::vector<std::streamsize> segment_sizes(num_threads);

    size_t segment_size = file_size / num_threads;
    size_t offset = 0;
    for (unsigned int i = 0; i < num_threads; ++i) {
        size_t next_offset = (i == num_threads - 1) ? file_size : offset + segment_size;
        segment_offsets[i] = offset;
        segment_sizes[i] = next_offset - offset;
        offset = next_offset;
    }

    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back([i, &filename, &segment_files, &segment_offsets, &segment_sizes, &local_indices]() {
            // Open the file segment for reading
            segment_files[i].open(filename, std::ios::binary);
            segment_files[i].seekg(segment_offsets[i]);
            std::vector<char> buffer(segment_sizes[i]);
            segment_files[i].read(buffer.data(), segment_sizes[i]);
            std::string segment(buffer.begin(), buffer.end());

            local_indices[i] = processSegment(segment, filename);
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    for (const auto& local_index : local_indices) {
        std::lock_guard<std::mutex> lock(index_mutex);
        for (const auto& [word, files] : local_index) {
            index[word].insert(files.begin(), files.end());
        }
    }
}



void buildIndex(const std::string& directory, InvertedIndex& index) {
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            updateIndex(entry.path().string(), index);
        }
    }
}

void search(const std::string& word, const InvertedIndex& index) {
    auto it = index.find(word);
    if (it != index.end()) {
        std::cout << "Word \"" << word << "\" found in files:\n";
        for (const auto& filename : it->second) {
            std::cout << "  " << filename << "\n";
        }
    } else {
        std::cout << "Word \"" << word << "\" not found in any files.\n";
    }
}

int main() {
    InvertedIndex index;
    std::string directory = "generated_files/";
    buildIndex(directory, index);
    while (true) {
        std::string searchWord;
        std::cout << "Enter a word to search: ";
        std::cin >> searchWord;

        searchWord.erase(std::remove_if(searchWord.begin(), searchWord.end(), ::ispunct), searchWord.end());
        std::transform(searchWord.begin(), searchWord.end(), searchWord.begin(), ::tolower);

        search(searchWord, index);
    }
    return 0;
}

