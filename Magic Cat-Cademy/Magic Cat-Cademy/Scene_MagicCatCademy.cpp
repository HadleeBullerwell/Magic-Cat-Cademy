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
		else if (action.name() == "QUIT") { m_game->quitLevel(); }
		else if (action.name() == "BACK") { m_game->backLevel(); }

		else if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
		else if (action.name() == "TOGGLE_COLLISION") { m_drawAABB = !m_drawAABB; }
		else if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }

		// Player control
		if (action.name() == "LEFT") m_player->getComponent<CInput>().dir = CInput::LEFT;
		else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().dir = CInput::RIGHT; }
		else if (action.name() == "JUMP") { m_player->getComponent<CInput>().dir = CInput::UP; }
		else if (action.name() == "MEOW") { SoundPlayer::getInstance().play("meow", m_player->getComponent<CTransform>().pos); }
		else if (action.name() == "SHOOT") {
			fireMagic();
			firingMagic = true;
			SoundPlayer::getInstance().play("magic", m_player->getComponent<CTransform>().pos);
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

	drawLives(lives);
}

void Scene_MagicCatCademy::init()
{
	auto pos = sf::Vector2f(200.f, 340.f);

	spawnPlayer(pos);
	spawnEnemies(sf::Vector2f(1500.f, 325.f));
	spawnGroundEntity(sf::Vector2f(0, 375.f));
}

void Scene_MagicCatCademy::sUpdate(sf::Time dt)
{
	m_entityManager.update();

	m_worldView.move(m_config.scrollSpeed * dt.asSeconds() * 1, 0);

	sMovement(dt);
	sCollision(dt);
	sAnimation(dt);
	sLifespan(dt);
	sEnemyAttack(dt);
	sDestroyOutOfBounds();
	keepPlayerInBounds();
	enemyAttack();
	checkPlayerState();
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

			if (e->getTag() == "hellhound") {
				auto& playerPos = m_player->getComponent<CTransform>().pos;
				sf::Vector2f direction = playerPos - tfm.pos;
				direction = normalize(direction);

				tfm.vel.x = direction.x * 50.f * dt.asSeconds();

				if (e->hasComponent<CGravity>()) {
					auto& g = e->getComponent<CGravity>().g;
					tfm.vel.y += g;
				}

				checkEnemyState(e);
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

				auto& prevAnim = p->getComponent<CAnimation>().animation;
				p->addComponent<CAnimation>(Assets::getInstance().getAnimation("lucyHurt"));

				if (p->getComponent<CAnimation>().animation.m_currentFrame == 1) {
					p->addComponent<CAnimation>(prevAnim);
				}

				auto& tx = p->getComponent<CTransform>();
				auto offset = (tx.scale.x > 0) ? 80 : -80;
				tx.pos.x -= offset;
				tx.pos.y -= 50;

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
				SoundPlayer::getInstance().play("hellhoundHurt", h->getComponent<CTransform>().pos);
				m->destroy();
				h->getComponent<CHealth>().hp -= 10;
				checkIfDead(h);

				if (h->hasComponent<CGravity>()) {
					auto& g = h->getComponent<CGravity>().g;
					auto& tfm = h->getComponent<CTransform>();
					tfm.vel.x += 50;
					tfm.vel.y -= 7.5;
					tfm.vel.y += g;
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
}

void Scene_MagicCatCademy::sEnemyAttack(sf::Time dt)
{
	timeSinceLastAttack += dt;
	if (timeSinceLastAttack >= attackInterval) {
		auto& pPos = m_player->getComponent<CTransform>().pos;
		float attackRange = 50;
		std::shared_ptr<Entity> closestEnemy = nullptr;

		for (auto& h : m_entityManager.getEntities("hellhound")) {
			auto& hPos = h->getComponent<CTransform>().pos;
			float distance = std::abs(hPos.x - pPos.x);

			if (distance >= attackRange) {
				attackRange = distance;
				closestEnemy = h;
			}
		}

		if (closestEnemy) {
			auto& tx = closestEnemy->getComponent<CTransform>();

			if (std::abs(tx.vel.x) > 0.1f)
				tx.scale.x = (tx.vel.x < 0) ? 1 : -1;

			SoundPlayer::getInstance().play("hellhoundGrowl");
			enemyAttacking = true;

			auto offset = (tx.scale.x < 0) ? 50 : -50;
			tx.pos.x += offset;
			tx.pos.y -= 50;

			checkEnemyState(closestEnemy);		
		}		
		//enemyAttacking = false;
		timeSinceLastAttack = sf::Time::Zero;
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
	m_player->addComponent<CMagic>();
}

void Scene_MagicCatCademy::spawnEnemies(sf::Vector2f pos)
{
	for (int i{ 0 }; i < 1; i++) {
		auto hellhound = m_entityManager.addEntity("hellhound");
		hellhound->addComponent<CTransform>(pos);
		hellhound->addComponent<CBoundingBox>(sf::Vector2f(90, 80));
		hellhound->addComponent<CAnimation>(Assets::getInstance().getAnimation("hellhoundWalk"));
		hellhound->addComponent<CHealth>(100);
		hellhound->addComponent<CGravity>(0.5f);
		hellhound->addComponent<CState>("idle");
		pos.x += 500.f;
	}
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

	sf::Vector2f pos{ 30, 30 };
	sf::Vector2f background = m_worldView.getCenter() - m_worldView.getSize() / 2.f;

	for (int i{ 0 }; i < lives; ++i) {
		auto life = m_entityManager.addEntity("life");
		life->addComponent<CTransform>(pos + background);
		life->addComponent<CAnimation>(Assets::getInstance().getAnimation("lucyHealthIcon"));
		pos.x += 50;
	}
}

void Scene_MagicCatCademy::fireMagic()
{
	int spell = rand() % 3;

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
	magic->getComponent<CTransform>().vel.y = 0;

	if (spell == 1) {
		magic->addComponent<CAnimation>(Assets::getInstance().getAnimation("magicOne"));
	}
	else if (spell == 2) {
		magic->addComponent<CAnimation>(Assets::getInstance().getAnimation("magicStar"));
	}
	else {
		magic->addComponent<CAnimation>(Assets::getInstance().getAnimation("magicStar"));
	}

}

void Scene_MagicCatCademy::enemyAttack()
{

	auto& pPos = m_player->getComponent<CTransform>().pos;
	float attackRange = 50;
	std::shared_ptr<Entity> closestEnemy = nullptr;

	for (auto& h : m_entityManager.getEntities("hellhound")) {
		auto& hPos = h->getComponent<CTransform>().pos;
		float distance = std::abs(hPos.x - pPos.x);

		if (distance >= attackRange) {
			attackRange = distance;
			closestEnemy = h;
			return;
		}

	}

	if (closestEnemy) {
		auto& tx = closestEnemy->getComponent<CTransform>();

		if (std::abs(tx.vel.x) > 0.1f)
			tx.scale.x = (tx.vel.x < 0) ? 1 : -1;

		enemyAttacking = true;
		SoundPlayer::getInstance().play("hellhoundGrowl");

		auto offset = (tx.scale.x < 0) ? 30 : -30;
		tx.pos.x += offset;

		checkEnemyState(closestEnemy);
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
		vel.x -= 1.f;
	}

	if (dir & CInput::RIGHT) {
		walking = true;
		vel.x += 1.f;
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

		if (enemyAttacking) newState = "attacking";

		auto& tx = e->getComponent<CTransform>();
		auto& state = e->getComponent<CState>().state;

		if (std::abs(tx.vel.x) > 0.1f)
			tx.scale.x = (tx.vel.x < 0) ? 1 : -1;

		if (state != "dead") {
			if (state != newState) {
				state = newState;

				Animation anim;
				if (state == "attacking") {
					checkEntityScale("hellhoundAttack", e);
				}

				if (state == "idle") {
					checkEntityScale("hellhoundIdle", e);
				}				
				
				if (state == "walking") {
					checkEntityScale("hellhoundWalk", e);
				}

			}
		}
	}

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
				spawnPlayer(sf::Vector2f(200.f, 340.f));
				spawnEnemies(sf::Vector2f(1500.f, 325.f));
				m_worldView.reset(sf::FloatRect(0.f, 0.f, m_worldView.getSize().x, m_worldView.getSize().y));

				if (lives == 0) {
					onEnd();
				}
			}
		}
	}
}

void Scene_MagicCatCademy::keepPlayerInBounds()
{
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

void Scene_MagicCatCademy::checkEntityScale(std::string animationName, std::shared_ptr<Entity> e)
{
	Animation anim;

	if (e->getComponent<CTransform>().scale.x < 0) {
		anim = Assets::getInstance().getAnimation(animationName);
		anim.setFlipped(true);
	}
	else {
		anim = Assets::getInstance().getAnimation(animationName);
	}

	e->addComponent<CAnimation>(anim);
}

void Scene_MagicCatCademy::onEnd()
{
	m_game->changeScene("MENU", nullptr, false);
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
	registerAction(sf::Keyboard::Escape, "BACK", Action::Keyboard);
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
