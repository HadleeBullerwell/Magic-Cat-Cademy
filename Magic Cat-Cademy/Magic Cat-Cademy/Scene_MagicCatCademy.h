#pragma once
#include "Scene.h"

struct LevelConfig {
	float       scrollSpeed{ 60.f };
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
	bool			isGrounded{ true };

	sf::Clock		attackTimer;
	sf::Time		attackInterval{ sf::seconds(5.f) };

	sf::Clock		enemySpawnTimer;
	sf::Time		enemySpawnInterval{ sf::seconds(20.f) };

	sf::Clock		powerupTimer;
	sf::Time		powerupTime{ sf::seconds(5.f) };
	float			speed{ 1.f };
	float			magicStrength{ 10 };
	std::map<std::string, float> powerupDuration;

	sf::Text		health;
	sf::Text		cooldown;

	bool			bossSpawned{ false };
	bool			bossPositioned{ false };
	bool			bossDefeated{ false };
	sf::Clock		bossAttackTimer;
	sf::Time		bossAttackInterval{ sf::seconds(5.f) };

	sf::Clock		endScene;
	sf::Time		endSceneTimer{ sf::seconds(2.f) };

	sf::RectangleShape	fadeOverlay;
	bool			fadingToBlack{ false };
	float			fadeAlpha = 0.0f;
	float			fadeSpeed = 50.0f;

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
	void			sBossBattle(sf::Time dt);
	void			sDestroyOutOfBounds();
	void			sManagePowerups(sf::Time dt);

	void			spawnPlayer(sf::Vector2f pos);
	void			spawnEnemies(sf::Vector2f pos, int amount);
	void			spawnBoss(sf::Vector2f pos);
	void			spawnSierra(sf::Vector2f pos);
	void			spawnGroundEntity(sf::Vector2f pos);
	void			spawnFireObstacles(sf::Vector2f pos);

	void			drawLives(int lives);
	void			drawLevelIntroduction(sf::Vector2f pos);
	void			drawHealthBar(int hp, sf::Vector2f pos, int barSize);
	void			drawMagicCooldownBar(sf::Time& cooldown, sf::Clock& cooldownTimer);
	void			drawCage(sf::Vector2f pos);

	void			dropPowerup(sf::Vector2f pos);
	void			fireMagic();
	void			enemyAttack(std::shared_ptr<Entity> e);
	void			bossAttack();
	void			freeSierra();
	void			playerMovement();
	void			fadeToBlack(sf::RenderWindow& window, sf::Time dt);

	void			checkPlayerState();
	void			checkEnemyState(std::shared_ptr<Entity> e);
	void			checkBossState();
	void			checkEntityScale(std::string animationName, std::shared_ptr<Entity> e);
	void			checkNPCState(std::shared_ptr<Entity> e);
	void			checkIfDead(std::shared_ptr<Entity> e);

	void			keepPlayerInBounds();
	void			keepBossInBounds();
	void			panToBossPosition(sf::Time dt);
	sf::FloatRect	getViewBounds();

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

