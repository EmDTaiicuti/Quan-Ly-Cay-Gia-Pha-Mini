#include "FileManager.h"
#include "TreeCore.h"
#include <fstream>
#include <sstream>

namespace {
struct PendingRecord {
    Member member; int parentId; bool built; PendingRecord* next;
    PendingRecord() : member(), parentId(0), built(false), next(0) {}
};
std::string EscapeCsv(const std::string& value) {
    bool quote = false; std::string result;
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == ',' || value[i] == '"' || value[i] == '\n' || value[i] == '\r') quote = true;
        if (value[i] == '"') result += "\"\""; else result += value[i];
    }
    return quote ? "\"" + result + "\"" : result;
}
bool ParseCsv(const std::string& line, std::string fields[], int capacity, int& count) {
    count = 0; std::string field; bool quoted = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        char ch = line[i];
        if (ch == '"') {
            if (quoted && i + 1 < line.size() && line[i + 1] == '"') { field += '"'; ++i; }
            else quoted = !quoted;
        } else if (ch == ',' && !quoted) {
            if (count >= capacity) return false;
            fields[count++] = field; field.clear();
        } else field += ch;
    }
    if (quoted || count >= capacity) return false;
    fields[count++] = field; return true;
}
bool ToInt(const std::string& text, int& value) {
    std::istringstream input(text); input >> value; return input && input.eof();
}
void StripBom(std::string& line) {
    if (line.size() >= 3 && static_cast<unsigned char>(line[0]) == 0xEF &&
        static_cast<unsigned char>(line[1]) == 0xBB && static_cast<unsigned char>(line[2]) == 0xBF) line.erase(0, 3);
}
void AppendPending(PendingRecord*& head, PendingRecord*& tail, PendingRecord* item) {
    if (!head) head = tail = item; else { tail->next = item; tail = item; }
}
bool ReadRecord(const std::string& line, PendingRecord*& head, PendingRecord*& tail) {
    std::string fields[12]; int count = 0;
    if (!ParseCsv(line, fields, 12, count) || (count != 7 && count != 12)) return false;
    PendingRecord* item = new PendingRecord();
    if (!ToInt(fields[0], item->member.id) || !ToInt(fields[1], item->parentId) ||
        !ToInt(fields[2], item->member.spouseId) || fields[4].size() != 1) { delete item; return false; }
    item->member.name = fields[3]; item->member.gender = fields[4][0];
    item->member.birthDate = NormalizeDate(fields[5]);
    if (count == 7) { item->member.notes = fields[6]; item->member.isInLaw = item->member.spouseId > 0; }
    else {
        item->member.deathDate = NormalizeDate(fields[6]); item->member.hometown = fields[7];
        item->member.biography = fields[8]; item->member.photoPath = fields[9];
        item->member.isInLaw = fields[10] == "1"; item->member.notes = fields[11];
    }
    AppendPending(head, tail, item); return true;
}
void SaveRecursive(std::ofstream& output, Node* node, int parentId) {
    if (!node) return;
    const Member& m = node->member;
    output << m.id << ',' << parentId << ',' << m.spouseId << ',' << EscapeCsv(m.name) << ','
           << m.gender << ',' << EscapeCsv(m.birthDate) << ',' << EscapeCsv(m.deathDate) << ','
           << EscapeCsv(m.hometown) << ',' << EscapeCsv(m.biography) << ',' << EscapeCsv(m.photoPath) << ','
           << (m.isInLaw ? 1 : 0) << ',' << EscapeCsv(m.notes) << '\n';
    SaveRecursive(output, node->firstChild, m.id);
    SaveRecursive(output, node->nextSibling, parentId);
}
}

bool LoadFromCsv(const std::string& filePath, FamilyTree& familyTree) {
    std::ifstream input(filePath.c_str());
    if (!input.is_open()) {
        std::ofstream createFile(filePath.c_str(), std::ios::binary);
        if (!createFile.is_open()) return false;
        createFile.write("\xEF\xBB\xBF", 3);
        createFile << "id,parentId,spouseId,name,gender,birthDate_DD/MM/YYYY,deathDate_DD/MM/YYYY,hometown,biography,photoPath,isInLaw,notes\n";
        return true;
    }
    PendingRecord* head = 0; PendingRecord* tail = 0; std::string line;
    bool firstLine = true;
    while (std::getline(input, line)) {
        if (line.empty()) continue;
        if (firstLine) { StripBom(line); firstLine = false; if (line.find("id,parentId,") == 0) continue; }
        ReadRecord(line, head, tail);
    }
    input.close();
    // Cho phép file sắp xếp tùy ý: mỗi lượt gắn những bản ghi có cha đã tồn tại;
    // vòng lặp dừng khi không còn tiến triển, tránh lặp vô hạn với ID cha sai/vòng.
    bool progress = true;
    while (progress) {
        progress = false;
        for (PendingRecord* item = head; item; item = item->next) {
            if (item->built) continue;
            bool added = item->parentId == 0 ? AddRoot(familyTree, item->member) :
                (FindMember(familyTree.root, item->parentId) && AddChild(familyTree, item->parentId, item->member));
            if (added) { item->built = true; progress = true; }
        }
    }
    while (head) { PendingRecord* item = head; head = head->next; delete item; }
    return true;
}

bool SaveToCsv(const std::string& filePath, const FamilyTree& familyTree) {
    std::ofstream output(filePath.c_str(), std::ios::binary); if (!output.is_open()) return false;
    output.write("\xEF\xBB\xBF", 3);
    output << "id,parentId,spouseId,name,gender,birthDate_DD/MM/YYYY,deathDate_DD/MM/YYYY,hometown,biography,photoPath,isInLaw,notes\n";
    SaveRecursive(output, familyTree.root, 0); return output.good();
}





