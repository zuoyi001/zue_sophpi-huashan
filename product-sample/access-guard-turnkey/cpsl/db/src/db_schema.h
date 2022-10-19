#ifndef __DEVICE_CONFIG_SCHEMA_H__
#define __DEVICE_CONFIG_SCHEMA_H__

// config
#define CONFIG_TABLE "config"

#define DEVICE_CONFIG_SCHEMA \
"CREATE TABLE IF NOT EXISTS " CONFIG_TABLE " (key TEXT PRIMARY KEY, value TEXT);"

#define INIT_DEVICE_CONFIG \
"BEGIN TRANSACTION;" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('company_name',           '');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('device_password',        '');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('device_name',            '');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('threshold',              '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('open_delay',             '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('interval',               '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('voice_mode',             '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('voice_custom',           '');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('display_mode',           '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('display_custom',         '');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('stranger_mode',          '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('stranger_custom',        '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('stranger_voice_mode',    '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('stranger_voice_custom',  '');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('max_face_size',          '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('liveness_type',          '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('sign_distance',          '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('success_retry_delay',    '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('success_retry',          '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('upload_record_image',    '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('ir_live_preview',        '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('reboot_every_day',       '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('reboot_hour',             '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('reboot_min',             '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('version_code',           '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('package_name',           '');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('version_name',           '');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('face_quality',           '0');" \
"INSERT OR IGNORE INTO " CONFIG_TABLE " VALUES ('face_quality_threshold', '0');" \
"COMMIT;"

// repo
#define ENABLE_FK "PRAGMA foreign_keys = ON;"

#define FACE_TABLE "faces"
#define REPO_SCHEMA \
"CREATE TABLE IF NOT EXISTS "FACE_TABLE" (id INTEGER PRIMARY KEY, name TEXT, identifier TEXT, serial TEXT UNIQUE, ic_card TEXT, image_path TEXT, type TEXT DEFAULT '4', remote_id INTEGER DEFAULT 0, add_time INTEGER, update_time INTEGER, status INTEGER DEFAULT 0, reserve INTEGER DEFAULT 0);"

#define FEATURE_TABLE "features"
#define FEATURE_SCHEMA \
"CREATE TABLE IF NOT EXISTS "FEATURE_TABLE \
" (id INTEGER PRIMARY KEY, face_id INTEGER, update_time DATETIME DEFAULT CURRENT_TIMESTAMP, feature BLOB NOT NULL, FOREIGN KEY (face_id) REFERENCES "FACE_TABLE"(id) ON DELETE CASCADE);"

#define RECORD_TABLE "records"
// we use int type for timestamp not date because sqlite is poor in time comparasion
// it's real hard if we use timestamp as filter
#define RECORD_SCHEMA \
"CREATE TABLE IF NOT EXISTS "RECORD_TABLE \
" (id INTEGER PRIMARY KEY, remote_id INTEGER DEFAULT 0, person_type TEXT, name TEXT, serial TEXT, identifier TEXT, ic_card TEXT, verification_type INTEGER, temperature REAL, timestamp INTEGER, status INTEGER DEFAULT 0, reserve INTEGER DEFAULT 0, image_path TEXT);"
#endif
