#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "FamilyNode.h"
#include <string>

bool LoadFromCsv(const std::string& filePath, FamilyTree& familyTree);
bool SaveToCsv(const std::string& filePath, const FamilyTree& familyTree);

#endif
