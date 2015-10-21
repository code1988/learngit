/***********************************************************************
 * @file		et850_sql.h
 * @version		v1.0.0.1
 * @auther		xiaole
 * @email		yixun731@hotmail.com
 * $Revision:	1 $
 * $Date: 		2015/04/28 14:35:00
 * @brief
 *				sqlite3 database sql for et850
 * @note
************************************************************************/
#ifndef _ET850_SQL_H_
#define _ET850_SQL_H_

/* db filename */
#define ET850_DB_FILENAME		"/db/et850.db"
#define ET850_RUN_DB			".//db//et850_run.db"
#define ET850_FSN_DB			"//media//sdcard//db//mediainfo.db"

#define SQL_TABLE_SELECT		"SELECT count(*) FROM sqlite_master \
								WHERE type = 'table' AND name = '%s'"

/* run db version table macro */
#define SQL_VERSION_TABLE_CREATE "CREATE TABLE IF NOT EXISTS version_table( \
								ID INTEGER PRIMARY KEY, \
								name VARCHAR(16) UNIQUE, \
								version VARCHAR(32))"

#define SQL_VERSION_TABLE_DESTROY "DROP TABLE version_table"

#define SQL_VERSION_INSERT		"INSERT INTO \"version_table\" VALUES(NULL, '%s', '%s')"
#define SQL_VERSION_DELETE		"DELETE FROM \"version_table\" WHERE name = '%s'"
#define SQL_VERSION_SELECT		"SELECT * FROM version_table WHERE name = '%s'"

/* run db param table macro */
#define SQL_PARAM_TABLE_CREATE	"CREATE TABLE IF NOT EXISTS param_table( \
								name VARCHAR(16) UNIQUE, \
								param VARCHAR(16))"
#define SQL_PARAM_TABLE_DESTROY	"DROP TABLE param_table"
#define SQL_PARAM_INSERT		"INSERT INTO \"param_table\" VALUES('%s', '%s')"
#define SQL_PARAM_DELETE		"DELETE FROM \"param_table\" WHERE name = '%s'"
#define SQL_PARAM_SELECT		"SELECT * FROM param_table WHERE name = '%s'"
#define SQL_PARAM_UPDATE		"UPDATE param_table SET param = '%s' WHERE name = '%s'"

/* db device_table macro */
#define SQL_DEVICE_TABLE_CREATE	"CREATE TABLE IF NOT EXISTS device_table( \
								ID INTEGER PRIMARY KEY, \
								name VARCHAR(16) UNIQUE, \
								str VARCHAR(32), \
								value INTEGER)"

#define SQL_DEVICE_TABLE_DESTROY "DROP TABLE device_table"
#define SQL_DEVICE_INSERT		"INSERT INTO \"device_table\" VALUES(NULL, '%s', '%s', 0)"
#define SQL_DEVICE_DELETE		"DELETE FROM \"device_table\" WHERE name = '%s'"
#define SQL_DEVICE_SELECT		"SELECT * FROM device_table WHERE name = '%s'"

/* user_table sql macro */
#define SQL_USER_TABLE_CREATE	"CREATE TABLE IF NOT EXISTS user_table( \
								ID INTEGER PRIMARY KEY, \
								username VARCHAR(16) UNIQUE, \
								password VARCHAR(16), \
								permission INTEGER)"
#define SQL_USER_TABLE_DESTROY	"DROP TABLE user_table"
#define SQL_USER_INSERT			"INSERT INTO \"user_table\" VALUES(NULL, '%s', '%s', %d)"
#define SQL_USER_DELETE			"DELETE FROM \"user_table\" WHERE username = '%s'"
#define SQL_USER_COUNT			"SELECT COUNT(*) FROM user_table"
#define SQL_USER_SELECT_BYID	"SELECT * FROM user_table LIMIT %d OFFSET %d"
#define SQL_USER_SELECT_BYNAME	"SELECT * FROM user_table WHERE username = '%s'"
#define SQL_USER_SELECT			"SELECT * FROM user_table"

/* db black table macro */
#define SQL_BLACK_TABLE_CREATE	"CREATE TABLE IF NOT EXISTS black_table( \
								ID INTEGER PRIMARY KEY, \
								name VARCHAR(16) UNIQUE)"
#define SQL_BLACK_TABLE_DESTROY	"DROP TABLE black_table"
#define SQL_BLACK_INSERT		"INSERT INTO \"black_table\" VALUES(NULL, '%s')"
#define SQL_BLACK_DELETE		"DELETE FROM \"black_table\" WHERE name = '%s'"
#define SQL_BLACK_COUNT			"SELECT COUNT(*) FROM black_table"
#define SQL_BALCK_SELECT_BYID	"SELECT * FROM black_table LIMIT %d OFFSET %d"

/* db net table macro */
#define SQL_NET_TABLE_CREATE	"CREATE TABLE IF NOT EXISTS net_table( \
								name VARCHAR(8) UNIQUE, \
								type INTEGER, \
								ip INTEGER,		\
								mask INTEGER,	\
								gate INTEGER,	\
								dns	INTEGER)"
#define SQL_NET_TABLE_DESTROY	"DROP TABLE net_table"
#define SQL_NET_INSERT			"INSERT INTO \"net_table\" VALUES('%s', %d, %d, %d, %d, %d)"
#define SQL_NET_DELETE			"DELETE FROM \"net_table\" WHERE name = '%s'"
#define SQL_NET_SELECT_BYNAME	"SELECT * FROM net_table WHERE name = '%s'"
#define SQL_NET_UPDATE_BYNAME	"UPDATE net_table SET type = %d, ip = %d, mask = %d, \
									gate = %d, dns = %d WHERE name = '%s'"
/* db communication table macro */
#define SQL_COMM_TABLE_CREATE	"CREATE TABLE IF NOT EXISTS comm_table( \
								name VARCHAR(16) UNIQUE, \
								ip INTEGER, \
								port INTEGER)"
#define SQL_COMM_TABLE_DESTROY	"DROP TABLE comm_table"
#define SQL_COMM_INSERT			"INSERT INTO \"comm_table\" VALUES('%s', %d, %d)"
#define SQL_COMM_DELETE			"DELETE FROM \"comm_table\" WHERE name = '%s'"
#define SQL_COMM_SELECT_BYNAME	"SELECT * FROM comm_table WHERE name = '%s'"
#define SQL_COMM_UPDATE_PORT	"UPDATE comm_table SET port = %d WHERE name = '%s'"
#define SQL_COMM_UPDATE_ALL		"UPDATE comm_table SET ip = %d, port = %d WHERE name = '%s'"

#endif /* _ET850_SQL_H_ */






















