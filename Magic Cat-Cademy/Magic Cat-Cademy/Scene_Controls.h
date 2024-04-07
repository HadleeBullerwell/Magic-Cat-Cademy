#pragma once

#include "Scene.h"


class Scene_Controls : public Scene
{
private:
	sf::View					m_worldView;
	sf::Vector2f				m_worldViewPosition;
	int							m_menuIndex{ 0 };
	sf::Text					backButton;

	void	displayBackground(sf::Vector2f pos);

	void	sAnimation(sf::Time dt);

	void	init();
	void	onEnd() override;
public:

	Scene_Controls(GameEngine* gameEngine);

	void	update(sf::Time dt) override;

	void	sRender() override;
	void	sDoAction(const Command& action) override;
};

