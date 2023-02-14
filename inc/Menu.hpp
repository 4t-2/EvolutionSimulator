#pragma once

#include "../lib/AGL/agl.hpp"

#define MENU_BORDER		  2
#define MENU_PADDING	  10
#define MENU_SHADOWSIZE	  1

#define MENU_BORDERCOLOR \
	{                    \
		140, 140, 140    \
	}
#define MENU_SHADOWCOLOR \
	{                    \
		100, 100, 100    \
	}
#define MENU_BODYCOLOR \
	{                  \
		210, 210, 210  \
	}

class Menu
{
	private:
		agl::Rectangle outerShadowShape;
		agl::Rectangle borderShape;
		agl::Rectangle bodyShape;
		agl::Rectangle innerShadowShape;

		agl::Text text;

		agl::Vec<float, 3> position;
		agl::Vec<float, 2> size;

	public:
		void setup(agl::Vec<float, 3> position, agl::Vec<float, 2> size, agl::Texture *texture, agl::Font *font);

		void setPosition(agl::Vec<float, 3> position); 
		void setText(std::string str);

		void destroy();

		agl::Rectangle	  *getBorderShape();
		agl::Rectangle	  *getBodyShape();
		agl::Rectangle	  *getOuterShadowShape();
		agl::Rectangle	  *getInnerShadowShape();
		agl::Text		  *getText();
		agl::Vec<float, 3> getPosition();
		agl::Vec<float, 2>   getSize();
};
