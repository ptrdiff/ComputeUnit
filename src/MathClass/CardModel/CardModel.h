#ifndef CARD_MODEL_H
#define CARD_MODEL_H
#include <cmath>
#include <cstdlib>
#include <string>
#include <iostream>
#include <array>

class CardModel
{
private:
    double _deltaX;
    double _deltaY;
    double _curAlpha;
    double _gotoAlpha;
    double _spdKoef;
    std::string _commandForTurning;
    std::string _commandForMoving;
    std::string _commandForMovingBySecondTypeOfMoving;

    std::array<double, 3> _arrayForTurningAllTelega;
    double _speedForRiding;
    const double _radius;
    const double _maxSpeed;
    const std::string _dotZero;
    const std::string _space;
    const std::string _zeroDotZero;

    double getCourseAngle();

public:
    CardModel();
    void setDeltas(double curX, double curY, double gotoX, double gotoY);
    void turnOnAngleForFirstMovingType();
    void goOnLineForFirstMovingType();
    void firstTypeOfMoving(double curX, double curY, double gotoX, double gotoY);
    std::array<double, 9> secondTypeOfMoving(double curX, double curY, double gotoX, double gotoY);
    void angleForSecondTypeOfMoving();

    std::string getCommandTurn();
    std::string getCommandMove();
    std::string getCommandMoveBySecondType();
};

double rad2deg(double rad);
std::string intToStr(double n);
#endif // CARD_MODEL_H