#pragma once

#include "../lib/AGL/agl.hpp"

#include <type_traits>

#define MENU_BORDER		2
#define MENU_PADDING	10
#define MENU_SHADOWSIZE 1

#define MENU_BORDERTHICKNESS (float)6
#define MENU_DECORATIONHEIGHT (float)(4 + (MENU_BORDERTHICKNESS * 4))

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

class MenuElement : public agl::Drawable
{
	public:
		agl::Vec<float, 3> position = {0, 0, 0};
		float			   height	= 0;
		agl::Text		  *text		= nullptr;
		agl::Rectangle	  *rect		= nullptr;

		void init(agl::Text *text, agl::Rectangle *rect);
		// void drawFunction(agl::RenderWindow &window);
};

class TextElement : public MenuElement
{
	public:
		std::string str = "null";

		void init(agl::Text *text, agl::Rectangle *rect)
		{
			this->text = text;
			this->rect = rect;

			height = text->getHeight() * text->getScale();
		}

		void drawFunction(agl::RenderWindow &window) override
		{
			text->setPosition(position);
			text->clearText();
			text->setText(str);

			window.drawText(*text);
		}
};

class ValueElement : public MenuElement
{
	public:
		std::string label = "null";
		std::string value = "null";

		void init(agl::Text *text, agl::Rectangle *rect)
		{
			this->text = text;
			this->rect = rect;

			height = text->getHeight() * text->getScale();
		}

		void drawFunction(agl::RenderWindow &window) override
		{
			text->setPosition(position);
			text->clearText();
			text->setText(label + " - " + value);

			window.drawText(*text);
		}
};

class SpacerElement : public MenuElement
{
	public:
		void init(agl::Text *text, agl::Rectangle *rect)
		{
			height = text->getHeight() * text->getScale();
		}

		void drawFunction(agl::RenderWindow &window) override
		{
		}
};

// template hell
template <typename... ElementType> class Menu : public agl::Drawable
{
	private:
		// agl::Rectangle outerShadowShape;
		// agl::Rectangle borderShape;
		// agl::Rectangle bodyShape;
		// agl::Rectangle innerShadowShape;

		agl::Texture border;

		agl::Rectangle body;

		agl::Rectangle upBorder;
		agl::Rectangle downBorder;
		agl::Rectangle leftBorder;
		agl::Rectangle rightBorder;

		agl::Rectangle tlCorner;
		agl::Rectangle blCorner;
		agl::Rectangle trCorner;
		agl::Rectangle brCorner;

		agl::Text	   text;
		agl::Rectangle rect;

		std::tuple<ElementType...> element;

		template <size_t i = 0> typename std::enable_if<i == sizeof...(ElementType), void>::type initTuple()
		{
			return;
		}

		template <size_t i = 0> typename std::enable_if < i<sizeof...(ElementType), void>::type initTuple()
		{
			std::get<i>(element).init(&text, &rect);

			initTuple<i + 1>();
		}

		template <std::size_t... Indices> static auto make_default_tuple(std::index_sequence<Indices...>)
		{
			return std::make_tuple(ElementType{}...);
		}

		template <size_t i = 0>
		typename std::enable_if<i == sizeof...(ElementType), void>::type draw(agl::RenderWindow	 &window,
																			  agl::Vec<float, 3> &pen)
		{
			return;
		}

		template <size_t i = 0>
			typename std::enable_if <
			i<sizeof...(ElementType), void>::type draw(agl::RenderWindow &window, agl::Vec<float, 3> &pen)
		{
			std::get<i>(element).position = pen;

			window.draw(std::get<i>(element));

			pen.y += std::get<i>(element).height;

			draw<i + 1>(window, pen);
		}

	public:
		agl::Vec<float, 3> position;
		agl::Vec<float, 2> size;

		Menu(agl::Texture *texture, agl::Font *font)
			: element(make_default_tuple(std::index_sequence_for<ElementType...>{}))

		{
			// left
			// right
			// up
			// down
			// tr
			// br
			// tl
			// bl
			border.loadFromFile("img/border.png");

			body.setTexture(texture);
			body.setColor(agl::Color::White);

			leftBorder.setTexture(&border);
			leftBorder.setSize({MENU_BORDERTHICKNESS, 1});
			leftBorder.setTextureTranslation({0, 0});
			leftBorder.setTextureScaling({1, 1 / MENU_DECORATIONHEIGHT});

			rightBorder.setTexture(&border);
			rightBorder.setSize({MENU_BORDERTHICKNESS, 1});
			rightBorder.setTextureTranslation({0, 1 / MENU_DECORATIONHEIGHT * 1});
			rightBorder.setTextureScaling({1, 1 / MENU_DECORATIONHEIGHT});

			upBorder.setTexture(&border);
			upBorder.setSize({MENU_BORDERTHICKNESS, 1});
			upBorder.setRotation({0, 0, 90});
			upBorder.setTextureTranslation({0, 1 / MENU_DECORATIONHEIGHT * 2});
			upBorder.setTextureScaling({1, 1 / MENU_DECORATIONHEIGHT});

			downBorder.setTexture(&border);
			downBorder.setSize({MENU_BORDERTHICKNESS, 1});
			downBorder.setRotation({0, 0, 90});
			downBorder.setTextureTranslation({0, 1 / MENU_DECORATIONHEIGHT * 3});
			downBorder.setTextureScaling({1, 1 / MENU_DECORATIONHEIGHT});

			trCorner.setTexture(&border);
			trCorner.setSize({MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			trCorner.setTextureTranslation({0, 1. - (MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT) * 4});
			trCorner.setTextureScaling({1, MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT});

			brCorner.setTexture(&border);
			brCorner.setSize({MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			brCorner.setTextureTranslation({0, 1. - (MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT) * 3});
			brCorner.setTextureScaling({1, MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT});

			tlCorner.setTexture(&border);
			tlCorner.setSize({MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			tlCorner.setTextureTranslation({0, 1. - (MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT) * 2});
			tlCorner.setTextureScaling({1, MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT});

			blCorner.setTexture(&border);
			blCorner.setSize({MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			blCorner.setTextureTranslation({0, 1. - (MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT)});
			blCorner.setTextureScaling({1, MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT});

			text.setFont(font);
			text.setColor(agl::Color::Black);
			text.setScale(1);

			initTuple();
		}

		void setup(agl::Vec<float, 3> position, agl::Vec<float, 2> size)
		{
			this->position = position;
			this->size	   = size;

			body.setPosition(position);
			body.setSize(size);

			position.z += .1;
			leftBorder.setSize({MENU_BORDERTHICKNESS, size.y});
			leftBorder.setPosition(position);
			rightBorder.setSize({MENU_BORDERTHICKNESS, size.y});
			rightBorder.setPosition(position + agl::Vec<float, 2>{size.x - MENU_BORDERTHICKNESS, 0});
			upBorder.setPosition(position + agl::Vec<float, 2>{0, MENU_BORDERTHICKNESS});
			upBorder.setSize({MENU_BORDERTHICKNESS, size.x});
			downBorder.setPosition(position + agl::Vec<float, 2>{0, size.y});
			downBorder.setSize({MENU_BORDERTHICKNESS, size.x});

			position.z += .1;
			trCorner.setPosition(position + agl::Vec<float, 2>{size.x - MENU_BORDERTHICKNESS, 0});
			tlCorner.setPosition(position);
			brCorner.setPosition(position + agl::Vec<float, 2>{size.x - MENU_BORDERTHICKNESS, size.y - MENU_BORDERTHICKNESS});
			blCorner.setPosition(position + agl::Vec<float, 2>{0, size.y - MENU_BORDERTHICKNESS});

			return;
		}

		template <int i> std::tuple_element_t<i, std::tuple<ElementType...>> &get()
		{
			return std::get<i>(element);
		}

		void setPosition(agl::Vec<float, 3> position)
		{
			this->position = position;

			agl::Vec<float, 3> textOffset = {MENU_BORDER + MENU_SHADOWSIZE + MENU_PADDING,
											 MENU_BORDER + MENU_SHADOWSIZE + MENU_PADDING, 0.4};

			text.setPosition(position + textOffset);
			body.setPosition(position);
		}

		void drawFunction(agl::RenderWindow &window) override
		{
			agl::Vec<float, 3> pen = {MENU_BORDER + MENU_SHADOWSIZE + MENU_PADDING,
									  MENU_BORDER + MENU_SHADOWSIZE + MENU_PADDING, 30};
			pen					   = pen + position;

			window.drawShape(body);
			
			window.drawShape(leftBorder);
			window.drawShape(rightBorder);
			window.drawShape(upBorder);
			window.drawShape(downBorder);

			window.drawShape(trCorner);
			window.drawShape(brCorner);
			window.drawShape(tlCorner);
			window.drawShape(blCorner);


			draw(window, pen);
		}

		void destroy()
		{
			text.clearText();
			border.deleteTexture();
		}
};
