#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>

using namespace std;

class CompressionDatabase {
private:
    sqlite3* db;
    
    void initializeDatabase() {
        const char* sql = "CREATE TABLE IF NOT EXISTS compression_history ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "original_filename TEXT NOT NULL,"
                          "compressed_filename TEXT NOT NULL,"
                          "original_size INTEGER NOT NULL,"
                          "compressed_size INTEGER NOT NULL,"
                          "compression_ratio REAL NOT NULL,"
                          "space_saving REAL NOT NULL,"
                          "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";
        
        char* errMsg = nullptr;
        if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            cerr << "SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
        }
    }

public:
    CompressionDatabase(const string& dbName) {
        if (sqlite3_open(dbName.c_str(), &db) != SQLITE_OK) {
            cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
            db = nullptr;
        } else {
            initializeDatabase();
        }
    }

    ~CompressionDatabase() {
        if (db) {
            sqlite3_close(db);
        }
    }

    bool isOpen() const {
        return db != nullptr;
    }

    void saveCompressionStats(const string& originalFile, 
                            const string& compressedFile,
                            size_t originalSize,
                            size_t compressedSize,
                            double ratio,
                            double spaceSaving) {
        if (!db) return;

        const char* sql = "INSERT INTO compression_history "
                         "(original_filename, compressed_filename, original_size, "
                         "compressed_size, compression_ratio, space_saving) "
                         "VALUES (?, ?, ?, ?, ?, ?);";
        
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
            return;
        }

        sqlite3_bind_text(stmt, 1, originalFile.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, compressedFile.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 3, originalSize);
        sqlite3_bind_int64(stmt, 4, compressedSize);
        sqlite3_bind_double(stmt, 5, ratio);
        sqlite3_bind_double(stmt, 6, spaceSaving);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << endl;
        }

        sqlite3_finalize(stmt);
    }

    void displayHistory() {
        if (!db) return;

        const char* sql = "SELECT id, original_filename, compressed_filename, "
                          "original_size, compressed_size, compression_ratio, "
                          "space_saving, timestamp FROM compression_history "
                          "ORDER BY timestamp DESC;";
        
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
            return;
        }

        cout << "\nCompression History:\n";
        cout << "========================================================================================================\n";
        cout << "ID  | Original File       | Compressed File     | Original Size | Compressed Size | Ratio   | Savings  | Timestamp\n";
        cout << "--------------------------------------------------------------------------------------------------------\n";

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char* origFile = sqlite3_column_text(stmt, 1);
            const unsigned char* compFile = sqlite3_column_text(stmt, 2);
            int origSize = sqlite3_column_int(stmt, 3);
            int compSize = sqlite3_column_int(stmt, 4);
            double ratio = sqlite3_column_double(stmt, 5);
            double savings = sqlite3_column_double(stmt, 6);
            const unsigned char* timestamp = sqlite3_column_text(stmt, 7);

            printf("%-3d | %-19s | %-19s | %-13d | %-15d | %-7.2f | %-7.2f%% | %s\n",
                   id, origFile, compFile, origSize, compSize, ratio, savings * 100, timestamp);
        }

        cout << "========================================================================================================\n";
        sqlite3_finalize(stmt);
    }

    void clearHistory() {
        if (!db) return;

        const char* sql = "DELETE FROM compression_history;";
        char* errMsg = nullptr;
        if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            cerr << "SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
        } else {
            cout << "Compression history cleared successfully.\n";
        }
    }
};

#endif // DATABASE_H