//
// Created by David Burchill on 2023-09-27.
//

#include <fstream>
#include <iostream>

#include "Scene_Frogger.h"
#include "Components.h"
#include "Physics.h"
#include "Utilities.h"
#include "MusicPlayer.h"
#include "Assets.h"
#include "SoundPlayer.h"
#include <random>
#include <thread>
#include <string>

namespace {
	std::random_device rd;
	std::mt19937 rng(rd());
}


Scene_Frogger::Scene_Frogger(GameEngine* gameEngine, const std::string& levelPath)
	: Scene(gameEngine)
	, m_worldView(gameEngine->window().getDefaultView()) {
	loadLevel(levelPath);
	registerActions();

	MusicPlayer::getInstance().play("gameTheme");
	MusicPlayer::getInstance().setVolume(50);

	init();
}


void Scene_Frogger::init() {
	auto pos = m_worldView.getSize();

	// spawn frog in middle of first row
	pos.x = pos.x / 2.f;
	pos.y -= 20.f;

	spawnVehicles();
	spawnLogsAndTurtles();
	spawnLilyPads();
	spawnPlayer(pos);

}

void Scene_Frogger::drawLives(int lives)
{
	for (auto life : m_entityManager.getEntities("life")) {
		life->destroy();
	}

	sf::Vector2f pos{ 10, 600.f - 580 };
	sf::Vector2f vel(0.f, 0.f);
	for (int i{ 0 }; i < lives; ++i) {
		auto life = m_entityManager.addEntity("life");
		auto& tfm = life->addComponent<CTransform>(pos, vel);
		life->addComponent<CBoundingBox>(sf::Vector2f(18.f, 21.f));
		life->addComponent<CAnimation>(Assets::getInstance().getAnimation("lives"));
		pos.x += 30;
	}
}

void Scene_Frogger::sMovement(sf::Time dt) {
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


void Scene_Frogger::registerActions() {
	registerAction(sf::Keyboard::P, "PAUSE");
	registerAction(sf::Keyboard::Escape, "BACK");
	registerAction(sf::Keyboard::Q, "QUIT");
	registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");

	registerAction(sf::Keyboard::A, "LEFT");
	registerAction(sf::Keyboard::Left, "LEFT");
	registerAction(sf::Keyboard::D, "RIGHT");
	registerAction(sf::Keyboard::Right, "RIGHT");
	registerAction(sf::Keyboard::W, "UP");
	registerAction(sf::Keyboard::Up, "UP");
	registerAction(sf::Keyboard::S, "DOWN");
	registerAction(sf::Keyboard::Down, "DOWN");
}


void Scene_Frogger::onEnd() {
	m_game->changeScene("MENU", nullptr, false);
	lives = 3;
	highScore = score;
	score = 0;
	gameTimer.restart();

	for (auto lilypads : m_entityManager.getEntities("lillypad")) {
		auto& state = lilypads->getComponent<CState>().state;
		state = "not occupied";
	}

	for (auto frogs : m_entityManager.getEntities("lilypadFrog")) {
		frogs->destroy();
	}
}

void Scene_Frogger::playerMovement() {
	// no movement if player is dead
	if (m_player->hasComponent<CState>() && m_player->getComponent<CState>().state == "dead")
		return;

	auto& dir = m_player->getComponent<CInput>().dir;
	auto& pos = m_player->getComponent<CTransform>().pos;

	if (dir & CInput::UP) {
		m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("up"));

		if (score >= highScore) {
			highScore += 10;
		}

		score += 10;
		pos.y -= 40.f;
	}
	if (dir & CInput::DOWN) {
		m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("down"));
		pos.y += 40.f;
	}

	if (dir & CInput::LEFT) {
		m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("left"));
		pos.x -= 40.f;
	}

	if (dir & CInput::RIGHT) {
		m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("right"));
		pos.x += 40.f;
	}

	if (dir != 0) {
		SoundPlayer::getInstance().play("hop", m_player->getComponent<CTransform>().pos);
		dir = 0;
	}
}


void Scene_Frogger::sRender() {
	m_game->window().setView(m_worldView);

	scoreText.setFont(Assets::getInstance().getFont("Arcade"));
	scoreText.setPosition({ 5, 600.f - 570 });
	scoreText.setString("SCORE ");
	scoreText.setCharacterSize(25);

	scoreNumber.setFont(Assets::getInstance().getFont("Arcade"));
	scoreNumber.setPosition({ 80, 600.f - 565 });
	std::string scoreString = std::to_string(score);
	scoreNumber.setString(scoreString);
	scoreNumber.setCharacterSize(20);

	highScoreText.setFont(Assets::getInstance().getFont("Arcade"));
	highScoreText.setPosition({ 150, 600.f - 570 });
	highScoreText.setString("HIGH SCORE ");
	highScoreText.setCharacterSize(25);

	highScoreNumber.setFont(Assets::getInstance().getFont("Arcade"));
	highScoreNumber.setPosition(285, 600.f - 565);
	std::string highScoreString = std::to_string(highScore);
	highScoreNumber.setString(highScoreString);
	highScoreNumber.setCharacterSize(20);

	gameTimeText.setFont(Assets::getInstance().getFont("Arcade"));
	gameTimeText.setPosition(330, 600.f - 599);
	gameTimeText.setString("TIME REMAINING ");
	gameTimeText.setCharacterSize(20);

	timeRemaining.setFont(Assets::getInstance().getFont("Arcade"));
	timeRemaining.setPosition(390, 600.f - 580);

	// draw bkg first
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
	m_game->window().draw(scoreText);
	m_game->window().draw(scoreNumber);
	m_game->window().draw(highScoreText);
	m_game->window().draw(highScoreNumber);
	m_game->window().draw(gameTimeText);
	m_game->window().draw(timeRemaining);
	drawLives(lives);
}

void Scene_Frogger::update(sf::Time dt) {
	sUpdate(dt);
}

void Scene_Frogger::sDoAction(const Command& action) {
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


void Scene_Frogger::spawnPlayer(sf::Vector2f pos) {
	m_player = m_entityManager.addEntity("player");
	m_player->addComponent<CTransform>(pos);
	m_player->addComponent<CBoundingBox>(sf::Vector2f(15.f, 15.f));
	m_player->addComponent<CInput>();
	m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("up"));
	m_player->addComponent<CState>("alive");
}

void Scene_Frogger::spawnLilypadFrogs(sf::Vector2f lilyPadPos)
{
	m_player = m_entityManager.addEntity("lilypadFrog");
	m_player->addComponent<CTransform>(lilyPadPos);
	m_player->addComponent<CBoundingBox>(sf::Vector2f(15.f, 15.f));
	m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("down"));
	m_player->addComponent<CState>("on lilypad");
}

void Scene_Frogger::spawnVehicles()
{
	spawnLane1();
	spawnLane2();
	spawnLane3();
	spawnLane4();
	spawnLane5();
}

void Scene_Frogger::spawnLane1()
{
	// cars going to left
	sf::Vector2f pos{ 480.f + 60, 600.f - 60 };
	sf::Vector2f vel(-60.f, 0.f);
	for (int i{ 0 }; i < 3; ++i) {
		auto car = m_entityManager.addEntity("car");
		auto& tfm = car->addComponent<CTransform>(pos, vel);
		car->addComponent<CBoundingBox>(sf::Vector2f(40.f, 40.f));
		car->addComponent<CAnimation>(Assets::getInstance().getAnimation("raceCarL"));
		pos.x += 140.f;
	}
}

void Scene_Frogger::spawnLane2()
{
	// tractors going to right
	sf::Vector2f pos{ 0.f - 60, 600.f - 100.f };
	sf::Vector2f vel(60.f, 0.f);
	for (int i{ 0 }; i < 3; ++i) {
		auto car = m_entityManager.addEntity("car");
		auto& tfm = car->addComponent<CTransform>(pos, vel);
		car->addComponent<CBoundingBox>(sf::Vector2f(40.f, 40.f));
		car->addComponent<CAnimation>(Assets::getInstance().getAnimation("tractor"));
		pos.x -= 140.f;
	}
}

void Scene_Frogger::spawnLane3()
{
	// cars going left
	sf::Vector2f pos{ 480.f + 20, 600.f - 140.f };
	sf::Vector2f vel(-60.f, 0.f);
	for (int i{ 0 }; i < 3; ++i) {
		auto car = m_entityManager.addEntity("car");
		auto& tfm = car->addComponent<CTransform>(pos, vel);
		car->addComponent<CBoundingBox>(sf::Vector2f(40.f, 40.f));
		car->addComponent<CAnimation>(Assets::getInstance().getAnimation("car"));
		pos.x += 140.f;
	}
}

void Scene_Frogger::spawnLane4()
{
	// tractors going to right
	sf::Vector2f pos{ 0.f - 40, 600.f - 180 };
	sf::Vector2f vel(60.f, 0.f);
	for (int i{ 0 }; i < 3; ++i) {
		auto car = m_entityManager.addEntity("car");
		auto& tfm = car->addComponent<CTransform>(pos, vel);
		car->addComponent<CBoundingBox>(sf::Vector2f(40.f, 40.f));
		car->addComponent<CAnimation>(Assets::getInstance().getAnimation("raceCarR"));
		pos.x -= 140.f;
	}
}

void Scene_Frogger::spawnLane5()
{
	// trucks going left
	sf::Vector2f pos{ 480.f - 20, 600.f - 220.f };
	sf::Vector2f vel(-60.f, 0.f);
	for (int i{ 0 }; i < 3; ++i) {
		auto car = m_entityManager.addEntity("car");
		auto& tfm = car->addComponent<CTransform>(pos, vel);
		car->addComponent<CBoundingBox>(sf::Vector2f(61.f, 40.f));
		car->addComponent<CAnimation>(Assets::getInstance().getAnimation("truck"));
		pos.x += 140.f;
	}
}

void Scene_Frogger::spawnLilyPads()
{
	sf::Vector2f pos{ 0 + 35, 600.f - 505 };
	sf::Vector2f vel(0.f, 0.f);
	for (int i{ 0 }; i < 5; ++i) {
		auto lilypad = m_entityManager.addEntity("lillypad");
		auto& tfm = lilypad->addComponent<CTransform>(pos, vel);
		lilypad->addComponent<CBoundingBox>(sf::Vector2f(32.f, 31.f));
		lilypad->addComponent<CAnimation>(Assets::getInstance().getAnimation("lillyPad"));
		lilypad->addComponent<CState>("not occupied");
		pos.x += 102;
	}
}

void Scene_Frogger::spawnLogsAndTurtles()
{
	spawnLogsLane1();
	spawnLogsLane2();
	spawnLogsLane3();

	spawnTurtleLane1();
	spawnTurtleLane2();
}

void Scene_Frogger::spawnLogsLane1()
{
	// logs going right
	sf::Vector2f pos{ 480.f, 600.f - 340.f };
	sf::Vector2f vel(-60.f, 0.f);
	for (int i{ 0 }; i < 3; ++i) {
		auto log = m_entityManager.addEntity("log");
		auto& tfm = log->addComponent<CTransform>(pos, vel);
		log->addComponent<CBoundingBox>(sf::Vector2f(95.f, 40.f));
		log->addComponent<CAnimation>(Assets::getInstance().getAnimation("tree1"));
		pos.x -= 140.f;
	}
}

void Scene_Frogger::spawnLogsLane2()
{
	// logs going right
	sf::Vector2f pos{ 480.f + 40, 600.f - 380.f };
	sf::Vector2f vel(-60.f, 0.f);
	for (int i{ 0 }; i < 3; ++i) {
		auto log = m_entityManager.addEntity("log");
		auto& tfm = log->addComponent<CTransform>(pos, vel);
		log->addComponent<CBoundingBox>(sf::Vector2f(196.f, 40.f));
		log->addComponent<CAnimation>(Assets::getInstance().getAnimation("tree2"));
		pos.x -= 230.f;
	}
}

void Scene_Frogger::spawnLogsLane3()
{
	// logs going right
	sf::Vector2f pos{ 480.f + 20, 600.f - 460.f };
	sf::Vector2f vel(-60.f, 0.f);
	for (int i{ 0 }; i < 3; ++i) {
		auto log = m_entityManager.addEntity("log");
		auto& tfm = log->addComponent<CTransform>(pos, vel);
		log->addComponent<CBoundingBox>(sf::Vector2f(95.f, 40.f));
		log->addComponent<CAnimation>(Assets::getInstance().getAnimation("tree1"));
		pos.x -= 140.f;
	}
}

void Scene_Frogger::spawnTurtleLane1()
{
	// turtles going left
	sf::Vector2f pos{ 480.f - 40, 600.f - 300 };
	sf::Vector2f vel(-60.f, 0.f);
	for (int i{ 0 }; i < 3; ++i) {
		auto turtle = m_entityManager.addEntity("turtle");
		auto& tfm = turtle->addComponent<CTransform>(pos, vel);
		turtle->addComponent<CBoundingBox>(sf::Vector2f(60.f, 40.f));
		turtle->addComponent<CAnimation>(Assets::getInstance().getAnimation("2-turtles"));
		pos.x -= 120.f;
	}
}

void Scene_Frogger::spawnTurtleLane2()
{
	// turtles going left
	sf::Vector2f pos{ 480.f - 80, 600.f - 420.f };
	sf::Vector2f vel(-60.f, 0.f);
	for (int i{ 0 }; i < 3; ++i) {
		auto turtle = m_entityManager.addEntity("turtle");
		auto& tfm = turtle->addComponent<CTransform>(pos, vel);
		turtle->addComponent<CBoundingBox>(sf::Vector2f(100.f, 40.f));
		turtle->addComponent<CAnimation>(Assets::getInstance().getAnimation("3-turtles"));
		pos.x -= 140.f;
	}
}

void Scene_Frogger::sManageVehicles()
{
	for (auto e : m_entityManager.getEntities("car")) {
		auto& tfm = e->getComponent<CTransform>();

		if (tfm.pos.x < -60.f && tfm.vel.x < 0)
			tfm.pos.x += 540;
		if (tfm.pos.x > 540.f && tfm.vel.x > 0)
			tfm.pos.x -= 540;

	}
}

void Scene_Frogger::sManageLogsAndTurtles()
{
	for (auto e : m_entityManager.getEntities("log")) {
		auto& tfm = e->getComponent<CTransform>();

		if (tfm.pos.x < -60.f && tfm.vel.x < 0)
			tfm.pos.x += 600;
		if (tfm.pos.x > 720.f && tfm.vel.x > 0)
			tfm.pos.x -= 600;
	}

	for (auto e : m_entityManager.getEntities("turtle")) {
		auto& tfm = e->getComponent<CTransform>();

		if (tfm.pos.x < -60.f && tfm.vel.x < 0)
			tfm.pos.x += 600;
		if (tfm.pos.x > 720.f && tfm.vel.x > 0)
			tfm.pos.x -= 600;

		std::uniform_int_distribution<> d(1, 100);
		int chance = d(rng);

		auto& animation = e->getComponent<CAnimation>().animation;
		int currentFrame = animation.m_currentFrame;
		int lastFrame = animation.m_frames.size() - 1;

		bool hasBoundingBox = e->hasComponent<CBoundingBox>();

		if (chance == 1 && currentFrame == lastFrame) {

			if (hasBoundingBox) {
				e->removeComponent<CBoundingBox>();
			}

			if (animation.getName() == "2-turtles") {
				animation = Assets::getInstance().getAnimation("2turtles");
			}
			else if (animation.getName() == "3-turtles") {
				animation = Assets::getInstance().getAnimation("3turtles");
			}
		}

		if (currentFrame == 0 && !hasBoundingBox) {

			if (animation.getName() == "2turtles") {
				e->addComponent<CBoundingBox>(sf::Vector2f(60.f, 40.f));
			}
			else if (animation.getName() == "3turtles") {
				e->addComponent<CBoundingBox>(sf::Vector2f(100.f, 40.f));
			}
		}
	}
}




sf::FloatRect Scene_Frogger::getViewBounds() {
	auto view = m_game->window().getView();
	return sf::FloatRect(
		(view.getCenter().x - view.getSize().x / 2.f), (view.getCenter().y - view.getSize().y / 2.f),
		view.getSize().x, view.getSize().y);
}

void Scene_Frogger::sCollisions() {

	auto cars = m_entityManager.getEntities("car");
	auto logs = m_entityManager.getEntities("log");
	auto turtles = m_entityManager.getEntities("turtle");
	auto lillypads = m_entityManager.getEntities("lillypad");
	auto& playerTfm = m_player->getComponent<CTransform>();

	// frog hitting cars
	for (auto& car : cars) {
		auto overlap = Physics::getOverlap(car, m_player);

		if (overlap.x > 0 and overlap.y > 0) {
			auto& state = m_player->getComponent<CState>().state;
			state = "dead";
			checkPlayerState();
		}
	}

	// frog hitting water 
	float waterLevelY = { 600 - 280 };

	bool onLogOrTurtle = false;

	// frog hitting logs
	for (auto& log : logs) {
		auto& tfm = log->getComponent<CTransform>();
		auto overlap = Physics::getOverlap(log, m_player);

		if (overlap.x > 0 and overlap.y > 0) {
			playerTfm.vel.x = tfm.vel.x;
			onLogOrTurtle = true;
		}
	}

	// frog hitting turtles
	for (auto& turtle : turtles) {
		auto& tfm = turtle->getComponent<CTransform>();
		auto overlap = Physics::getOverlap(turtle, m_player);

		if (overlap.x > 0 and overlap.y > 0) {
			playerTfm.vel.x = tfm.vel.x;
			onLogOrTurtle = true;
			break;
		}
	}

	bool onLillypad = false;
	// frog hitting lillypad
	for (auto& lilypad : lillypads) {
		auto overlap = Physics::getOverlap(lilypad, m_player);
		auto& tfm = lilypad->getComponent<CTransform>();
		auto& playerTfm = m_player->getComponent<CTransform>();
		auto& state = lilypad->getComponent<CState>().state;

		if (overlap.x > 0 and overlap.y > 0) {
			m_player->destroy();
			lilypad->removeComponent<CBoundingBox>();
			state = "occupied";
			onLillypad = true;

			checkLilypadState();
			spawnLilypadFrogs({ tfm.pos.x, tfm.pos.y });
			checkIfWon();
		}
	}

	if (onLillypad && playerTfm.pos.y < waterLevelY) {
		checkIfWon();

		auto pos = m_worldView.getSize();
		pos.x = pos.x / 2.f;
		pos.y -= 20.f;
		gameTimer.restart();
		spawnPlayer(pos);
	}
	else if (!onLogOrTurtle && playerTfm.pos.y < waterLevelY) {
		auto& state = m_player->getComponent<CState>().state;
		state = "dead";

		checkPlayerState();
	}

	adjustPlayerPosition();
}

void Scene_Frogger::sUpdate(sf::Time dt) {
	SoundPlayer::getInstance().removeStoppedSounds();
	m_entityManager.update();

	if (m_isPaused)
		return;

	sf::Time elapsedTime = gameTimer.getElapsedTime();
	sf::Time remainingTime = std::max(timeLimit - elapsedTime, sf::Time::Zero);
	int seconds = static_cast<int>(remainingTime.asSeconds());
	std::string secondsLeft = std::to_string(seconds);
	timeRemaining.setString(secondsLeft);

	if (elapsedTime >= timeLimit) {
		m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("die"));
		m_player->destroy();
		m_player->addComponent<CState>("dead");

		auto pos = m_worldView.getSize();
		pos.x = pos.x / 2.f;
		pos.y -= 20.f;

		lives--;
		if (lives == 0) {
			onEnd();
		}

		drawLives(lives);
		spawnPlayer(pos);

		gameTimer.restart();
	}

	sAnimation(dt);
	sMovement(dt);
	sCollisions();
	sManageVehicles();
	sManageLogsAndTurtles();
	adjustPlayerPosition();
}

void Scene_Frogger::sAnimation(sf::Time dt) {
	auto list = m_entityManager.getEntities();
	for (auto e : m_entityManager.getEntities()) {
		// update all animations
		if (e->hasComponent<CAnimation>()) {
			auto& anim = e->getComponent<CAnimation>();
			anim.animation.update(dt);

			checkPlayerState();
		}
		// do nothing if animation has ended
	}
}


void Scene_Frogger::adjustPlayerPosition() {
	auto center = m_worldView.getCenter();
	sf::Vector2f viewHalfSize = m_worldView.getSize() / 2.f;


	auto left = center.x - viewHalfSize.x;
	auto right = center.x + viewHalfSize.x;
	auto top = center.y - viewHalfSize.y;
	auto bot = center.y + viewHalfSize.y;

	auto& player_pos = m_player->getComponent<CTransform>().pos;
	auto halfSize = sf::Vector2f{ 20, 20 };
	// keep player in bounds
	player_pos.x = std::max(player_pos.x, left + halfSize.x);
	player_pos.x = std::min(player_pos.x, right - halfSize.x);
	player_pos.y = std::max(player_pos.y, top + halfSize.y);
	player_pos.y = std::min(player_pos.y, bot - halfSize.y);
}

void Scene_Frogger::checkPlayerState() {
	auto& state = m_player->getComponent<CState>().state;

	if (m_player->hasComponent<CState>()) {
		if (state == "dead") {
			auto& deathAnimation = m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("die"));
			m_player->destroy();

			auto pos = m_worldView.getSize();
			pos.x = pos.x / 2.f;
			pos.y -= 20.f;
			lives--;

			if (lives == 0) {
				onEnd();
			}

			gameTimer.restart();
			drawLives(lives);
			spawnPlayer(pos);
		}
	}
}

void Scene_Frogger::checkLilypadState()
{
	auto& lilypads = m_entityManager.getEntities("lillypad");

	for (auto lilypad : lilypads) {
		auto& state = lilypad->getComponent<CState>().state;
		auto& tfm = lilypad->getComponent<CTransform>();

		if (state == "occupied") {
			lilypad->removeComponent<CBoundingBox>();
			m_player->destroy();
		}
	}
}

void Scene_Frogger::checkIfWon()
{
	auto& lillypads = m_entityManager.getEntities("lillypad");
	int occupiedLilyPads = 0;

	for (auto lilypad : lillypads) {
		auto& state = lilypad->getComponent<CState>().state;

		if (state == "occupied") {
			occupiedLilyPads++;
		}
		else {
			continue;
		}
	}

	if (occupiedLilyPads == 5) {
		onEnd();
	}
}

void Scene_Frogger::loadLevel(const std::string& path) {
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

			// for background, no textureRect its just the whole texture
			// and no center origin, position by top left corner
			// stationary so no CTransfrom required.
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
