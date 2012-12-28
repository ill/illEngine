#ifndef __GAME_H__
#define __GAME_H__

namespace Demo {

class GameControllerBase
{
public:
    virtual ~GameControllerBase() {}

    virtual void update(float seconds) = 0;
    virtual void updateSound(float seconds) = 0;
    virtual void render() = 0;

protected:
    GameControllerBase() {}
};

}

#endif