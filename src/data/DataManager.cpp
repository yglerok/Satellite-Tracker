#include "DataManager.h"

#include <iostream>
#include <thread>

DataManager::DataManager(std::string urlStr, std::chrono::minutes updInterval, const std::string& dbPath) :
	url(std::move(urlStr)), updateInterval(updInterval), curl(curl_easy_init()), retryCount(0)
{
	database = std::make_unique<Database>(dbPath);
	parser = std::make_unique<TleParser>();

	lastUpdate = std::chrono::system_clock::now() - updateInterval;
	lastAttempt = std::chrono::system_clock::now();
}

DataManager::~DataManager()
{
	if (curl)
		curl_easy_cleanup(curl);
}

bool DataManager::initialize()
{
	if (!database->isOpen() || !database->createTables()) {
		std::cerr << "Failed to initialize database!" << std::endl;
		return false;
	}

	if (!curl) {
		std::cerr << "Failed to initialize CURL" << std::endl;
		return false;
	}

	return true;
}

void DataManager::update()
{
	if (isUpdateNeeded()) {
		std::cout << "Starting data update..." << std::endl;
		bool success = downloadAndProcessData();
		if (success) {
			lastUpdate = std::chrono::system_clock::now();
			retryCount = 0;
			std::cout << "Data update successful!" << std::endl;
		}
		else {
			retryCount++;
			std::cerr << "Data update failed! Retry count: " << retryCount << std::endl;
		}

		if (callback)
			callback(success);
	}
}

bool DataManager::forceUpdate()
{
	std::cout << "Forcing data update..." << std::endl;
	bool success = downloadAndProcessData();
	if (success) {
		lastUpdate = std::chrono::system_clock::now();
		retryCount = 0;
	}
	return success;
}

bool DataManager::isUpdateNeeded() const
{
	auto now = std::chrono::system_clock::now();
	auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::minutes>(now - lastUpdate);
	return timeSinceLastUpdate >= updateInterval;
}

std::chrono::minutes DataManager::timeUntilUpdate() const
{
	auto now = std::chrono::system_clock::now();
	auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::minutes>(now - lastUpdate);
	return max(std::chrono::minutes(0), updateInterval - timeSinceLastUpdate);
}

void DataManager::setUpdateCallback(std::function<void(bool success)> callback)
{
	this->callback = callback;
}

bool DataManager::downloadAndProcessData()
{
	std::string data;
	CURLcode res;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "SatelliteTracker/1.0");
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

	res = curl_easy_perform(curl);

	if (res != CURLE_OK) {
		std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
		return false;
	}

	long httpCode = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

	if (httpCode != 200) {
		std::cerr << "HTTP error: " << httpCode << std::endl;
		return false;
	}

	return processDownloadedData(data);
}

bool DataManager::processDownloadedData(const std::string& data)
{
	if (data.empty()) {
		std::cerr << "Download data is empty!" << std::endl;
		return false;
	}

	auto satellites = parser->parseTleData(data);
	if (satellites.empty()) {
		std::cerr << "No satellites parsed from downloaded data!" << std::endl;
		return false;
	}

	// Начинаем транзакцию для быстрой вставки
	//if (!database->beginTransaction)
	return true;
}

size_t DataManager::writeCallback(void* contents, size_t size, size_t nmemb, std::string* data)
{
	size_t totalSize = size * nmemb;
	data->append(static_cast<char*>(contents), totalSize);
	return totalSize;
}
