#pragma once

#include "../lib/AGL/agl.hpp"

#include <type_traits>

#define MENU_BORDER		2
#define MENU_PADDING	10
#define MENU_SHADOWSIZE 1

#define MENU_BORDERTHICKNESS  (float)6
#define MENU_DECORATIONHEIGHT (float)(4 + (MENU_BORDERTHICKNESS * 4))

#define MENU_BORDEREDGE (float)2

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
		agl::Texture	  *border	= nullptr;
		agl::Texture	  *blank	= nullptr;
		agl::Event		  *event	= nullptr;

		void init(agl::Text *text, agl::Rectangle *rect, agl::Texture *border, agl::Texture *blank, agl::Event *event)
		{
			this->text	 = text;
			this->rect	 = rect;
			this->border = border;
			this->blank	 = blank;
			this->event	 = event;

			height = text->getHeight() * text->getScale();
		}

		// void drawFunction(agl::RenderWindow &window);
};

class TextElement : public MenuElement
{
	public:
		std::string str = "null";

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
		void drawFunction(agl::RenderWindow &window) override
		{
		}
};

class ButtonElement : public MenuElement
{
	public:
		float mouseHeld = false;

	public:
		std::string label = "null";
		float		width = 200;
		bool		state = false;

		void init(agl::Text *text, agl::Rectangle *rect, agl::Texture *border, agl::Texture *blank, agl::Event *event)
		{
			this->text	 = text;
			this->rect	 = rect;
			this->border = border;
			this->blank	 = blank;
			this->event	 = event;

			height = text->getHeight() * text->getScale() + MENU_BORDEREDGE * 2;
		}

		bool pointInArea(agl::Vec<float, 2> point)
		{
			if (point.x < position.x)
			{
				return false;
			}
			if (point.x > position.x + width)
			{
				return false;
			}
			if (point.y < position.y)
			{
				return false;
			}
			if (point.y > position.y + height)
			{
				return false;
			}

			return true;
		}

		void drawFunction(agl::RenderWindow &window)
		{
			// update button (toggle)

			if (this->pointInArea(event->getPointerWindowPosition()))
			{
				if (event->isPointerButtonPressed(Button1Mask))
				{
					if(!mouseHeld)
					{
						state = !state;
					}
					mouseHeld = true;
				} else {
					mouseHeld = false;
				}
			}

			// draw button

			rect->setTexture(border);
			rect->setColor(agl::Color::White);

			// left
			if (state)
			{
				rect->setTextureTranslation({0, (1 / MENU_DECORATIONHEIGHT) * 3});
			}
			else
			{
				rect->setTextureTranslation({0, 0});
			}
			rect->setSize({MENU_BORDERTHICKNESS, height});
			rect->setTextureScaling({1, 1 / MENU_DECORATIONHEIGHT});
			rect->setPosition(position);
			window.drawShape(*rect);

			// right
			rect->setSize({MENU_BORDERTHICKNESS, height});
			rect->setTextureTranslation({0, 1 / MENU_DECORATIONHEIGHT * 1});
			rect->setTextureScaling({1, 1 / MENU_DECORATIONHEIGHT});
			rect->setPosition(position + agl::Vec<float, 2>{width - MENU_BORDERTHICKNESS, 0});
			window.drawShape(*rect);

			// up
			if (state)
			{
				rect->setTextureTranslation({0, 1 / MENU_DECORATIONHEIGHT * 1});
			}
			else
			{
				rect->setTextureTranslation({0, 1 / MENU_DECORATIONHEIGHT * 2});
			}
			rect->setSize({MENU_BORDERTHICKNESS, width});
			rect->setRotation({0, 0, 90});
			rect->setTextureScaling({1, 1 / MENU_DECORATIONHEIGHT});
			rect->setPosition(position + agl::Vec<float, 2>{0, MENU_BORDERTHICKNESS});
			window.drawShape(*rect);

			// down
			rect->setSize({MENU_BORDERTHICKNESS, width});
			rect->setRotation({0, 0, 90});
			rect->setTextureTranslation({0, 1 / MENU_DECORATIONHEIGHT * 3});
			rect->setTextureScaling({1, 1 / MENU_DECORATIONHEIGHT});
			rect->setPosition(position + agl::Vec<float, 2>{0, height});
			window.drawShape(*rect);

			rect->setRotation({0, 0, 0});

			// tr
			if (state)
			{
				rect->setTextureTranslation({0, 1. - (MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT) * 3});
				rect->setRotation({0, 0, 90});
				rect->setPosition(position + agl::Vec<float, 2>{width - MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			}
			else
			{
				rect->setTextureTranslation({0, 1. - (MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT) * 4});
				rect->setRotation({0, 0, 0});
				rect->setPosition(position + agl::Vec<float, 2>{width - MENU_BORDERTHICKNESS, 0});
			}
			rect->setSize({MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			rect->setTextureScaling({1, MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT});
			window.drawShape(*rect);

			// br
			rect->setRotation({0, 0, 0});
			rect->setSize({MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			rect->setTextureTranslation({0, 1. - (MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT) * 3});
			rect->setTextureScaling({1, MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT});
			rect->setPosition(position +
							  agl::Vec<float, 2>{width - MENU_BORDERTHICKNESS, height - MENU_BORDERTHICKNESS});
			window.drawShape(*rect);

			// tl
			if (state)
			{
				rect->setRotation({0, 0, 180});
				rect->setTextureTranslation({0, 1. - (MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT) * 3});
				rect->setPosition(position + agl::Vec<float, 2>{MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			}
			else
			{
				rect->setRotation({0, 0, 0});
				rect->setTextureTranslation({0, 1. - (MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT) * 2});
				rect->setPosition(position);
			}
			rect->setSize({MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			rect->setTextureScaling({1, MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT});
			window.drawShape(*rect);

			// bl
			if (state)
			{
				rect->setRotation({0, 0, 270});
				rect->setTextureTranslation({0, 1. - (MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT) * 3});
				rect->setPosition(position + agl::Vec<float, 2>{MENU_BORDERTHICKNESS, height - MENU_BORDERTHICKNESS});
			}
			else
			{
				rect->setRotation({0, 0, 0});
				rect->setTextureTranslation({0, 1. - (MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT)});
				rect->setPosition(position + agl::Vec<float, 2>{0, height - MENU_BORDERTHICKNESS});
			}
			rect->setSize({MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			rect->setTextureScaling({1, MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT});
			window.drawShape(*rect);

			// body
			rect->setRotation({0, 0, 0});
			rect->setTexture(blank);
			rect->setSize(agl::Vec<float, 2>{width, height} - agl::Vec<float, 2>{MENU_BORDEREDGE, MENU_BORDEREDGE} * 2);
			rect->setPosition(position + agl::Vec<float, 2>{MENU_BORDEREDGE, MENU_BORDEREDGE});
			rect->setColor({0xc0, 0xc0, 0xc0});
			window.drawShape(*rect);

			text->setPosition(position + agl::Vec<float, 2>{MENU_BORDEREDGE * 2, 0});
			text->clearText();
			text->setText(label);
			window.drawText(*text);
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

		agl::Texture  border;
		agl::Texture *blank;

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

		agl::Event *event;

		std::tuple<ElementType...> element;

		template <size_t i = 0> typename std::enable_if<i == sizeof...(ElementType), void>::type initTuple()
		{
			return;
		}

		template <size_t i = 0> typename std::enable_if < i<sizeof...(ElementType), void>::type initTuple()
		{
			std::get<i>(element).init(&text, &rect, &border, blank, event);

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

		Menu(agl::Texture *texture, agl::Font *font, agl::Event *event)
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

			rect.setTexture(texture);

			this->blank = texture;
			this->event = event;

			initTuple();
		}

		void setup(agl::Vec<float, 3> position, agl::Vec<float, 2> size)
		{
			this->position = position;
			this->size	   = size;

			this->setPosition(position);

			body.setSize(size);
			leftBorder.setSize({MENU_BORDERTHICKNESS, size.y});
			rightBorder.setSize({MENU_BORDERTHICKNESS, size.y});
			upBorder.setSize({MENU_BORDERTHICKNESS, size.x});
			downBorder.setSize({MENU_BORDERTHICKNESS, size.x});

			return;
		}

		template <int i> std::tuple_element_t<i, std::tuple<ElementType...>> &get()
		{
			return std::get<i>(element);
		}

		void setPosition(agl::Vec<float, 3> position)
		{
			this->position = position;

			body.setPosition(position);
			leftBorder.setPosition(position);
			rightBorder.setPosition(position + agl::Vec<float, 2>{size.x - MENU_BORDERTHICKNESS, 0});
			upBorder.setPosition(position + agl::Vec<float, 2>{0, MENU_BORDERTHICKNESS});
			downBorder.setPosition(position + agl::Vec<float, 2>{0, size.y});
			trCorner.setPosition(position + agl::Vec<float, 2>{size.x - MENU_BORDERTHICKNESS, 0});
			tlCorner.setPosition(position);
			brCorner.setPosition(position +
								 agl::Vec<float, 2>{size.x - MENU_BORDERTHICKNESS, size.y - MENU_BORDERTHICKNESS});
			blCorner.setPosition(position + agl::Vec<float, 2>{0, size.y - MENU_BORDERTHICKNESS});
		}

		void drawFunction(agl::RenderWindow &window) override
		{
			agl::Vec<float, 3> pen = {MENU_BORDERTHICKNESS + MENU_PADDING, MENU_BORDERTHICKNESS + MENU_PADDING};
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
