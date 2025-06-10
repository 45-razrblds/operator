#ifndef VERSION_H
#define VERSION_H

#define OS_VERSION "1.8"
#define BUILD_NUMBER "20250610.2135.5cd92bb"
#define BUILD_DATE "2025-06-10 21:35:19"

const char* get_os_version(void) { return OS_VERSION; }
const char* get_build_number(void) { return BUILD_NUMBER; }
const char* get_build_date(void) { return BUILD_DATE; }

#endif // VERSION_H
