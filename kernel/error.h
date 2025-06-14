#ifndef ERROR_H
#define ERROR_H

#include <stdint.h>
#include "terminal.h"
#include "module.h"  // *grinst* Ah, da ist es ja, du vergesslicher Idiot!

// *grinst böse* Willkommen in der Hölle der Fehler, du elender Wurm!
enum error_code_t {
    ERR_NONE = 0,                    // *lacht hämisch* Alles gut? HAHAHA, du naive Seele!
    ERR_INVALID_ARGUMENT = 1,        // *spuckt* Was für ein Müll! Genau wie dein Code!
    ERR_NOT_PERMITTED = 2,          // *grinst* Nein, das darfst du nicht! Weil du es nicht kannst, du Versager!
    ERR_NO_SUCH_FILE = 3,           // *lacht* Die Datei existiert nicht? Wie deine Erfolge, du Loser!
    ERR_DEVICE_BUSY = 4,            // *spottet* Das Gerät ist beschäftigt! Mit WICHTIGEN Dingen, die du nie verstehen wirst!
    ERR_OUT_OF_MEMORY = 5,          // *grinst* Kein Speicher mehr? Wie deine Gehirnzellen, du Idiot!
    ERR_INVALID_OPERATION = 6,       // *lacht* Dieser Befehl macht keinen Sinn! Wie dein Leben, du Versager!
    ERR_UNKNOWN = 7,                // *spottet* Ich weiß es nicht! Und du erst recht nicht, du Dummkopf!
    ERR_DEVICE_NOT_FOUND = 8,       // *grinst* Das Gerät ist weg! Wie deine Hoffnungen, du Verlierer!
    ERR_CRIT_MEMORY = 9,            // *lacht* Kritischer Fehler! Alles ist verloren! Wie deine Zukunft, du Versager!
    ERR_INVALID_PARAMETER = 10,     // *spottet* Dieser Parameter ist Müll! Wie deine Ideen, du Idiot!
    ERR_WARN_NOT_IMPLEMENTED = 11   // *grinst* Nicht implementiert? Wie deine Pläne, du Versager!
};

// *lacht böse* Kritische Fehler! Wenn diese passieren, ist alles vorbei! Wie deine Karriere, du Loser!

#define ERR_CRIT_START ERR_CRIT_MEMORY
#define ERR_CRIT_END ERR_CRIT_MEMORY

// *grinst* Fehler setzen und die Welt brennen lassen! Wie deine Träume, du Versager!
#define ERROR_SET(code, message) do { \
    error_set(code); \
    terminal_writestring("*spottet* "); \
    terminal_writestring(message); \
    terminal_writestring("\n"); \
} while(0)

// *lacht* Funktionen zum Versagen! Wie dein Leben, du Loser!
void error_set(enum error_code_t code);  // *grinst* Setze einen Fehler! Wie deine Erwartungen, du Idiot!
enum error_code_t error_get(void);       // *spottet* Sag mir, was schiefgelaufen ist! Wie immer bei dir!
void error_print(void);                  // *lacht* Schreie es in die Welt! Wie deine Verzweiflung, du Versager!
void error_init(void);                   // *grinst* Bereite dich vor... auf das Versagen, du Loser!
const char* error_to_string(enum error_code_t code);  // *spottet* Übersetze es! In die Sprache des Scheiterns, du Dummkopf!

#endif // ERROR_H 