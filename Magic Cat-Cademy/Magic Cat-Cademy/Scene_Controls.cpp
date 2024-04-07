#include "Scene_Controls.h"
#include "Scene_Controls.h"
#include "Scene_Menu.h"
#include "Scene_MagicCatCademy.h"
#include "SoundPlayer.h"
#include "Entity.h"
#include "MusicPlayer.h"
#include <memory>
#include <iostream>
#include <istream>
#include "Components.h"

void Scene_Controls::displayBackground(sf::Vector2f pos)
{
	auto e = m_entityManager.addEntity("bkg");
	auto& sprite = e->addComponent<CSprite>(Assets::getInstance().getTexture("ControlsBackground")).sprite;
	sprite.setOrigin(0.f, 0.f);
	sprite.setPosition(pos);
}

void Scene_Controls::sAnimation(sf::Time dt)
{
	auto list = m_entityManager.getEntities();
	for (auto e : m_entityManager.getEntities()) {
		// update all animations
		if (e->hasComponent<CAnimation>()) {
			auto& anim = e->getComponent<CAnimation>();
			anim.animation.update(dt);
		}
	}
}

void Scene_Controls::init()
{
	displayBackground(sf::Vector2f(0, 0));

	registerAction(sf::Keyboard::D, "BACK", Action::Keyboard);

	m_worldViewPosition.x = (m_game->window().getSize().x / 2.f) - 55;
	m_worldViewPosition.y = (m_game->window().getSize().y / 2.f) + 315;

	backButton.setString("BACK");
	backButton.setCharacterSize(50);
	backButton.setOutlineThickness(3);
	backButton.setOrigin(backButton.getLocalBounds().width / 2, backButton.getLocalBounds().height / 2);
	backButton.setPosition(m_worldViewPosition);
}

void Scene_Controls::onEnd()
{
	m_game->window().close();
}

Scene_Controls::Scene_Controls(GameEngine* gameEngine) 
	: Scene(gameEngine)
	, m_worldView(gameEngine->window().getDefaultView())
{
	init();
}

void Scene_Controls::update(sf::Time dt)
{
	m_entityManager.update();

	sAnimation(dt);
}

void Scene_Controls::sRender()
{
	m_game->window().setView(m_worldView);

	for (auto e : m_entityManager.getEntities("bkg")) {
		if (e->getComponent<CSprite>().has) {
			auto& sprite = e->getComponent<CSprite>().sprite;
			m_game->window().draw(sprite);
		}
	}

	static const sf::Color selectedColor(255, 255, 255);
	static const sf::Color outlineColor(54, 1, 77);
	static const sf::Color normalColor(177, 100, 245);

	backButton.setFont(Assets::getInstance().getFont("main"));
	backButton.setFillColor(selectedColor);
	backButton.setOutlineColor(outlineColor);
	backButton.setPosition(m_worldViewPosition);

	m_game->window().draw(backButton);
}

void Scene_Controls::sDoAction(const Command& action)
{
	if (action.type() == "START") {
		
		if (action.name() == "BACK") {
			m_game->changeScene("MENU", std::make_shared<Scene_Menu>(m_game));
		}
	}
}
