// Text.h
#pragma once

namespace GG_Framework
{
	namespace UI
	{
		namespace OSG
		{
class FRAMEWORK_UI_OSG_API Text {
public:
	enum Font {
		BitmapFont,
		FilledFont,
		OutlineFont,
		StrokeFont
	};	
	Text();
	void drawString( Font, std::string );

private :

	std::map<Font,GLuint> fontmap;
	GLuint loadFont(Font font);

	bool createStrokeFont( GLuint fontBase );
	bool createOutlineFont( GLuint fontBase );
	bool createFilledFont( GLuint fontBase );
	bool createBitmapFont( GLuint fontBase );
};
		}
	}
}