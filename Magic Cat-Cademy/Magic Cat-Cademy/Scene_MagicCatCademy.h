#pragma once
#include "Scene.h"

class Scene_MagicCatCademy : public Scene
{
private:
	sPtrEntt        m_player{ nullptr };
	sf::View        m_worldView;
	sf::FloatRect   m_worldBounds;

	bool			m_drawTextures{ true };
	bool			m_drawAABB{ false };
	bool			m_drawGrid{ false };

	void			sUpdate(sf::Time dt);

	void			spawnPlayer(sf::Vector2f pos);

	void			onEnd() override;	

	void			init();
	void			loadLevel(const std::string& path);

	void			registerActions();

public:
	Scene_MagicCatCademy(GameEngine* gameEngine, const std::string& levelPath);

	void		  update(sf::Time dt) override;
	void		  sDoAction(const Command& command) override;
	void		  sRender() override;

};

