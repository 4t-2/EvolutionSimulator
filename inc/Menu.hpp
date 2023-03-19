#pragma once

#include "../lib/AGL/agl.hpp"

#define MENU_BORDER		2
#define MENU_PADDING	10
#define MENU_SHADOWSIZE 1

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

enum MenuElement
{
	TEXT = 0,
};

class TextElement : public agl::Drawable
{
	private:
		agl::Text &text;

	public:
		TextElement(agl::Text &text) : text(text)
		{
		}

		agl::Vec<float, 3> position = {0, 0, 0};
		std::string		   str		= "null";

		void drawFunction(agl::RenderWindow &window) override
		{
			text.setPosition(position);
			text.clearText();
			text.setText(str);
			window.drawText(text);
		}
};

class Menu : public agl::Drawable
{
	private:
		agl::Rectangle outerShadowShape;
		agl::Rectangle borderShape;
		agl::Rectangle bodyShape;
		agl::Rectangle innerShadowShape;

		agl::Text text;

		std::vector<MenuElement> menuElement;
		std::vector<TextElement> textElement;

	public:
		agl::Vec<float, 3> position;
		agl::Vec<float, 2> size;

		Menu(agl::Texture *texture, agl::Font *font, std::vector<MenuElement> element);

		void setup(agl::Vec<float, 3> position, agl::Vec<float, 2> size);

		void setElement(int i, std::string str);

		void setPosition(agl::Vec<float, 3> position);

		void drawFunction(agl::RenderWindow &window) override
		{
			agl::Vec<float, 3> pen = {MENU_BORDER + MENU_SHADOWSIZE + MENU_PADDING,
									  MENU_BORDER + MENU_SHADOWSIZE + MENU_PADDING, 30};
			pen					   = pen + position;

			for (int i = 0; i < menuElement.size(); i++)
			{
				if (menuElement[i] == MenuElement::TEXT)
				{
					textElement[i].position = pen;
					window.draw(textElement[i]);

					pen.y += text.getHeight();
				}
			}
			
			window.drawShape(outerShadowShape);
			window.drawShape(borderShape);
			window.drawShape(bodyShape);
			window.drawShape(innerShadowShape);
		}

		void destroy();
};
