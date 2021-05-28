
// How many characters are packed into the single texture
// #define CHARACTERS_IN_RANGE 128

// //
// #define DEFAULT_FONT_HEIGHT 32

// //
// #define DEFAULT_FONT_WIDTH 32

// Max amount of characters per strip
#ifndef STRIP_BUFFER_SIZE
#define STRIP_BUFFER_SIZE 128
#endif

// Maybe it's better to have it in config ?
#ifndef DEFAULT_FONT
#define DEFAULT_FONT "resources/fonts/FSEX300.ttf"
#endif

int init_text_subsystem();
