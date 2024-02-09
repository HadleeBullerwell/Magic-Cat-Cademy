#include "Scene_Menu.h"
#include "Scene_MagicCatCademy.h"
#include "SoundPlayer.h"
#include "Entity.h"
#include "MusicPlayer.h"
#include <memory>
#include <iostream>
#include "Components.h"

void Scene_Menu::onEnd()
{
	m_game->window().close();
}

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	MusicPlayer::getInstance().play("mainMenuMusic");
	MusicPlayer::getInstance().setVolume(50);

	init();
}



void Scene_Menu::displayLogo(sf::Vector2f pos)
{
	auto logo = m_entityManager.addEntity("logo");
	logo->addComponent<CTransform>(pos);
	logo->addComponent<CAnimation>(Assets::getInstance().getAnimation("logo"));
}

void Scene_Menu::init()
{
	displayLogo(sf::Vector2f(1200, 375));

	registerAction(sf::Keyboard::W, "UP");
	registerAction(sf::Keyboard::Up, "UP");
	registerAction(sf::Keyboard::S, "DOWN");
	registerAction(sf::Keyboard::Down, "DOWN");
	registerAction(sf::Keyboard::D, "PLAY");
	registerAction(sf::Keyboard::Escape, "QUIT");

	m_title = "Magic Cat-Cademy";
	m_menuStrings.push_back("PLAY");
	m_menuStrings.push_back("SETTINGS");
	m_menuStrings.push_back("QUIT");

	m_levelPaths.push_back("../assets/level1.txt");
	m_levelPaths.push_back("../assets/level1.txt");
	m_levelPaths.push_back("../assets/level1.txt");

	m_menuText.setFont(Assets::getInstance().getFont("main"));

	const size_t CHAR_SIZE{ 64 };
	m_menuText.setCharacterSize(CHAR_SIZE);

}

void Scene_Menu::update(sf::Time dt)
{
	m_entityManager.update();
}

void Scene_Menu::sRender()
{
	sf::View view = m_game->window().getView();
	view.setCenter(m_game->window().getSize().x / 2.f, m_game->window().getSize().y / 2.f);
	m_game->window().setView(view);

	static const sf::Color selectedColor(255, 255, 255);
	static const sf::Color normalColor(0, 0, 0);

	static const sf::Color backgroundColor(126, 45, 162);

	m_game->window().clear(backgroundColor);

	m_menuText.setFillColor(normalColor);
	m_menuText.setString(m_title);
	m_menuText.setCharacterSize(75);
	m_menuText.setOrigin(m_menuText.getLocalBounds().width / 2, m_menuText.getLocalBounds().height / 2);
	m_menuText.setPosition(300, 225);
	m_game->window().draw(m_menuText);

	for (size_t i{ 0 }; i < m_menuStrings.size(); ++i)
	{
		sf::Text menuItem;
		menuItem.setFont(Assets::getInstance().getFont("main"));
		menuItem.setCharacterSize(50);
		menuItem.setFillColor((i == m_menuIndex ? selectedColor : normalColor));
		menuItem.setOrigin(menuItem.getLocalBounds().width / 2, menuItem.getLocalBounds().height / 2);
		menuItem.setPosition(menuItem.getPosition().x + 20 , 185 + (i + 1) * 96);
		menuItem.setString(m_menuStrings.at(i));
		m_game->window().draw(menuItem);
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
	}
}

void Scene_Menu::sDoAction(const Command& action)
{
	if (action.type() == "START")
	{
		if (action.name() == "UP")
		{
			m_menuIndex = (m_menuIndex + m_menuStrings.size() - 1) % m_menuStrings.size();
		}
		else if (action.name() == "DOWN")
		{
			m_menuIndex = (m_menuIndex + 1) % m_menuStrings.size();
		}
		else if (action.name() == "PLAY")
		{
			SoundPlayer::getInstance().play("select");
			m_game->changeScene("PLAY", std::make_shared<Scene_MagicCatCademy>(m_game, m_levelPaths[m_menuIndex]));
		}
		else if (action.name() == "QUIT")
		{
			onEnd();
		}
	}

}
