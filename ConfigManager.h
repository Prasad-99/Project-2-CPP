#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H
#include <string>
#include <unordered_map>

class ConfigManager {
public:
   static ConfigManager* GetInstance();
   void loadConfig(std::string filePath);
   std::string getConfig(std::string configKey);
   void printAllConfig();
private:
   ConfigManager();
   static ConfigManager* pSingleton;		// singleton instance
   std::string configKey;
   std::string configValue;
   std::unordered_map<std::string, std::string> configs;
};

#endif // CONFIGMANAGER_H_INCLUDED
