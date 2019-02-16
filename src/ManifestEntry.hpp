#ifndef MANIFESTENTRY_H_
#define MANIFESTENTRY_H_
#include <string>
enum class ManifestAction{
    Update,
    Get,
    Local,
    Extract,
	None
};
class ManifestEntry {
    public:
        std::string path;
        ManifestAction action;
        ManifestEntry(std::string line)
        {
            switch (line.at(0))
            {
                case 'U':
                    this->action = ManifestAction::Update;
                    break;
                case 'G':
                    this->action = ManifestAction::Get;
                    break;
                case 'L':
                    this->action = ManifestAction::Local;
                    break;
                case 'E':
                    this->action = ManifestAction::Extract;
                    break;
                default:
                    this->action = ManifestAction::None;
                    break;
            }
            this->path = line.substr(3);
        }
};
#endif