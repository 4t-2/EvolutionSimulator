#include "Menu.hpp"

#define BARHEIGHT 22

class MenuBar : public agl::Drawable, public MenuShare
{
	private:
		template <int i, typename T> void assign(T menu)
		{
			this->menu[i] = menu;
		}

		template <int i, typename T, typename... Ts> void assign(T menu, Ts... menus)
		{
			this->menu[i] = menu;

			assign<i + 1>(menus...);
		}

	public:
		int			 length;
		SimpleMenu **menu;
		bool		 exists = true;

		template <typename... Ts> MenuBar(Ts... menus)
		{
			length = sizeof...(menus);

			this->menu = new SimpleMenu *[length];

			assign<0>(menus...);
		}

		void drawFunction(agl::RenderWindow &window) override
		{
			if (!exists)
			{
				return;
			}

			// draw child menus

			for (int i = 0; i < length; i++)
			{
				window.draw(*menu[i]);
			}

			// draw bar

			OuterArea background;

			background.position = {-2, -2};
			background.size		= {(float)window.getState().size.x + 4, BARHEIGHT + 4};

			window.draw(background);

			ThinAreaOut test;

			agl::Vec<float, 2> pen = {0, 0};

			for (int i = 0; i < length; i++)
			{
				agl::Vec<float, 2> oldPen = pen;

				text->clearText();
				text->setText(menu[i]->title);
				text->setPosition(oldPen + agl::Vec<float, 2>{9, 0});
				text->setColor(agl::Color::White);
				pen = window.drawText(*text) + text->getPosition();

				agl::Vec<float, 2> position = oldPen;
				agl::Vec<float, 2> size		= {(pen.x - oldPen.x) + 9, 20};

				if (pointInArea(event->getPointerWindowPosition(), position, size))
				{
					static bool prev = false;

					if (event->isPointerButtonPressed(agl::Button::Left))
					{
						if (!prev)
						{
							prev = true;

							if (!menu[i]->exists)
							{
								menu[i]->open({500, 500});
							}
							else
							{
								menu[i]->close();
							}
						}
					}
					else
					{
						prev = false;
					}

					if (!menu[i]->exists)
					{
						ThinAreaOut area;
						area.position = oldPen;
						area.size	  = {(pen.x - oldPen.x) + 9, 22};
						window.draw(area);
						window.drawText(*text);
					}
				}

				if (menu[i]->exists)
				{
					ThinAreaIn area;
					area.position = oldPen;
					area.size	  = {(pen.x - oldPen.x) + 9, 22};
					window.draw(area);
					window.drawText(*text);
				}

				pen.x += 9;
			}
		}

		~MenuBar()
		{
			delete[] menu;
		}
};
