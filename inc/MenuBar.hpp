#include "Menu.hpp"

#define BARHEIGHT 22

class MenuBar : public agl::Drawable, public MenuShare
{
	public:
		int				 length;
		ToggleableMenu **menu;

		MenuBar(int length) : length(length)
		{
			menu = new ToggleableMenu *[length];
		}

		void drawFunction(agl::RenderWindow &window) override
		{
			OuterArea background;

			background.position = {-2, -2};
			background.size		= {(float)window.getWindowAttributes().width + 4, BARHEIGHT + 2};

			window.draw(background);

			ThinAreaOut test;

			agl::Vec<float, 2> pen = {0, 0};

			for (int i = 0; i < length; i++)
			{
				agl::Vec<float, 2> oldPen = pen;

				text->clearText();
				text->setText(menu[i]->title);
				text->setPosition(oldPen + agl::Vec<float, 2>{9, 0});
				text->setColor(agl::Color::Black);
				pen = window.drawText(*text) + text->getPosition();

				agl::Vec<float, 2> position = oldPen;
				agl::Vec<float, 2> size		= {(pen.x - oldPen.x) + 9, 20};

				if (pointInArea(event->getPointerWindowPosition(), position, size))
				{
					static bool prev = false;

					if (event->isPointerButtonPressed(Button1Mask))
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
						area.size	  = {(pen.x - oldPen.x) + 9, 20};
						window.draw(area);
						window.drawText(*text);
					}
				}

				if (menu[i]->exists)
				{
					ThinAreaIn area;
					area.position = oldPen;
					area.size	  = {(pen.x - oldPen.x) + 9, 20};
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
