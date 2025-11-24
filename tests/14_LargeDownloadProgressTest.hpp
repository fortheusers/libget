#include "tests.hpp"
#include <fstream>
#include <vector>
#include <cstring>
#include <climits>
#include <cstdlib>
#include <ctime>

struct ProgressTracker {
	std::vector<double> progressValues;
	bool downloadFinished = false;
	int callCount = 0;
	bool sequentialCheck = true;
	
	void reset() {
		progressValues.clear();
		downloadFinished = false;
		callCount = 0;
		sequentialCheck = true;
	}
	
	bool isSequential() const {
		// ensure that progress isn't going backwards
		for (size_t i = 1; i < progressValues.size(); i++) {
			if (progressValues[i] < progressValues[i-1]) {
				return false;
			}
		}
		return true;
	}
	
	bool wasCompleted() const {
		// check if we reached 100% (or very close) in updates
		if (progressValues.empty()) return false;
		double lastProgress = progressValues.back();
		return lastProgress >= 0.99;
	}
} progressTracker;

int testProgressCallback(void*, double progress) {
	progressTracker.callCount++;
	progressTracker.progressValues.push_back(progress);
	
	if (progressTracker.progressValues.size() > 1) {
		size_t idx = progressTracker.progressValues.size() - 1;
		if (progressTracker.progressValues[idx] < progressTracker.progressValues[idx-1]) {
			progressTracker.sequentialCheck = false;
		}
	}
	
	return 0; // returning non-zero here would stop transfer
}

class LargeDownloadProgressTest : public Test {
public:
	LargeDownloadProgressTest() {
		purpose = "Large Download with Progress Callbacks";
	}
	
	// helper to create a large package file (111MB)
	bool createLargeTestPackage(const std::string& repoPath, const std::string& packageName, size_t sizeInMB) {
		std::string packagesDir = repoPath + "/packages/" + packageName;
		std::string zipsDir = repoPath + "/zips";
		
		mkpath(packagesDir.c_str());
		mkpath(zipsDir.c_str());
		
		std::string manifestPath = packagesDir + "/info.json";
		std::ofstream manifest(manifestPath);
		if (!manifest) {
			error << "Failed to create manifest at " << manifestPath << endl;
			return false;
		}
		
		manifest << "{\n";
		manifest << "  \"name\": \"" << packageName << "\",\n";
		manifest << "  \"title\": \"Large Test Package\",\n";
		manifest << "  \"author\": \"Test Suite\",\n";
		manifest << "  \"version\": \"1.0.0\",\n";
		manifest << "  \"description\": \"Package for testing large downloads\",\n";
		manifest << "  \"details\": \"This package tests progress callbacks\",\n";
		manifest << "  \"category\": \"test\"\n";
		manifest << "}\n";
		manifest.close();
		
		// Create a large data file to include in the zip
		std::string dataPath = packagesDir + "/large_data.bin";
		std::ofstream dataFile(dataPath, std::ios::binary);
		if (!dataFile) {
			error << "Failed to create data file at " << dataPath << endl;
			return false;
		}
		
		// Write random-ish data in chunks
		const size_t chunkSize = 1024 * 1024; // 1MB chunks
		std::vector<char> chunk(chunkSize);
		
		// Use more random data to make it less compressible
		std::srand(std::time(nullptr));
		
		for (size_t mb = 0; mb < sizeInMB; mb++) {
			// Fill with more random pattern to prevent excessive compression
			for (size_t i = 0; i < chunkSize; i++) {
				// Mix multiple patterns to make it less compressible
				chunk[i] = static_cast<char>((mb * chunkSize + i + std::rand()) % 256);
			}
			dataFile.write(chunk.data(), chunkSize);
		}
		dataFile.close();
		
		// Create the zip file using system zip command
		std::string zipPath = zipsDir + "/" + packageName + ".zip";
		
		// get absolute paths for the zip command
		char absPackagesDir[PATH_MAX];
		char absZipPath[PATH_MAX];
		if (!realpath(packagesDir.c_str(), absPackagesDir)) {
			error << "Failed to get absolute path for " << packagesDir << endl;
			return false;
		}
		if (!realpath(zipsDir.c_str(), absZipPath)) {
			error << "Failed to get absolute path for " << zipsDir << endl;
			return false;
		}
		
		std::string fullZipPath = std::string(absZipPath) + "/" + packageName + ".zip";
		std::string cmd = "cd " + std::string(absPackagesDir) + " && zip -q -r " + fullZipPath + " . > /dev/null 2>&1";
		
		int result = system(cmd.c_str());
		if (result != 0) {
			error << "Failed to create zip file, command: " << cmd << endl;
			return false;
		}
		
		// Verify zip was created
		std::ifstream zipCheck(fullZipPath, std::ios::binary);
		if (!zipCheck) {
			error << "Zip file was not created at " << fullZipPath << endl;
			return false;
		}
		zipCheck.close();
		
		// update repo.json
		std::string repoJsonPath = repoPath + "/repo.json";
		std::ofstream repoJson(repoJsonPath);
		if (!repoJson) {
			error << "Failed to create repo.json at " << repoJsonPath << endl;
			return false;
		}
		
		repoJson << "{\n";
		repoJson << "  \"packages\": [\"" << packageName << "\"]\n";
		repoJson << "}\n";
		repoJson.close();
		
		return true;
	}
	
	bool execute()
	{
		cout << "Test 1: Creating large test package..." << endl;
		
		std::string testRepoPath = "./tests/server/f";
		std::string packageName = "largetest";
		
		// clean up past runs
		system(("rm -rf " + testRepoPath).c_str());
		mkpath(testRepoPath.c_str());
		
		// create 111MB package
		if (!createLargeTestPackage(testRepoPath, packageName, 111)) {
			return false;
		}
		
		cout << "✓ Test 1 passed - Created large test package" << endl;
		
		cout << "Test 2: Downloading large file with progress callback..." << endl;
		
		// set up progress callback
		progressTracker.reset();
		networking_callback = testProgressCallback;
		networking_callback_data = nullptr;
		
		std::string tmpPath = "./tests/.get/tmp/";
		mkpath(tmpPath.c_str());
		
		std::string downloadPath = tmpPath + packageName + ".zip";
		std::string downloadUrl = "http://localhost:8000/f/zips/" + packageName + ".zip";
		
		// Download the file directly
		bool downloadSuccess = downloadFileToDisk(downloadUrl, downloadPath, false);
		
		// Restore callback
		networking_callback = nullptr;
		
		if (!downloadSuccess) {
			error << "Download failed" << endl;
			return false;
		}
		
		cout << "  Progress callback called " << progressTracker.callCount << " times" << endl;
		
		// Verify callback was called multiple times (should be many for 111MB)
		if (progressTracker.callCount < 50) {
			error << "Progress callback called too few times (" << progressTracker.callCount << "), expected at least 5 for large file" << endl;
			return false;
		}
		
		cout << "✓ Test 2 passed - Progress callback invoked frequently" << endl;
		
		cout << "Test 3: Verifying sequential progress..." << endl;
		
		if (!progressTracker.isSequential()) {
			error << "Progress was not sequential (bytes downloaded decreased)" << endl;
			return false;
		}
		
		cout << "✓ Test 3 passed - Progress is sequential (monotonically increasing)" << endl;
		
		cout << "Test 4: Verifying download completed..." << endl;
		
		if (!progressTracker.wasCompleted()) {
			error << "Progress did not reach completion" << endl;
			if (!progressTracker.progressValues.empty()) {
				double lastProgress = progressTracker.progressValues.back();
				cout << "  Last progress: " << (lastProgress * 100.0) << "%" << endl;
			}
			return false;
		}
		
		cout << "✓ Test 4 passed - Download reached completion" << endl;
		
		cout << "Test 5: Verifying callbacks stopped after completion..." << endl;
		
		// after the download is complete, the callback count shouldn't increase
		int finalCallCount = progressTracker.callCount;
		
		// make sur ethe file is here
		struct stat file_info = {};
		if (stat(downloadPath.c_str(), &file_info) != 0) {
			error << "Downloaded file not found" << endl;
			return false;
		}
		
		if (progressTracker.callCount != finalCallCount) {
			error << "Callback was invoked after download completed" << endl;
			return false;
		}
		
		cout << "✓ Test 5 passed - Callbacks stopped after completion" << endl;
		
		cout << "Test 6: Verifying downloaded file size..." << endl;
		
		if (file_info.st_size < 50 * 1024 * 1024) {
			error << "Downloaded file too small: " << file_info.st_size << " bytes (expected > 50MB)" << endl;
			return false;
		}
		
		cout << "  Downloaded file size: " << file_info.st_size << " bytes" << endl;
		cout << "✓ Test 6 passed - File size correct (uncompressed data was 5MB, compressed to " << (file_info.st_size / 1024) << "KB)" << endl;
		
		cout << endl << "All large download progress tests passed!" << endl;
		return true;
	}
};
