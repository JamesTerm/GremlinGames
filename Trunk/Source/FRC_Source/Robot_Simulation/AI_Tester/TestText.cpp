#include "stdafx.h"

void createHUDText(osg::Group *rootNode,osg::Geode* geode )
{
	osgText::Font* font = osgText::readFontFile("fonts/arial.ttf");

	float windowHeight = 1024.0f;
	float windowWidth = 1280.0f;
	float margin = 50.0f;

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//    
	// Examples of how to set up different text layout
	//

	osg::Vec4 layoutColor(1.0f,1.0f,0.0f,1.0f);
	float layoutCharacterSize = 20.0f;    

	{
		osgText::Text* text = new osgText::Text;
		text->setFont(font);
		text->setColor(layoutColor);
		text->setCharacterSize(layoutCharacterSize);
		text->setPosition(osg::Vec3(margin,windowHeight-margin,0.0f));

		// the default layout is left to right, typically used in languages
		// originating from europe such as English, French, German, Spanish etc..
		text->setLayout(osgText::Text::LEFT_TO_RIGHT);

		text->setText("text->setLayout(osgText::Text::LEFT_TO_RIGHT);");
		geode->addDrawable(text);
	}

	{
		osgText::Text* text = new osgText::Text;
		text->setFont(font);
		text->setColor(layoutColor);
		text->setCharacterSize(layoutCharacterSize);
		text->setPosition(osg::Vec3(windowWidth-margin,windowHeight-margin,0.0f));

		// right to left layouts would be used for hebrew or arabic fonts.
		text->setLayout(osgText::Text::RIGHT_TO_LEFT);
		text->setAlignment(osgText::Text::RIGHT_BASE_LINE);

		text->setText("text->setLayout(osgText::Text::RIGHT_TO_LEFT);");
		geode->addDrawable(text);
	}

	{
		osgText::Text* text = new osgText::Text;
		text->setFont(font);
		text->setColor(layoutColor);
		text->setPosition(osg::Vec3(margin,windowHeight-margin,0.0f));
		text->setCharacterSize(layoutCharacterSize);

		// vertical font layout would be used for asian fonts.
		text->setLayout(osgText::Text::VERTICAL);

		text->setText("text->setLayout(osgText::Text::VERTICAL);");
		geode->addDrawable(text);
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//    
	// Examples of how to set up different font resolution
	//

	osg::Vec4 fontSizeColor(0.0f,1.0f,1.0f,1.0f);
	float fontSizeCharacterSize = 30;

	osg::Vec3 cursor = osg::Vec3(margin*2,windowHeight-margin*2,0.0f);

	{
		osgText::Text* text = new osgText::Text;
		text->setFont(font);
		text->setColor(fontSizeColor);
		text->setCharacterSize(fontSizeCharacterSize);
		text->setPosition(cursor);

		// use text that uses 10 by 10 texels as a target resolution for fonts.
		text->setFontResolution(10,10); // blocky but small texture memory usage

		text->setText("text->setFontResolution(10,10); // blocky but small texture memory usage");
		geode->addDrawable(text);
	}

	cursor.y() -= fontSizeCharacterSize;
	{
		osgText::Text* text = new osgText::Text;
		text->setFont(font);
		text->setColor(fontSizeColor);
		text->setCharacterSize(fontSizeCharacterSize);
		text->setPosition(cursor);

		// use text that uses 20 by 20 texels as a target resolution for fonts.
		text->setFontResolution(20,20); // smoother but higher texture memory usage (but still quite low).

		text->setText("text->setFontResolution(20,20); // smoother but higher texture memory usage (but still quite low).");
		geode->addDrawable(text);
	}

	cursor.y() -= fontSizeCharacterSize;
	{
		osgText::Text* text = new osgText::Text;
		text->setFont(font);
		text->setColor(fontSizeColor);
		text->setCharacterSize(fontSizeCharacterSize);
		text->setPosition(cursor);

		// use text that uses 40 by 40 texels as a target resolution for fonts.
		text->setFontResolution(40,40); // even smoother but again higher texture memory usage.

		text->setText("text->setFontResolution(40,40); // even smoother but again higher texture memory usage.");
		geode->addDrawable(text);
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//    
	// Examples of how to set up different sized text
	//

	osg::Vec4 characterSizeColor(1.0f,0.0f,1.0f,1.0f);

	cursor.y() -= fontSizeCharacterSize*2.0f;

	{
		osgText::Text* text = new osgText::Text;
		text->setFont(font);
		text->setColor(characterSizeColor);
		text->setFontResolution(20,20);
		text->setPosition(cursor);

		// use text that is 20 units high.
		text->setCharacterSize(20); // small

		text->setText("text->setCharacterSize(20.0f); // small");
		geode->addDrawable(text);
	}

	cursor.y() -= 30.0f;
	{
		osgText::Text* text = new osgText::Text;
		text->setFont(font);
		text->setColor(characterSizeColor);
		text->setFontResolution(30,30);
		text->setPosition(cursor);

		// use text that is 30 units high.
		text->setCharacterSize(30.0f); // medium

		text->setText("text->setCharacterSize(30.0f); // medium");
		geode->addDrawable(text);
	}

	cursor.y() -= 50.0f;
	{
		osgText::Text* text = new osgText::Text;
		text->setFont(font);
		text->setColor(characterSizeColor);
		text->setFontResolution(40,40);
		text->setPosition(cursor);

		// use text that is 60 units high.
		text->setCharacterSize(60.0f); // large

		text->setText("text->setCharacterSize(60.0f); // large");
		geode->addDrawable(text);
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//    
	// Examples of how to set up different alignments
	//

	osg::Vec4 alignmentSizeColor(0.0f,1.0f,0.0f,1.0f);
	float alignmentCharacterSize = 25.0f;
	cursor.x() = 640;
	cursor.y() = margin*4.0f;

	typedef std::pair<osgText::Text::AlignmentType,std::string> AlignmentPair;
	typedef std::vector<AlignmentPair> AlignmentList;
	AlignmentList alignmentList;
	alignmentList.push_back(AlignmentPair(osgText::Text::LEFT_TOP,"text->setAlignment(\nosgText::Text::LEFT_TOP);"));
	alignmentList.push_back(AlignmentPair(osgText::Text::LEFT_CENTER,"text->setAlignment(\nosgText::Text::LEFT_CENTER);"));
	alignmentList.push_back(AlignmentPair(osgText::Text::LEFT_BOTTOM,"text->setAlignment(\nosgText::Text::LEFT_BOTTOM);"));
	alignmentList.push_back(AlignmentPair(osgText::Text::CENTER_TOP,"text->setAlignment(\nosgText::Text::CENTER_TOP);"));
	alignmentList.push_back(AlignmentPair(osgText::Text::CENTER_CENTER,"text->setAlignment(\nosgText::Text::CENTER_CENTER);"));
	alignmentList.push_back(AlignmentPair(osgText::Text::CENTER_BOTTOM,"text->setAlignment(\nosgText::Text::CENTER_BOTTOM);"));
	alignmentList.push_back(AlignmentPair(osgText::Text::RIGHT_TOP,"text->setAlignment(\nosgText::Text::RIGHT_TOP);"));
	alignmentList.push_back(AlignmentPair(osgText::Text::RIGHT_CENTER,"text->setAlignment(\nosgText::Text::RIGHT_CENTER);"));
	alignmentList.push_back(AlignmentPair(osgText::Text::RIGHT_BOTTOM,"text->setAlignment(\nosgText::Text::RIGHT_BOTTOM);"));
	alignmentList.push_back(AlignmentPair(osgText::Text::LEFT_BASE_LINE,"text->setAlignment(\nosgText::Text::LEFT_BASE_LINE);"));
	alignmentList.push_back(AlignmentPair(osgText::Text::CENTER_BASE_LINE,"text->setAlignment(\nosgText::Text::CENTER_BASE_LINE);"));
	alignmentList.push_back(AlignmentPair(osgText::Text::RIGHT_BASE_LINE,"text->setAlignment(\nosgText::Text::RIGHT_BASE_LINE);"));
	alignmentList.push_back(AlignmentPair(osgText::Text::LEFT_BOTTOM_BASE_LINE,"text->setAlignment(\nosgText::Text::LEFT_BOTTOM_BASE_LINE);"));
	alignmentList.push_back(AlignmentPair(osgText::Text::CENTER_BOTTOM_BASE_LINE,"text->setAlignment(\nosgText::Text::CENTER_BOTTOM_BASE_LINE);"));
	alignmentList.push_back(AlignmentPair(osgText::Text::RIGHT_BOTTOM_BASE_LINE,"text->setAlignment(\nosgText::Text::RIGHT_BOTTOM_BASE_LINE);"));


	osg::Sequence* sequence = new osg::Sequence;
	{
		for(AlignmentList::iterator itr=alignmentList.begin();
			itr!=alignmentList.end();
			++itr)
		{
			osg::Geode* alignmentGeode = new osg::Geode;
			sequence->addChild(alignmentGeode);
			sequence->setTime(sequence->getNumChildren(), 1.0f);

			osgText::Text* text = new osgText::Text;
			text->setFont(font);
			text->setColor(alignmentSizeColor);
			text->setCharacterSize(alignmentCharacterSize);
			text->setPosition(cursor);
			text->setDrawMode(osgText::Text::TEXT|osgText::Text::ALIGNMENT|osgText::Text::BOUNDINGBOX);

			text->setAlignment(itr->first);
			text->setText(itr->second);

			alignmentGeode->addDrawable(text);


		}

	}

	sequence->setMode(osg::Sequence::START);
	sequence->setInterval(osg::Sequence::LOOP, 0, -1);
	sequence->setDuration(1.0f, -1);

	rootNode->addChild(sequence);


	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//    
	// Examples of how to set up different fonts...
	//

	cursor.x() = margin*2.0f;
	cursor.y() = margin*2.0f;

	osg::Vec4 fontColor(1.0f,0.5f,0.0f,1.0f);
	float fontCharacterSize = 20.0f;
	float spacing = 40.0f;

	{
		osgText::Text* text = new osgText::Text;
		text->setColor(fontColor);
		text->setPosition(cursor);
		text->setCharacterSize(fontCharacterSize);

		text->setFont(0);
		text->setText("text->setFont(0); // inbuilt font.");
		geode->addDrawable(text);

		cursor.x() = text->getBound().xMax() + spacing ;
	}

	{
		osgText::Font* arial = osgText::readFontFile("fonts/arial.ttf");

		osgText::Text* text = new osgText::Text;
		text->setColor(fontColor);
		text->setPosition(cursor);
		text->setCharacterSize(fontCharacterSize);

		text->setFont(arial);
		text->setText(arial!=0?
			"text->setFont(\"fonts/arial.ttf\");":
		"unable to load \"fonts/arial.ttf\"");
		geode->addDrawable(text);

		cursor.x() = text->getBound().xMax() + spacing ;
	}

	{
		osgText::Font* times = osgText::readFontFile("fonts/times.ttf");

		osgText::Text* text = new osgText::Text;
		text->setColor(fontColor);
		text->setPosition(cursor);
		text->setCharacterSize(fontCharacterSize);

		geode->addDrawable(text);
		text->setFont(times);
		text->setText(times!=0?
			"text->setFont(\"fonts/times.ttf\");":
		"unable to load \"fonts/times.ttf\"");

		cursor.x() = text->getBound().xMax() + spacing ;
	}

	cursor.x() = margin*2.0f;
	cursor.y() = margin;

	{
		osgText::Font* dirtydoz = osgText::readFontFile("fonts/dirtydoz.ttf");

		osgText::Text* text = new osgText::Text;
		text->setColor(fontColor);
		text->setPosition(cursor);
		text->setCharacterSize(fontCharacterSize);

		text->setFont(dirtydoz);
		text->setText(dirtydoz!=0?
			"text->setFont(\"fonts/dirtydoz.ttf\");":
		"unable to load \"fonts/dirtydoz.ttf\"");
		geode->addDrawable(text);

		cursor.x() = text->getBound().xMax() + spacing ;
	}

	{
		osgText::Font* fudd = osgText::readFontFile("fonts/fudd.ttf");

		osgText::Text* text = new osgText::Text;
		text->setColor(fontColor);
		text->setPosition(cursor);
		text->setCharacterSize(fontCharacterSize);

		text->setFont(fudd);
		text->setText(fudd!=0?
			"text->setFont(\"fonts/fudd.ttf\");":
		"unable to load \"fonts/fudd.ttf\"");
		geode->addDrawable(text);

		cursor.x() = text->getBound().xMax() + spacing ;
	}

}

