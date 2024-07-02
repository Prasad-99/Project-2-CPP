#include "ConfigManager.h"
#include "helper.h"
#include <fstream>
#include <iostream>
#include "Constants.h"


ConfigManager* ConfigManager::pSingleton= NULL;

ConfigManager::ConfigManager()
{
   // do init stuff
    cout<<"Initalise Config Manager"<<endl;
}

ConfigManager* ConfigManager::GetInstance()
{
	if (pSingleton== NULL) {
		pSingleton = new ConfigManager();
	}
	return pSingleton;
}

void ConfigManager::loadConfig(std::string configFilePathString){
    std::string fileLine = "";
    std::ifstream configFilePath(configFilePathString);

    cout<<"--- Loading Configs ---"<<endl;
    while (getline(configFilePath, fileLine))
    {
        string configKey = fileLine.substr(0, fileLine.find(FILE_DELIMITER));
        string configValue = (fileLine.substr(fileLine.find(FILE_DELIMITER) + FILE_DELIMITER.length()));
        cout << "Config: " << configKey << " Value: " << configValue << endl;
        configs[configKey] = configValue;

    }
    cout<<"---Finish Loading Configs ---"<<endl;
    cout<<endl;
}
string ConfigManager::getConfig(std::string configKey){

    if (configs.find(configKey) != configs.end())
    {
        return configs[configKey];
    }
    cout<<"Unable to find config for key:"<<configKey<<endl;
    return "";
}

// prints values of member variables.
void ConfigManager::printAllConfig()
{
    if(configs.size() == 0){
        cout<<"--- No Configs to print------ "<<endl;
        return;
    }

    for (size_t i =0 ; i<configs.size();i++){
            cout<<"Printing"<<endl;
        cout << "Config: " << configKey << " Value: " << configs[configKey] << endl;
    }
}
