#include "PatternDatabases/CornerPatternDatabase.h"

#include <cstring>
#include <fstream>

namespace {
const char MAGIC[8] = {'R', 'C', 'C', 'O', 'R', 'N', 'v', '1'};
}

bool CornerPatternDatabase::save(const std::string& path) const {
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    out.write(MAGIC, sizeof MAGIC);
    const uint64_t n = db.size();
    out.write(reinterpret_cast<const char*>(&n), sizeof n);
    out.write(reinterpret_cast<const char*>(db.data()), static_cast<std::streamsize>(db.storageBytes()));
    return static_cast<bool>(out);
}

bool CornerPatternDatabase::load(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    char magic[8];
    uint64_t n = 0;
    in.read(magic, sizeof magic);
    in.read(reinterpret_cast<char*>(&n), sizeof n);
    if (!in || std::memcmp(magic, MAGIC, sizeof MAGIC) != 0 || n != db.size()) return false;
    in.read(reinterpret_cast<char*>(db.data()), static_cast<std::streamsize>(db.storageBytes()));
    return static_cast<bool>(in);
}
