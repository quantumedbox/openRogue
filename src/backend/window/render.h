#pragma once

// Default texture size in both dimensions
#ifndef FONT_TEXTURE_SIZE
#define FONT_TEXTURE_SIZE 1024
#endif

// Max amount of characters per strip
#ifndef TEXT_BUFFER_SIZE
#define TEXT_BUFFER_SIZE 128
#endif

// Maybe it's better to have it in config ?
#ifndef DEFAULT_FONT
#define DEFAULT_FONT "resources/fonts/FSEX300.ttf"
#endif

#ifndef ENCODING
#define ENCODING "utf-32-le"
#endif


/*
    @brief  Initialize everything that is needed for text functionality

    @warn   OpenGL should be already initialized before this call

    @return Returns non-zero value on error
*/
int    init_text_subsystem();

/*
    @brief  Returns string that hints towards encoding in which all drawing strings should be
*/
ROGUE_EXPORT
const char* get_encoding();

/*
    @brief  Returns the valid font hash
            Makes sure that given font at path is loaded

    @warn   API caller should listen to ERRORCODE value for panics!

    @param  path -- hint to a font file

    @return Key identification for a font
*/
ROGUE_EXPORT
key_t   resolve_font    ( const char* path );

/*
    @brief  Renders given string to the current drawing window with specified font at the offset position

    @warn   Bytes should be encoded in 'utf-32-le'

    @param  font_hash   -- Represents a loaded font from resolve_font() function
            size        -- Height of the font
            x_offset    -- Offset pixels relative to top-left corner
            y_offset    -- Offset pixels relative to top-left corner
            utf_string  -- Bytes of string data that should be rendered
            string_len  -- Number of characters in the string

    @return Non-zero value on error, otherwise 0
*/
ROGUE_EXPORT
int     draw_text       ( key_t font_hash,
                          uint32_t size,
                          int32_t x_offset,
                          int32_t y_offset,
                          uint32_t* utf_string,
                          uint32_t string_len,
                          hex_t color );

/*
    @brief  Draw rectangle at currently bound window
*/
ROGUE_EXPORT
void    draw_rect       ( int32_t x_offset,
                          int32_t y_offset,
                          int32_t width,
                          int32_t height,
                          hex_t color );
