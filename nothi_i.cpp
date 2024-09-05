#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <filesystem>
#include <algorithm> // Include this header for std::remove_if and std::transform
#include <cctype>    // Include this header for std::ispunct and std::tolower

namespace fs = std::filesystem;

// Define an inverted index type
using InvertedIndex = std::unordered_map<std::string, std::unordered_set<std::string>>;

// Function to read a file and update the index
void updateIndex(const std::string& filename, InvertedIndex& index) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string word;
        while (iss >> word) {
            // Remove punctuation and convert to lowercase
            word.erase(std::remove_if(word.begin(), word.end(), ::ispunct), word.end());
            std::transform(word.begin(), word.end(), word.begin(), ::tolower);
            
            // Add the file to the set of files containing the word
            index[word].insert(filename);
        }
    }
}

// Function to build the index from all files in a directory
void buildIndex(const std::string& directory, InvertedIndex& index) {
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            updateIndex(entry.path().string(), index);
        }
    }
}

// Function to search for a word in the index
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
    std::string directory = "generated_files/"; // Replace with your directory path
    buildIndex(directory, index);
    while (true) {
	std::string searchWord;
    	std::cout << "Enter a word to search: ";
    	std::cin >> searchWord;
    
    	// Remove punctuation and convert to lowercase for search
    	searchWord.erase(std::remove_if(searchWord.begin(), searchWord.end(), ::ispunct), searchWord.end());
    	std::transform(searchWord.begin(), searchWord.end(), searchWord.begin(), ::tolower);
    
    	search(searchWord, index);
    }
    return 0;
}
