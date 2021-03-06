#ifndef FLAT_VIDEO_FONT_FONT_H
#define FLAT_VIDEO_FONT_FONT_H

#include <array>
#include <string>
#include <GL/glew.h>
#include <SDL2/SDL_ttf.h>

#include "misc/vector.h"

namespace flat
{
namespace video
{
namespace font
{

class Font
{
	public:
		struct CharInfoUv
		{
			float x;
			float y;
		};
		struct CharInfo
		{
			std::array<CharInfoUv, 6> uv;
			int minX;
			int maxX;
			int minY;
			int maxY;
			int advance;
			bool visible;
		};
		
	public:
		Font() = delete;
		Font(const Font&) = delete;
		Font(Font&&) = delete;
		Font(const std::string& font, int size);
		~Font();
		Font& operator=(const Font&) = delete;
		
		GLint getAtlasId() const { return m_atlasId; }
		const Vector2& getAtlasSize() const { return m_atlasSize; }
		
		static void open();
		static void close();
		
		const CharInfo& getCharInfo(char c) const;
		bool isValidChar(char c) const;
		
	protected:
		CharInfo& getCharInfo(char c);
		
		enum { ATLAS_FIRST_CHAR = 32, ATLAS_LAST_CHAR = 126, ATLAS_NUM_CHARS = ATLAS_LAST_CHAR - ATLAS_FIRST_CHAR + 1 };
		
		std::array<CharInfo, ATLAS_NUM_CHARS> m_chars;
		TTF_Font* m_font;
		int m_fontSize;
		GLuint m_atlasId;
		Vector2 m_atlasSize;
};

} // font
} // video
} // flat

#endif // FLAT_VIDEO_FONT_FONT_H


