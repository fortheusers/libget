#ifndef OSC_REPO_H
#define OSC_REPO_H
#include "Package.hpp"
#include "Repo.hpp"
#include <iostream>
#include <vector>

class OSCRepo : public Repo
{
public:
	using Repo::Repo;
	[[nodiscard]] std::string getType() const override;
	[[nodiscard]] std::string getZipUrl(const Package& package) const override;
	[[nodiscard]] std::string getIconUrl(const Package& package) const override;

private:
	[[maybe_unused]] std::vector<std::unique_ptr<Package>> loadPackages() override;
};
#endif
