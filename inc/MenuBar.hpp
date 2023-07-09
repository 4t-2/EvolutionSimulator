#include"Menu.hpp"

#define BARHEIGHT 22

class MenuBar : public agl::Drawable, public MenuShare
{
	public:
		MenuBar()
		{
		}

		void drawFunction(agl::RenderWindow &window) override
		{
			OuterArea background;

			background.position = {-2, -2};
			background.size = {600 + 4, BARHEIGHT + 2};

			ThinAreaOut test;

			test.position = {0, 0};
			test.size = {100, 20};

			window.draw(background);
			// window.draw(test);
		}
};
