#include "Scene_MagicCatCademy.h"
#include "Components.h"
#include "SoundPlayer.h"
#include "MusicPlayer.h"
#include "Entity.h"
#include "Physics.h"
#include <fstream>
#include <iostream>

Scene_MagicCatCademy::Scene_MagicCatCademy(GameEngine* gameEngine, const std::string& levelPath) :
	Scene(gameEngine),
	m_worldView(gameEngine->window().getDefaultView()) {
	loadLevel(levelPath);
	registerActions();

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
		if (action.name() == "PAUSE") { setPaused(!m_isPaused); }
		else if (action.name() == "QUIT") { m_game->quitLevel(); }
		else if (action.name() == "BACK") { m_game->backLevel(); }

		else if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
		else if (action.name() == "TOGGLE_COLLISION") { m_drawAABB = !m_drawAABB; }
		else if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }

		// Player control
		if (action.name() == "LEFT") {
			m_player->getComponent<CInput>().dir = CInput::LEFT;
			std::cout << "left";
		}
		else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().dir = CInput::RIGHT; }
		else if (action.name() == "JUMP") { m_player->getComponent<CInput>().dir = CInput::UP; }
		else if (action.name() == "MEOW") { SoundPlayer::getInstance().play("meow", m_player->getComponent<CTransform>().pos); }
		else if (action.name() == "SHOOT") {
			auto mousePos = sf::Mouse::getPosition();
			sf::Vector2f mouse(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
			fireMagic(mouse);
		}
	}

	else if (action.type() == "END" && (action.name() == "LEFT" || action.name() == "RIGHT" || action.name() == "JUMP")) {
		m_player->getComponent<CInput>().dir = 0;

		if (action.name() == "LEFT")
			walkingLeft = false;

		if (action.name() == "RIGHT")
			walkingRight = false;

		if (action.name() == "JUMP") {
			jumping = false;
		}
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

	drawLives(lives);
}

void Scene_MagicCatCademy::init()
{
	auto pos = sf::Vector2f(200.f, 340.f);

	spawnPlayer(pos);
	spawnEnemies(sf::Vector2f(600.f, 340.f));
	spawnGroundEntity(sf::Vector2f(0, 375.f));
}

void Scene_MagicCatCademy::sUpdate(sf::Time dt)
{
	m_entityManager.update();

	m_worldView.move(m_config.scrollSpeed * dt.asSeconds() * 1, 0);

	sMovement(dt);
	sCollision(dt);
	sAnimation(dt);
	checkPlayerState();
}

void Scene_MagicCatCademy::sAnimation(sf::Time dt)
{
	auto list = m_entityManager.getEntities();
	for (auto e : m_entityManager.getEntities()) {
		// update all animations
		if (e->hasComponent<CAnimation>()) {
			auto& anim = e->getComponent<CAnimation>();
			anim.animation.update(dt);

			if (e->getTag() == "magic" && anim.animation.hasEnded()) {
				e->destroy();
			}

			checkPlayerState();
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
				}
			}
		}

		for (auto h : hellhounds) {
			auto overlap = Physics::getOverlap(p, h);

			if (overlap.x > 0 && overlap.y > 0) {
				p->getComponent<CHealth>().hp -= 10;
			}
		}
	}
}

void Scene_MagicCatCademy::spawnPlayer(sf::Vector2f pos)
{
	m_player = m_entityManager.addEntity("lucy");
	m_player->addComponent<CTransform>(pos);
	m_player->addComponent<CBoundingBox>(sf::Vector2f(80, 60));
	m_player->addComponent<CInput>();
	m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("lucyIdle"));
	m_player->addComponent<CState>("idle");
	m_player->addComponent<CGravity>(0.5f);
	m_player->addComponent<CHealth>(100);
}

void Scene_MagicCatCademy::spawnEnemies(sf::Vector2f pos)
{
	auto hellhound = m_entityManager.addEntity("hellhound");
	hellhound->addComponent<CTransform>(pos);
	hellhound->addComponent<CAnimation>(Assets::getInstance().getAnimation("hellhoundIdle"));
	hellhound->addComponent<CHealth>(100);
}

void Scene_MagicCatCademy::spawnGroundEntity(sf::Vector2f pos)
{
	auto ground = m_entityManager.addEntity("ground");
	ground->addComponent<CTransform>(pos);
	ground->addComponent<CBoundingBox>(sf::Vector2f(15000, 2.f));
}

void Scene_MagicCatCademy::drawLives(int lives)
{
	for (auto life : m_entityManager.getEntities("life")) {
		life->destroy();
	}

	sf::Vector2f pos{30, 30};

	for (int i{ 0 }; i < lives; ++i) {
		auto life = m_entityManager.addEntity("life");
		life->addComponent<CTransform>(pos);
		life->addComponent<CAnimation>(Assets::getInstance().getAnimation("lucyHealthIcon"));
		pos.x += 50;
	}

}

void Scene_MagicCatCademy::fireMagic(sf::Vector2f pos)
{
	auto spawn = m_player->getComponent<CTransform>().pos;
	spawn.x += 80;
	spawn.y -= 20;
	sf::Vector2f mouseClickLocation = pos - spawn;
	float length = std::sqrt(mouseClickLocation.x * mouseClickLocation.x + mouseClickLocation.y * mouseClickLocation.y);

	auto magic = m_entityManager.addEntity("magic");
	magic->addComponent<CTransform>(spawn);
	magic->addComponent<CBoundingBox>();
	magic->addComponent<CState>();
	magic->addComponent<CAnimation>(Assets::getInstance().getAnimation("magic"));
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
	auto& ground = m_player->getComponent<CTransform>().pos.y;

	if (dir & CInput::LEFT) {
		walkingLeft = true;
		pos.x -= 3.f;
	}

	if (dir & CInput::RIGHT) {
		walkingRight = true;
		pos.x += 3.f;
	}
	
	if (dir & CInput::UP) {
		jumping = true;
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
	if (m_player->hasComponent<CState>()) {
		std::string newState = "idle";

		if (walkingRight)
			newState = "walkingRight";

		if (walkingLeft)
			newState = "walkingLeft";

		if (jumping)
			newState = "jumping";

		auto& state = m_player->getComponent<CState>().state;
		if (state != "dead") {

			if (state != newState) {
				state = newState;

				Animation anim;

				if (state == "walkingLeft" || state == "walkingRight")
					anim = Assets::getInstance().getAnimation("lucyWalk");

				if (state == "jumping")
					anim = Assets::getInstance().getAnimation("lucyJump");

				if (state == "idle")
					anim = Assets::getInstance().getAnimation("lucyIdle");


				if (state == "walkingLeft")
					anim.setFlipped(true);
				else
					anim.setFlipped(false);

				m_player->addComponent<CAnimation>(anim);
			}
		}
	}
}

void Scene_MagicCatCademy::checkIfDead(std::shared_ptr<Entity> e)
{
	if (e->hasComponent<CHealth>()) {
		if (e->getComponent<CHealth>().hp <= 0) {
			e->destroy();
		}
	}
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
	registerAction(sf::Keyboard::T, "SHOOT");
	registerAction(sf::Keyboard::M, "MEOW");
}
