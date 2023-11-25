#ifndef REPO_H
#define REPO_H
#include "Package.hpp"
#include <iostream>
#include <vector>

class Get;

#ifdef NETWORK_MOCK
#include "../tests/network_mock.hpp"
#endif

class Repo
{
public:
	Repo();
	Repo(std::string_view name, std::string_view url, bool enabled);
	virtual ~Repo() = default;

	[[nodiscard]] virtual std::string getType() const = 0;
	[[nodiscard]] virtual std::string getZipUrl(const Package& package) const = 0;
	[[nodiscard]] virtual std::string getIconUrl(const Package& package) const = 0;

	[[nodiscard]] std::string toJson() const;
	[[nodiscard]] std::string toString() const;

	[[nodiscard]] std::string getName() const;
	[[nodiscard]] std::string getUrl() const;
	[[nodiscard]] bool isEnabled() const;
	[[nodiscard]] bool isLoaded() const; // whether this server could be reached or not

	void setEnabled(bool enabled);

protected:

	std::string name;
	std::string url;
	bool enabled {};
	bool loaded = true;

	static std::string generateRepoJson(const Repo& repo);

	static std::unique_ptr<Repo> createRepo(std::string_view name, std::string_view url, bool enabled, std::string_view type, std::string_view package_path);

private:
	virtual std::vector<std::unique_ptr<Package>> loadPackages() = 0;

	friend Get;
};

#endif
