#include "../lib/AGL/agl.hpp"

int main()
{
	agl::RenderWindow window;
	window.setup({1000, 1000}, "EvolutionSimulator");
	window.setClearColor(agl::Color::Black);
	window.setFPS(60);

	agl::Event event;
	event.setWindow(window);

	agl::Shader shader;
	shader.loadFromFile("./vert.glsl", "./frag.glsl");
	window.getShaderUniforms(shader);
	shader.use();

	agl::Camera camera;
	camera.setOrthographicProjection(0, 1000, 1000, 0, 0.1, 100);
	camera.setView({0, 0, 1}, {0, 0, 0}, {0, 1, 0});

	agl::Texture blank;
	blank.setBlank();

	agl::Rectangle rectangle;
	rectangle.setTexture(&blank);
	rectangle.setSize({500, 500, 0});
	rectangle.setPosition({250, 250, 0});
	rectangle.setColor(agl::Color::Red);

	while(!event.windowClose())
	{
		window.updateMvp(camera);
		event.pollWindow();

		window.clear();

		window.drawShape(rectangle);

		window.display();
	}

	blank.deleteTexture();
	shader.deleteProgram();
	window.close();

	return 0;
}
