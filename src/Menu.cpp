#include "../inc/Menu.hpp"

void Menu::setup(agl::Vec<float, 3> position, agl::Vec<float, 2> size, agl::Texture *texture, agl::Font *font)
{
	this->position = position;
	this->size	   = size;

	outerShadowShape.setPosition(position);
	outerShadowShape.setSize(size);
	outerShadowShape.setTexture(texture);
	outerShadowShape.setColor(MENU_SHADOWCOLOR);

	position.z += .1;

	size.x -= MENU_SHADOWSIZE;
	size.y -= MENU_SHADOWSIZE;

	borderShape.setPosition(position);
	borderShape.setSize(size);
	borderShape.setTexture(texture);
	borderShape.setColor(MENU_BORDERCOLOR);

	position.x += MENU_BORDER;
	position.y += MENU_BORDER;
	position.z += .1;

	size.x -= MENU_BORDER * 2;
	size.y -= MENU_BORDER * 2;

	innerShadowShape.setPosition(position);
	innerShadowShape.setSize(size);
	innerShadowShape.setTexture(texture);
	innerShadowShape.setColor(MENU_SHADOWCOLOR);

	position.x += MENU_SHADOWSIZE;
	position.y += MENU_SHADOWSIZE;
	position.z += .1;

	size.x -= MENU_SHADOWSIZE;
	size.y -= MENU_SHADOWSIZE;

	bodyShape.setPosition(position);
	bodyShape.setSize(size);
	bodyShape.setTexture(texture);
	bodyShape.setColor(MENU_BODYCOLOR);

	position.x += MENU_PADDING;
	position.y += MENU_PADDING;
	position.z += 1;

	text.setFont(font);
	text.setColor(agl::Color::Black);
	text.setPosition(position);
	text.setText(" ");
	text.setScale(1);

	return;
}

void Menu::setText(std::string str)
{
	text.clearText();
	text.setText(str);
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
