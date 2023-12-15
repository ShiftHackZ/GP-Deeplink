#include <iostream>
#include <string>
#include <fstream>

#include "models/Deeplink.h"
#include "models/Configuration.h"
#include "libs/json.hpp"

namespace fs = std::filesystem;

using json = nlohmann::json;

const std::string VERSION = "0.0.1";

const std::string COMMAND_CLEAN = "clean";
const std::string COMMAND_GENERATE = "generate";
const std::string COMMAND_VERSION = "version";

const std::string PARAM_URL = "{{URL}}";

Configuration readConfiguration() {
    std::ifstream file("config.json");

    Configuration config;

    if (!file.is_open()) {
        std::cerr << "Failed to open the JSON file." << std::endl;
        return config;
    }

    json jsonData;
    file >> jsonData;

    config.outputFolder = jsonData["output_folder"];
    for (const auto& deeplinkData : jsonData["deeplinks"]) {
        Deeplink deeplink;
        deeplink.templateName = deeplinkData["template"];
        deeplink.deeplink = deeplinkData["deeplink"];
        deeplink.url = deeplinkData["url"];
        config.deeplinks.push_back(deeplink);
    }

    return config;
}

std::string getTemplateContents(const Deeplink& deeplink) {
    try {
        std::string templateFilePath = "templates/" + deeplink.templateName + ".template";

        std::ifstream templateFile(templateFilePath);
        if (!templateFile.is_open()) {
            std::cerr << "Error: Unable to open template file: " << templateFilePath << std::endl;
            return "";
        }

        std::stringstream buffer;
        buffer << templateFile.rdbuf();
        std::string templateContents = buffer.str();

        templateFile.close();

        size_t urlPos = templateContents.find(PARAM_URL);
        while (urlPos != std::string::npos) {
            templateContents.replace(urlPos, PARAM_URL.length(), deeplink.url); 
            urlPos = templateContents.find(PARAM_URL, urlPos + 1);
        }

        return templateContents;
    } catch (const std::exception& e) {
        return "";
    }
}

bool clean(const std::string& path) {
    try {
        if (!fs::exists(path)) {
            return false;
        }
        if (!fs::is_directory(path)) {
            return false;
        }
        fs::remove_all(path);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool createDirectory(const std::string& path) {
    try {
        if (fs::exists(path)) {
            return false;
        }
        fs::create_directory(path);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool generate(const Configuration& config) {
    try {
        createDirectory(config.outputFolder);

        for (const auto& deeplink : config.deeplinks) {
            std::string deeplinkPath = config.outputFolder + "/" + deeplink.deeplink;
            createDirectory(deeplinkPath);

            std::string indexPath = deeplinkPath + "/index.html";
            std::string templateContent = getTemplateContents(deeplink);
            std::ofstream outputFile(indexPath);

            if (outputFile.is_open()) {
                outputFile << templateContent;
                outputFile.close();
            } 
        }

        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void printUsage(char* argv[]) {
    std::cerr << "Usage: " << argv[0] << " command [--verbose] [--help]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Commands: " << std::endl;
    std::cerr << "  clean       Cleans previously generated static redirect sitemap files." << std::endl;
    std::cerr << "  generate    Generates a deeplink redirect sitemap files based on config." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv);
        return 1;
    }

    std::string command = argv[1];

    if (command == COMMAND_VERSION) {
        std::cout << "GitHub Pages Deeplink generator v" << VERSION << std::endl;
        return 0;
    }

    if (command == COMMAND_CLEAN) {
        Configuration config = readConfiguration();
        if (clean(config.outputFolder)) {
            return 0;
        }
        std::cerr << "Error cleaning ouptut folder: " << config.outputFolder << std::endl;
        return 1;
    }

    if (command == COMMAND_GENERATE) {
        Configuration config = readConfiguration();
        if (fs::exists(config.outputFolder)) {
            std::cerr << "Seems like output folder already exists: " << config.outputFolder << std::endl;
            std::cerr << "Try to run: gdeeplink clean" << std::endl;
            return 1;
        }
        if (generate(config)) {
            return 0;
        }
        std::cerr << "Error generating deeplinks: " << std::endl;
        return 1;
    }

    std::cerr << "Unknown command: " << command << std::endl;
    return 1;
}
