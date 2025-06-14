#ifndef MODULE_H
#define MODULE_H

#include <stdint.h>

// uwu~ Forward declare error_code_t
enum error_code_t;

// kawaii Module status structure :3
typedef struct {
    const char* name;  // kawaii module name >w<
    int status;        // 0 = not initialized, 1 = initialized uwu
} module_status_t;

// uwu~ Module status functions
void module_status_init(module_status_t* module, const char* name);  // initialize module with kawaii name :3
void module_status_set_error(module_status_t* module, enum error_code_t code);    // set error status uwu~
int module_status_is_healthy(module_status_t* module);               // check if module is healthy >w<

#endif // MODULE_H 