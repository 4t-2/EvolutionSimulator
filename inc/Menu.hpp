#pragma once

#include "../lib/AGL/agl.hpp"
#include "macro.hpp"

#include <type_traits>

inline bool pointInArea(agl::Vec<float, 2> point, agl::Vec<float, 2> position, agl::Vec<float, 2> size)
{
	if (point.x < position.x)
	{
		return false;
	}
	if (point.x > position.x + size.x)
	{
		return false;
	}
	if (point.y < position.y)
	{
		return false;
	}
	if (point.y > position.y + size.y)
	{
		return false;
	}

	return true;
}

class MenuShare
{
	public:
		static agl::Event	  *event;
		static agl::Text	  *text;
		static agl::Text	  *smallText;
		static agl::Rectangle *rect;
		static agl::Texture	  *border;
		static agl::Texture	  *blank;

		static void init(agl::Texture *blank, agl::Font *font, agl::Font *smallFont, agl::Event *event)
		{
			// left
			// right
			// up
			// down
			// tr
			// br
			// tl
			// bl

			border = new agl::Texture();
			border->loadFromFile("img/border.png");

			text = new agl::Text();
			text->setFont(font);
			text->setColor(agl::Color::Black);
			text->setScale(1);

			smallText = new agl::Text();
			smallText->setFont(font);
			smallText->setColor(agl::Color::Black);
			smallText->setScale(1);

			rect = new agl::Rectangle();
			rect->setTexture(blank);

			MenuShare::blank = blank;
			MenuShare::event = event;
		}

		static void destroy()
		{
			text->clearText();
			smallText->clearText();
			border->deleteTexture();

			delete text;
			delete smallText;
			delete border;
			delete rect;
		}
};

class OuterArea : public agl::Drawable, public MenuShare
{
	public:
		agl::Vec<float, 2> position;
		agl::Vec<float, 2> size;

		void drawFunction(agl::RenderWindow &window) override
		{
			// body
			rect->setTexture(border);
			rect->setColor(agl::Color::White);

			// left
			rect->setTextureTranslation({0, 0});
			rect->setSize({MENU_BORDERTHICKNESS, size.y});
			rect->setTextureScaling({1, 1 / MENU_DECORATIONHEIGHT});
			rect->setPosition(position);
			window.drawShape(*rect);

			// right
			rect->setSize({MENU_BORDERTHICKNESS, size.y});
			rect->setTextureTranslation({0, 1 / MENU_DECORATIONHEIGHT * 1});
			rect->setTextureScaling({1, 1 / MENU_DECORATIONHEIGHT});
			rect->setPosition(position + agl::Vec<float, 2>{size.x - MENU_BORDERTHICKNESS, 0});
			window.drawShape(*rect);

			// up
			rect->setTextureTranslation({0, 1 / MENU_DECORATIONHEIGHT * 2});
			rect->setSize({MENU_BORDERTHICKNESS, size.x});
			rect->setRotation({0, 0, 90});
			rect->setTextureScaling({1, 1 / MENU_DECORATIONHEIGHT});
			rect->setPosition(position + agl::Vec<float, 2>{0, MENU_BORDERTHICKNESS});
			window.drawShape(*rect);

			// down
			rect->setSize({MENU_BORDERTHICKNESS, size.x});
			rect->setRotation({0, 0, 90});
			rect->setTextureTranslation({0, 1 / MENU_DECORATIONHEIGHT * 3});
			rect->setTextureScaling({1, 1 / MENU_DECORATIONHEIGHT});
			rect->setPosition(position + agl::Vec<float, 2>{0, size.y});
			window.drawShape(*rect);

			rect->setRotation({0, 0, 0});

			// tr
			rect->setTextureTranslation({0, 1. - (MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT) * 4});
			rect->setRotation({0, 0, 0});
			rect->setPosition(position + agl::Vec<float, 2>{size.x - MENU_BORDERTHICKNESS, 0});
			rect->setSize({MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			rect->setTextureScaling({1, MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT});
			window.drawShape(*rect);

			// br
			rect->setRotation({0, 0, 0});
			rect->setSize({MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			rect->setTextureTranslation({0, 1. - (MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT) * 3});
			rect->setTextureScaling({1, MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT});
			rect->setPosition(position +
							  agl::Vec<float, 2>{size.x - MENU_BORDERTHICKNESS, size.y - MENU_BORDERTHICKNESS});
			window.drawShape(*rect);

			// tl
			rect->setRotation({0, 0, 0});
			rect->setTextureTranslation({0, 1. - (MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT) * 2});
			rect->setPosition(position);
			rect->setSize({MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			rect->setTextureScaling({1, MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT});
			window.drawShape(*rect);

			// bl
			rect->setRotation({0, 0, 0});
			rect->setTextureTranslation({0, 1. - (MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT)});
			rect->setPosition(position + agl::Vec<float, 2>{0, size.y - MENU_BORDERTHICKNESS});
			rect->setSize({MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			rect->setTextureScaling({1, MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT});
			window.drawShape(*rect);

			// body
			rect->setRotation({0, 0, 0});
			rect->setTexture(blank);
			rect->setSize(agl::Vec<float, 2>{size.x, size.y} -
						  agl::Vec<float, 2>{MENU_BORDEREDGE, MENU_BORDEREDGE} * 2);
			rect->setPosition(position + agl::Vec<float, 2>{MENU_BORDEREDGE, MENU_BORDEREDGE});
			rect->setColor({0xc0, 0xc0, 0xc0});
			window.drawShape(*rect);
		}
};

class InnerArea : public agl::Drawable, public MenuShare
{
	public:
		agl::Vec<float, 2> position;
		agl::Vec<float, 2> size;

		void drawFunction(agl::RenderWindow &window) override
		{
			bool state = true;

			// body
			rect->setRotation({0, 0, 0});
			rect->setTexture(blank);
			rect->setSize(size + agl::Vec<float, 2>{-1, -1});
			rect->setPosition(position + agl::Vec<float, 2>{1, 1});
			rect->setColor(agl::Color::White);
			window.drawShape(*rect);

			rect->setTexture(border);

			// left
			rect->setTextureTranslation({1 - ((1 / MENU_BORDERTHICKNESS) * 2), 0});
			rect->setTextureScaling({1. / MENU_BORDERTHICKNESS, 1. / MENU_DECORATIONHEIGHT});
			rect->setSize({1, size.y - 1});
			rect->setPosition(position);
			window.drawShape(*rect);

			// up
			rect->setTextureTranslation({0, 1 / MENU_DECORATIONHEIGHT * 2});
			rect->setSize({2, size.x - 2});
			rect->setRotation({0, 0, 90});
			rect->setTextureScaling({2 / MENU_BORDERTHICKNESS, 1 / MENU_DECORATIONHEIGHT});
			rect->setPosition(position + agl::Vec<float, 2>{1, 2});
			window.drawShape(*rect);

			rect->setRotation({0, 0, 0});

			// down
			rect->setSize({2, size.x - 2});
			rect->setRotation({0, 0, 90});
			rect->setTextureTranslation({1 - (2 / MENU_BORDERTHICKNESS), 3 / MENU_DECORATIONHEIGHT});
			rect->setTextureScaling({2 / MENU_BORDERTHICKNESS, 1 / MENU_DECORATIONHEIGHT});
			rect->setPosition(position + agl::Vec<float, 2>{1, size.y});
			window.drawShape(*rect);

			rect->setRotation({0, 0, 0});
		}
};

class PressedArea : public agl::Drawable, public MenuShare
{
	public:
		agl::Vec<float, 2> position;
		agl::Vec<float, 2> size;

		void drawFunction(agl::RenderWindow &window) override
		{
			// draw button

			rect->setTexture(border);
			rect->setColor(agl::Color::White);

			// left
			rect->setTextureTranslation({0, (1 / MENU_DECORATIONHEIGHT) * 3});

			rect->setSize({MENU_BORDERTHICKNESS, size.y});
			rect->setTextureScaling({1, 1 / MENU_DECORATIONHEIGHT});
			rect->setPosition(position);
			window.drawShape(*rect);

			// right
			rect->setSize({MENU_BORDERTHICKNESS, size.y});
			rect->setTextureTranslation({0, 1 / MENU_DECORATIONHEIGHT * 1});
			rect->setTextureScaling({1, 1 / MENU_DECORATIONHEIGHT});
			rect->setPosition(position + agl::Vec<float, 2>{size.x - MENU_BORDERTHICKNESS, 0});
			window.drawShape(*rect);

			rect->setTextureTranslation({0, 1 / MENU_DECORATIONHEIGHT * 1});
			rect->setSize({MENU_BORDERTHICKNESS, size.x});
			rect->setRotation({0, 0, 90});
			rect->setTextureScaling({1, 1 / MENU_DECORATIONHEIGHT});
			rect->setPosition(position + agl::Vec<float, 2>{0, MENU_BORDERTHICKNESS});
			window.drawShape(*rect);

			// down
			rect->setSize({MENU_BORDERTHICKNESS, size.x});
			rect->setRotation({0, 0, 90});
			rect->setTextureTranslation({0, 1 / MENU_DECORATIONHEIGHT * 3});
			rect->setTextureScaling({1, 1 / MENU_DECORATIONHEIGHT});
			rect->setPosition(position + agl::Vec<float, 2>{0, size.y});
			window.drawShape(*rect);

			rect->setRotation({0, 0, 0});

			// tr
			rect->setTextureTranslation({0, 1. - (MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT) * 3});
			rect->setRotation({0, 0, 90});
			rect->setPosition(position + agl::Vec<float, 2>{size.x - MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			rect->setSize({MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			rect->setTextureScaling({1, MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT});
			window.drawShape(*rect);

			// br
			rect->setRotation({0, 0, 0});
			rect->setSize({MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			rect->setTextureTranslation({0, 1. - (MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT) * 3});
			rect->setTextureScaling({1, MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT});
			rect->setPosition(position +
							  agl::Vec<float, 2>{size.x - MENU_BORDERTHICKNESS, size.y - MENU_BORDERTHICKNESS});
			window.drawShape(*rect);

			// tl
			rect->setRotation({0, 0, 180});
			rect->setTextureTranslation({0, 1. - (MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT) * 3});
			rect->setPosition(position + agl::Vec<float, 2>{MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			rect->setSize({MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			rect->setTextureScaling({1, MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT});
			window.drawShape(*rect);

			// bl
			rect->setRotation({0, 0, 270});
			rect->setTextureTranslation({0, 1. - (MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT) * 3});
			rect->setPosition(position + agl::Vec<float, 2>{MENU_BORDERTHICKNESS, size.y - MENU_BORDERTHICKNESS});
			rect->setSize({MENU_BORDERTHICKNESS, MENU_BORDERTHICKNESS});
			rect->setTextureScaling({1, MENU_BORDERTHICKNESS / MENU_DECORATIONHEIGHT});
			window.drawShape(*rect);

			// body
			rect->setRotation({0, 0, 0});
			rect->setTexture(blank);
			rect->setSize(agl::Vec<float, 2>{size.x, size.y} -
						  agl::Vec<float, 2>{MENU_BORDEREDGE, MENU_BORDEREDGE} * 2);
			rect->setPosition(position + agl::Vec<float, 2>{MENU_BORDEREDGE, MENU_BORDEREDGE});
			rect->setColor({0xc0, 0xc0, 0xc0});
			window.drawShape(*rect);
		}
};

class MenuElement : public agl::Drawable, public MenuShare
{
	public:
		agl::Vec<float, 3> position = {0, 0, 0};
		agl::Vec<float, 2> size		= {0, 0};
		bool			  *focused	= nullptr;

		bool pointInElement(agl::Vec<float, 2> point)
		{
			return pointInArea(point, position, size);
		}

		void init(bool *focused)
		{
			this->focused = focused;

			if (size.y == 0)
			{
				size.y = text->getHeight() * text->getScale();
			}
		}

		// void drawFunction(agl::RenderWindow &window);
};

class TextElement : public MenuElement
{
	public:
		std::string str = "null";

		TextElement()
		{
		}

		TextElement(std::string str)
		{
			this->str = str;
		}

		void drawFunction(agl::RenderWindow &window) override
		{
			text->setPosition(position);
			text->clearText();
			text->setText(str);

			window.drawText(*text);
		}
};

template <typename T> inline std::string toString(T value)
{
	return std::to_string(value);
}

template <> inline std::string toString<std::string>(std::string value)
{
	return value;
}

template <typename T> class ValueElement : public MenuElement
{
	public:
		std::string label = "null";
		T		   *value = nullptr;

		ValueElement()
		{
		}

		ValueElement(std::string label, T *value)
		{
			this->label = label;
			this->value = value;
		}

		void operator=(ValueElement valueElement)
		{
			this->label = valueElement.label;
			this->value = valueElement.value;
		}

		void drawFunction(agl::RenderWindow &window) override
		{
			text->setPosition(position);
			text->clearText();
			text->setText(label + " - " + toString(*value));

			window.drawText(*text);
		}
};

class SpacerElement : public MenuElement
{
	public:
		SpacerElement()
		{
		}

		SpacerElement(float height)
		{
			this->size.y = height;
		}

		void drawFunction(agl::RenderWindow &window) override
		{
		}
};

class ButtonElement : public MenuElement
{
	public:
		float		mouseHeld = false;
		std::string label	  = "null";
		bool		state	  = false;

		ButtonElement()
		{
		}

		ButtonElement(std::string label, float width)
		{
			this->label	 = label;
			this->size.x = width;
		}

		void init(				  bool *focused)
		{
			size.y = text->getHeight() * text->getScale() + MENU_BORDEREDGE * 2;
		}

		void drawFunction(agl::RenderWindow &window)
		{
			if (event->pointerButton == 1 && this->pointInElement(event->getPointerWindowPosition()))
			{
				state = !state;
			}

			if (state)
			{
				PressedArea pressedArea;
				pressedArea.size	 = size;
				pressedArea.position = position;

				window.draw(pressedArea);
			}
			else
			{
				OuterArea outerArea;
				outerArea.size	   = size;
				outerArea.position = position;

				window.draw(outerArea);
			}

			text->setPosition(position + agl::Vec<float, 2>{MENU_BORDEREDGE * 2, 0});
			text->clearText();
			text->setText(label);
			window.drawText(*text);
			text->clearText();
		}
};

class FieldElement : public MenuElement
{
	public:
		std::string label	  = "null";
		std::string value	  = "";
		std::string liveValue = "";

		float fieldWidth = 0;

		bool textFocus = false;

		bool hovered = false;

		FieldElement()
		{
		}

		FieldElement(std::string label, std::string startValue)
		{
			this->label		= label;
			this->value		= startValue;
			this->liveValue = startValue;
		}

		void drawFunction(agl::RenderWindow &window)
		{
			size.x	   = 225;
			fieldWidth = 125;

			// bad fix, too lazy to add utf-8 fonts yet

			bool utf8 = false;

			for (const char &byte : event->keybuffer)
			{
				if (byte & (1 << 8))
				{
					utf8 = true;
					break;
				}
			}

			if (this->pointInElement(event->getPointerWindowPosition()))
			{
				if (event->pointerButton == 1)
				{
					textFocus = !textFocus;
					value	  = liveValue;
				}

				if (hovered) // holding
				{
					window.setCursorShape(XC_xterm);
				}
				else // first
				{
					window.setCursorShape(XC_xterm);
					hovered = true;
				}
			}
			else if (hovered) // last
			{
				window.setCursorShape(XC_left_ptr);
				hovered = false;
			}

			if (textFocus && !utf8)
			{
				liveValue += event->keybuffer;

				while (liveValue[0] == 127 || liveValue[0] == 8)
				{
					liveValue.erase(0, 1);
				}

				for (int i = 1; i < liveValue.length(); i++)
				{
					if (liveValue[i] == 127 || liveValue[i] == 8)
					{
						liveValue.erase(i - 1, 2);
						i--;
					}
				}
			}

			text->setPosition(position);
			text->clearText();
			text->setText(label);

			window.drawText(*text);

			text->clearText();

			if (textFocus)
			{
				rect->setColor({200, 200, 200});
			}
			else
			{
				rect->setColor(agl::Color::Gray);
			}
			rect->setPosition(position + agl::Vec<float, 2>{size.x - fieldWidth, 0});
			rect->setSize({fieldWidth, size.y});

			window.drawShape(*rect);

			text->setPosition(position + agl::Vec<float, 2>{size.x - fieldWidth, 0});
			text->setText(liveValue);

			window.drawText(*text);

			text->clearText();
		}
};

// template hell
template <typename... ElementType> class Menu : public agl::Drawable, public MenuShare
{
	private:
		bool focused = false;

		std::string title;

		std::tuple<ElementType...> element;

		template <size_t i = 0> typename std::enable_if<i == sizeof...(ElementType), void>::type initTuple()
		{
			return;
		}

		template <size_t i = 0> typename std::enable_if < i<sizeof...(ElementType), void>::type initTuple()
		{
			this->get<i>().init(&focused);

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
			this->get<i>().position = pen;

			window.draw(this->get<i>());

			pen.y += this->get<i>().size.y + MENU_PADDING;

			draw<i + 1>(window, pen);
		}

		template <size_t i = 0, typename Element> void assign(Element newElement)
		{
			this->get<i>() = newElement;
			this->get<i>().init(&focused);
		}

		template <size_t i = 0, typename Element, typename... Elements>
		void assign(Element newElement, Elements... newElements)
		{
			this->get<i>() = newElement;
			this->get<i>().init(&focused);

			assign<i + 1>(newElements...);
		}

		template <size_t i = 0>
		typename std::enable_if<i == sizeof...(ElementType), void>::type pointerAssign(void *pointerStruct)
		{
			return;
		}

		template <size_t i = 0>
			typename std::enable_if < i<sizeof...(ElementType), void>::type pointerAssign(void *pointerStruct)
		{
			long  address = (long)(pointerStruct);
			long  offset  = i * 8;
			long *item	  = (long *)(address + offset);

			*item = (long)&this->get<i>();

			pointerAssign<i + 1>(pointerStruct);
		}

	public:
		agl::Vec<float, 3> position;
		agl::Vec<float, 2> size;

		Menu(std::string title) : element(make_default_tuple(std::index_sequence_for<ElementType...>{})), title(title)

		{
			initTuple();
		}

		void setup(agl::Vec<float, 3> position, agl::Vec<float, 2> size)
		{
			this->position = position;
			this->size	   = size;

			return;
		}

		void setupElements(ElementType... Element)
		{
			assign(Element...);
		}

		void bindPointers(void *pointerStruct)
		{
			pointerAssign(pointerStruct);
		}

		template <int i> std::tuple_element_t<i, std::tuple<ElementType...>> &get()
		{
			return std::get<i>(element);
		}

		void drawFunction(agl::RenderWindow &window) override
		{
			agl::Vec<float, 3> pen = {MENU_BORDERTHICKNESS + MENU_PADDING, MENU_BORDERTHICKNESS + MENU_PADDING};
			pen					   = pen + position;

			OuterArea outerArea;
			outerArea.position = position;
			outerArea.size	   = size;

			window.draw(outerArea);

			InnerArea innerArea;
			innerArea.position = position + agl::Vec<float, 2>{4, 4};
			innerArea.size	   = size + agl::Vec<float, 2>{-8, -8};

			window.draw(innerArea);

			draw(window, pen);
		}
};
