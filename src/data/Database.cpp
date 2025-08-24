#include "Database.h"
#include <iostream>
#include <algorithm>

Database::Database(const std::string& path) : dbPath(path)
{
	open();
}

Database::~Database()
{
	close();
}

bool Database::isOpen() const
{
	return db != nullptr;
}

bool Database::createTables()
{
	const char* sql = R"(
		CREATE TABLE IF NOT EXISTS satellites (
			id INTEGER PRIMARY KEY AUTOINCREMENT,
			name TEXT NOT NULL,
			norad_id INTEGER UNIQUE NOT NULL,
			tle_line1 TEXT NOT NULL,
			tle_line2 TEXT NOT NULL,
			epoch TEXT NOT NULL,
			last_update DATETIME DEFAULT CURRENT_TIMESTAMP
		);

		CREATE TABLE IF NOT EXISTS satellite_groups (
			id INTEGER PRIMARY KEY AUTOINCREMENT,
			norad_id INTEGER NOT NULL,
			group_name TEXT NOT NULL,
			FOREIGN KEY (norad_id) REFERENCES satellite (norad_id) ON DELETE CASCADE,
			UNIQUE(norad_id, group_name)
		);

		CREATE INDEX IF NOT EXISTS idx_satellites_norad ON satellites(norad_id);
        CREATE INDEX IF NOT EXISTS idx_satellites_epoch ON satellites(epoch);
        CREATE INDEX IF NOT EXISTS idx_groups_norad ON satellite_groups(norad_id);
        CREATE INDEX IF NOT EXISTS idx_groups_name ON satellite_groups(group_name);)";

	return executeSQL(sql);
}

bool Database::insertSatellite(const SatelliteTle& satellite)
{
	const char* sql = R"(
		INSERT OR REPLACE INTO satellites (name, norad_id, tle_line1, tle_line2, epoch)
		VALUES (?, ?, ?, ?, ?)
	)";

	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		std::cerr << "Failed to prepair statement: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}

	sqlite3_bind_text(stmt, 1, satellite.name.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, satellite.noradId);
	sqlite3_bind_text(stmt, 3, satellite.tleLine1.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 4, satellite.tleLine2.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 5, satellite.epoch.c_str(), -1, SQLITE_STATIC);

	rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return rc == SQLITE_DONE;
}

bool Database::updateSatellite(const SatelliteTle& satellite)
{
	const char* sql = R"(UPDATE satellites SET name = ?, tle_line1 = ?, tle_line2 = ?, epoch = ?,
		last_updated = CURRENT_TIMESTAMP WHERE norad_id = ?)";

	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}
		
	sqlite3_bind_text(stmt, 1, satellite.name.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, satellite.tleLine1.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 3, satellite.tleLine2.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 4, satellite.epoch.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 5, satellite.noradId);

	rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return rc == SQLITE_DONE;
}

bool Database::deleteSatellite(int noradId)
{
	const char* sql = "DELETE FROM satellites WHERE norad_id = ?";

	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}

	sqlite3_bind_int(stmt, 1, noradId);

	rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return rc == SQLITE_DONE;
}

std::optional<SatelliteTle> Database::getSatelliteByNoradId(int noradId)
{
	const char* sql = R"(SELECT id, name, norad_id, tle_line1, tle_line2, epoch
		FROM satellites WHERE norad_id = ?)";

	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		return std::nullopt;
	}

	SatelliteTle satellite;
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		satellite.id = sqlite3_column_int(stmt, 0);
		satellite.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
		satellite.noradId = sqlite3_column_int(stmt, 2);
		satellite.tleLine1 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
		satellite.tleLine2 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
		satellite.epoch = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

		sqlite3_finalize(stmt);
		return satellite;
	}

	sqlite3_finalize(stmt);
	return std::nullopt;
}

std::vector<SatelliteTle> Database::getAllSatellites()
{
	std::vector<SatelliteTle> satellites;
	const char* sql = R"(SELECT id, name, norad_id, tle_line1, tle_line2, epoch
						FROM satellites)";

	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		return satellites;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		SatelliteTle satellite;
		satellite.id = sqlite3_column_int(stmt, 0);
		satellite.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
		satellite.noradId = sqlite3_column_int(stmt, 2);
		satellite.tleLine1 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
		satellite.tleLine2 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
		satellite.epoch = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

		satellites.push_back(satellite);
	}

	sqlite3_finalize(stmt);
	return satellites;
}

std::vector<SatelliteTle> Database::getSatellitesByGroups(const std::string& group)
{
	std::vector<SatelliteTle> satellites;
	const char* sql = R"(SELECT s.id, s.name, s.norad_id, s.tle_line1, s.tle_line2, s.epoch
						FROM satellites s
						JOIN satellite_groups sg ON s.norad_id = sg.norad_id
						WHERE sg.group_name = ? 
						ORDER BY s.name)";

	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		return satellites;
	}

	sqlite3_bind_text(stmt, 1, group.c_str(), -1, SQLITE_STATIC);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		SatelliteTle satellite;
		satellite.id = sqlite3_column_int(stmt, 0);
		satellite.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
		satellite.noradId = sqlite3_column_int(stmt, 2);
		satellite.tleLine1 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
		satellite.tleLine2 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
		satellite.epoch = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

		satellites.push_back(satellite);
	}

	sqlite3_finalize(stmt);
	return satellites;
}

int Database::getSatelliteCount()
{
	const char* sql = "SELECT COUNT(*) FROM satellites";

	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) 
		return 0;

	int count = 0;
	if (sqlite3_step(stmt) == SQLITE_ROW)
		count = sqlite3_column_int(stmt, 0);

	sqlite3_finalize(stmt);
	return count;
}

bool Database::clearAllData()
{
	if (!beginTransaction())
		return false;

	const char* sql1 = "DELETE FROM satellite_groups";
	const char* sql2 = "DELETE FROM satellites";

	if (!executeSQL(sql1) || !executeSQL(sql2)) {
		rollbackTransaction();
		return false;
	}
	return commitTransaction();
}

bool Database::addSatelliteToGroup(int noradId, const std::string& group)
{
	const char* sql = "INSERT OR IGNORE INTO satellite_groups (norad_id, group_name) VALUES (?, ?)";

	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}

	sqlite3_bind_int(stmt, 1, noradId);
	sqlite3_bind_text(stmt, 2, group.c_str(), -1, SQLITE_STATIC);

	rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return rc == SQLITE_DONE;
}

bool Database::removeSatelliteFromGroup(int noradId, const std::string& group)
{
	const char* sql = "DELETE FROM sqtellite_groups WHERE norad_id = ? AND group_name = ?";

	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
	if (rc != SQLITE_DONE) {
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}

	sqlite3_bind_int(stmt, 1, noradId);
	sqlite3_bind_text(stmt, 2, group.c_str(), -1, SQLITE_STATIC);

	rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return rc == SQLITE_DONE;
}

std::vector<std::string> Database::getSatelliteGroups(int noradId)
{
	std::vector<std::string> groups;
	const char* sql = R"(SELECT group_name FROM satellite_groups WHERE norad_id = ? 
						ORDER BY group_name)";

	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		return groups;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		groups.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
	}

	sqlite3_finalize(stmt);
	return groups;
}

bool Database::open()
{
	if (db)
		close();

	int rc = sqlite3_open(dbPath.c_str(), &db);
	if (rc != SQLITE_OK) {
		std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}

	// ¬ключаем foreign keys и улучшаем производительность
	executeSQL("PRAGMA foreign_keys = ON;");
	executeSQL("PRAGMA journal_mode = WAL;");
	executeSQL("PRAGMA synchronous = NORMAL;");

	return true;
}

void Database::close()
{
	if (db) {
		if (isTransactionActive)
			rollbackTransaction();
		sqlite3_close(db);
		db = nullptr;
	}
}

bool Database::executeSQL(const std::string& sql)
{
	if (!db)
		return false;

	char* errorMessage = nullptr;
	int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errorMessage);

	if (rc != SQLITE_OK) {
		std::cerr << "SQL error: " << (errorMessage ? errorMessage : "unknown error") << std::endl;
		if (errorMessage) 
			sqlite3_free(errorMessage);
		return false;
	}

	return true;
}

bool Database::beginTransaction()
{
	if (isTransactionActive)
		return true;

	if (executeSQL("BEGIN TRANSACTION;")) {
		isTransactionActive = true;
		return true;
	}

	return false;
}

bool Database::commitTransaction()
{
	if (!isTransactionActive)
		return false;

	if (executeSQL("COMMIT TRANSACTION;")) {
		isTransactionActive = false;
		return true;
	}

	return false;
}

bool Database::rollbackTransaction()
{
	if (!isTransactionActive)
		return false;

	if (executeSQL("ROLLBACK TRANSACTION;")) {
		isTransactionActive = false;
		return true;
	}

	return false;
}
