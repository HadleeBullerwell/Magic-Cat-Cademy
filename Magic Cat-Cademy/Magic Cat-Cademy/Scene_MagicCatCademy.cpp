#include "Scene_MagicCatCademy.h"
#include "Components.h"
#include "Entity.h"
#include <fstream>
#include <iostream>

Scene_MagicCatCademy::Scene_MagicCatCademy(GameEngine* gameEngine, const std::string& levelPath) : 
	Scene(gameEngine), 
	m_worldView(gameEngine->window().getDefaultView()) {
	loadLevel(levelPath);
	registerActions();

	init();
}

void Scene_MagicCatCademy::update(sf::Time dt)
{
	sUpdate(dt);
}

void Scene_MagicCatCademy::sDoAction(const Command& action)
{
	// On Key Press
	if (action.type() == "START") {
		if (action.name() == "PAUSE") { setPaused(!m_isPaused); }
		else if (action.name() == "QUIT") { m_game->quitLevel(); }
		else if (action.name() == "BACK") { m_game->backLevel(); }

		else if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
		else if (action.name() == "TOGGLE_COLLISION") { m_drawAABB = !m_drawAABB; }
		else if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }

		// Player control
		if (action.name() == "LEFT") { m_player->getComponent<CInput>().dir = CInput::LEFT; }
		else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().dir = CInput::RIGHT; }
		else if (action.name() == "UP") { m_player->getComponent<CInput>().dir = CInput::UP; }
		else if (action.name() == "DOWN") { m_player->getComponent<CInput>().dir = CInput::DOWN; }
	}
	// on Key Release
	// the frog can only go in one direction at a time, no angles
	// use a bitset and exclusive setting.
	else if (action.type() == "END" && (action.name() == "LEFT" || action.name() == "RIGHT" || action.name() == "UP" ||
		action.name() == "DOWN")) {
		m_player->getComponent<CInput>().dir = 0;
	}
}

void Scene_MagicCatCademy::sRender()
{
	m_game->window().setView(m_worldView);

	for (auto e : m_entityManager.getEntities("bkg")) {
		if (e->getComponent<CSprite>().has) {
			auto& sprite = e->getComponent<CSprite>().sprite;
			m_game->window().draw(sprite);
		}
	}

	for (auto& e : m_entityManager.getEntities()) {
		if (!e->hasComponent<CAnimation>())
			continue;

		// Draw Sprite
		auto& anim = e->getComponent<CAnimation>().animation;
		auto& tfm = e->getComponent<CTransform>();
		anim.getSprite().setPosition(tfm.pos);
		anim.getSprite().setRotation(tfm.angle);
		m_game->window().draw(anim.getSprite());

		if (m_drawAABB) {
			if (e->hasComponent<CBoundingBox>()) {
				auto box = e->getComponent<CBoundingBox>();
				sf::RectangleShape rect;
				rect.setSize(sf::Vector2f{ box.size.x, box.size.y });
				centerOrigin(rect);
				rect.setPosition(e->getComponent<CTransform>().pos);
				rect.setFillColor(sf::Color(0, 0, 0, 0));
				rect.setOutlineColor(sf::Color{ 0, 255, 0 });
				rect.setOutlineThickness(2.f);
				m_game->window().draw(rect);
			}
		}
	}
}

void Scene_MagicCatCademy::init()
{
	auto pos = sf::Vector2f(200.f, 350.f);

	spawnPlayer(pos);
}

void Scene_MagicCatCademy::sUpdate(sf::Time dt)
{
	m_entityManager.update();
}

void Scene_MagicCatCademy::spawnPlayer(sf::Vector2f pos)
{
	m_player = m_entityManager.addEntity("lucy");
	m_player->addComponent<CTransform>(pos);
	m_player->addComponent<CBoundingBox>(sf::Vector2f(100.f, 100.f));
	m_player->addComponent<CInput>();
	m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("lucyWalk"));
}

void Scene_MagicCatCademy::onEnd()
{
}

void Scene_MagicCatCademy::loadLevel(const std::string& path)
{
	std::ifstream config(path);
	if (config.fail()) {
		std::cerr << "Open file " << path << " failed\n";
		config.close();
		exit(1);
	}

	std::string token{ "" };
	config >> token;
	while (!config.eof()) {
		if (token == "Bkg") {
			std::string name;
			sf::Vector2f pos;
			config >> name >> pos.x >> pos.y;
			auto e = m_entityManager.addEntity("bkg");

			auto& sprite = e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
			sprite.setOrigin(0.f, 0.f);
			sprite.setPosition(pos);
		}
		else if (token[0] == '#') {
			std::cout << token;
		}

		config >> token;
	}

	config.close();
}

void Scene_MagicCatCademy::registerActions()
{
	registerAction(sf::Keyboard::P, "PAUSE");
	registerAction(sf::Keyboard::Escape, "BACK");
	registerAction(sf::Keyboard::Q, "QUIT");
	registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");

	registerAction(sf::Keyboard::A, "LEFT");
	registerAction(sf::Keyboard::Left, "LEFT");
	registerAction(sf::Keyboard::D, "RIGHT");
	registerAction(sf::Keyboard::Right, "RIGHT");
	registerAction(sf::Keyboard::Space, "JUMP");
	registerAction(sf::Mouse::Left, "SHOOT");
}
