#include "../inc/Menu.hpp"

void Menu::setup(agl::Vec<float, 3> position, agl::Vec<float, 2> size, agl::Texture *texture, agl::Font *font)
{
	this->position = position;
	this->size	   = size;

	outerShadowShape.setPosition(position);
	outerShadowShape.setOffset({0, 0, 0});
	outerShadowShape.setSize(size);
	outerShadowShape.setTexture(texture);
	outerShadowShape.setColor(MENU_SHADOWCOLOR);

	size.x -= MENU_SHADOWSIZE;
	size.y -= MENU_SHADOWSIZE;

	borderShape.setPosition(position);
	borderShape.setOffset({0, 0, 0.1});
	borderShape.setSize(size);
	borderShape.setTexture(texture);
	borderShape.setColor(MENU_BORDERCOLOR);

	size.x -= MENU_BORDER * 2;
	size.y -= MENU_BORDER * 2;

	innerShadowShape.setPosition(position);
	innerShadowShape.setOffset({MENU_BORDER, MENU_BORDER, 0.2});
	innerShadowShape.setSize(size);
	innerShadowShape.setTexture(texture);
	innerShadowShape.setColor(MENU_SHADOWCOLOR);

	size.x -= MENU_SHADOWSIZE;
	size.y -= MENU_SHADOWSIZE;

	bodyShape.setPosition(position);
	bodyShape.setOffset({MENU_BORDER + MENU_SHADOWSIZE, MENU_BORDER + MENU_SHADOWSIZE, 0.3});
	bodyShape.setSize(size);
	bodyShape.setTexture(texture);
	bodyShape.setColor(MENU_BODYCOLOR);

	agl::Vec<float, 3> textOffset = {MENU_BORDER + MENU_SHADOWSIZE + MENU_PADDING,
									 MENU_BORDER + MENU_SHADOWSIZE + MENU_PADDING, 0.4};

	text.setFont(font);
	text.setColor(agl::Color::Black);
	text.setPosition(position + textOffset);
	text.setText(" ");
	text.setScale(1);

	return;
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

void Menu::setText(std::string str)
{
	text.clearText();
	text.setText(str);
}

void Menu::destroy()
{
	text.clearText();
}

agl::Rectangle *Menu::getBorderShape()
{
	return &borderShape;
}

agl::Rectangle *Menu::getBodyShape()
{
	return &bodyShape;
}

agl::Rectangle *Menu::getOuterShadowShape()
{
	return &outerShadowShape;
}

agl::Rectangle *Menu::getInnerShadowShape()
{
	return &innerShadowShape;
}

agl::Text *Menu::getText()
{
	return &text;
}

agl::Vec<float, 3> Menu::getPosition()
{
	return position;
}

agl::Vec<float, 2> Menu::getSize()
{
	return size;
}
