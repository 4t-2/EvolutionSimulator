#pragma once

#include "../lib/AGL/agl.hpp"

#include <type_traits>

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
		agl::Rectangle outerShadowShape;
		agl::Rectangle borderShape;
		agl::Rectangle bodyShape;
		agl::Rectangle innerShadowShape;

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
			outerShadowShape.setTexture(texture);
			borderShape.setTexture(texture);
			innerShadowShape.setTexture(texture);
			bodyShape.setTexture(texture);

			text.setFont(font);
			text.setColor(agl::Color::Black);
			text.setScale(1);

			initTuple();
		}

		void setup(agl::Vec<float, 3> position, agl::Vec<float, 2> size)
		{
			this->position = position;
			this->size	   = size;

			outerShadowShape.setPosition(position);
			outerShadowShape.setOffset({0, 0, 0});
			outerShadowShape.setSize(size);
			outerShadowShape.setColor(MENU_SHADOWCOLOR);

			size.x -= MENU_SHADOWSIZE;
			size.y -= MENU_SHADOWSIZE;

			borderShape.setPosition(position);
			borderShape.setOffset({0, 0, 0.1});
			borderShape.setSize(size);
			borderShape.setColor(MENU_BORDERCOLOR);

			size.x -= MENU_BORDER * 2;
			size.y -= MENU_BORDER * 2;

			innerShadowShape.setPosition(position);
			innerShadowShape.setOffset({MENU_BORDER, MENU_BORDER, 0.2});
			innerShadowShape.setSize(size);
			innerShadowShape.setColor(MENU_SHADOWCOLOR);

			size.x -= MENU_SHADOWSIZE;
			size.y -= MENU_SHADOWSIZE;

			bodyShape.setPosition(position);
			bodyShape.setOffset({MENU_BORDER + MENU_SHADOWSIZE, MENU_BORDER + MENU_SHADOWSIZE, 0.3});
			bodyShape.setSize(size);
			bodyShape.setColor(MENU_BODYCOLOR);

			return;
		}

		template <int i> std::tuple_element_t<i, std::tuple<ElementType...>> &get()
		{
			return std::get<i>(element);
		}

		void setPosition(agl::Vec<float, 3> position)
		{
			this->position = position;

			outerShadowShape.setPosition(position);
			borderShape.setPosition(position);
			innerShadowShape.setPosition(position);
			bodyShape.setPosition(position);

			agl::Vec<float, 3> textOffset = {MENU_BORDER + MENU_SHADOWSIZE + MENU_PADDING,
											 MENU_BORDER + MENU_SHADOWSIZE + MENU_PADDING, 0.4};

			text.setPosition(position + textOffset);
		}

		void drawFunction(agl::RenderWindow &window) override
		{
			agl::Vec<float, 3> pen = {MENU_BORDER + MENU_SHADOWSIZE + MENU_PADDING,
									  MENU_BORDER + MENU_SHADOWSIZE + MENU_PADDING, 30};
			pen					   = pen + position;

			draw(window, pen);

			window.drawShape(outerShadowShape);
			window.drawShape(borderShape);
			window.drawShape(bodyShape);
			window.drawShape(innerShadowShape);
		}

		void destroy()
		{
			text.clearText();
		}
};
