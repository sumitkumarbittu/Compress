#include "crow.h"
#include "lzw_compressor.h"
#include "database.h"
#include <fstream>
#include <sstream>
#include <filesystem>
namespace fs = std::filesystem;

int main() {
    crow::SimpleApp app;
    CompressionDatabase db("compression.db");

    // Serve static files from public directory
    CROW_ROUTE(app, "/static/<string>")
    ([](const crow::request&, crow::response& res, std::string filename) {
        std::string path = "public/" + filename;
        if (fs::exists(path)) {
            res.set_static_file_info(path);
            res.end();
        } else {
            res.code = 404;
            res.end("File not found");
        }
    });

    // Home page - compression form
    CROW_ROUTE(app, "/")
    ([]() {
        std::ifstream in("public/index.html");
        std::ostringstream content;
        content << in.rdbuf();
        return crow::response(content.str());
    });

    // Compression endpoint
    CROW_ROUTE(app, "/compress").methods("POST"_method)
    ([&db](const crow::request& req) {
        crow::multipart::message msg(req);
        if (msg.parts.size() == 0) {
            return crow::response(400, "No file uploaded");
        }

        auto& part = msg.parts[0];
        std::string filename = part.get_header_value("filename");
        std::string content = part.body;

        // Save original file
        fs::create_directory("uploads");
        std::string original_path = "uploads/" + filename;
        std::ofstream out(original_path, std::ios::binary);
        out << content;
        out.close();

        // Compress
        LZWTrieCompressor lzw;
        std::vector<std::string> original = {content}; // Simplified for demo
        std::vector<int> compressed = lzw.compress(original);
        
        // Save compressed data
        std::string compressed_path = "uploads/compressed_" + filename + ".lzw";
        std::ofstream comp_out(compressed_path, std::ios::binary);
        for (int num : compressed) {
            comp_out << num << " ";
        }
        comp_out.close();

        // Get sizes
        size_t original_size = fs::file_size(original_path);
        size_t compressed_size = fs::file_size(compressed_path);
        double ratio = static_cast<double>(compressed_size) / original_size;
        double space_saving = 1.0 - ratio;

        // Save to database
        db.saveCompressionStats(filename, "compressed_" + filename + ".lzw",
                              original_size, compressed_size, ratio, space_saving);

        // Return results
        crow::json::wvalue result;
        result["original_size"] = original_size;
        result["compressed_size"] = compressed_size;
        result["ratio"] = ratio;
        result["space_saving"] = space_saving;
        result["compressed_filename"] = "compressed_" + filename + ".lzw";
        return crow::response(result);
    });

    // Decompression endpoint
    CROW_ROUTE(app, "/decompress").methods("POST"_method)
    ([&db](const crow::request& req) {
        crow::multipart::message msg(req);
        if (msg.parts.size() == 0) {
            return crow::response(400, "No file uploaded");
        }

        auto& part = msg.parts[0];
        std::string filename = part.get_header_value("filename");
        std::string content = part.body;

        // Save compressed file
        fs::create_directory("uploads");
        std::string compressed_path = "uploads/" + filename;
        std::ofstream out(compressed_path, std::ios::binary);
        out << content;
        out.close();

        // Read compressed data
        std::ifstream comp_in(compressed_path);
        std::vector<int> compressed;
        int num;
        while (comp_in >> num) {
            compressed.push_back(num);
        }
        comp_in.close();

        // Decompress
        LZWTrieCompressor lzw;
        std::vector<std::string> decompressed = lzw.decompress(compressed);
        
        // Save decompressed data
        std::string decompressed_path = "uploads/decompressed_" + filename.substr(0, filename.find(".lzw"));
        std::ofstream decomp_out(decompressed_path);
        for (const auto& str : decompressed) {
            decomp_out << str;
        }
        decomp_out.close();

        // Return results
        crow::json::wvalue result;
        result["decompressed_filename"] = "decompressed_" + filename.substr(0, filename.find(".lzw"));
        return crow::response(result);
    });

    // History page
    CROW_ROUTE(app, "/history")
    ([&db]() {
        std::ifstream in("public/history.html");
        std::ostringstream content;
        content << in.rdbuf();
        return crow::response(content.str());
    });

    // API to get history data
    CROW_ROUTE(app, "/api/history")
    ([&db]() {
        auto history = db.getHistory(); // You'll need to implement this in database.h
        crow::json::wvalue result;
        int i = 0;
        for (const auto& entry : history) {
            result[i]["id"] = entry.id;
            result[i]["original_filename"] = entry.original_filename;
            result[i]["compressed_filename"] = entry.compressed_filename;
            result[i]["original_size"] = entry.original_size;
            result[i]["compressed_size"] = entry.compressed_size;
            result[i]["ratio"] = entry.ratio;
            result[i]["space_saving"] = entry.space_saving;
            result[i]["timestamp"] = entry.timestamp;
            i++;
        }
        return crow::response(result);
    });

    // File download
    CROW_ROUTE(app, "/download/<string>")
    ([](const crow::request&, crow::response& res, std::string filename) {
        std::string path = "uploads/" + filename;
        if (fs::exists(path)) {
            res.set_static_file_info(path);
            res.add_header("Content-Disposition", "attachment; filename=\"" + filename + "\"");
            res.end();
        } else {
            res.code = 404;
            res.end("File not found");
        }
    });

    app.port(8080).multithreaded().run();
}