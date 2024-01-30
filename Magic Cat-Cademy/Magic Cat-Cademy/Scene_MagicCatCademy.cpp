#include "Scene_MagicCatCademy.h"

void Scene_MagicCatCademy::onEnd()
{
}

Scene_MagicCatCademy::Scene_MagicCatCademy(GameEngine* gameEngine, const std::string& levelPath) : 
	Scene(gameEngine), 
	m_worldView(gameEngine->window().getDefaultView()) {
	loadLevel(levelPath);
	registerActions();

	init();
}

void Scene_MagicCatCademy::update(sf::Time dt)
{
}

void Scene_MagicCatCademy::sDoAction(const Command& command)
{
}

void Scene_MagicCatCademy::sRender()
{
}

void Scene_MagicCatCademy::init()
{
}

void Scene_MagicCatCademy::loadLevel(const std::string& path)
{
}

void Scene_MagicCatCademy::registerActions()
{
}
