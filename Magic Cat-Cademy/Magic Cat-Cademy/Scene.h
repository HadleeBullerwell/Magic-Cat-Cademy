#pragma once

#include "EntityManager.h"
#include "GameEngine.h"
#include "Command.h"
#include <map>
#include <string>


using CommandMap = std::map<int, std::string>;
using ActionMap = std::map<std::string, Action::Actions>;

class Scene_MagicCatCademy;

class Scene
{

protected:

	GameEngine	    *m_game;
	EntityManager	m_entityManager;
	CommandMap		m_commands;
	ActionMap		m_actions;
	bool			m_isPaused{false};
	bool			m_hasEnded{false};
	size_t			m_currentFrame{ 0 };

	virtual void	onEnd() = 0;
	void			setPaused(bool paused);

public:
	Scene(GameEngine* gameEngine);
    virtual ~Scene();

	virtual void		update(sf::Time dt) = 0;
	virtual void		sDoAction(const Command& action) = 0;
	virtual void		sRender() = 0;

	void				simulate(int);
	void				doAction(Command);
	void				registerAction(int, std::string, Action::Actions type);
	const CommandMap	getActionMap() const;
};

