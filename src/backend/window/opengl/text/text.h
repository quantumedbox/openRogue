
// How many characters are packed into the single texture
// #define CHARACTERS_IN_RANGE 128

// //
// #define DEFAULT_FONT_HEIGHT 32

// //
// #define DEFAULT_FONT_WIDTH 32

#ifndef FONT_TEXTURE_SIZE
#define FONT_TEXTURE_SIZE 1024
#endif

// Max amount of characters per strip
#ifndef STRIP_BUFFER_SIZE
#define STRIP_BUFFER_SIZE 128
#endif

// Maybe it's better to have it in config ?
#ifndef DEFAULT_FONT
#define DEFAULT_FONT "resources/fonts/FSEX300.ttf"
#endif


#define ENCODING "utf-32-le"

// Should return C-string containing the hint in which encoding rendering text should be passed
const char*
get_encoding ()
{
	return ENCODING;
}

// Function by which all fonts should be acquired
key_t 	resolve_font 	(const char* path,
                         uint32_t size);

// Create a new string texture for the future rendering
void	new_buffer_strip (size_t font_hash,
                          uint32_t size,
                          int32_t x_offset,
                          int32_t y_offset,
                          uint32_t* utf_string,
                          uint32_t string_len);
