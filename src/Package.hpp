#ifndef PACKAGE_H
#define PACKAGE_H
#include "Manifest.hpp"
#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include <optional>
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

class Get;
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

	Package(const Package& other) = default;

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

	Manifest manifest;

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

	[[nodiscard]] int getStatus() const
	{
		return status;
	}

	[[nodiscard]] int getDownloadCount() const {
		return downloads;
	}

	[[nodiscard]] int getDownloadSize() const {
		return download_size;
	}

	[[nodiscard]] int getExtractedSize() const {
		return download_size;
	}

	[[nodiscard]] int getScreenshotCount() const {
		return screens;
	}

	[[nodiscard]] int getUpdatedAtTimestamp() const {
		return updated_timestamp;
	}

	[[nodiscard]] const std::string& getUpdatedAt() const {
		return updated;
	}

	[[nodiscard]] const std::string& getCategory() const {
		return category;
	}

	[[nodiscard]] const std::string& getBinary() const {
		return binary;
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

	int updated_timestamp = 0;

	int downloads = 0;
	int extracted_size = 0;
	int download_size = 0;
	int screens = 0;

	std::string category;

	// Sorting attributes
	std::shared_ptr<Repo> mRepo;

	int status; // local, update, installed, get

	// bitmask for permissions, from left to right:
	// unused, iosu, kernel, nand, usb, sd, wifi, sound
	char permissions{};

	friend Get;
	friend Repo;
	friend GetRepo;
	friend LocalRepo;
	friend OSCRepo;
};

#endif
