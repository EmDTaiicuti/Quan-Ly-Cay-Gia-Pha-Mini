#include "Registry.h"
#include "TreeCore.h"
#include <fstream>
#include <sstream>

namespace {
std::string Escape(const std::string& value) {
    std::string result; bool quote = false;
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == ',' || value[i] == '"' || value[i] == '\n' || value[i] == '\r') quote = true;
        if (value[i] == '"') result += "\"\""; else result += value[i];
    }
    return quote ? "\"" + result + "\"" : result;
}
bool Parse(const std::string& line, std::string fields[], int count) {
    int index = 0; bool quoted = false; std::string field;
    for (std::size_t i = 0; i < line.size(); ++i) {
        char ch = line[i];
        if (ch == '"') {
            if (quoted && i + 1 < line.size() && line[i + 1] == '"') { field += '"'; ++i; }
            else quoted = !quoted;
        } else if (ch == ',' && !quoted) {
            if (index >= count) return false;
            fields[index++] = field; field.clear();
        } else field += ch;
    }
    if (quoted || index >= count) return false;
    fields[index++] = field;
    return index == count;
}
bool ParseInt(const std::string& text, int& value) {
    std::istringstream input(text); input >> value; return input && input.eof();
}
std::string UnquoteBOM(std::string value) {
    if (value.size() >= 3 && static_cast<unsigned char>(value[0]) == 0xEF &&
        static_cast<unsigned char>(value[1]) == 0xBB && static_cast<unsigned char>(value[2]) == 0xBF)
        value.erase(0, 3);
    return value;
}
}

void DestroyAchievements(AchievementList& list) {
    while (list.head) { Achievement* item = list.head; list.head = item->next; delete item; }
}

bool LoadAchievements(const std::string& path, AchievementList& list) {
    std::ifstream input(path.c_str());
    if (!input.is_open()) {
        std::ofstream create(path.c_str(), std::ios::binary); if (!create.is_open()) return false;
        create.write("\xEF\xBB\xBF", 3);
        create << "id,memberId,year,title,organization,description\n"; return true;
    }
    std::string line; std::getline(input, line);
    while (std::getline(input, line)) {
        std::string f[6]; if (line.empty() || !Parse(line, f, 6)) continue;
        Achievement* item = new Achievement();
        if (!ParseInt(f[0], item->id) || !ParseInt(f[1], item->memberId) || !ParseInt(f[2], item->year)) { delete item; continue; }
        item->title = f[3]; item->organization = f[4]; item->description = f[5];
        Achievement** link = &list.head; while (*link) link = &((*link)->next); *link = item;
    }
    return true;
}

bool SaveAchievements(const std::string& path, const AchievementList& list) {
    std::ofstream output(path.c_str(), std::ios::binary); if (!output.is_open()) return false;
    output.write("\xEF\xBB\xBF", 3);
    output << "id,memberId,year,title,organization,description\n";
    for (Achievement* a = list.head; a; a = a->next)
        output << a->id << ',' << a->memberId << ',' << a->year << ',' << Escape(a->title) << ','
               << Escape(a->organization) << ',' << Escape(a->description) << '\n';
    return output.good();
}

bool AddAchievement(AchievementList& list, int memberId, int year, const std::string& title,
                    const std::string& organization, const std::string& description) {
    if (memberId <= 0 || year < 1 || title.empty()) return false;
    int nextId = 1; for (Achievement* a = list.head; a; a = a->next) if (a->id >= nextId) nextId = a->id + 1;
    Achievement* item = new Achievement(); item->id = nextId; item->memberId = memberId; item->year = year;
    item->title = title; item->organization = organization; item->description = description;
    Achievement** link = &list.head; while (*link) link = &((*link)->next); *link = item;
    return true;
}

bool RemoveAchievement(AchievementList& list, int id) {
    Achievement** link = &list.head;
    while (*link && (*link)->id != id) link = &((*link)->next);
    if (!*link) return false;
    Achievement* item = *link; *link = item->next; delete item; return true;
}

void PrintAchievements(const AchievementList& list, int memberId, std::ostream& output) {
    bool found = false;
    for (Achievement* a = list.head; a; a = a->next) if (a->memberId == memberId) {
        found = true; output << "  #" << a->id << " " << a->title << " (" << a->year << ") - "
                             << a->organization << " | " << a->description << '\n';
    }
    if (!found) output << "  Chua co thanh tich duoc ghi nhan.\n";
}

void DestroyRules(RuleList& list) {
    while (list.head) { FamilyRule* item = list.head; list.head = item->next; delete item; }
}

bool LoadRules(const std::string& path, RuleList& list) {
    std::ifstream input(path.c_str());
    if (!input.is_open()) {
        std::ofstream create(path.c_str(), std::ios::binary); if (!create.is_open()) return false;
        create.write("\xEF\xBB\xBF", 3);
        create << "id,title,content,effectiveDate,active\n"; return true;
    }
    std::string line; std::getline(input, line);
    while (std::getline(input, line)) {
        std::string f[5]; if (line.empty() || !Parse(line, f, 5)) continue;
        FamilyRule* item = new FamilyRule();
        if (!ParseInt(f[0], item->id)) { delete item; continue; }
        item->title = f[1]; item->content = f[2]; item->effectiveDate = f[3]; item->active = f[4] == "1";
        FamilyRule** link = &list.head; while (*link) link = &((*link)->next); *link = item;
    }
    return true;
}

bool SaveRules(const std::string& path, const RuleList& list) {
    std::ofstream output(path.c_str(), std::ios::binary); if (!output.is_open()) return false;
    output.write("\xEF\xBB\xBF", 3);
    output << "id,title,content,effectiveDate,active\n";
    for (FamilyRule* r = list.head; r; r = r->next)
        output << r->id << ',' << Escape(r->title) << ',' << Escape(r->content) << ','
               << Escape(r->effectiveDate) << ',' << (r->active ? 1 : 0) << '\n';
    return output.good();
}

bool AddRule(RuleList& list, const std::string& title, const std::string& content,
             const std::string& effectiveDate) {
    if (title.empty() || content.empty() || !IsValidDate(effectiveDate)) return false;
    int nextId = 1; for (FamilyRule* r = list.head; r; r = r->next) if (r->id >= nextId) nextId = r->id + 1;
    FamilyRule* item = new FamilyRule(); item->id = nextId; item->title = title;
    item->content = content; item->effectiveDate = effectiveDate;
    FamilyRule** link = &list.head; while (*link) link = &((*link)->next); *link = item;
    return true;
}

void PrintRules(const RuleList& list, std::ostream& output, bool includeInactive) {
    bool found = false;
    for (FamilyRule* r = list.head; r; r = r->next) if (r->active || includeInactive) {
        found = true; output << "  #" << r->id << " " << r->title << " | Hieu luc: " << r->effectiveDate
                             << " | " << (r->active ? "Dang ap dung" : "Het hieu luc") << '\n'
                             << "    " << r->content << '\n';
    }
    if (!found) output << "  Chua co quy dinh.\n";
}

bool DeactivateRule(RuleList& list, int id) {
    for (FamilyRule* rule = list.head; rule; rule = rule->next) {
        if (rule->id == id && rule->active) { rule->active = false; return true; }
    }
    return false;
}





