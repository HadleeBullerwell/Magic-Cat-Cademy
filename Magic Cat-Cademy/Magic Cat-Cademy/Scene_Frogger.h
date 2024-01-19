//
// Created by David Burchill on 2023-09-27.
//

#ifndef BREAKOUT_SCENE_BREAKOUT_H
#define BREAKOUT_SCENE_BREAKOUT_H

#include <SFML/System/Time.hpp>
#include <SFML/Graphics.hpp>
#include "EntityManager.h"
#include "Entity.h"
#include "Scene.h"
#include "GameEngine.h"



class Scene_Frogger : public Scene {
private:
    sPtrEntt        m_player{nullptr};
    sf::View        m_worldView;
    sf::FloatRect   m_worldBounds;

    bool			m_drawTextures{true};
    bool			m_drawAABB{false};
    bool			m_drawGrid{false};

    sf::Clock       gameTimer;
    sf::Time        timeLimit = sf::seconds(30);
    sf::Text        timeRemaining{};
    sf::Text        gameTimeText{};

    sf::Text        scoreText{};
    sf::Text        scoreNumber{};
    int             score = 0;

    sf::Text        highScoreText{};  
    sf::Text        highScoreNumber{};
    int             highScore = 0;
    int             highestScore = 0;

    void            drawLives(int lives);
    int             lives = 3;

    //systems
    void            sMovement(sf::Time dt);
    void            sCollisions();
    void            sUpdate(sf::Time dt);
    void            sAnimation(sf::Time dt);
    void            sManageVehicles();
    void            sManageLogsAndTurtles();

    void	        onEnd() override;


    // helper functions
    void            playerMovement();
    void            adjustPlayerPosition();
    void            checkPlayerState();
    void            checkLilypadState();
    void            checkIfWon();
    void	        registerActions();
    void            spawnPlayer(sf::Vector2f pos);
    void            spawnLilypadFrogs(sf::Vector2f lilyPadPos);
    void            spawnVehicles();
    void            spawnLogsAndTurtles();
    void            spawnLogsLane1();
    void            spawnLogsLane2();
    void            spawnLogsLane3();
    void            spawnTurtleLane1();
    void            spawnTurtleLane2();
    void            spawnLane1();
    void            spawnLane2();
    void            spawnLane3();
    void            spawnLane4();
    void            spawnLane5();
    void            spawnLilyPads();
 
    void            init();
    void            loadLevel(const std::string &path);
    sf::FloatRect   getViewBounds();

public:

    Scene_Frogger(GameEngine *gameEngine, const std::string &levelPath);

    void		  update(sf::Time dt) override;
    void		  sDoAction(const Command& command) override;
    void		  sRender() override;

};



#endif //BREAKOUT_SCENE_BREAKOUT_H
