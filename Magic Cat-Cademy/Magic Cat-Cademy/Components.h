//
// Created by David Burchill on 2023-09-27.
//

#ifndef BREAKOUT_COMPONENTS_H
#define BREAKOUT_COMPONENTS_H


#include <memory>
#include <SFML/Graphics.hpp>
#include "Utilities.h"
#include "Animation.h"
#include <bitset>


struct Component
{
    bool		has{ false };
    Component() = default;
};


struct CAnimation : public Component {
    Animation   animation;

    CAnimation() = default;
    CAnimation(const Animation& a) : animation(a) {}

};

struct CSprite : public Component {
    sf::Sprite sprite;

    CSprite() = default;

    CSprite(const sf::Texture &t)
            : sprite(t) {
        centerOrigin(sprite);
    }

    CSprite(const sf::Texture &t, sf::IntRect r)
            : sprite(t, r) {
        centerOrigin(sprite);
    }
};


struct CTransform : public Component
{

    sf::Transformable  tfm;
    sf::Vector2f	pos			{ 0.f, 0.f };
    sf::Vector2f	prevPos		{ 0.f, 0.f };
    sf::Vector2f	vel			{ 0.f, 0.f };
    sf::Vector2f	scale		{ 1.f, 1.f };

    float           angVel{ 0 };
    float	        angle{ 0.f };

    CTransform() = default;
    CTransform(const sf::Vector2f& p) : pos(p)  {}
    CTransform(const sf::Vector2f& p, const sf::Vector2f& v)
            : pos(p), prevPos(p),  vel(v){}

};



struct CBoundingBox : public Component
{
    sf::Vector2f size{0.f, 0.f};
    sf::Vector2f halfSize{ 0.f, 0.f };

    CBoundingBox() = default;
    CBoundingBox(const sf::Vector2f& s) : size(s), halfSize(0.5f * s)
    {}
};

struct CState : public Component {
    std::string state{"none"};

    CState() = default;
    CState(const std::string& s) : state(s){}
};

struct CInput : public Component
{
    enum dirs {
        UP = 1 << 0,
        DOWN = 1 << 1,
        LEFT = 1 << 2,
        RIGHT = 1 << 3
    };

    unsigned char dir{0};

    CInput() = default;
};

struct CGravity : public Component
{
    float g{ 0 };

    CGravity() = default;
    CGravity(float g) : g(g) {}
};

struct CHealth : public Component {
    int         hp{ 1 };

    CHealth() = default;
    CHealth(int hp) : hp(hp) {}
};

struct CMagic : public Component {
    sf::Time cooldown{ sf::Time::Zero };
    sf::Clock cooldownTimer;


    CMagic() = default;
    CMagic(sf::Time cooldown) : cooldown(cooldown) {};
};

struct CLifespan : public Component
{
    int total{ 0 };
    int remaining{ 0 };

    CLifespan() = default;
    CLifespan(int t) : total(t), remaining{ t } {}

};

struct CAttack : public Component
{
    bool isAttacking{ false };

    CAttack() = default;
    CAttack(bool isAttacking) : isAttacking(isAttacking) {}
};

struct CBoss : public Component
{
    bool isFireAttacking{ false };
    bool isAttacking{ false };
    bool isWalking{ false };
    bool isDodging{ false };
    bool isReturning{ false };

    CBoss() = default;
    CBoss(bool isFireAttacking, 
        bool isAttacking,
        bool isWalking, bool 
        isDodging) 
        : isFireAttacking(isFireAttacking), isAttacking(isAttacking), isWalking(isWalking), isDodging(isDodging), isReturning(isReturning) {}
};

struct CImmunity : public Component
{
public:
    float duration;
    bool isImmune;

    CImmunity() : duration(0.0f), isImmune(false) {};
    CImmunity(float immunityDuration) : duration(immunityDuration), isImmune(true) {};
};

struct CNPC : public Component {
public:
    bool isWalking;

    CNPC() : isWalking(false) {};
    CNPC(bool isWalking) : isWalking(isWalking) {};
};

#endif //BREAKOUT_COMPONENTSH
