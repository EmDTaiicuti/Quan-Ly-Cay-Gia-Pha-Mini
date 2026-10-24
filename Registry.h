#ifndef REGISTRY_H
#define REGISTRY_H
#include <string>
#include <iosfwd>
struct Achievement {
    int id; int memberId; int year;
    std::string title; std::string organization; std::string description;
    Achievement* next;
    Achievement() : id(0), memberId(0), year(0), title(), organization(), description(), next(0) {}
};
struct AchievementList { Achievement* head; AchievementList() : head(0) {} };
struct FamilyRule {
    int id; std::string title; std::string content; std::string effectiveDate;
    bool active; FamilyRule* next;
    FamilyRule() : id(0), title(), content(), effectiveDate(), active(true), next(0) {}
};
struct RuleList { FamilyRule* head; RuleList() : head(0) {} };
void DestroyAchievements(AchievementList& list);
bool LoadAchievements(const std::string& path, AchievementList& list);
bool SaveAchievements(const std::string& path, const AchievementList& list);
bool AddAchievement(AchievementList& list, int memberId, int year,
                    const std::string& title, const std::string& organization,
                    const std::string& description);
bool RemoveAchievement(AchievementList& list, int id);
void PrintAchievements(const AchievementList& list, int memberId, std::ostream& output);
void DestroyRules(RuleList& list);
bool LoadRules(const std::string& path, RuleList& list);
bool SaveRules(const std::string& path, const RuleList& list);
bool AddRule(RuleList& list, const std::string& title, const std::string& content,
             const std::string& effectiveDate);
bool DeactivateRule(RuleList& list, int id);
void PrintRules(const RuleList& list, std::ostream& output, bool includeInactive);
#endif
