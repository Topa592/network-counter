#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include <asio.hpp>
#include <sqlite3.h>

using asio::ip::tcp;
sqlite3* db;
std::string dbName = "counter.db";
std::string tableName = "Messages";
std::string columName = "msg";

static int callback(void* data, int argc, char** argv, char** azColName) {
	int i;
	fprintf(stderr, "%s: ", (const char*)data);

	for (i = 0; i < argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}

	printf("\n");
	return 0;
}

int execSql(const std::string& sqlMsg) {
	char* zErrMsg = 0;
	int rc;
	const char* sql = sqlMsg.c_str();
	const char* data = "Callback function called";

	/* Open database */
	rc = sqlite3_open(dbName.c_str(), &db);

	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return 1;
	}
	else {
		fprintf(stderr, "Opened database successfully\n");
	}

	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else {
		fprintf(stdout, "Operation done successfully\n");
	}
	sqlite3_close(db);
	return 0;
}

void storeToSql(const std::string& msg) {
	std::string sqlMsg = "INSERT INTO " + tableName + " (" + columName + ")" \
		"VALUES ('" + msg + "');";
	execSql(sqlMsg);
}

void createTableIfNotExist() {
	std::string sqlMsg = "CREATE TABLE IF NOT EXISTS " + tableName + " (" \
		+ columName \
		+ ");";
	execSql(sqlMsg);
}

int main() {
	createTableIfNotExist();
	try
	{
		asio::io_context io_context;
		tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 14517));
		for (;;) {
			tcp::socket socket(io_context);
			acceptor.accept(socket);
			
			std::vector<char> buf(128);
			asio::error_code error;

			size_t len = socket.read_some(asio::buffer(buf), error);
			if (error == asio::error::eof)
				break; //Connection closed cleanly by peer.
			else if (error)
				throw asio::system_error(error);

			if (buf[0] == 'x') break;
			//std::cout.write(buf.data(), len);
			storeToSql(buf.data());
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}