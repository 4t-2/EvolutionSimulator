#include "../inc/Menu.hpp"

Menu::Menu(agl::Texture *texture, agl::Font *font, std::vector<MenuElement> element)
{
	this->menuElement = element;

	textElement.reserve(menuElement.size());

	for(int i = 0; i < menuElement.size(); i++)
	{
		textElement.emplace_back(text);
	}

	outerShadowShape.setTexture(texture);
	borderShape.setTexture(texture);
	innerShadowShape.setTexture(texture);
	bodyShape.setTexture(texture);
	
	text.setFont(font);
	text.setColor(agl::Color::Black);
	text.setScale(1);

}

void Menu::setup(agl::Vec<float, 3> position, agl::Vec<float, 2> size)
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

void Menu::setElement(int i, std::string str)
{
	textElement[i].str = str;
}

void Menu::setPosition(agl::Vec<float, 3> position)
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

void Menu::destroy()
{
	text.clearText();
}
