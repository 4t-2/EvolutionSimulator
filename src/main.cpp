#include <AGL/agl.hpp>

#include "../inc/MenuBar.hpp"
#include "../inc/Simulation.hpp"

#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <math.h>
#include <string>
#include <thread>

class Listener
{
	private:
		std::function<void()> first;
		std::function<void()> hold;
		std::function<void()> last;
		bool				  pastState = false;

	public:
		Listener(std::function<void()> first, std::function<void()> hold, std::function<void()> last);
		void update(bool state);
};

Listener::Listener(std::function<void()> first, std::function<void()> hold, std::function<void()> last)
{
	this->first = first;
	this->hold	= hold;
	this->last	= last;
}

void Listener::update(bool state)
{
	if (state)
	{
		if (pastState)
		{
			hold();
		}
		else
		{
			first();

			pastState = true;
		}
	}
	else if (pastState)
	{
		last();
		pastState = false;
	}
}

int getMillisecond()
{
	auto timepoint = std::chrono::system_clock::now().time_since_epoch();
	return std::chrono::duration_cast<std::chrono::milliseconds>(timepoint).count();
}

agl::Vec<float, 2> getCursorScenePosition(agl::Vec<float, 2> cursorWinPos, agl::Vec<float, 2> winSize, float winScale,
										  agl::Vec<float, 2> cameraPos)
{
	return ((cursorWinPos - (winSize * .5)) * winScale) + cameraPos;
}

template <typename T> bool contains(std::list<BaseEntity *> &list, T *p)
{
	return std::find_if(list.begin(), list.end(), [&](BaseEntity *&d) { return d == (BaseEntity *)(DoNotUse *)p; }) !=
		   list.end();
}

int main()
{
	printf("Starting AGL\n");

	agl::RenderWindow window;
	window.setup({WIDTH, HEIGHT}, "EvolutionSimulator");
	window.setClearColor(CLEARCOLOR);
	window.setFPS(0);

	agl::Vec<float, 2> windowSize;

	glDisable(GL_DEPTH_TEST);

	// window.GLEnable(GL_ALPHA_TEST);
	// glAlphaFunc(GL_GREATER, 0.1f);

	window.setSwapInterval(1);

	agl::Event event;
	event.setWindow(window);

	agl::Shader simpleShader;
	simpleShader.loadFromFile("./shader/vert.glsl", "./shader/frag.glsl");

	agl::Shader gridShader;
	gridShader.loadFromFile("./shader/gridvert.glsl", "./shader/grid.glsl");

	agl::Shader viteShader;
	viteShader.loadFromFile("./shader/viteVert.glsl", "./shader/vite.glsl");

	agl::Shader menuShader;
	menuShader.loadFromFile("./shader/menuVert.glsl", "./shader/menu.glsl");

	agl::Camera camera;
	camera.setOrthographicProjection(0, WIDTH, HEIGHT, 0, 0.1, 100);
	agl::Vec<float, 2> cameraPosition = {0, 0};
	camera.setView({cameraPosition.x, cameraPosition.y, 50}, {0, 0, 0}, {0, 1, 0});

	agl::Camera guiCamera;
	guiCamera.setOrthographicProjection(0, WIDTH, HEIGHT, 0, 0.1, 100);
	guiCamera.setView({0, 0, 50}, {0, 0, 0}, {0, 1, 0});

	agl::Texture foodTexture;
	foodTexture.loadFromFile("./img/food.png");

	agl::Texture creatureBodyTexture;
	creatureBodyTexture.loadFromFile("./img/creatureBody.png");

	agl::Texture creatureExtraTexture;
	creatureExtraTexture.loadFromFile("./img/creatureExtra.png");

	agl::Texture eggTexture;
	eggTexture.loadFromFile("./img/egg.png");

	agl::Texture meatTexture;
	meatTexture.loadFromFile("./img/meat.png");

	agl::Texture blank;
	blank.setBlank();

	agl::Font font;
	font.setup("./font/font.ttf", 16);

	agl::Font smallFont;
	smallFont.setup("./font/font.ttf", 12);

	agl::Rectangle background;
	background.setTexture(&blank);
	background.setColor(CLEARCOLOR);
	background.setPosition({0, 0, 0});

	agl::Rectangle foodShape;
	foodShape.setTexture(&foodTexture);
	foodShape.setColor(agl::Color::Green);
	foodShape.setSize(agl::Vec<float, 3>{-10, -10, 0});
	foodShape.setOffset({5, 5, 30});

	agl::Rectangle creatureShape;
	creatureShape.setTexture(&creatureBodyTexture);
	creatureShape.setColor(agl::Color::White);
	creatureShape.setSize(agl::Vec<float, 3>{25, 60, 0});
	creatureShape.setOffset(agl::Vec<float, 3>{-12.5, -12.5, 30});

	agl::Rectangle eggShape;
	eggShape.setTexture(&eggTexture);
	eggShape.setColor(agl::Color::White);
	eggShape.setSize(agl::Vec<float, 3>{15, 15, 0});
	eggShape.setOffset({7.5, 7.5, 30});

	agl::Rectangle meatShape;
	meatShape.setTexture(&meatTexture);
	meatShape.setColor(agl::Color::White);
	meatShape.setSize(agl::Vec<float, 3>{-10, -10, 0});
	meatShape.setOffset({5, 5, 30});

	agl::Rectangle rayShape;
	rayShape.setTexture(&blank);
	rayShape.setColor(agl::Color::White);
	rayShape.setSize(agl::Vec<float, 3>{1, RAY_LENGTH});
	rayShape.setOffset(agl::Vec<float, 3>{-0.5, 0, 0});

	agl::Rectangle blankRect;
	blankRect.setTexture(&blank);

	printf("loading simulation rules from sim.conf\n");

	SimulationRules simulationRules;

	{
		std::fstream fs("./conf/sim.conf", std::ios::in);
		recurse(Input(fs), simulationRules, "simulationRules");
		recurse(Output(std::cout), simulationRules, "simulationRules");
	}

	background.setSize(simulationRules.size);

	printf("starting sim\n");

	// background.setPosition(simulationRules.size * .5);

	Simulation simulation{};
  simulation.active = 0;
	// simulation.create(simulationRules, 0);

	Creature *focusCreature = nullptr;

	float		fps = 0;
	std::string nodeName;

	bool leftClick;
	bool previousLeftClick;
  (void) previousLeftClick;

	Listener leftClickListener([&]() { leftClick = true; }, [&]() { leftClick = false; }, [&]() { leftClick = false; });

	// menu shit

	MenuShare::init(&blank, &font, &smallFont, &event, &leftClick);
	MenuShare::menuShader = &menuShader;
	MenuShare::baseShader = &simpleShader;
	MenuShare::camera	  = &guiCamera;

	// simulation info

	struct
	{
			ValueElement<int>	*creatures;
			ValueElement<int>	*eggs;
			ValueElement<int>	*food;
			ValueElement<int>	*meat;
			ValueElement<int>	*frame;
			ValueElement<float> *fps;
	} simulationInfoPointers;

	struct
	{
			int creatures;
			int eggs;
			int food;
			int meat;
	} statsForSimInfo;

	Menu simulationInfo("SimInfo", 125, //
						ValueElement<int>{"Creatures",
										  [&]() {
											  statsForSimInfo.creatures = simulation.env.getList<Creature>().size();
											  return &statsForSimInfo.creatures;
										  }}, //
						ValueElement<int>{"Eggs",
										  [&]() {
											  statsForSimInfo.eggs = simulation.env.getList<Egg>().size();
											  return &statsForSimInfo.eggs;
										  }}, //
						ValueElement<int>{"Food",
										  [&]() {
											  statsForSimInfo.food = simulation.env.getList<Food>().size();
											  return &statsForSimInfo.food;
										  }}, //
						ValueElement<int>{"Meat",
										  [&]() {
											  statsForSimInfo.meat = simulation.env.getList<Meat>().size();
											  return &statsForSimInfo.meat;
										  }},											 //
						ValueElement<int>{"Frame", [&]() { return &simulation.frame; }}, //
						ValueElement<float>{"FPS", [&]() { return &fps; }}				 //
	);

	simulationInfo.bindPointers(&simulationInfoPointers);

	simulationInfo.requirement = [&]() { return simulation.active; };

	// creatureNetwork

	struct
	{
			NetworkGraph			  *network;
			ValueElement<std::string> *node;
	} creatureNetworkPointers;

	Menu creatureNetwork("CreatureNetwork", 325,										//
						 NetworkGraph{[&]() { return &focusCreature->network; }},		//
						 ValueElement<std::string>{"Node", [&]() { return &nodeName; }} //
	);

	creatureNetwork.bindPointers(&creatureNetworkPointers);

	creatureNetwork.requirement = [&]() {
		if (focusCreature != nullptr)
		{
			return true;
		}
		else
		{
			return false;
		}
	};

	// creatureVectors

	struct
	{
			TextElement			*posText;
			ValueElement<float> *posX;
			ValueElement<float> *posY;
			TextElement			*velText;
			ValueElement<float> *velX;
			ValueElement<float> *velY;
			TextElement			*forText;
			ValueElement<float> *forX;
			ValueElement<float> *fooY;

	} creatureVectorsPointers;

	Menu creatureVectors("CreatureVectors", 200,												 //
						 TextElement{"- Position -"},											 //
						 ValueElement<float>{"X", [&]() { return &focusCreature->position.x; }}, //
						 ValueElement<float>{"Y", [&]() { return &focusCreature->position.y; }}, //
						 TextElement{"- Velocity -"},											 //
						 ValueElement<float>{"X", [&]() { return &focusCreature->velocity.x; }}, //
						 ValueElement<float>{"Y", [&]() { return &focusCreature->velocity.y; }}, //
						 TextElement{"- Force -"},												 //
						 ValueElement<float>{"X", [&]() { return &focusCreature->force.x; }},	 //
						 ValueElement<float>{"Y", [&]() { return &focusCreature->force.y; }}	 //
	);

	creatureVectors.bindPointers(&creatureVectorsPointers);

	creatureVectors.requirement = creatureNetwork.requirement;

	// creatureMisc

	struct
	{
			ValueElement<bool>	*eating;
			ValueElement<bool>	*layingEgg;
			SpacerElement		*sp1;
			ValueElement<float> *health;
			ValueElement<float> *energy;
			ValueElement<float> *lifeLeft;
			SpacerElement		*sp2;
			ValueElement<int>	*hue;
			SpacerElement		*sp3;
			ValueElement<float> *biomass;
			ValueElement<float> *energyDensity;
	} creatureMiscPointers;

	Menu creatureMisc("CreatureMisc", 200,																	 //
					  ValueElement<bool>{"Eating", [&]() { return &focusCreature->eating; }},				 //
					  ValueElement<bool>{"Laying Egg", [&]() { return &focusCreature->layingEgg; }},		 //
					  SpacerElement{},																		 //
					  ValueElement<float>{"Health", [&]() { return &focusCreature->health; }},				 //
					  ValueElement<float>{"Energy", [&]() { return &focusCreature->energy; }},				 //
					  ValueElement<int>{"Life Left", [&]() { return &focusCreature->life; }},				 //
					  SpacerElement{},																		 //
					  ValueElement<int>{"Hue", [&]() { return &focusCreature->creatureData.hue; }},			 //
					  SpacerElement{},																		 //
					  ValueElement<float>{"Biomass", [&]() { return &focusCreature->biomass; }},			 //
					  ValueElement<float>{"Energy Density", [&]() { return &focusCreature->energyDensity; }} //
	);

	creatureMisc.bindPointers(&creatureMiscPointers);

	creatureMisc.requirement = creatureVectors.requirement;

	// leftMenu

	struct
	{
			ButtonElement<Toggle> *food;
			ButtonElement<Toggle> *meat;
			ButtonElement<Toggle> *select;
			ButtonElement<Toggle> *kill;
			SpacerElement		  *sp1;
			ButtonElement<Toggle> *forceFood;
			ButtonElement<Toggle> *forceMeat;
			ButtonElement<Toggle> *forceCreature;
			FieldElement<float>	  *forceMultiplier;
			SpacerElement		  *sp2;
			ButtonElement<Toggle> *sendTo;
	} leftMenuPointers;

	Menu leftMenu("LeftMenu", 200,						  //
				  ButtonElement<Toggle>{"Food"},		  //
				  ButtonElement<Toggle>{"Meat"},		  //
				  ButtonElement<Toggle>{"Select"},		  //
				  ButtonElement<Toggle>{"Kill"},		  //
				  SpacerElement{},						  //
				  ButtonElement<Toggle>{"ForceFood"},	  //
				  ButtonElement<Toggle>{"ForceMeat"},	  //
				  ButtonElement<Toggle>{"ForceCreature"}, //
				  FieldElement<float>{"ForceMult", 1.0},  //
				  SpacerElement{},						  //
				  ButtonElement<Toggle>{"SendTo"}		  //
	);

	leftMenu.bindPointers(&leftMenuPointers);

	// simRules

	struct
	{
			FieldElement<int>	*maxFood;
			FieldElement<float> *energyCostMultiplier;
			FieldElement<float> *learnRate;
			FieldElement<int>	*brainMutation;
			FieldElement<int>	*bodyMutation;
			FieldElement<float> *exploration;
			FieldElement<float> *vaporize;
	} simRulesPointers;

	simulation.foodCap = simulationRules.foodCap;

	Menu simRules("SimRules", 200,													//
				  FieldElement<int>{"maxFd", (simulation.foodCap)},					//
				  FieldElement<float>{"EnCoMu", (simulation.energyCostMultiplier)}, //
				  FieldElement<float>{"Lrate", (simulationRules.learningRate)},		//
				  FieldElement<int>{"braMut", (simulationRules.brainMutation)},		//
				  FieldElement<int>{"bodMut", (simulationRules.bodyMutation)},		//
				  FieldElement<float>{"xplor", (simulationRules.exploration)},		//
				  FieldElement<float>{"vapor", (simulationRules.vaporize)}			//
	);

	simRules.bindPointers(&simRulesPointers);

	// quitmenu

	struct
	{
			TextElement			*text;
			ButtonElement<Hold> *confirm;
			ButtonElement<Hold> *cancel;
	} quitMenuPointers;

	Menu quitMenu("Quit", 100,					  //
				  TextElement{"Quit"},			  //
				  ButtonElement<Hold>{"Confirm"}, //
				  ButtonElement<Hold>{"Cancel"}	  //
	);
	quitMenu.bindPointers(&quitMenuPointers);

	// simMenu

	struct
	{
			FieldElement<int>	  *sizeX;
			FieldElement<int>	  *sizeY;
			FieldElement<int>	  *gridX;
			FieldElement<int>	  *gridY;
			FieldElement<int>	  *threads;
			FieldElement<int>	  *memory;
			FieldElement<int>	  *startingCreatures;
			FieldElement<int>	  *seed;
			FieldElement<float>	  *simCycles;
			ButtonElement<Hold>	  *start;
			ButtonElement<Hold>	  *kill;
			ButtonElement<Toggle> *pause;
	} simMenuPointers;

	Menu simMenu("simMenu", 175,														  //
				 FieldElement<int>{"sizeX", (simulationRules.size.x)},					  //
				 FieldElement<int>{"sizeY", (simulationRules.size.y)},					  //
				 FieldElement<int>{"gridX", (simulationRules.gridResolution.x)},		  //
				 FieldElement<int>{"gridY", (simulationRules.gridResolution.y)},		  //
				 FieldElement<int>{"Thread", (simulationRules.threads)},				  //
				 FieldElement<int>{"CreMem", (simulationRules.memory)},					  //
				 FieldElement<int>{"startCreature", (simulationRules.startingCreatures)}, //
				 FieldElement<int>{"seed", 0},											  //
				 FieldElement<float>{"simCycles", 1.0},									  //
				 ButtonElement<Hold>{"START"},											  //
				 ButtonElement<Hold>{"KILL"},											  //
				 ButtonElement<Toggle>{"PAUSE"}											  //
	);

	simMenu.bindPointers(&simMenuPointers);

	// debugLog

	struct
	{
			TextElement *t1;
			TextElement *t2;
			TextElement *t3;
			TextElement *t4;
			TextElement *t5;
			TextElement *t6;
			TextElement *t7;
			TextElement *t8;
	} debugLogPointers;

	Menu debugLog("DebugLog", 400, TextElement{""}, TextElement{""}, TextElement{""}, TextElement{""}, TextElement{""},
				  TextElement{""}, TextElement{""}, TextElement{""});

	debugLog.bindPointers(&debugLogPointers);

	auto sendDebugLog = [&](std::string str) {
		debugLogPointers.t1->str = debugLogPointers.t2->str;
		debugLogPointers.t2->str = debugLogPointers.t3->str;
		debugLogPointers.t3->str = debugLogPointers.t4->str;
		debugLogPointers.t4->str = debugLogPointers.t5->str;
		debugLogPointers.t5->str = debugLogPointers.t6->str;
		debugLogPointers.t6->str = debugLogPointers.t7->str;
		debugLogPointers.t7->str = debugLogPointers.t8->str;

		debugLogPointers.t8->str = str;
	};
  (void)sendDebugLog;
	// menubar

	MenuBar menuBar(&quitMenu,		  //
					&simulationInfo,  //
					&simMenu,		  //
					&leftMenu,		  //
					&simRules,		  //
					&creatureNetwork, //
					&creatureVectors, //
					&creatureMisc,	  //
					&debugLog		  //
	);

	bool mHeld		= false;
	bool b1Held		= false;
	bool ReturnHeld = false;
  (void) mHeld;
  (void) ReturnHeld;

	bool skipRender = false;

	float sizeMultiplier = 1;

	printf("entering sim loop\n");

	bool quiting = false;
  (void) quiting;

	while (!event.windowClose())
	{
		static int milliDiff = 0;
		int		   start	 = getMillisecond();

		event.poll();

		if (!simMenuPointers.pause->state && simulation.active)
		{
			static float cycle = 0;

			cycle += simMenuPointers.simCycles->value;

			for (int i = 0; i < cycle; i++)
			{
				cycle--;
				simulation.update();
			}
		}

		agl::Vec<int, 2> topLeftGrid;
		agl::Vec<int, 2> bottomRightGrid;

		if (skipRender)
		{
			goto skipRendering;
		}

		window.clear();

		// Simulation rendering

		window.getShaderUniforms(gridShader);
		gridShader.use();

		window.updateMvp(camera);

		glUniform1f(gridShader.getUniformLocation("scale"), sizeMultiplier);

		window.drawShape(background);

		window.getShaderUniforms(simpleShader);
		simpleShader.use();

		if (!simulation.active)
		{
			goto skipSimRender;
		}

		window.updateMvp(camera);

		{
			agl::Vec<float, 2> tlPos = getCursorScenePosition({0, 0}, windowSize, sizeMultiplier, cameraPosition);
			topLeftGrid				 = simulation.env.toGridPosition(tlPos);

			agl::Vec<float, 2> brPos = getCursorScenePosition(windowSize, windowSize, sizeMultiplier, cameraPosition);
			bottomRightGrid			 = simulation.env.toGridPosition(brPos);
		}

		// Draw food
		simulation.env.view<Food>(
			[&](auto &food, auto) {
				agl::Vec<float, 2> position = food.position;
				foodShape.setPosition(position);
				foodShape.setOffset(food.size / -2);
				foodShape.setSize(food.size);
				foodShape.setRotation({0, 0, -food.radToDeg()});
				window.drawShape(foodShape);
			},
			topLeftGrid, bottomRightGrid);

		simulation.env.view<Meat>(
			[&](auto &meat, auto) {
				meatShape.setPosition(meat.position);
				meatShape.setSize(meat.size);
				meatShape.setOffset(meat.size / -2);
				meatShape.setRotation({0, 0, -meat.radToDeg()});
				window.drawShape(meatShape);
			},
			topLeftGrid, bottomRightGrid);

		// draw eggs
		simulation.env.view<Egg>(
			[&](Egg &egg, auto) {
				eggShape.setPosition(egg.position);
				window.drawShape(eggShape);
			},
			topLeftGrid, bottomRightGrid);

		window.getShaderUniforms(viteShader);
		viteShader.use();

		window.updateMvp(camera);

		// draw creature
		simulation.env.view<Creature>(
			[&](Creature &obj, auto) {
				for (auto seg : obj.segments)
				{
					glUniform1f(viteShader.getUniformLocation("scaleX"), seg->size.x);
					glUniform1f(viteShader.getUniformLocation("scaleY"), seg->size.y);

					blankRect.setPosition(seg->position);
					blankRect.setOffset(seg->size * -.5);
					blankRect.setRotation({0, 0, -seg->radToDeg()});
					blankRect.setSize(seg->size);

					agl::Color c =
						&obj == focusCreature ? hueToRGB(obj.creatureData.hue + 20) : hueToRGB(obj.creatureData.hue);

					blankRect.setColor(c);

					window.drawShape(blankRect);
				}
			},
			topLeftGrid, bottomRightGrid);

	skipSimRender:;

		window.getShaderUniforms(simpleShader);
		simpleShader.use();

		// gui rendering

		fps = (1000. / milliDiff);

		window.updateMvp(guiCamera);

		if (focusCreature != nullptr)
		{
			if (contains(simulation.env.getList<Creature>(), focusCreature))
			{
				int node = creatureNetworkPointers.network->selectedID;

				nodeName = "";

				if (node < focusCreature->network->structure.totalInputNodes)
				{
					switch (node)
					{
						case 0:
							nodeName = "Constant";
							break;
						case 1:
							nodeName = "Sin";
							break;
						default:
							if (node % 2 == 0)
							{
								nodeName = "Angle " + std::to_string(node - 1);
							}
							else
							{
								nodeName = "Motor " + std::to_string(node - 2);
							}
							break;
					}
				}
				else
				{
					if (node < focusCreature->network->structure.totalNodes -
								   focusCreature->network->structure.totalOutputNodes)
					{
						nodeName =
							"Hidden " + std::to_string(node - focusCreature->network->structure.totalInputNodes + 1);
					}
					else
					{
						nodeName =
							"Move " + std::to_string(((node - focusCreature->network->structure.totalInputNodes) -
													  focusCreature->network->structure.totalHiddenNodes) +
													 1);
					}
				}

				nodeName +=
					": " +
					std::to_string(focusCreature->network->getNode(creatureNetworkPointers.network->selectedID).value);
			}
			else
			{
				focusCreature = nullptr;
			}
		}

		for (std::string &msgLog : Debug::log)
		{
			debugLog.get<0>().str = debugLog.get<1>().str;
			debugLog.get<1>().str = debugLog.get<2>().str;
			debugLog.get<2>().str = debugLog.get<3>().str;
			debugLog.get<3>().str = debugLog.get<4>().str;
			debugLog.get<4>().str = debugLog.get<5>().str;
			debugLog.get<5>().str = debugLog.get<6>().str;
			debugLog.get<6>().str = debugLog.get<7>().str;

			debugLog.get<7>().str = msgLog;
		}

		Debug::log.clear();

		// window.getShaderUniforms(menuShader);
		// menuShader.use();
		//
		// window.updateMvp(guiCamera);

		window.draw(menuBar);

		window.display();

	skipRendering:;

		// quit?

		if (quitMenuPointers.confirm->state)
		{
			break;
		}
		if (quitMenuPointers.cancel->state)
		{
			quitMenu.close();
			quitMenuPointers.cancel->state = false;
		}

		if (simMenuPointers.kill->state && simulation.active)
		{
			simulation.destroy();
			focusCreature = nullptr;
		}
		else if (simMenuPointers.start->state && !simulation.active)
		{
			simulationRules.size.x			  = simMenuPointers.sizeX->value;
			simulationRules.size.y			  = simMenuPointers.sizeY->value;
			simulationRules.gridResolution.x  = simMenuPointers.gridX->value;
			simulationRules.gridResolution.y  = simMenuPointers.gridY->value;
			simulationRules.startingCreatures = simMenuPointers.startingCreatures->value;
			simulationRules.threads			  = simMenuPointers.threads->value;
			simulationRules.foodCap			  = simRulesPointers.maxFood->value;

			simulation.create(simulationRules, simMenuPointers.seed->value);

			background.setSize(simulationRules.size);
		}

		if (event.keybuffer.find('h') != std::string::npos && FocusableElement::focusedField == nullptr)
		{
			menuBar.exists = !menuBar.exists;
		}

		if (!simulation.active)
		{
			goto deadSim;
		}

		// input

		if (event.isKeyPressed(agl::Key::R))
		{
			focusCreature = nullptr;
		}

		{
		}

		if (event.isPointerButtonPressed(agl::Button::Left))
		{
			if (menuBar.exists)
			{
				for (int i = 0; i < menuBar.length; i++)
				{
					SimpleMenu *menu = menuBar.menu[i];

					if (pointInArea(event.getPointerWindowPosition(), menu->position, menu->size) && menu->exists)
					{
						goto endif;
					}
				}
			}

			if (leftMenuPointers.food->state) // add food
			{
				simulation.addFood(getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier,
														  cameraPosition));
			}
			if (leftMenuPointers.meat->state) // add meat
			{
				simulation.addMeat(getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier,
														  cameraPosition));
			}
			if (leftMenuPointers.select->state) // select creature
			{
				simulation.env.view<Creature>([&](auto &creature, auto it) {
					agl::Vec<float, 2> mouse;
					mouse.x = ((event.getPointerWindowPosition().x - (windowSize.x * .5)) * sizeMultiplier) +
							  cameraPosition.x;
					mouse.y = ((event.getPointerWindowPosition().y - (windowSize.y * .5)) * sizeMultiplier) +
							  cameraPosition.y;

					float distance = (mouse - creature.position).length();

					if (distance < creature.size.x)
					{
						focusCreature			= &creature;
						simulation.env.selected = focusCreature;

						return;
					}
				});
			}
			if (leftMenuPointers.kill->state) // kill creature
			{
				// simulation.env.view<Creature>([&](auto &creature, auto
				// it) { 	agl::Vec<float, 2> mouse; 	mouse.x =
				// ((event.getPointerWindowPosition().x - (windowSize.x *
				// .5)) * sizeMultiplier) + 			  cameraPosition.x;
				// mouse.y
				// =
				// ((event.getPointerWindowPosition().y - (windowSize.y *
				// .5)) * sizeMultiplier) +
				// cameraPosition.y;
				//
				// 	float distance = (mouse - creature.position).length();
				//
				// 	if (distance < creature.radius)
				// 	{
				// 		it--;
				// 		simulation.addMeat(creature.position,
				// creature.maxHealth / 4);
				// simulation.env.removeEntity<Creature>(it);
				//
				// 		return;
				// 	}
				// });
			}
			if (leftMenuPointers.forceFood->state)
			{
				agl::Vec<int, 2> cursorRelPos = getCursorScenePosition(event.getPointerWindowPosition(), windowSize,
																	   sizeMultiplier, cameraPosition);

				topLeftGrid = simulation.env.toGridPosition(cursorRelPos - agl::Vec<float, 2>{1000, 1000});

				bottomRightGrid = simulation.env.toGridPosition(cursorRelPos + agl::Vec<float, 2>{1000, 1000});

				simulation.env.view<Food>(
					[&](Food &food, auto) {
						agl::Vec<float, 2> offset	= food.position - cursorRelPos;
						float			   distance = offset.length();

						float forceScalar = leftMenuPointers.forceMultiplier->value / distance;

						agl::Vec<float, 2> force = offset.normalized() * forceScalar;

						food.ApplyForceToCenter(force);
					},
					topLeftGrid, bottomRightGrid);
				{
				}
			}
			if (leftMenuPointers.forceMeat->state)
			{
				agl::Vec<int, 2> cursorRelPos = getCursorScenePosition(event.getPointerWindowPosition(), windowSize,
																	   sizeMultiplier, cameraPosition);
        (void) cursorRelPos;

				// simulation.env.getArea<Meat>(
				// 	[&](Meat &meat) {
				// 		agl::Vec<float, 2> offset	= meat.position
				// - cursorRelPos; 		float distance =
				// offset.length();
				//
				// 		float forceScalar =
				// leftMenuPointers.forceMultiplier->value / distance;
				//
				// 		agl::Vec<float, 2> force = offset.normalized() *
				// forceScalar;
				//
				// 		meat.force += force;
				// 	},
				// 	simulation.env.toGridPosition(cursorRelPos));
			}
			if (leftMenuPointers.forceCreature->state)
			{
				agl::Vec<int, 2> cursorRelPos = getCursorScenePosition(event.getPointerWindowPosition(), windowSize,
																	   sizeMultiplier, cameraPosition);
        (void) cursorRelPos;
				// NOTE force is disabled
				// simulation.env.getArea<Creature>(
				// 	[&](Creature &creature) {
				// 		agl::Vec<float, 2> offset	= creature.position
				// - cursorRelPos; 		float distance =
				// offset.length();
				//
				// 		float forceScalar =
				// leftMenuPointers.forceMultiplier->value / distance;
				//
				// 		agl::Vec<float, 2> force = offset.normalized() *
				// forceScalar;
				//
				// 		creature.force += force;
				//
				// 		sendDebugLog(std::to_string(force.length()));
				// 	},
				// 	simulation.env.toGridPosition(cursorRelPos));
			}
			if (leftMenuPointers.sendTo->state)
			{
				if (focusCreature != nullptr)
				{
					focusCreature->position = getCursorScenePosition(event.getPointerWindowPosition(), windowSize,
																	 sizeMultiplier, cameraPosition);
				}
			}

		endif:;
		}

		simulation.foodCap						 = simRulesPointers.maxFood->value;
		simulation.energyCostMultiplier			 = simRulesPointers.energyCostMultiplier->value;
		simulation.simulationRules.learningRate	 = simRulesPointers.learnRate->value;
		simulation.simulationRules.brainMutation = simRulesPointers.brainMutation->value;
		simulation.simulationRules.bodyMutation	 = simRulesPointers.bodyMutation->value;
		simulation.simulationRules.exploration	 = simRulesPointers.exploration->value;
		simulation.simulationRules.vaporize		 = simRulesPointers.vaporize->value;

	deadSim:;

		// camera movement

		static agl::Vec<float, 2> cameraOffset;
		static agl::Vec<float, 2> startPos;

		if (event.isPointerButtonPressed(agl::Button::Middle))
		{
			if (b1Held) // holding click
			{
				cameraPosition = cameraPosition - cameraOffset;

				cameraOffset = startPos - event.getPointerWindowPosition();
				cameraOffset.x *= sizeMultiplier;
				cameraOffset.y *= sizeMultiplier;

				cameraPosition.x += cameraOffset.x;
				cameraPosition.y += cameraOffset.y;
			}
			else // first click
			{
				window.setCursorShape(agl::CursorType::Arrow);
				startPos = event.getPointerWindowPosition();
				b1Held	 = true;
			}
		}
		else if (b1Held) // let go
		{
			window.setCursorShape(agl::CursorType::Arrow);
			cameraOffset = {0, 0};
			b1Held		 = false;
		}

		static float cameraSpeed = 4;
    (void) cameraSpeed;

		const float sizeDelta = .2;

		if (event.scroll == agl::Up)
		{
			float scale = sizeDelta * sizeMultiplier;

			agl::Vec<float, 2> oldPos =
				getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier, cameraPosition);

			sizeMultiplier -= scale;

			agl::Vec<float, 2> newPos =
				getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier, cameraPosition);

			agl::Vec<float, 2> offset = oldPos - newPos;

			cameraPosition = cameraPosition + offset;
		}
		if (event.scroll == agl::Down)
		{
			float scale = sizeDelta * sizeMultiplier;

			agl::Vec<float, 2> oldPos =
				getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier, cameraPosition);

			sizeMultiplier += scale;

			agl::Vec<float, 2> newPos =
				getCursorScenePosition(event.getPointerWindowPosition(), windowSize, sizeMultiplier, cameraPosition);

			agl::Vec<float, 2> offset = oldPos - newPos;

			cameraPosition = cameraPosition + offset;
		}

		window.setViewport(0, 0, windowSize.x, windowSize.y);

		camera.setOrthographicProjection(-((windowSize.x / 2.) * sizeMultiplier),
										 ((windowSize.x / 2.) * sizeMultiplier), ((windowSize.y / 2.) * sizeMultiplier),
										 -((windowSize.y / 2.) * sizeMultiplier), 0.1, 100);
		camera.setView({cameraPosition.x, cameraPosition.y, 50}, {cameraPosition.x, cameraPosition.y, 0}, {0, 1, 0});

		guiCamera.setOrthographicProjection(0, windowSize.x, windowSize.y, 0, 0.1, 100);

		windowSize = window.getState().size;

		leftClickListener.update(event.isPointerButtonPressed(agl::Button::Left));

		milliDiff = getMillisecond() - start;

		// std::cout << simulation.foodCap << '\n';
	}

	if (simulation.active)
	{
		simulation.destroy();
	}

	MenuShare::destroy();

	font.deleteFont();

	foodTexture.deleteTexture();
	creatureBodyTexture.deleteTexture();
	creatureExtraTexture.deleteTexture();
	eggTexture.deleteTexture();
	blank.deleteTexture();

	simpleShader.deleteProgram();
	gridShader.deleteProgram();

	window.close();

	return 0;
}
