#ifndef GET_REPO_H
#define GET_REPO_H
#include "Package.hpp"
#include "Repo.hpp"
#include <iostream>
#include <vector>

#ifdef NETWORK_MOCK
#include "../tests/network_mock.hpp"
#endif

class GetRepo : public Repo
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
