#pragma once
#include "Scene.h"

struct LevelConfig {
	float       scrollSpeed{ 60.f };

	std::map<std::string,
		std::vector<std::pair<float, sf::Time>>> directions;
};

class Scene_MagicCatCademy : public Scene
{
private:
	sPtrEntt        m_player{ nullptr };
	sf::View        m_worldView;
	sf::FloatRect   m_worldBounds;

	LevelConfig		m_config;

	bool			walking{ false };
	bool			jumping{ false };
	bool			firingMagic{ false };

	bool			enemyAttacking{ false };

	sf::Clock		attackTimer;
	sf::Time		attackInterval{ sf::seconds(5.f) };

	sf::Clock		enemySpawnTimer;
	sf::Time		enemySpawnInterval{ sf::seconds(20.f) };

	sf::Clock		powerupTimer;
	sf::Time		powerupTime{ sf::seconds(5.f) };
	float			speed{ 1.f };
	float			magicStrength{ 10 };
	std::map<std::string, float> powerupDuration;

	sf::Time		bossTimer;

	int				lives{ 3 };

	bool			m_drawTextures{ true };
	bool			m_drawAABB{ false };
	bool			m_drawGrid{ false };

	void			sUpdate(sf::Time dt);
	void			sAnimation(sf::Time dt);
	void			sMovement(sf::Time dt);
	void			sCollision(sf::Time dt);
	void			sLifespan(sf::Time dt);
	void			sEnemyAttack(sf::Time dt);
	void			sSpawnEnemies(sf::Time dt);
	void			sDestroyOutOfBounds();
	void			sManagePowerups(sf::Time dt);

	void			spawnPlayer(sf::Vector2f pos);
	void			spawnEnemies(sf::Vector2f pos, int amount);
	void			spawnGroundEntity(sf::Vector2f pos);
	void			spawnFireObstacles(sf::Vector2f pos);
	void			drawLives(int lives);
	void			dropPowerup(sf::Vector2f pos);
	void			fireMagic();
	void			enemyAttack(std::shared_ptr<Entity> e);
	void			playerMovement();
	void			checkPlayerState();
	void			checkEnemyState(std::shared_ptr<Entity> e);
	void			checkIfDead(std::shared_ptr<Entity> e);
	void			keepPlayerInBounds();
	sf::FloatRect	getViewBounds();
	void			checkEntityScale(std::string animationName, std::shared_ptr<Entity> e);
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

