#ifndef GL_FONT_H
#define GL_FONT_H

#include "GL/glew.h"
#if defined(__APPLE__)
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif
#include <string>
#include <vector>

struct GLGlyph {
    GLGlyph()
        : id(-1)
        , width(0)
        , height(0)
        , u(0)
        , v(0)
        , offset_x(0)
        , offset_y(0)
        , advance_x(0){};
    void reset()
    {
        id = -1;
        width = 0;
        height = 0;
        u = 0;
        v = 0;
        offset_x = 0, offset_y = 0, advance_x = 0;
        bitmap.clear();
    };
    std::vector<unsigned char> bitmap;
    unsigned short id;
    unsigned short width;
    unsigned short height;
    unsigned short u;
    unsigned short v;
    short offset_x;
    short offset_y;
    short advance_x;
};

/** @brief A class for storing OpenGL font information */
class GLFont
{
  public:
    GLFont();
    ~GLFont();

    bool load(const char* file);
    bool loadFromArray(const char* array, unsigned int size);
    void render_textWorldSpace(const char* text, float x, float y, float scale, unsigned int rgba = 0x00000000);
    void render_textScreenSpace(const char* text, float x, float y, float scale, unsigned int rgba = 0x00000000);

    unsigned int lineHeight() const;
    unsigned int char_width(const char c) const;
    unsigned int string_width(const char* str) const;
    void getTextSize(const char* str, int& width, int& height, float fontSize);

    /**
      @brief Gets the texture ID of the font.
      @return The ID of the texture.
     */
    unsigned int getTextureID() { return m_texID; };

    int m_texwidth; ///< The width of one texel
    int m_texheight; ///< The height of one texel
    GLuint m_texID; ///< The ID of the texel
    GLuint m_vboID; ///< The ID of the vertex buffer object
    GLGlyph* m_glyphs; ///< The pointer to the font glyphs
    const unsigned short m_glyphCount; ///< The count of glyphs in the font
    unsigned char m_fontSize; ///< The font size
};

#endif
