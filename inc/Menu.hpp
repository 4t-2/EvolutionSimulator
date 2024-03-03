#pragma once

#include "macro.hpp"
#include <AGL/agl.hpp>
#include <IN/intnet.hpp>

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
		static agl::Circle	  *circ;
		static agl::Texture	  *border;
		static agl::Texture	  *blank;
		static void			  *focusedMenu;
		static bool			  *leftClick;
		static agl::Shader	  *menuShader;
		static agl::Shader	  *baseShader;
		static agl::Camera	  *camera;

		static void init(agl::Texture *blank, agl::Font *font, agl::Font *smallFont, agl::Event *event, bool *leftClick)
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
			smallText->setFont(smallFont);
			smallText->setColor(agl::Color::Black);
			smallText->setScale(1);

			rect = new agl::Rectangle();
			rect->setTexture(blank);

			circ = new agl::Circle(30);
			circ->setTexture(blank);

			MenuShare::blank	 = blank;
			MenuShare::event	 = event;
			MenuShare::leftClick = leftClick;
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
			delete circ;
		}
};

class OuterArea : public agl::Drawable, public MenuShare
{
	public:
		agl::Vec<float, 2> position;
		agl::Vec<float, 2> size;

		void drawFunction(agl::RenderWindow &window) override
		{
			window.getShaderUniforms(*menuShader);
			menuShader->use();
			window.updateMvp(*camera);

			glUniform1f(menuShader->getUniformLocation("scaleX"), size.x);
			glUniform1f(menuShader->getUniformLocation("scaleY"), size.y);
			glUniform1i(menuShader->getUniformLocation("type"), 1);

			rect->setTexture(blank);
			rect->setColor(agl::Color::White);

			rect->setPosition(position);
			rect->setSize(size);
			window.drawShape(*rect);
			
			window.getShaderUniforms(*baseShader);
			baseShader->use();
			window.updateMvp(*camera);
		}
};

class ThinAreaOut : public agl::Drawable, public MenuShare
{
	public:
		agl::Vec<float, 2> position;
		agl::Vec<float, 2> size;

		void drawFunction(agl::RenderWindow &window) override
		{
			window.getShaderUniforms(*menuShader);
			menuShader->use();
			window.updateMvp(*camera);

			glUniform1f(menuShader->getUniformLocation("scaleX"), size.x);
			glUniform1f(menuShader->getUniformLocation("scaleY"), size.y);
			glUniform1i(menuShader->getUniformLocation("type"), 0);

			rect->setColor(agl::Color::White);
			rect->setPosition(position);
			rect->setSize(size);
			rect->setTextureScaling({1, 1, 1});
			rect->setTextureTranslation({0, 0, 0});
			window.drawShape(*rect);
			
			window.getShaderUniforms(*baseShader);
			baseShader->use();
			window.updateMvp(*camera);
		}
};

class ThinAreaIn : public agl::Drawable, public MenuShare
{
	public:
		agl::Vec<float, 2> position;
		agl::Vec<float, 2> size;

		void drawFunction(agl::RenderWindow &window) override
		{
			window.getShaderUniforms(*menuShader);
			menuShader->use();
			window.updateMvp(*camera);

			glUniform1f(menuShader->getUniformLocation("scaleX"), size.x);
			glUniform1f(menuShader->getUniformLocation("scaleY"), size.y);
			glUniform1i(menuShader->getUniformLocation("type"), 2);

			rect->setTexture(blank);

			rect->setColor(agl::Color::White);
			rect->setPosition(position);
			rect->setSize(size);

			window.getShaderUniforms(*baseShader);
			baseShader->use();
			window.updateMvp(*camera);
		}
};

class DipArea : public agl::Drawable, public MenuShare
{
	public:
		agl::Vec<float, 2> position;
		agl::Vec<float, 2> size;

		void drawFunction(agl::RenderWindow &window) override
		{
			window.getShaderUniforms(*menuShader);
			menuShader->use();
			window.updateMvp(*camera);

			glUniform1f(menuShader->getUniformLocation("scaleX"), size.x);
			glUniform1f(menuShader->getUniformLocation("scaleY"), size.y);
			glUniform1i(menuShader->getUniformLocation("type"), 3);

			rect->setTexture(blank);
			rect->setColor(agl::Color::White);

			rect->setColor({0, 0, 0});
			rect->setPosition(position);
			rect->setSize(size);
			window.drawShape(*rect);

			window.getShaderUniforms(*baseShader);
			baseShader->use();
			window.updateMvp(*camera);
		}
};

class InnerArea : public agl::Drawable, public MenuShare
{
	public:
		agl::Vec<float, 2> position;
		agl::Vec<float, 2> size;

		void drawFunction(agl::RenderWindow &window) override
		{
			window.getShaderUniforms(*menuShader);
			menuShader->use();
			window.updateMvp(*camera);

			glUniform1f(menuShader->getUniformLocation("scaleX"), size.x);
			glUniform1f(menuShader->getUniformLocation("scaleY"), size.y);
			glUniform1i(menuShader->getUniformLocation("type"), 4);

			bool state = true;
      (void)state;
			// body
			rect->setRotation({0, 0, 0});
			rect->setTexture(blank);
			rect->setSize(size);
			rect->setPosition(position);
			rect->setColor(agl::Color::White);
			window.drawShape(*rect);

			window.getShaderUniforms(*baseShader);
			baseShader->use();
			window.updateMvp(*camera);
		}
};

class PressedArea : public agl::Drawable, public MenuShare
{
	public:
		agl::Vec<float, 2> position;
		agl::Vec<float, 2> size;

		void drawFunction(agl::RenderWindow &window) override
		{
			window.getShaderUniforms(*menuShader);
			menuShader->use();
			window.updateMvp(*camera);

			glUniform1f(menuShader->getUniformLocation("scaleX"), size.x);
			glUniform1f(menuShader->getUniformLocation("scaleY"), size.y);
			glUniform1i(menuShader->getUniformLocation("type"), 5);

			bool state = true;

			// body
			rect->setRotation({0, 0, 0});
			rect->setTexture(blank);
			rect->setSize(size);
			rect->setPosition(position);
			rect->setColor(agl::Color::White);
			window.drawShape(*rect);

			window.getShaderUniforms(*baseShader);
			baseShader->use();
			window.updateMvp(*camera);
		}
};

class MenuElement : public agl::Drawable, public MenuShare
{
	public:
		agl::Vec<float, 3> position = {0, 0, 0};
		agl::Vec<float, 2> size		= {0, 0};

		bool pointInElement(agl::Vec<float, 2> point)
		{
			return pointInArea(point, position, size);
		}

		virtual void init(float width)
		{
			if (size.y == 0)
			{
				size.y = text->getHeight() * text->getScale();
			}

			size.x = width;
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

template <> inline std::string toString<const char *>(const char *value)
{
	return value;
}

template <typename T> class ValueElement : public MenuElement
{
	public:
		std::string			 label = "null";
		std::function<T *()> valueFunc;

		ValueElement()
		{
		}

		ValueElement(std::string label, std::function<T *()> valueFunc)
		{
			this->label		= label;
			this->valueFunc = valueFunc;
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
			text->setText(label + " - " + toString(*valueFunc()));

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

enum ButtonType
{
	Toggle,
	Hold,
};

template <ButtonType buttonType> class ButtonElement : public MenuElement
{
	public:
		float		mouseHeld = false;
		std::string label	  = "null";
		bool		state	  = false;

		ButtonElement()
		{
		}

		ButtonElement(std::string label)
		{
			this->label = label;
		}

		void init(float width) override
		{
			size.y = text->getHeight() * text->getScale() + 7;
			size.x = width;
		}

		void drawFunction(agl::RenderWindow &window) override
		{
			if constexpr (buttonType == ButtonType::Toggle)
			{
				if (*leftClick && this->pointInElement(event->getPointerWindowPosition()))
				{
					state = !state;
				}
			}
			else if constexpr (buttonType == ButtonType::Hold)
			{
				if (event->isPointerButtonPressed(agl::Button::Left) &&
					this->pointInElement(event->getPointerWindowPosition()))
				{
					state = true;
				}
				else
				{
					state = false;
				}
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

class FocusableElement
{
	public:
		virtual void unFocus()
		{
		}
		static FocusableElement *focusedField;
};

template <typename T> class FieldElement : public MenuElement, public FocusableElement
{
	public:
		std::string label = "null";
		T			value;
		std::string liveValue = "";

		float labelBuffer = 0;

		bool textFocus = false;
		bool valid	   = true;
		bool hovered   = false;

		FieldElement()
		{
		}

		FieldElement(std::string label, T startValue)
		{
			this->label		= label;
			this->value		= startValue;
			this->liveValue = toString(this->value);
		}

		void init(float width) override
		{
			size.y		= text->getHeight() + 7;
			size.x		= width;
			labelBuffer = 80;
		}

		void unFocus() override
		{
			textFocus = false;

			try
			{
				if constexpr (std::is_same_v<T, std::string>)
				{
					value = liveValue;
				}
				else if constexpr (std::is_same_v<T, int>)
				{
					value = std::stoi(liveValue);
				}
				else if constexpr (std::is_same_v<T, float>)
				{
					value = std::stof(liveValue);
				}

				valid = true;
			}
			catch (const std::invalid_argument &)
			{
				valid = false;
			}

			focusedField = nullptr;
		}

		void drawFunction(agl::RenderWindow &window) override
		{
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
				if (*leftClick)
				{
					textFocus = !textFocus;

					if (textFocus)
					{
						if (focusedField != nullptr)
						{
							focusedField->unFocus();
						}

						focusedField = this;
					}
					else
					{
						this->unFocus();
					}
				}

				if (hovered) // holding
				{
					window.setCursorShape(agl::CursorType::Beam);
				}
				else // first
				{
					window.setCursorShape(agl::CursorType::Beam);
					hovered = true;
				}
			}
			else if (hovered) // last
			{
				window.setCursorShape(agl::CursorType::Arrow);
				hovered = false;
			}

			if (event->keybuffer.find('\r') != std::string::npos && textFocus)
			{
				this->unFocus();
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

			text->setPosition(position + agl::Vec<float, 2>{0, 0});
			text->clearText();
			text->setText(label);

			window.drawText(*text);

			text->clearText();

			InnerArea innerArea;
			innerArea.position = position + agl::Vec<float, 2>{labelBuffer, 0};
			innerArea.size	   = {size.x - labelBuffer, size.y};

			window.draw(innerArea);

			text->setPosition(position + agl::Vec<float, 2>{labelBuffer + 5, 0});
			text->setText(liveValue);

			if (valid)
			{
				text->setColor(agl::Color::Black);
				rect->setColor(agl::Color::Black);
			}
			else
			{
				text->setColor(agl::Color::Red);
				rect->setColor(agl::Color::Red);
			}

			agl::Vec<float, 2> start = window.drawText(*text) + position + agl::Vec<float, 2>{labelBuffer + 5, 0};

			if (textFocus)
			{
				rect->setPosition(start + agl::Vec<float, 2>{0, 2});
				rect->setSize({1, (float)text->getHeight() - 1});
				window.drawShape(*rect);
			}

			text->setColor(agl::Color::Black);
			rect->setColor(agl::Color::Black);

			text->clearText();
		}
};

class NetworkGraph : public MenuElement
{
	public:
		int selectedID;

		in::NeuralNetwork **network;

		std::function<in::NeuralNetwork **()> networkFunc;

		NetworkGraph()
		{
		}

		NetworkGraph(std::function<in::NeuralNetwork **()> networkFunc) : networkFunc(networkFunc)
		{
		}

		void init(float width) override
		{
			size = {300, 300};
		}

		void drawFunction(agl::RenderWindow &window) override
		{
			circ->setTexture(blank);
			circ->setColor({15, 15, 15});
			circ->setSize(agl::Vec<float, 3>{150, 150, 0});
			circ->setPosition(position + agl::Vec<float, 2>{150, 150});
			window.drawShape(*circ);

			// draw node connections
			for (int i = 0; i < (*networkFunc())->structure.totalConnections; i++)
			{
				in::Connection connection = (*networkFunc())->getConnection(i);

				if (!connection.valid)
				{
					continue;
				}

				float startAngle = connection.startNode + 1;
				startAngle /= (*networkFunc())->getTotalNodes();
				startAngle *= PI * 2;

				float endAngle = connection.endNode + 1;
				endAngle /= (*networkFunc())->getTotalNodes();
				endAngle *= PI * 2;

				agl::Vec<float, 2> startPosition = agl::pointOnCircle(startAngle);
				startPosition.x = (startPosition.x * (circ->getSize().x - NETWORK_PADDING)) + circ->getPosition().x;
				startPosition.y = (startPosition.y * (circ->getSize().x - NETWORK_PADDING)) + circ->getPosition().y;

				agl::Vec<float, 2> endPosition = agl::pointOnCircle(endAngle);
				endPosition.x = (endPosition.x * (circ->getSize().x - NETWORK_PADDING)) + circ->getPosition().x;
				endPosition.y = (endPosition.y * (circ->getSize().x - NETWORK_PADDING)) + circ->getPosition().y;

				agl::Vec<float, 2> offset = startPosition - endPosition;

				float angle = agl::radianToDegree(-offset.angle());

				float weight = connection.weight;

				rect->setTexture(blank);

				if (weight > 0)
				{
					rect->setColor({0, (unsigned char)(weight * 255), BASE_B_VALUE});
				}
				else
				{
					rect->setColor({(unsigned char)(-weight * 255), 0, BASE_B_VALUE});
				}

				rect->setSize(agl::Vec<float, 2>{2, offset.length()});
				rect->setPosition(startPosition);
				rect->setRotation(agl::Vec<float, 3>{0, 0, angle});
				window.drawShape(*rect);
			}

			// draw nodes
			for (int i = 0; i < (*networkFunc())->getTotalNodes(); i++)
			{
				float angle = (360. / (*networkFunc())->getTotalNodes()) * (i + 1);

				float x = cos(angle * (3.14159 / 180));
				float y = sin(angle * (3.14159 / 180));

				agl::Vec<float, 2> pos;
				pos.x = x * (150 - NETWORK_PADDING);
				pos.y = y * (150 - NETWORK_PADDING);

				pos.x += position.x + 150;
				pos.y += position.y + 150;

				circ->setSize({10, 10});

				circ->setPosition(pos);

				float nodeValue = (*networkFunc())->getNode(i).value;

				if (nodeValue > 0)
				{
					circ->setColor({0, (unsigned char)(nodeValue * 255), BASE_B_VALUE});
				}
				else
				{
					circ->setColor({(unsigned char)(-nodeValue * 255), 0, BASE_B_VALUE});
				}

				if ((pos - event->getPointerWindowPosition()).length() < 10)
				{
					selectedID = i;
				}

				window.drawShape(*circ);
			}

			rect->setRotation({0, 0, 0});
		}
};

class SimpleMenu : public agl::Drawable, public MenuShare
{
	public:
		bool			 exists	  = false;
		agl::Vec<int, 3> position = {0, 0};
		agl::Vec<int, 2> size	  = {0, 0};
		std::string		 title	  = "";

		void open(agl::Vec<int, 2> position)
		{
			exists		   = true;
			this->position = position;
		}

		void close()
		{
			exists = false;

			if (focusedMenu == this)
			{
				focusedMenu = nullptr;
			}
		}
};

// template hell
template <typename... ElementType> class Menu : public SimpleMenu
{
	public:
		std::tuple<ElementType...> element;

		std::function<bool()> requirement = []() { return true; };

		template <size_t i = 0> typename std::enable_if<i == sizeof...(ElementType), int>::type initTuple()
		{
			return 0;
		}

		template <size_t i = 0> typename std::enable_if < i<sizeof...(ElementType), int>::type initTuple()
		{
			this->get<i>().init(size.x - (2 * (MENU_BORDERTHICKNESS + MENU_PADDING)));

			int height = this->get<i>().size.y + MENU_PADDING;

			height += initTuple<i + 1>();

			return height;
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
			this->get<i>().init(size.x - (2 * (MENU_BORDERTHICKNESS + MENU_PADDING)));
		}

		template <size_t i = 0, typename Element, typename... Elements>
		void assign(Element newElement, Elements... newElements)
		{
			this->get<i>() = newElement;
			this->get<i>().init(size.x - (2 * (MENU_BORDERTHICKNESS + MENU_PADDING)));

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
			int64_t	 address = (int64_t)(pointerStruct);
			int64_t	 offset	 = i * 8;
			int64_t *item	 = (int64_t *)(address + offset);

			*item = (int64_t) & this->get<i>();

			pointerAssign<i + 1>(pointerStruct);
		}

		Menu(std::string title, int width, ElementType... args) : element(std::make_tuple(args...))
		{
			this->title	 = title;
			this->size.x = width;

			int height = initTuple();

			this->size.y = height + (2 * (MENU_BORDERTHICKNESS + MENU_PADDING));
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
			if (!exists)
			{
				return;
			}

			static agl::Vec<float, 2> offset;

			if (event->isPointerButtonPressed(agl::Button::Left))
			{
				if (pointInArea(event->getPointerWindowPosition(), position,
								agl::Vec<int, 2>{size.x, 4 + (int)smallText->getHeight()}) &&
					focusedMenu == nullptr)
				{
					offset = position - event->getPointerWindowPosition();

					focusedMenu = this;
				}
			}
			else if (focusedMenu == this)
			{
				focusedMenu = nullptr;
			}

			if (focusedMenu == this)
			{
				position = event->getPointerWindowPosition() + offset;
			}

			if (requirement())
			{
				agl::Vec<float, 3> pen = {MENU_BORDERTHICKNESS + MENU_PADDING,
										  MENU_BORDERTHICKNESS + MENU_PADDING + smallText->getHeight()};
				pen					   = pen + position;

				OuterArea outerArea;
				outerArea.position = position;
				outerArea.size	   = size + agl::Vec<float, 2>{0, (float)smallText->getHeight()};

				window.draw(outerArea);

				DipArea innerArea;
				innerArea.position = position + agl::Vec<float, 2>{4, 4 + (float)smallText->getHeight()};
				innerArea.size	   = size + agl::Vec<float, 2>{-8, -8};

				window.draw(innerArea);

				draw(window, pen);
			}
			else
			{
				OuterArea outerArea;
				outerArea.position = position;
				outerArea.size	   = {(float)size.x, (float)smallText->getHeight() + 6};

				window.draw(outerArea);
			}

			smallText->clearText();
			smallText->setText(title);
			smallText->setColor(agl::Color::Black);
			smallText->setPosition(position + agl::Vec<float, 2>{3.5, 0});
			window.drawText(*smallText);
		}
};
