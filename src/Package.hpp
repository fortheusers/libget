#ifndef PACKAGE_H
#define PACKAGE_H
#include "Manifest.hpp"
#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include <string>
#if defined(SWITCH) || defined(WII)
#define ROOT_PATH "/"
#elif defined(__WIIU__)
#define ROOT_PATH "fs:/vol/external01/"
#elif defined(_3DS)
#define ROOT_PATH "sdmc:/"
#else
#define ROOT_PATH "sdroot/"
#endif

#ifndef APP_VERSION
#define APP_VERSION "0.0.0"
#endif

#define APP_SHORTNAME "appstore"

class Repo;
class GetRepo;
class LocalRepo;
class OSCRepo;

/**
 * A package is a single application that can be installed. It contains the URL to the zip file and any instructions to install it (like a GET manifest).
 *
 * The download and install process is handled here, but they will use logic in the parentRepo's class to get the zip URL and installation logic.
 */
class Package
{
public:
	explicit Package(int state);
	~Package();

	// Copy constructor
	Package(const Package& other)
		: pkg_name(other.pkg_name)
		, title(other.title)
		, author(other.author)
		, short_desc(other.short_desc)
		, long_desc(other.long_desc)
		, version(other.version)
		, license(other.license)
		, changelog(other.changelog)
		, url(other.url)
		, sourceUrl(other.sourceUrl)
		, iconUrl(other.iconUrl)
		, updated(other.updated)
		, binary(other.binary)
		, manifest(other.manifest ? std::make_unique<Manifest>(*other.manifest) : nullptr)
		, updated_timestamp(other.updated_timestamp)
		, downloads(other.downloads)
		, extracted_size(other.extracted_size)
		, download_size(other.download_size)
		, screens(other.screens)
		, category(other.category)
		, parentRepo(other.parentRepo)
		, mRepoUrl(other.mRepoUrl)
		, status(other.status)
		, permissions(other.permissions)
	{
		// If you have other member variables, make sure to copy them as well
	}

	[[nodiscard]] std::string toString() const;
	bool downloadZip(std::string_view tmp_path, float* progress = nullptr) const;
	bool install(const std::string& pkg_path, const std::string& tmp_path);
	bool remove(std::string_view pkg_path);
	[[nodiscard]] const char* statusString() const;
	void updateStatus(const std::string& pkg_path);

	[[nodiscard]] std::string getIconUrl() const;
	[[nodiscard]] std::string getBannerUrl() const;
	[[nodiscard]] std::string getManifestUrl() const;
	[[nodiscard]] std::string getScreenShotUrl(int count) const;

	int isPreviouslyInstalled();

	[[nodiscard]] const std::string& getPackageName() const
	{
		return pkg_name;
	}

	[[nodiscard]] const std::string& getTitle() const
	{
		return title;
	}

	[[nodiscard]] const std::string& getAuthor() const
	{
		return author;
	}

	[[nodiscard]] const std::string& getShortDescription() const
	{
		return short_desc;
	}

	[[nodiscard]] const std::string& getLongDescription() const
	{
		return long_desc;
	}

	[[nodiscard]] const std::string& getVersion() const
	{
		return version;
	}

	[[nodiscard]] const std::string& getLicense() const
	{
		return license;
	}

	[[nodiscard]] const std::string& getChangelog() const
	{
		return changelog;
	}

	[[nodiscard]] const std::string& getRepoURL() const
	{
		return mRepoUrl;
	}

	[[nodiscard]] int getStatus() const
	{
		return status;
	}

private:
	// Package attributes
	std::string pkg_name;
	std::string title;
	std::string author;
	std::string short_desc;
	std::string long_desc;
	std::string version;

	std::string license;
	std::string changelog;

	std::string url;
	std::string sourceUrl;
	std::string iconUrl;

	std::string updated;
	std::string binary;

	std::unique_ptr<Manifest> manifest = nullptr;
	int updated_timestamp = 0;

	int downloads = 0;
	int extracted_size = 0;
	int download_size = 0;
	int screens = 0;

	std::string category;

	// Sorting attributes
	Repo* parentRepo;
	std::string mRepoUrl;

	int status; // local, update, installed, get

	// bitmask for permissions, from left to right:
	// unused, iosu, kernel, nand, usb, sd, wifi, sound
	char permissions;

	friend Repo;
	friend GetRepo;
	friend LocalRepo;
	friend OSCRepo;
};

#endif
