#ifndef VERSION_H
#define VERSION_H

#define OS_VERSION "1.8"
#define BUILD_NUMBER "20250609.1317.35c387e"
#define BUILD_DATE "2025-06-09 13:17:56"

const char* get_os_version(void) { return OS_VERSION; }
const char* get_build_number(void) { return BUILD_NUMBER; }
const char* get_build_date(void) { return BUILD_DATE; }

#endif // VERSION_H
