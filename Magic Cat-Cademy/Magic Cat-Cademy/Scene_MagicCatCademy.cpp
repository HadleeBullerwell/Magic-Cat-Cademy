#include "Scene_MagicCatCademy.h"
#include "Components.h"
#include "SoundPlayer.h"
#include "MusicPlayer.h"
#include "Entity.h"
#include "Physics.h"
#include <fstream>
#include <cmath>
#include <random>
#include <iostream>

namespace {
	std::random_device rd;
	std::mt19937 rng(rd());
}

Scene_MagicCatCademy::Scene_MagicCatCademy(GameEngine* gameEngine, const std::string& levelPath) :
	Scene(gameEngine),
	m_worldView(gameEngine->window().getDefaultView()) {
	loadLevel(levelPath);
	registerActions();

	fadeOverlay.setSize(sf::Vector2f(m_worldView.getSize().x, m_worldView.getSize().y));
	fadeOverlay.setFillColor(sf::Color(0, 0, 0, 0));

	MusicPlayer::getInstance().play("mainTune");
	MusicPlayer::getInstance().setVolume(50);

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

		else if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
		else if (action.name() == "TOGGLE_COLLISION") { m_drawAABB = !m_drawAABB; }
		else if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }

		if (!bossDefeated) {
			if (action.name() == "LEFT") m_player->getComponent<CInput>().dir = CInput::LEFT;
			else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().dir = CInput::RIGHT; }
			else if (action.name() == "JUMP" && isGrounded) { m_player->getComponent<CInput>().dir = CInput::UP; }
			else if (action.name() == "MEOW") { SoundPlayer::getInstance().play("meow", m_player->getComponent<CTransform>().pos); }
			else if (action.name() == "SHOOT") {
				if (m_player->getComponent<CMagic>().cooldownTimer.getElapsedTime() >= m_player->getComponent<CMagic>().cooldown) {
					fireMagic();
					firingMagic = true;
					SoundPlayer::getInstance().play("magic", m_player->getComponent<CTransform>().pos);
					m_player->getComponent<CMagic>().cooldownTimer.restart();
				}
			}
		}
	}

	else if (action.type() == "END" && (action.name() == "LEFT" || action.name() == "RIGHT" || action.name() == "JUMP" || action.name() == "SHOOT")) {
		m_player->getComponent<CInput>().dir = 0;

		if (action.name() == "LEFT")
			walking = false;

		if (action.name() == "RIGHT")
			walking = false;

		if (action.name() == "JUMP") {
			jumping = false;
		}

		if (action.name() == "SHOOT") {
			firingMagic = false;
		}
	}
}

void Scene_MagicCatCademy::sRender()
{
	m_game->window().setView(m_worldView);

	sf::Vector2f pos{ 1275, 0 };
	sf::Vector2f background = m_worldView.getCenter() - m_worldView.getSize() / 2.f;

	health.setFont(Assets::getInstance().getFont("meow"));
	health.setPosition(pos + background);
	health.setString("HP ");
	health.setCharacterSize(60);
	health.setFillColor(sf::Color(177, 100, 245));
	health.setOutlineColor(sf::Color(54, 1, 77));
	health.setOutlineThickness(1.5f);

	pos = { 1150, 50 };

	cooldown.setFont(Assets::getInstance().getFont("meow"));
	cooldown.setPosition(pos + background);
	cooldown.setString("COOLDOWN ");
	cooldown.setCharacterSize(60);
	cooldown.setFillColor(sf::Color(177, 100, 245));
	cooldown.setOutlineColor(sf::Color(54, 1, 77));
	cooldown.setOutlineThickness(1.5f);


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
		anim.getSprite().setScale(tfm.scale.x, tfm.scale.y);
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
	for (auto& e : m_entityManager.getEntities()) {
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

	auto hellhounds = m_entityManager.getEntities("hellhound");
	auto mars = m_entityManager.getEntities("mars");

	for (auto h : hellhounds) {
		auto tfm = h->getComponent<CTransform>();

		sf::Vector2f hellhoundPos = tfm.pos;

		sf::Vector2f healthBarPosition = hellhoundPos + sf::Vector2f(-50.0f, -70.f);

		healthBarPosition -= m_worldView.getCenter() - m_worldView.getSize() / 2.f;

		drawHealthBar(h->getComponent<CHealth>().hp, healthBarPosition, 10);
	}


	for (auto m : mars) {
		auto tfm = m->getComponent<CTransform>();

		sf::Vector2f hellhoundPos = tfm.pos;

		sf::Vector2f healthBarPosition = hellhoundPos + sf::Vector2f(-125, -80.f);

		healthBarPosition -= m_worldView.getCenter() - m_worldView.getSize() / 2.f;

		drawHealthBar(m->getComponent<CHealth>().hp, healthBarPosition, 10);
	}



	m_game->window().draw(health);
	m_game->window().draw(cooldown);
	drawLives(lives);
	drawHealthBar(m_player->getComponent<CHealth>().hp, sf::Vector2f{ 1350, 30 }, 30);
	drawMagicCooldownBar(m_player->getComponent<CMagic>().cooldown, m_player->getComponent<CMagic>().cooldownTimer);

	if (fadeAlpha > 0.0f) {
		fadeOverlay.setPosition(m_worldView.getCenter() - (m_worldView.getSize() / 2.0f));
		m_game->window().draw(fadeOverlay);
	}
}

void Scene_MagicCatCademy::init()
{
	auto pos = sf::Vector2f(200.f, 340.f);

	spawnPlayer(pos);

	// when demonstrating boss battle comment
	spawnEnemies(sf::Vector2f(1500.f, 325.f), 3);
	spawnGroundEntity(sf::Vector2f(0, 375.f));
	spawnFireObstacles(sf::Vector2f(750.f, 340.f));
	drawLevelIntroduction(sf::Vector2f(m_worldView.getSize().x / 2, m_worldView.getSize().y / 2 - 100));
}

void Scene_MagicCatCademy::sUpdate(sf::Time dt)
{
	SoundPlayer::getInstance().removeStoppedSounds();

	if (m_isPaused)
		return;

	m_entityManager.update();

	auto intro = m_entityManager.getEntities("intro");

	if (bossSpawned && !bossPositioned) {
		panToBossPosition(dt);
	}
	else if (intro.empty()) {
		if (!bossPositioned)
			m_worldView.move(m_config.scrollSpeed * dt.asSeconds() * 1, 0);
		m_isPaused = false;
	}

	if (fadingToBlack) {
		fadeAlpha += fadeSpeed * dt.asSeconds();
		if (fadeAlpha >= 255.f) {
			fadeAlpha = 255.f;
			fadingToBlack = false;
			onEnd();
		}
		fadeOverlay.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(fadeAlpha)));
	}

	// when demonstrating boss battle comment
	sSpawnEnemies(dt);
	sMovement(dt);
	sCollision(dt);
	sBossBattle(dt);
	sAnimation(dt);
	sLifespan(dt);
	sEnemyAttack(dt);
	sManagePowerups(dt);
	keepPlayerInBounds();
	keepBossInBounds();
	checkPlayerState();
	checkBossState();
	drawLives(lives);
}

void Scene_MagicCatCademy::sAnimation(sf::Time dt)
{
	auto list = m_entityManager.getEntities();
	for (auto e : m_entityManager.getEntities()) {
		// update all animations
		if (e->hasComponent<CAnimation>()) {
			auto& anim = e->getComponent<CAnimation>();
			anim.animation.update(dt);

			if (anim.animation.hasEnded() && anim.animation.getName() == "hellhoundAttack") {
				e->getComponent<CAttack>().isAttacking = false;
			}
			else if (anim.animation.hasEnded() && anim.animation.getName() == "marsAttack") {
				e->getComponent<CBoss>().isAttacking = false;
				e->getComponent<CBoss>().isDodging = false;
			}
			else if (anim.animation.hasEnded() && anim.animation.getName() == "marsFire") {
				e->getComponent<CBoss>().isFireAttacking = false;
			}
			else if (anim.animation.hasEnded() && anim.animation.getName() == "explosion") {
				e->destroy();
			}
			else if (anim.animation.hasEnded() && anim.animation.getName() == "lucyHurt") {
				m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("lucyIdle"));
			}
		}
	}
}

void Scene_MagicCatCademy::sMovement(sf::Time dt)
{
	playerMovement();

	// move all objects
	for (auto e : m_entityManager.getEntities()) {

		if (e->hasComponent<CTransform>()) {
			auto& tfm = e->getComponent<CTransform>();

			if (e->getTag() == "hellhound") {
				auto& playerPos = m_player->getComponent<CTransform>().pos;
				sf::Vector2f direction = playerPos - tfm.pos;
				direction = normalize(direction);

				tfm.vel.x = direction.x * 40.f * dt.asSeconds();

				if (e->hasComponent<CGravity>()) {
					auto& g = e->getComponent<CGravity>().g;
					tfm.vel.y += g;
				}

				checkEnemyState(e);
			}

			if (e->getTag() == "mars") {

				if (e->hasComponent<CGravity>()) {
					auto& g = e->getComponent<CGravity>().g;
					tfm.vel.y += g;
				}

				if (e->getComponent<CBoss>().isWalking) {
					auto playerPos = m_player->getComponent<CTransform>().pos;
					auto marsPos = e->getComponent<CTransform>().pos;
					float distance = sqrt(pow(playerPos.x - marsPos.x, 2) + pow(playerPos.y - marsPos.y, 2));

					if (distance <= 75) {
						e->addComponent<CBoundingBox>(sf::Vector2f(120, 100));
						e->getComponent<CTransform>().vel.x = 0;
						e->getComponent<CBoss>().isWalking = false;
						e->getComponent<CBoss>().isReturning = true;
						checkBossState();
						bossAttackTimer.restart();
					}
					else {
						e->getComponent<CTransform>().vel.x -= 1;
						e->getComponent<CBoss>().isWalking = true;
						checkBossState();
					}
				}

				if (e->getComponent<CBoss>().isReturning) {
					e->addComponent<CBoundingBox>(sf::Vector2f(100, 100));
					auto& tfm = e->getComponent<CTransform>();
					tfm.vel.x += 2;
					auto originalPos = sf::Vector2f(2500.f, 325.f);
					tfm.scale.x = -1;

					if (tfm.pos.x >= originalPos.x) {
						tfm.vel.x = 0;
						tfm.scale.x = 1;
						e->getComponent<CBoss>().isReturning = false;
						checkBossState();
					}
				}


			}

			if (e->getTag() == "sierra") {
				auto playerPos = m_player->getComponent<CTransform>().pos;
				auto sierraPos = e->getComponent<CTransform>().pos;
				float distance = sqrt(pow(playerPos.x - sierraPos.x, 2) + pow(playerPos.y - sierraPos.y, 2));

				if (distance <= 100) {
					m_player->getComponent<CTransform>().vel.x = 0;
					e->getComponent<CTransform>().vel.x = 0;
					checkNPCState(e);

					if (endScene.getElapsedTime() >= endSceneTimer) {
						speed = 0.5;
						m_player->getComponent<CInput>().dir = CInput::RIGHT;
						e->getComponent<CTransform>().vel.x += 2;
						e->getComponent<CTransform>().scale.x = 1;
						checkNPCState(e);

						fadingToBlack = true;
					}
				}
			}

			tfm.pos += tfm.vel * dt.asSeconds();
			tfm.angle += tfm.angVel * dt.asSeconds();
		}
	}
}

void Scene_MagicCatCademy::sCollision(sf::Time dt)
{
	auto ground = m_entityManager.getEntities("ground");
	auto hellhounds = m_entityManager.getEntities("hellhound");
	auto player = m_entityManager.getEntities("lucy");
	auto magic = m_entityManager.getEntities("magic");
	auto fire = m_entityManager.getEntities("fire");
	auto powerups = m_entityManager.getEntities("powerup");

	for (auto p : player) {
		for (auto g : ground) {
			auto overlap = Physics::getOverlap(p, g);

			if (overlap.x > 0 && overlap.y > 0) {
				auto prevOverlap = Physics::getPreviousOverlap(p, g);
				auto& pTransform = p->getComponent<CTransform>();
				auto& gTransform = g->getComponent<CTransform>();

				if (prevOverlap.x > 0) {
					if (pTransform.prevPos.y < gTransform.prevPos.y) {
						p->getComponent<CTransform>().pos.y -= overlap.y;
					}
					else {
						p->getComponent<CTransform>().pos.y += overlap.y;
					}
					p->getComponent<CTransform>().vel.y = 0.f;
					isGrounded = true;
				}
			}
		}

		for (auto h : hellhounds) {
			auto overlap = Physics::getOverlap(p, h);

			if (overlap.x > 0 && overlap.y > 0) {
				if (m_player->hasComponent<CImmunity>()) {
					auto& immunity = m_player->getComponent<CImmunity>().isImmune;

					if (immunity) {
						continue;
					}
				}

				SoundPlayer::getInstance().play("hellhoundBark");
				SoundPlayer::getInstance().play("sadMeow");


				p->getComponent<CHealth>().hp -= 15;
				p->addComponent<CAnimation>(Assets::getInstance().getAnimation("lucyHurt"));

				auto& tx = p->getComponent<CTransform>();
				auto offset = (tx.scale.x > 0) ? 100 : -100;
				tx.pos.x -= offset;
				tx.pos.y -= 50;

				checkIfDead(p);
			}
		}

		for (auto f : fire) {
			auto overlap = Physics::getOverlap(f, p);

			if (overlap.x > 0 && overlap.y > 0) {
				if (m_player->hasComponent<CImmunity>()) {
					auto& immunity = m_player->getComponent<CImmunity>().isImmune;

					if (immunity) {
						continue;
					}
				}

				SoundPlayer::getInstance().play("sadMeow");
				p->getComponent<CHealth>().hp -= 5;

				auto& tx = p->getComponent<CTransform>();
				auto offset = (tx.scale.x > 0) ? 30 : -30;
				tx.pos.x -= offset;
				tx.pos.y -= 50;

				p->addComponent<CAnimation>(Assets::getInstance().getAnimation("lucyHurt"));

				checkIfDead(p);

			}
		}

		for (auto powerup : powerups) {
			auto overlap = Physics::getOverlap(p, powerup);

			if (overlap.x > 0 && overlap.y > 0) {
				auto powerupName = powerup->getComponent<CAnimation>().animation.getName();

				if (powerupName == "shieldPowerup") {
					m_player->addComponent<CImmunity>(8.f);
				}
				else if (powerupName == "speedPowerup") {
					speed = 3.f;
					powerupDuration["speedPowerup"] = 3.f;
				}
				else if (powerupName == "magicCooldownPowerup") {
					m_player->getComponent<CMagic>().cooldown = sf::seconds(0);
					powerupDuration["magicCooldownPowerup"] = 5.f;
				}
				else if (powerupName == "magicStrengthPowerup") {
					magicStrength = 20;
					powerupDuration["magicStrengthPowerup"] = 5.f;
				}
				else if (powerupName == "extraLifePowerup") {
					if (lives == 9) {
						powerup->destroy();
					}
					else {
						lives += 1;
					}
				}

				powerup->destroy();
			}
		}

		for (auto boss : m_entityManager.getEntities("mars")) {
			auto overlap = Physics::getOverlap(boss, p);

			if (overlap.x > 0 && overlap.y > 0) {
				SoundPlayer::getInstance().play("sadMeow");
				p->getComponent<CHealth>().hp -= 10;

				auto& tx = p->getComponent<CTransform>();
				auto offset = (tx.scale.x > 0) ? 30 : -30;
				tx.pos.x -= offset;
				tx.pos.y -= 50;

				p->addComponent<CAnimation>(Assets::getInstance().getAnimation("lucyHurt"));
				checkIfDead(p);
			}
		}
	}

	for (auto h : hellhounds) {
		for (auto g : ground) {
			auto overlap = Physics::getOverlap(h, g);

			if (overlap.x > 0 && overlap.y > 0) {
				auto prevOverlap = Physics::getPreviousOverlap(h, g);
				auto& hTransform = h->getComponent<CTransform>();
				auto& gTransform = g->getComponent<CTransform>();

				if (prevOverlap.x > 0) {
					if (hTransform.prevPos.y < gTransform.prevPos.y) {
						h->getComponent<CTransform>().pos.y -= overlap.y;
					}
					else {
						h->getComponent<CTransform>().pos.y += overlap.y;
					}
					h->getComponent<CTransform>().vel.y = 0.f;
				}
			}
		}
	}


	for (auto m : magic) {
		for (auto h : hellhounds) {
			auto overlap = Physics::getOverlap(m, h);

			if (overlap.x > 0 && overlap.y > 0) {
				SoundPlayer::getInstance().play("hellhoundHurt");
				m->destroy();
				h->getComponent<CHealth>().hp -= magicStrength;
				checkIfDead(h);

				auto& tx = h->getComponent<CTransform>();
				auto offset = (tx.scale.x > 0) ? -50 : 50;

				auto& g = h->getComponent<CGravity>().g;
				auto& tfm = h->getComponent<CTransform>();
				tfm.pos.x -= offset;
				tfm.vel.y -= 5;
				tfm.vel.y += g;
			}
		}


		for (auto mars : m_entityManager.getEntities("mars")) {
			auto overlap = Physics::getOverlap(m, mars);

			if (overlap.x > 0 && overlap.y > 0) {
				SoundPlayer::getInstance().play("hellhoundHurt", mars->getComponent<CTransform>().pos);
				m->destroy();
				mars->getComponent<CHealth>().hp -= magicStrength;
				checkIfDead(mars);
			}
		}
	}

	for (auto mars : m_entityManager.getEntities("mars")) {
		for (auto g : ground) {
			auto overlap = Physics::getOverlap(mars, g);

			if (overlap.x > 0 && overlap.y > 0) {
				auto prevOverlap = Physics::getPreviousOverlap(mars, g);
				auto& mTransform = mars->getComponent<CTransform>();
				auto& gTransform = g->getComponent<CTransform>();

				if (prevOverlap.x > 0) {
					if (mTransform.prevPos.y < gTransform.prevPos.y) {
						mars->getComponent<CTransform>().pos.y -= overlap.y;
					}
					else {
						mars->getComponent<CTransform>().pos.y += overlap.y;
					}
					mars->getComponent<CTransform>().vel.y = 0.f;
				}
			}
		}
	}
}

void Scene_MagicCatCademy::sLifespan(sf::Time dt)
{
	for (auto m : m_entityManager.getEntities("magic")) {
		auto& lifespan = m->getComponent<CLifespan>();

		if (lifespan.has) {
			lifespan.remaining -= 1;

			if (lifespan.remaining < 0) {
				m->destroy();
			}
		}
	}

	for (auto m : m_entityManager.getEntities("intro")) {
		auto& lifespan = m->getComponent<CLifespan>();

		if (lifespan.has) {
			lifespan.remaining -= 1;

			if (lifespan.remaining < 0) {
				m->destroy();
			}
		}
	}

	for (auto f : m_entityManager.getEntities("fire")) {
		auto& lifespan = f->getComponent<CLifespan>();

		if (lifespan.has) {
			lifespan.remaining -= 1;

			if (lifespan.remaining < 0) {
				f->destroy();
			}
		}
	}
}

void Scene_MagicCatCademy::sEnemyAttack(sf::Time dt)
{
	auto hellhounds = m_entityManager.getEntities("hellhound");
	auto attackRange = 250.f;

	if (attackTimer.getElapsedTime() >= attackInterval) {
		auto pPos = m_player->getComponent<CTransform>().pos;
		std::shared_ptr<Entity> closestHellhound = nullptr;
		float closestDistance = std::numeric_limits<float>::max();

		for (auto h : hellhounds) {

			auto hPos = h->getComponent<CTransform>().pos;

			float distance = sqrt(pow(pPos.x - hPos.x, 2) + pow(pPos.y - hPos.y, 2));

			if (distance < attackRange && distance < closestDistance) {
				closestDistance = distance;
				closestHellhound = h;
				break;
			}
		}

		if (closestHellhound != nullptr) {
			enemyAttack(closestHellhound);
		}
		attackTimer.restart();
	}
}

void Scene_MagicCatCademy::sSpawnEnemies(sf::Time dt)
{
	if (!bossSpawned) {
		int amount = 4;
		if (enemySpawnTimer.getElapsedTime() >= enemySpawnInterval) {
			spawnEnemies(sf::Vector2f(m_player->getComponent<CTransform>().pos.x + 1000.f, 325.f), amount);
			sDestroyOutOfBounds();
			enemySpawnTimer.restart();
		}
	}
}

void Scene_MagicCatCademy::sBossBattle(sf::Time dt)
{
	auto hellhounds = m_entityManager.getEntities("hellhound");
	auto fire = m_entityManager.getEntities("fire");

	if (m_player->getComponent<CTransform>().pos.x >= 7500 && !bossSpawned) {

		//if (m_player->getComponent<CTransform>().pos.x >= 1500 && !bossSpawned) { use to demonstrate boss battle
		bossAttackTimer.restart();

		for (auto h : hellhounds) {
			h->destroy();
		}

		for (auto f : fire) {
			f->destroy();
		}

		spawnBoss(sf::Vector2f(8500, 325.f));
		//spawnBoss(sf::Vector2f(2500, 325.f)); // use to demonstrate boss battle
		bossSpawned = true;

		spawnSierra(sf::Vector2f(8650, 340.f));
		drawCage(sf::Vector2f(8650, 320.f));
		//spawnSierra(sf::Vector2f(2650, 340.f)); // use to demonstrate boss battle
		//drawCage(sf::Vector2f(2650, 320.f)); // use to demonstrate boss battle

		MusicPlayer::getInstance().stop();
		MusicPlayer::getInstance().play("bossMusic");
	}

	if (bossAttackTimer.getElapsedTime() >= bossAttackInterval && bossSpawned) {
		bossAttack();
		bossAttackTimer.restart();
	}
}


void Scene_MagicCatCademy::sDestroyOutOfBounds()
{
	auto bounds = getViewBounds();
	for (auto h : m_entityManager.getEntities("hellhound")) {
		if (h->hasComponent<CTransform>() && h->hasComponent<CBoundingBox>()) {
			auto pos = h->getComponent<CTransform>().pos;
			if (!bounds.contains(pos)) {
				h->destroy();
			}
		}
	}
}

void Scene_MagicCatCademy::sManagePowerups(sf::Time dt)
{
	for (auto& [powerupName, duration] : powerupDuration) {
		duration -= dt.asSeconds();

		if (duration <= 0.0f) {
			if (powerupName == "speedPowerup") {
				speed = 1.f;
			}
			else if (powerupName == "magicCooldownPowerup") {
				m_player->getComponent<CMagic>().cooldown = sf::seconds(0.5f);
			}
			else if (powerupName == "magicStrengthPowerup") {
				magicStrength = 10;
			}
		}
	}

	if (m_player->hasComponent<CImmunity>()) {
		auto& immunityTime = m_player->getComponent<CImmunity>();

		if (immunityTime.isImmune) {
			immunityTime.duration -= dt.asSeconds();

			if (immunityTime.duration <= 0.0f) {
				immunityTime.isImmune = false;
				m_player->removeComponent<CImmunity>();
			}
		}
	}
}

void Scene_MagicCatCademy::spawnPlayer(sf::Vector2f pos)
{
	m_player = m_entityManager.addEntity("lucy");
	m_player->addComponent<CTransform>(pos);
	m_player->addComponent<CBoundingBox>(sf::Vector2f(55, 60));
	m_player->addComponent<CInput>();
	m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("lucyIdle"));
	m_player->addComponent<CState>("idle");
	m_player->addComponent<CGravity>(0.5f);
	m_player->addComponent<CHealth>(100);
	m_player->addComponent<CMagic>(sf::seconds(0.5f));
}

void Scene_MagicCatCademy::spawnEnemies(sf::Vector2f pos, int amount)
{
	for (int i{ 0 }; i < amount; i++) {
		auto hellhound = m_entityManager.addEntity("hellhound");
		hellhound->addComponent<CTransform>(pos);
		hellhound->addComponent<CBoundingBox>(sf::Vector2f(80, 80));
		hellhound->addComponent<CAnimation>(Assets::getInstance().getAnimation("hellhoundWalk"));
		hellhound->addComponent<CHealth>(100);
		hellhound->addComponent<CGravity>(0.5f);
		hellhound->addComponent<CState>("idle");
		hellhound->addComponent<CAttack>(false);
		pos.x += 250.f;
	}
}

void Scene_MagicCatCademy::spawnBoss(sf::Vector2f pos)
{
	auto mars = m_entityManager.addEntity("mars");
	mars->addComponent<CTransform>(pos);
	mars->addComponent<CBoundingBox>(sf::Vector2f(100, 100));
	mars->addComponent<CAnimation>(Assets::getInstance().getAnimation("marsIdle"));
	mars->addComponent<CBoss>();
	mars->addComponent<CHealth>(250);
	mars->addComponent<CGravity>(0.5f);
	mars->addComponent<CState>("idle");
}

void Scene_MagicCatCademy::spawnSierra(sf::Vector2f pos)
{
	auto sierra = m_entityManager.addEntity("sierra");
	sierra->addComponent<CTransform>(pos);
	sierra->addComponent<CBoundingBox>(sf::Vector2f(55, 60));
	sierra->addComponent<CAnimation>(Assets::getInstance().getAnimation("sierraIdle"));
	sierra->addComponent<CNPC>();
	sierra->addComponent<CState>("idle");
}

void Scene_MagicCatCademy::spawnGroundEntity(sf::Vector2f pos)
{
	auto ground = m_entityManager.addEntity("ground");
	ground->addComponent<CTransform>(pos);
	ground->addComponent<CBoundingBox>(sf::Vector2f(30000, 2.f));
}

void Scene_MagicCatCademy::spawnFireObstacles(sf::Vector2f pos)
{
	for (int i{ 0 }; i < 50; i++) {
		auto fire = m_entityManager.addEntity("fire");
		fire->addComponent<CTransform>(pos);
		fire->addComponent<CBoundingBox>(sf::Vector2f(30, 60));
		fire->addComponent<CAnimation>(Assets::getInstance().getAnimation("fire"));
		pos.x += 1500.f;
	}
}

void Scene_MagicCatCademy::drawLives(int lives)
{
	for (auto life : m_entityManager.getEntities("life")) {
		life->destroy();
	}

	sf::Vector2f pos{ 30, 30 };
	sf::Vector2f background = m_worldView.getCenter() - m_worldView.getSize() / 2.f;

	for (int i{ 0 }; i < lives; ++i) {
		auto life = m_entityManager.addEntity("life");
		life->addComponent<CTransform>(pos + background);
		life->addComponent<CAnimation>(Assets::getInstance().getAnimation("lucyHealthIcon"));
		pos.x += 50;
	}
}

void Scene_MagicCatCademy::drawLevelIntroduction(sf::Vector2f pos)
{
	auto intro = m_entityManager.addEntity("intro");
	intro->addComponent<CAnimation>(Assets::getInstance().getAnimation("levelOneIntro"));
	intro->addComponent<CTransform>(pos);
	intro->addComponent<CLifespan>(75.f);
}

void Scene_MagicCatCademy::drawHealthBar(int hp, sf::Vector2f pos, int barSize)
{
	sf::Vector2f background = m_worldView.getCenter() - m_worldView.getSize() / 2.f;

	sf::RectangleShape healthBar;
	healthBar.setFillColor(sf::Color::Red);
	healthBar.setOutlineColor(sf::Color::Black);
	healthBar.setOutlineThickness(1.5f);
	healthBar.setSize(sf::Vector2f(hp, barSize));
	healthBar.setPosition(pos + background);
	m_game->window().draw(healthBar);
}

void Scene_MagicCatCademy::drawMagicCooldownBar(sf::Time& cooldown, sf::Clock& cooldownTimer)
{
	sf::Vector2f pos{ 1350, 70 };
	sf::Vector2f background = m_worldView.getCenter() - m_worldView.getSize() / 2.f;

	float cooldownPercentage = 0.0f;
	if (cooldown != sf::Time::Zero) {
		sf::Time remainingTime = cooldown - cooldownTimer.getElapsedTime();
		cooldownPercentage = std::max(0.0f, std::min(1.0f, (remainingTime.asSeconds() / cooldown.asSeconds())));
	}

	sf::RectangleShape cooldownBar;
	cooldownBar.setFillColor(sf::Color::Blue);
	cooldownBar.setOutlineColor(sf::Color::Black);
	cooldownBar.setOutlineThickness(1.5f);
	cooldownBar.setSize(sf::Vector2f(cooldownPercentage * 100, 30.f));
	cooldownBar.setPosition(pos + background);
	m_game->window().draw(cooldownBar);
}

void Scene_MagicCatCademy::drawCage(sf::Vector2f pos)
{
	auto cage = m_entityManager.addEntity("cage");
	cage->addComponent<CTransform>(pos);
	cage->addComponent<CAnimation>(Assets::getInstance().getAnimation("cageLocked"));
}

void Scene_MagicCatCademy::dropPowerup(sf::Vector2f pos)
{
	static const std::string powerups[] =
	{ "shieldPowerup", "speedPowerup", "magicCooldownPowerup", "magicStrengthPowerup", "extraLifePowerup" };

	std::uniform_int_distribution<int> d1(1, 5);
	std::uniform_int_distribution<int> d2(0, 4);

	if (d1(rng) <= 3) {
		auto powerup = powerups[d2(rng)];
		auto p = m_entityManager.addEntity("powerup");
		p->addComponent<CTransform>(pos);
		auto bb = p->addComponent<CAnimation>(Assets::getInstance().getAnimation(powerup)).animation.getBB();
		p->addComponent<CBoundingBox>(bb);
	}
}

void Scene_MagicCatCademy::fireMagic()
{
	std::uniform_int_distribution<int> rand(1, 2);

	int spell = rand(rng);

	auto magic = m_entityManager.addEntity("magic");
	magic->addComponent<CTransform>(m_player->getComponent<CTransform>().pos);
	magic->addComponent<CBoundingBox>(sf::Vector2f(50.f, 50.f));
	magic->addComponent<CLifespan>(100);

	if (m_player->getComponent<CTransform>().scale.x < 0) {
		magic->getComponent<CTransform>().vel.x = -10;
	}
	else {
		magic->getComponent<CTransform>().vel.x = 10;
	}

	if (spell == 1) {
		magic->addComponent<CAnimation>(Assets::getInstance().getAnimation("magicOne"));
	}
	else if (spell == 2) {
		magic->addComponent<CAnimation>(Assets::getInstance().getAnimation("magicStar"));
	}
}

void Scene_MagicCatCademy::enemyAttack(std::shared_ptr<Entity> e)
{
	auto& tx = e->getComponent<CTransform>();
	auto& attack = e->getComponent<CAttack>();
	auto& anim = e->getComponent<CAnimation>().animation;

	if (std::abs(tx.vel.x) > 0.1f)
		tx.scale.x = (tx.vel.x < 0) ? 1 : -1;

	SoundPlayer::getInstance().play("hellhoundGrowl", tx.pos);

	auto offset = (tx.scale.x < 0) ? 60 : -60;
	tx.pos.x += offset;
	tx.pos.y -= 50;
	tx.vel.x += 2.f;

	attack.isAttacking = true;
	checkEnemyState(e);
}

void Scene_MagicCatCademy::bossAttack()
{
	auto mars = m_entityManager.getEntities("mars");

	for (auto m : mars) {
		std::uniform_int_distribution<int> rand(1, 3);
		std::uniform_int_distribution<int> rand2(3, 5);

		int attack = rand(rng);

		if (attack == 1) {
			m->getComponent<CBoss>().isFireAttacking = true;
			checkBossState();
			SoundPlayer::getInstance().play("fire");
			int fireAmount = rand2(rng);
			auto pos = m_player->getComponent<CTransform>().pos;
			for (int i{ 0 }; i < fireAmount; i++) {
				auto fire = m_entityManager.addEntity("fire");
				fire->addComponent<CTransform>(sf::Vector2f{ pos.x, 340.f });
				fire->addComponent<CBoundingBox>(sf::Vector2f(30, 60));
				fire->addComponent<CLifespan>(150);
				fire->addComponent<CAnimation>(Assets::getInstance().getAnimation("fire"));
				pos.x += 250;
			}
		}
		else if (attack == 2) {
			m->getComponent<CTransform>().vel.y -= 15;
			m->getComponent<CBoss>().isDodging = true;
			checkBossState();
		}
		else if (attack == 3) {
			m->getComponent<CTransform>().vel.x -= 1;
			m->getComponent<CBoss>().isWalking = true;
			checkBossState();

		}
	}
}

void Scene_MagicCatCademy::freeSierra()
{
	MusicPlayer::getInstance().play("mainTune");

	for (auto c : m_entityManager.getEntities("cage")) {
		c->addComponent<CAnimation>(Assets::getInstance().getAnimation("cageUnlocked"));
	}

	for (auto f : m_entityManager.getEntities("fire")) {
		f->destroy();
	}

	// cats walk towards each other, meow, then walk off screen, screen fades to black
	for (auto s : m_entityManager.getEntities("sierra")) {
		s->getComponent<CTransform>().vel.x -= 2;
		s->getComponent<CNPC>().isWalking = true;
		checkNPCState(s);
		SoundPlayer::getInstance().play("meow", s->getComponent<CTransform>().pos);
		SoundPlayer::getInstance().play("meow", m_player->getComponent<CTransform>().pos);
		endScene.restart();
	}
}

void Scene_MagicCatCademy::playerMovement()
{
	// no movement if player is dead
	if (m_player->hasComponent<CState>() && m_player->getComponent<CState>().state == "dead")
		return;

	auto& dir = m_player->getComponent<CInput>().dir;
	auto& pos = m_player->getComponent<CTransform>().pos;
	auto& vel = m_player->getComponent<CTransform>().vel;
	auto& gravity = m_player->getComponent<CGravity>().g;
	auto& state = m_player->getComponent<CState>().state;

	vel.x = 0.f;

	if (dir & CInput::LEFT) {
		walking = true;
		vel.x -= speed;
	}

	if (dir & CInput::RIGHT) {
		walking = true;
		vel.x = speed;
	}

	if (dir & CInput::UP) {
		jumping = true;
		isGrounded = false;
		vel.y = -10.f;
	}

	vel.y += gravity;
	vel.x = vel.x * 5.f;

	for (auto e : m_entityManager.getEntities()) {
		auto& tx = e->getComponent<CTransform>();
		tx.prevPos = tx.pos;
		tx.pos += tx.vel;
	}
}

void Scene_MagicCatCademy::checkPlayerState()
{
	auto& tx = m_player->getComponent<CTransform>();

	if (m_player->hasComponent<CState>()) {
		std::string newState = "idle";

		if (std::abs(tx.vel.x) > 0.1f)
			tx.scale.x = (tx.vel.x > 0) ? 1 : -1;

		if (walking) newState = "walking";
		if (jumping) newState = "jumping";
		if (firingMagic) newState = "firingMagic";

		auto& state = m_player->getComponent<CState>().state;
		if (state != "dead") {

			if (state != newState) {
				state = newState;

				if (state == "walking")
					checkEntityScale("lucyWalk", m_player);

				if (state == "jumping")
					checkEntityScale("lucyJump", m_player);

				if (state == "firingMagic")
					checkEntityScale("lucyMagic", m_player);

				if (state == "idle")
					checkEntityScale("lucyIdle", m_player);

			}
		}
	}
}

void Scene_MagicCatCademy::checkEnemyState(std::shared_ptr<Entity> e)
{
	std::string newState = "walking";

	if (e->hasComponent<CState>()) {

		if (e->getComponent<CAttack>().isAttacking) newState = "attacking";

		auto& tx = e->getComponent<CTransform>();
		auto& state = e->getComponent<CState>().state;
		auto& animation = e->getComponent<CAnimation>().animation;
		auto& attack = e->getComponent<CAttack>();

		if (std::abs(tx.vel.x) > 0.1f)
			tx.scale.x = (tx.vel.x < 0) ? 1 : -1;

		if (state != "dead") {
			if (state != newState) {
				state = newState;

				if (state == "attacking") {
					checkEntityScale("hellhoundAttack", e);
				}

				if (state == "walking") {
					checkEntityScale("hellhoundWalk", e);
				}

				if (state == "idle") {
					checkEntityScale("hellhoundIdle", e);
				}
			}
		}
	}

}

void Scene_MagicCatCademy::checkBossState()
{
	std::string newState = "idle";

	auto mars = m_entityManager.getEntities("mars");

	for (auto m : mars) {
		if (m->hasComponent<CState>()) {

			if (m->getComponent<CBoss>().isAttacking) newState = "attacking";
			if (m->getComponent<CBoss>().isFireAttacking) newState = "fireAttacking";
			if (m->getComponent<CBoss>().isWalking) newState = "walking";
			if (m->getComponent<CBoss>().isDodging) newState = "dodging";
			if (m->getComponent<CBoss>().isReturning) newState = "returning";

			auto& tx = m->getComponent<CTransform>();
			auto& state = m->getComponent<CState>().state;

			if (std::abs(tx.vel.x) > 0.1f)
				tx.scale.x = (tx.vel.x < 0) ? 1 : -1;

			if (state != "dead") {
				if (state != newState) {
					state = newState;

					if (state == "fireAttacking") {
						checkEntityScale("marsFire", m);
					}

					if (state == "attacking" || state == "dodging") {
						checkEntityScale("marsAttack", m);
					}

					if (state == "walking" || state == "returning") {
						checkEntityScale("marsWalk", m);
					}

					if (state == "idle") {
						checkEntityScale("marsIdle", m);
					}
				}
			}
		}
	}
}

void Scene_MagicCatCademy::checkNPCState(std::shared_ptr<Entity> e)
{
	std::string newState = "idle";

	auto sierra = m_entityManager.getEntities("sierra");

	for (auto s : sierra) {
		if (s->hasComponent<CState>()) {

			if (s->getComponent<CNPC>().isWalking) newState = "walking";

			auto& tx = s->getComponent<CTransform>();
			auto& state = s->getComponent<CState>().state;

			if (std::abs(tx.vel.x) > 0.1f)
				tx.scale.x = (tx.vel.x < 0) ? 1 : -1;

			if (state != "dead") {
				if (state != newState) {
					state = newState;

					if (state == "walking") {
						checkEntityScale("sierraWalk", e);
					}

					if (state == "idle") {
						checkEntityScale("sierraIdle", e);
					}

				}
			}

		}
	}
}

void Scene_MagicCatCademy::checkEntityScale(std::string animationName, std::shared_ptr<Entity> e)
{
	Animation anim = Assets::getInstance().getAnimation(animationName);

	bool flip = (e->getComponent<CTransform>().scale.x < 0);
	anim.setFlipped(flip);

	e->getComponent<CAnimation>().animation = anim;
}

void Scene_MagicCatCademy::checkIfDead(std::shared_ptr<Entity> e)
{
	if (e->hasComponent<CHealth>()) {
		if (e->getComponent<CHealth>().hp <= 0) {
			e->destroy();

			if (e->getTag() == "lucy") {
				lives--;

				for (auto h : m_entityManager.getEntities("hellhound")) {
					h->destroy();
				}
				for (auto p : m_entityManager.getEntities("powerup")) {
					p->destroy();
				}

				if (lives == 0) {
					onEnd();
				}

				spawnPlayer(sf::Vector2f(200.f, 340.f));
				spawnEnemies(sf::Vector2f(1500.f, 325.f), 3);
				m_worldView.reset(sf::FloatRect(0.f, 0.f, m_worldView.getSize().x, m_worldView.getSize().y));
				enemySpawnTimer.restart();
			}
			else if (e->getTag() == "hellhound") {
				dropPowerup(e->getComponent<CTransform>().pos);
			}
			else if (e->getTag() == "mars") {
				auto explosion = m_entityManager.addEntity("explosion");
				explosion->addComponent<CTransform>(sf::Vector2f(e->getComponent<CTransform>().pos));
				explosion->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
				bossDefeated = true;

				freeSierra();
			}
		}

	}
}

void Scene_MagicCatCademy::keepPlayerInBounds()
{
	if (!bossDefeated) {
		auto center = m_worldView.getCenter();
		sf::Vector2f viewHalfSize = m_worldView.getSize() / 2.f;


		auto left = center.x - viewHalfSize.x;
		auto right = center.x + viewHalfSize.x;
		auto top = center.y - viewHalfSize.y;
		auto bot = center.y + viewHalfSize.y;

		auto& player_pos = m_player->getComponent<CTransform>().pos;

		auto halfSize = sf::Vector2f{ 20, 20 };

		player_pos.x = std::max(player_pos.x, left + halfSize.x);
		player_pos.x = std::min(player_pos.x, right - halfSize.x);
		player_pos.y = std::max(player_pos.y, top + halfSize.y);
		player_pos.y = std::min(player_pos.y, bot - halfSize.y);
	}
}

void Scene_MagicCatCademy::keepBossInBounds()
{
	if (bossSpawned && bossPositioned) {
		for (auto m : m_entityManager.getEntities("mars")) {
			auto center = m_worldView.getCenter();
			sf::Vector2f viewHalfSize = m_worldView.getSize() / 2.f;


			auto left = center.x - viewHalfSize.x;
			auto right = center.x + viewHalfSize.x;
			auto top = center.y - viewHalfSize.y;
			auto bot = center.y + viewHalfSize.y;

			auto& marsPos = m->getComponent<CTransform>().pos;

			auto halfSize = sf::Vector2f{ 20, 20 };

			marsPos.x = std::max(marsPos.x, left + halfSize.x);
			marsPos.x = std::min(marsPos.x, right - halfSize.x);
			marsPos.y = std::max(marsPos.y, top + halfSize.y);
			marsPos.y = std::min(marsPos.y, bot - halfSize.y);
		}
	}
}

void Scene_MagicCatCademy::panToBossPosition(sf::Time dt)
{
	auto mars = m_entityManager.getEntities("mars");

	sf::Vector2f bossPos{};

	for (auto m : mars) {
		bossPos = { m->getComponent<CTransform>().pos.x - 500, m->getComponent<CTransform>().pos.y };
	}

	speed = 0.f;

	sf::Vector2f currentView = m_worldView.getCenter();
	sf::Vector2f panToView = bossPos;
	panToView.y = currentView.y;

	sf::Vector2f distanceToPan = panToView - currentView;
	float distance = std::sqrt(distanceToPan.x);
	float panSpeed = 50.f;

	if (distance <= panSpeed * dt.asSeconds()) {
		m_worldView.setCenter(panToView);
		speed = 1.f;
		bossPositioned = true;
		return;
	}

	sf::Vector2f panDirection = distanceToPan / distance;
	sf::Vector2f panIncrement = panDirection * panSpeed * dt.asSeconds() * 0.5f;
	m_worldView.move(panIncrement);

}

sf::FloatRect Scene_MagicCatCademy::getViewBounds()
{
	auto center = m_worldView.getCenter();
	auto size = m_worldView.getSize();
	auto leftTop = center - size / 2.f;

	leftTop.y -= size.y / 2.f;
	size.y += size.y;
	leftTop.x -= size.x / 2.f;
	size.x += size.x;
	return sf::FloatRect(leftTop, size);
}



void Scene_MagicCatCademy::onEnd()
{
	MusicPlayer::getInstance().play("mainMenuMusic");
	m_game->quitLevel();
	lives = 3;
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
	registerAction(sf::Keyboard::P, "PAUSE", Action::Keyboard);
	registerAction(sf::Keyboard::Q, "QUIT", Action::Keyboard);
	registerAction(sf::Keyboard::C, "TOGGLE_COLLISION", Action::Keyboard);
	registerAction(sf::Keyboard::A, "LEFT", Action::Keyboard);
	registerAction(sf::Keyboard::Left, "LEFT", Action::Keyboard);
	registerAction(sf::Keyboard::D, "RIGHT", Action::Keyboard);
	registerAction(sf::Keyboard::Right, "RIGHT", Action::Keyboard);
	registerAction(sf::Keyboard::Space, "JUMP", Action::Keyboard);
	registerAction(1024, "SHOOT", Action::Mouse);
	registerAction(sf::Keyboard::M, "MEOW", Action::Keyboard);
}
