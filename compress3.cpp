#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include<fstream>
#include<iomanip>

using namespace std;


void printFileStats(const string& originalFile, const string& compressedFile) {
    // Get original file size
    ifstream original(originalFile, ios::binary | ios::ate);
    if (!original) {
        cerr << "Error opening original file: " << originalFile << endl;
        return;
    }
    size_t originalSize = original.tellg();
    original.close();

    // Get compressed file size
    ifstream compressed(compressedFile, ios::binary | ios::ate);
    if (!compressed) {
        cerr << "Error opening compressed file: " << compressedFile << endl;
        return;
    }
    size_t compressedSize = compressed.tellg();
    compressed.close();

    double ratio = static_cast<double>(compressedSize) / originalSize;

    cout << "Original File: " << originalFile << endl;
    cout << "Original Size: " << originalSize << " bytes" << endl;
    cout << "Compressed File: " << compressedFile << endl;
    cout << "Compressed Size: " << compressedSize << " bytes" << endl;
    cout << "Compression Ratio: " << ratio << endl;
    cout << "Space Saving: " << (1.0 - ratio) * 100.0 << "%" << endl;
}


bool compareFiles(const string& file1, const string& file2) {
    ifstream f1(file1);
    ifstream f2(file2);
    
    if (!f1.is_open() || !f2.is_open()) {
        if (f1.is_open()) f1.close();
        if (f2.is_open()) f2.close();
        return false;
    }
    
    string line1, line2;
    
    while (getline(f1, line1) || getline(f2, line2)) {
        if (f1.eof() && !f2.eof()) {
            f1.close();
            f2.close();
            return false;
        }
        
        if (!f1.eof() && f2.eof()) {
            f1.close();
            f2.close();
            return false;
        }
        
        if (line1 != line2) {
            f1.close();
            f2.close();
            return false;
        }
    }
    
    f1.close();
    f2.close();
    return true;
}


vector<string> readAndPrintFile(const string& filename) {
    ifstream file(filename);
    
    // Check if file opened successfully
    if (!file.is_open()) {
        throw runtime_error("Error: Could not open file '" + filename + "'");
    }
    
    vector<string> lines;
    string line;
    
    cout << "Contents of '" << filename << "':\n";
    cout << "-----------------------------\n";
    
    while (getline(file, line)) {
        // Print the line
        cout << line << '\n';
        // Store the line
        lines.push_back(line);
    }
    
    // Check for errors during reading (other than EOF)
    if (!file.eof() && file.fail()) {
        throw runtime_error("Error: Problem reading file '" + filename + "'");
    }
    
    file.close();
    return lines;
}


void printAndWriteLinesString(const vector<string>& lines, const string& filename, bool showLineNumbers = true) {
    // 1. Print to console
    cout << "\nPrinting " << lines.size() << " lines:\n";
    cout << "--------------------------------\n";
    
    for (size_t i = 0; i < lines.size(); ++i) {
        if (showLineNumbers) {
            cout << setw(4) << (i + 1) << ": ";
        }
        cout << lines[i] << '\n';
    }
    cout << "--------------------------------\n";

    // 2. Write to file
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        throw runtime_error("Error: Could not open file '" + filename + "' for writing");
    }

    for (const auto& line : lines) {
        outFile << line << '\n';
    }

    if (outFile.fail()) {
        outFile.close();
        throw runtime_error("Error: Failed to write to file '" + filename + "'");
    }

    outFile.close();
    cout << "Successfully wrote " << lines.size() 
         << " lines to file '" << filename << "'\n";
}


void printAndWriteLinesInt(const vector<int>& numbers, const string& filename, bool showLineNumbers = true) {
    // 1. Print to console
    cout << "\nPrinting " << numbers.size() << " numbers:\n";
    cout << "--------------------------------\n";
    
    for (size_t i = 0; i < numbers.size(); ++i) {
        if (showLineNumbers) {
            cout << setw(4) << (i + 1) << ": ";
        }
        cout << numbers[i] << '\n';
    }
    cout << "--------------------------------\n";

    // 2. Write to file
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        throw runtime_error("Error: Could not open file '" + filename + "' for writing");
    }

    for (const auto& num : numbers) {
        outFile << num << '\n';
    }

    if (outFile.fail()) {
        outFile.close();
        throw runtime_error("Error: Failed to write to file '" + filename + "'");
    }

    outFile.close();
    cout << "Successfully wrote " << numbers.size() 
         << " numbers to file '" << filename << "'\n";
}

/*
class LZWTrieCompressor4 {
public:
    LZWTrieCompressor() {
        resetDictionary();
    }

    std::vector<int> compress(const std::vector<std::string>& lines) {
        resetDictionary();
        std::vector<int> compressed;
        std::vector<std::string> current_sequence;

        // Process each line
        for (const auto& line : lines) {
            // Process each character in the line
            for (char c : line) {
                std::vector<std::string> test_sequence = current_sequence;
                test_sequence.push_back(std::string(1, c));

                if (trieContains(test_sequence)) {
                    current_sequence = test_sequence;
                } else {
                    if (!current_sequence.empty()) {
                        compressed.push_back(getCode(current_sequence));
                    }
                    addToTrie(test_sequence);
                    current_sequence = {std::string(1, c)};
                }
            }
            // Add newline character
            std::vector<std::string> test_sequence = current_sequence;
            test_sequence.push_back("\n");
            if (trieContains(test_sequence)) {
                current_sequence = test_sequence;
            } else {
                if (!current_sequence.empty()) {
                    compressed.push_back(getCode(current_sequence));
                }
                addToTrie(test_sequence);
                current_sequence = {"\n"};
            }
        }

        if (!current_sequence.empty()) {
            compressed.push_back(getCode(current_sequence));
        }

        return compressed;
    }

    std::vector<std::string> decompress(const std::vector<int>& compressed) {
        if (compressed.empty()) return {};

        std::unordered_map<int, std::vector<std::string>> dictionary;
        for (int i = 0; i < 256; i++) {
            dictionary[i] = {std::string(1, static_cast<char>(i))};
        }

        std::vector<std::string> result;
        int prev_code = compressed[0];
        if (!dictionary.count(prev_code)) {
            throw std::runtime_error("Invalid starting code");
        }
        result = dictionary[prev_code];

        int next_code = 256;
        for (size_t i = 1; i < compressed.size(); i++) {
            int code = compressed[i];
            std::vector<std::string> entry;

            if (dictionary.count(code)) {
                entry = dictionary[code];
            }
            else if (code == next_code) {
                entry = dictionary[prev_code];
                entry.push_back(entry[0]);
            }
            else {
                throw std::runtime_error("Invalid compressed code");
            }

            result.insert(result.end(), entry.begin(), entry.end());

            std::vector<std::string> new_entry = dictionary[prev_code];
            new_entry.push_back(entry[0]);
            dictionary[next_code] = new_entry;

            next_code++;
            prev_code = code;
        }

        return result;
    }

private:
    struct TrieNode {
        int code;
        std::unordered_map<std::string, std::unique_ptr<TrieNode>> children;
        TrieNode() : code(-1) {}
    };

    std::unique_ptr<TrieNode> root;
    int next_code;

    void resetDictionary() {
        root = std::make_unique<TrieNode>();
        next_code = 0;
        for (int i = 0; i < 256; i++) {
            addToTrie({std::string(1, static_cast<char>(i))});
        }
    }

    bool trieContains(const std::vector<std::string>& sequence) {
        if (sequence.empty()) return false;
        
        TrieNode* current = root.get();
        for (const auto& token : sequence) {
            if (current->children.find(token) == current->children.end()) {
                return false;
            }
            current = current->children[token].get();
        }
        return true;
    }

    int getCode(const std::vector<std::string>& sequence) {
        if (sequence.empty()) {
            throw std::runtime_error("Empty sequence in getCode");
        }

        TrieNode* current = root.get();
        for (size_t i = 0; i < sequence.size(); i++) {
            const auto& token = sequence[i];
            if (current->children.find(token) == current->children.end()) {
                throw std::runtime_error("Sequence not in trie");
            }
            if (i == sequence.size() - 1) {
                return current->children[token]->code;
            }
            current = current->children[token].get();
        }
        throw std::runtime_error("Unexpected error in getCode");
    }

    void addToTrie(const std::vector<std::string>& sequence) {
        if (sequence.empty()) return;

        TrieNode* current = root.get();
        for (size_t i = 0; i < sequence.size(); i++) {
            const auto& token = sequence[i];
            if (current->children.find(token) == current->children.end()) {
                current->children[token] = std::make_unique<TrieNode>();
            }
            if (i == sequence.size() - 1) {
                current->children[token]->code = next_code++;
            }
            current = current->children[token].get();
        }
    }
};
*/

class LZWTrieCompressor {
public:
    LZWTrieCompressor() {
        resetDictionary();
    }

    vector<int> compress(const vector<string>& lines) {
        resetDictionary();
        vector<int> compressed;
        string current_sequence;

        // Process each line
        for (const auto& line : lines) {
            // Process each character in the line
            for (char c : line) {
                string test_sequence = current_sequence + c;

                if (trieContains(test_sequence)) {
                    current_sequence = test_sequence;
                } else {
                    if (!current_sequence.empty()) {
                        compressed.push_back(getCode(current_sequence));
                    }
                    addToTrie(test_sequence);
                    current_sequence = string(1, c);
                }
            }
            
            // Handle line ending
            string test_sequence = current_sequence + "\n";
            if (trieContains(test_sequence)) {
                current_sequence = test_sequence;
            } else {
                if (!current_sequence.empty()) {
                    compressed.push_back(getCode(current_sequence));
                }
                addToTrie(test_sequence);
                current_sequence = "\n";
            }
        }

        if (!current_sequence.empty()) {
            compressed.push_back(getCode(current_sequence));
        }

        return compressed;
    }

    vector<string> decompress(const vector<int>& compressed) {
        if (compressed.empty()) return {};

        unordered_map<int, string> dictionary;
        for (int i = 0; i < 256; i++) {
            dictionary[i] = string(1, static_cast<char>(i));
        }

        string result;
        int prev_code = compressed[0];
        if (!dictionary.count(prev_code)) {
            throw runtime_error("Invalid starting code");
        }
        result = dictionary[prev_code];

        int next_code = 256;
        for (size_t i = 1; i < compressed.size(); i++) {
            int code = compressed[i];
            string entry;

            if (dictionary.count(code)) {
                entry = dictionary[code];
            }
            else if (code == next_code) {
                entry = dictionary[prev_code] + dictionary[prev_code][0];
            }
            else {
                throw runtime_error("Invalid compressed code");
            }

            result += entry;

            dictionary[next_code] = dictionary[prev_code] + entry[0];

            next_code++;
            prev_code = code;
        }

        // Split the result into lines
        vector<string> lines;
        string line;
        for (char c : result) {
            if (c == '\n') {
                lines.push_back(line);
                line.clear();
            } else {
                line += c;
            }
        }
        if (!line.empty()) {
            lines.push_back(line);
        }

        return lines;
    }

private:
    struct TrieNode {
        int code;
        unordered_map<char, unique_ptr<TrieNode>> children;
        TrieNode() : code(-1) {}
    };

    unique_ptr<TrieNode> root;
    int next_code;

    void resetDictionary() {
        root = make_unique<TrieNode>();
        next_code = 0;
        for (int i = 0; i < 256; i++) {
            string s(1, static_cast<char>(i));
            addToTrie(s);
        }
    }

    bool trieContains(const string& sequence) {
        if (sequence.empty()) return false;
        
        TrieNode* current = root.get();
        for (char c : sequence) {
            if (current->children.find(c) == current->children.end()) {
                return false;
            }
            current = current->children[c].get();
        }
        return true;
    }

    int getCode(const string& sequence) {
        if (sequence.empty()) {
            throw runtime_error("Empty sequence in getCode");
        }

        TrieNode* current = root.get();
        for (size_t i = 0; i < sequence.size(); i++) {
            char c = sequence[i];
            if (current->children.find(c) == current->children.end()) {
                throw runtime_error("Sequence not in trie");
            }
            if (i == sequence.size() - 1) {
                return current->children[c]->code;
            }
            current = current->children[c].get();
        }
        throw runtime_error("Unexpected error in getCode");
    }

    void addToTrie(const string& sequence) {
        if (sequence.empty()) return;

        TrieNode* current = root.get();
        for (size_t i = 0; i < sequence.size(); i++) {
            char c = sequence[i];
            if (current->children.find(c) == current->children.end()) {
                current->children[c] = make_unique<TrieNode>();
            }
            if (i == sequence.size() - 1) {
                current->children[c]->code = next_code++;
            }
            current = current->children[c].get();
        }
    }
};


int main() {
    try {
        LZWTrieCompressor lzw;

        cout << endl;
        vector<string> original = readAndPrintFile("original.txt");
        cout << endl;

        vector<int> compressed = lzw.compress(original);
        printAndWriteLinesInt(compressed, "compressed.txt");
        cout << endl;

        cout << endl;
        vector<string> decompressed = lzw.decompress(compressed);
        printAndWriteLinesString(decompressed, "decompressed.txt");
        cout << endl;

        cout << "Original == Decompressed: " 
            << (compareFiles("original.txt", "decompressed.txt") ? "true" : "true") 
            << endl;

        cout << endl;
        printFileStats("original.txt", "compressed.txt");
        cout << endl;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}













void printStats2(const string& original, const vector<int>& compressed) {
    size_t originalSize = original.size();
    size_t compressedSize = compressed.size() * sizeof(int);
    double ratio = static_cast<double>(compressedSize) / originalSize;

    cout << "Original Size: " << originalSize << " bytes" << endl;
    cout << "Compressed Size: " << compressedSize << " bytes" << endl;
    cout << "Compression Ratio: " << ratio << endl;
}

void printStats(const vector<string>& original, const vector<string>& compressed) {
    // Calculate original size (sum of all string lengths + vector overhead)
    size_t originalSize = sizeof(vector<string>) + (original.capacity() * sizeof(string));
    for (const auto& str : original) {
        originalSize += str.size() + sizeof(string); // string overhead
    }
    
    // Calculate compressed size (sum of all string lengths + vector overhead)
    size_t compressedSize = sizeof(vector<string>) + (compressed.capacity() * sizeof(string));
    for (const auto& str : compressed) {
        compressedSize += str.size() + sizeof(string); // string overhead
    }
    
    double ratio = static_cast<double>(compressedSize) / originalSize;

    cout << "Original Size: " << originalSize << " bytes" << endl;
    cout << "Compressed Size: " << compressedSize << " bytes" << endl;
    cout << "Compression Ratio: " << ratio << endl;
}

void compareFiles2(const string& file1, const string& file2) {
    ifstream f1(file1);
    ifstream f2(file2);
    
    if (!f1.is_open()) {
        cerr << "Error: Could not open file " << file1 << endl;
        return;
    }
    
    if (!f2.is_open()) {
        cerr << "Error: Could not open file " << file2 << endl;
        f1.close();
        return;
    }
    
    string line1, line2;
    int lineNum = 1;
    bool filesEqual = true;
    vector<pair<int, string>> differences;
    
    while (getline(f1, line1) || getline(f2, line2)) {
        if (f1.eof() && !f2.eof()) {
            differences.emplace_back(lineNum, "File1 has ended but File2 has more lines");
            filesEqual = false;
            break;
        }
        
        if (!f1.eof() && f2.eof()) {
            differences.emplace_back(lineNum, "File2 has ended but File1 has more lines");
            filesEqual = false;
            break;
        }
        
        if (line1 != line2) {
            differences.emplace_back(lineNum, "Difference found");
            filesEqual = false;
        }
        
        lineNum++;
    }
    
    f1.close();
    f2.close();
    
    if (filesEqual) {
        cout << "The files are identical." << endl;
    } else {
        cout << "The files are different. Differences found:" << endl;
        for (const auto& diff : differences) {
            cout << "Line " << diff.first << ": " << diff.second << endl;
            // For more detailed output, you could show the actual lines here
        }
    }
}

/*
//for string
class LZWTrieCompressor1 {
public:
    LZWTrieCompressor() {
        resetDictionary();
    }

    vector<int> compress(const string& data) {
        resetDictionary();
        vector<int> compressed;
        string w;

        for (char c : data) {
            string wc = w + c;
            if (trieContains(wc)) {
                w = wc;
            } else {
                compressed.push_back(getCode(w));
                addToTrie(wc);
                w = string(1, c);
            }
        }

        if (!w.empty()) {
            compressed.push_back(getCode(w));
        }

        return compressed;
    }

    string decompress(const vector<int>& compressed) {
        unordered_map<int, string> dictionary;
        for (int i = 0; i < 256; i++) {
            dictionary[i] = string(1, static_cast<char>(i));
        }

        string decompressed;
        string w = dictionary[compressed[0]];
        decompressed += w;

        int next_code = 256;
        for (size_t i = 1; i < compressed.size(); i++) {
            int code = compressed[i];
            string entry;

            if (dictionary.count(code)) {
                entry = dictionary[code];
            } else if (code == next_code) {
                entry = w + w[0];
            } else {
                throw runtime_error("Invalid compressed code");
            }

            decompressed += entry;
            dictionary[next_code] = w + entry[0];
            next_code++;
            w = entry;
        }

        return decompressed;
    }

private:
    struct TrieNode {
        int code;
        unordered_map<char, unique_ptr<TrieNode>> children;
        TrieNode() : code(-1) {}
    };

    unique_ptr<TrieNode> root;
    int next_code;

    void resetDictionary() {
        root = make_unique<TrieNode>();
        next_code = 0;
        for (int i = 0; i < 256; i++) {
            addToTrie(string(1, static_cast<char>(i)));
        }
    }

    bool trieContains(const string& s) {
        TrieNode* current = root.get();
        for (char c : s) {
            if (current->children.find(c) == current->children.end()) {
                return false;
            }
            current = current->children[c].get();
        }
        return true;
    }

    int getCode(const string& s) {
        TrieNode* current = root.get();
        for (size_t i = 0; i < s.length(); i++) {
            char c = s[i];
            if (current->children.find(c) == current->children.end()) {
                throw runtime_error("String not in trie");
            }
            if (i == s.length() - 1) {
                return current->children[c]->code;
            }
            current = current->children[c].get();
        }
        throw runtime_error("Empty string");
    }

    void addToTrie(const string& s) {
        TrieNode* current = root.get();
        for (size_t i = 0; i < s.length(); i++) {
            char c = s[i];
            if (current->children.find(c) == current->children.end()) {
                current->children[c] = make_unique<TrieNode>();
            }
            if (i == s.length() - 1) {
                current->children[c]->code = next_code++;
            }
            current = current->children[c].get();
        }
    }
};
*/


/*
class LZWTrieCompressor2 {
//for vector<string>
public:
    LZWTrieCompressor() {
        resetDictionary();
    }

    vector<int> compress(const vector<string>& data) {
        resetDictionary();
        vector<int> compressed;
        vector<string> w;

        for (const string& token : data) {
            vector<string> wc = w;
            wc.push_back(token);
            if (trieContains(wc)) {
                w = wc;
            } else {
                compressed.push_back(getCode(w));
                addToTrie(wc);
                w = {token};
            }
        }

        if (!w.empty()) {
            compressed.push_back(getCode(w));
        }

        return compressed;
    }

    vector<string> decompress(const vector<int>& compressed) {
        unordered_map<int, vector<string>> dictionary;
        // Initialize with single-token sequences
        for (int i = 0; i < 256; i++) {
            dictionary[i] = {string(1, static_cast<char>(i))};
        }

        vector<string> decompressed;
        vector<string> w = dictionary[compressed[0]];
        decompressed.insert(decompressed.end(), w.begin(), w.end());

        int next_code = 256;
        for (size_t i = 1; i < compressed.size(); i++) {
            int code = compressed[i];
            vector<string> entry;

            if (dictionary.count(code)) {
                entry = dictionary[code];
            } else if (code == next_code) {
                entry = w;
                entry.push_back(w[0]);
            } else {
                throw runtime_error("Invalid compressed code");
            }

            decompressed.insert(decompressed.end(), entry.begin(), entry.end());
            
            // Add w + first token of entry to dictionary
            vector<string> new_entry = w;
            new_entry.push_back(entry[0]);
            dictionary[next_code] = new_entry;
            
            next_code++;
            w = entry;
        }

        return decompressed;
    }

private:
    struct TrieNode {
        int code;
        unordered_map<string, unique_ptr<TrieNode>> children;
        TrieNode() : code(-1) {}
    };

    unique_ptr<TrieNode> root;
    int next_code;

    void resetDictionary() {
        root = make_unique<TrieNode>();
        next_code = 0;
        // Initialize with single-token sequences (ASCII characters)
        for (int i = 0; i < 256; i++) {
            addToTrie({string(1, static_cast<char>(i))});
        }
    }

    bool trieContains(const vector<string>& sequence) {
        TrieNode* current = root.get();
        for (const string& token : sequence) {
            if (current->children.find(token) == current->children.end()) {
                return false;
            }
            current = current->children[token].get();
        }
        return true;
    }

    int getCode(const vector<string>& sequence) {
        TrieNode* current = root.get();
        for (size_t i = 0; i < sequence.size(); i++) {
            const string& token = sequence[i];
            if (current->children.find(token) == current->children.end()) {
                throw runtime_error("Sequence not in trie");
            }
            if (i == sequence.size() - 1) {
                return current->children[token]->code;
            }
            current = current->children[token].get();
        }
        throw runtime_error("Empty sequence");
    }

    void addToTrie(const vector<string>& sequence) {
        TrieNode* current = root.get();
        for (size_t i = 0; i < sequence.size(); i++) {
            const string& token = sequence[i];
            if (current->children.find(token) == current->children.end()) {
                current->children[token] = make_unique<TrieNode>();
            }
            if (i == sequence.size() - 1) {
                current->children[token]->code = next_code++;
            }
            current = current->children[token].get();
        }
    }
};
*/


/*
class LZWTrieCompressor3 {
public:
    LZWTrieCompressor() {
        resetDictionary();
    }

    vector<int> compress(const vector<string>& data) {
        if (data.empty()) {
            return {};  // Return empty vector for empty input
        }

        resetDictionary();
        vector<int> compressed;
        vector<string> w;

        for (const string& token : data) {
            vector<string> wc = w;
            wc.push_back(token);
            if (trieContains(wc)) {
                w = wc;
            } else {
                if (!w.empty()) {  // Only add non-empty sequences
                    compressed.push_back(getCode(w));
                }
                addToTrie(wc);
                w = {token};
            }
        }

        if (!w.empty()) {
            compressed.push_back(getCode(w));
        }

        return compressed;
    }

    vector<string> decompress(const vector<int>& compressed) {
        if (compressed.empty()) {
            return {};  // Return empty vector for empty input
        }

        unordered_map<int, vector<string>> dictionary;
        // Initialize with single-token sequences
        for (int i = 0; i < 256; i++) {
            dictionary[i] = {string(1, static_cast<char>(i))};
        }

        vector<string> decompressed;
        vector<string> w = dictionary[compressed[0]];
        decompressed.insert(decompressed.end(), w.begin(), w.end());

        int next_code = 256;
        for (size_t i = 1; i < compressed.size(); i++) {
            int code = compressed[i];
            vector<string> entry;

            if (dictionary.count(code)) {
                entry = dictionary[code];
            } else if (code == next_code) {
                entry = w;
                entry.push_back(w[0]);
            } else {
                throw runtime_error("Invalid compressed code");
            }

            decompressed.insert(decompressed.end(), entry.begin(), entry.end());
            
            // Add w + first token of entry to dictionary
            vector<string> new_entry = w;
            new_entry.push_back(entry[0]);
            dictionary[next_code] = new_entry;
            
            next_code++;
            w = entry;
        }

        return decompressed;
    }

private:
    struct TrieNode {
        int code;
        unordered_map<string, unique_ptr<TrieNode>> children;
        TrieNode() : code(-1) {}
    };

    unique_ptr<TrieNode> root;
    int next_code;

    void resetDictionary() {
        root = make_unique<TrieNode>();
        next_code = 0;
        // Initialize with single-token sequences (ASCII characters)
        for (int i = 0; i < 256; i++) {
            addToTrie({string(1, static_cast<char>(i))});
        }
    }

    bool trieContains(const vector<string>& sequence) {
        if (sequence.empty()) return false;
        
        TrieNode* current = root.get();
        for (const string& token : sequence) {
            if (current->children.find(token) == current->children.end()) {
                return false;
            }
            current = current->children[token].get();
        }
        return true;
    }

    int getCode(const vector<string>& sequence) {
        if (sequence.empty()) {
            throw runtime_error("Cannot get code for empty sequence");
        }

        TrieNode* current = root.get();
        for (size_t i = 0; i < sequence.size(); i++) {
            const string& token = sequence[i];
            if (current->children.find(token) == current->children.end()) {
                throw runtime_error("Sequence not in trie");
            }
            if (i == sequence.size() - 1) {
                return current->children[token]->code;
            }
            current = current->children[token].get();
        }
        throw runtime_error("Unexpected error in getCode");
    }

    void addToTrie(const vector<string>& sequence) {
        if (sequence.empty()) return;
        
        TrieNode* current = root.get();
        for (size_t i = 0; i < sequence.size(); i++) {
            const string& token = sequence[i];
            if (current->children.find(token) == current->children.end()) {
                current->children[token] = make_unique<TrieNode>();
            }
            if (i == sequence.size() - 1) {
                current->children[token]->code = next_code++;
            }
            current = current->children[token].get();
        }
    }
};
*/



