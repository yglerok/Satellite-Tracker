#pragma once

#include <string>
#include <chrono>
#include <functional>
#include <memory>

#include <curl/curl.h>

#include "Database.h"
#include "TleParser.h"

class DataManager
{
public:
	DataManager(std::string urlStr, std::chrono::minutes updInterval,
		const std::string& dbPath);
	~DataManager();

	bool initialize();
	void update();
	bool forceUpdate();

	bool isUpdateNeeded() const;
	std::chrono::minutes timeUntilUpdate() const;

	void setUpdateCallback(std::function<void(bool success)> callback);

private:
	bool downloadAndProcessData();
	bool processDownloadedData(const std::string& data);

	std::string url;
	std::chrono::minutes updateInterval;
	std::chrono::system_clock::time_point lastUpdate;
	std::chrono::system_clock::time_point lastAttempt;

	std::function<void(bool success)> callback;
	std::unique_ptr<Database> database;
	std::unique_ptr<TleParser> parser;

	CURL* curl;
	int retryCount;

	static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* data);
};