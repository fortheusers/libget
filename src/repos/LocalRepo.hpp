#ifndef LOCAL_REPO_H
#define LOCAL_REPO_H
#include "../Package.hpp"
#include "Repo.hpp"
#include <iostream>
#include <vector>

/**
 * A local repository has packages that use the internal format of installed packages.
 *
 * The structure resemblers the packages in GetRepo, however they are loaded from the local
 * directory rather than from a server + json.
 */

class LocalRepo : public Repo
{
public:
	explicit LocalRepo(std::string package_path)
		: Repo()
		, mPkg_path(std::move(package_path))
	{
	}
	[[nodiscard]] std::string getType() const override;
	[[nodiscard]] std::string getZipUrl(const Package& package) const override;
	[[nodiscard]] std::string getIconUrl(const Package& package) const override;

private:
	std::string mPkg_path;
	[[maybe_unused]] std::vector<std::unique_ptr<Package>> loadPackages() override;
};
#endif
