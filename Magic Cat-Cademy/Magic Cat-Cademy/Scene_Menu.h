#pragma once

#include "Scene.h"

class Scene_Menu : public Scene
{
private:
	std::vector<std::string>	m_menuStrings;
	sf::Text					m_menuText;
	std::vector<std::string>	m_levelPaths;
	int							m_menuIndex{0};
	std::string					m_title;
	sf::View					m_worldView;

	void displayLogo(sf::Vector2f pos);
	void spawnCat(sf::Vector2f pos);
	void displayBackground(sf::Vector2f pos);

	void sAnimation(sf::Time dt);

	void init();
	void onEnd() override;
public:

	Scene_Menu(GameEngine* gameEngine);

	void update(sf::Time dt) override;

	void sRender() override;
	void sDoAction(const Command& action) override;
	

};

