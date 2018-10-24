#include "CardModel.h"


std::string intToStr(double n)
{
    return std::to_string(static_cast<int>(n));
}


double rad2deg(double rad)
{
    return rad * 180.0 / M_PI;
}


CardModel::CardModel()
    :
    _deltaX(0.0),
    _deltaY(0.0),
    _curAlpha(0.0),
    _gotoAlpha(0.0),
    _spdKoef(0.0),
    _commandForTurning(""),
    _commandForMoving(""),
    _commandForMovingBySecondTypeOfMoving(""),
    _arrayForTurningAllTelega({ { 30.0, -30.0, -90.0 } }),
    _speedForRiding(255.0),
    _radius(300.0),
    _maxSpeed(255.0),
    _dotZero(".0"),
    _space(" "),
    _zeroDotZero("0.0")
{}

void CardModel::setDeltas(double curX, double curY, double gotoX, double gotoY)
{
    _deltaX = gotoX - curX;
    _deltaY = gotoY - curY;
}

void CardModel::turnOnAngleForFirstMovingType()
{
    double alpha = getCourseAngle();
    double thisAngleSmaller;
    double orThisAngleSmaller;

    if (_curAlpha > alpha)
    {
        thisAngleSmaller = _curAlpha - alpha;
        orThisAngleSmaller = alpha - _curAlpha + 360.0;
    }
    else
    {
        thisAngleSmaller = alpha - _curAlpha;
        orThisAngleSmaller = _curAlpha - alpha + 360.0;
    }

    (thisAngleSmaller > orThisAngleSmaller) ? _gotoAlpha = orThisAngleSmaller : _gotoAlpha = thisAngleSmaller;

    (_gotoAlpha + _curAlpha == alpha) ? _spdKoef = -1.0 : _spdKoef = 1.0;

    auto arcLength = static_cast<int>(M_PI * _radius * _gotoAlpha / 180.0);
    _speedForRiding = _maxSpeed * _spdKoef;

    _curAlpha = alpha;

    if (arcLength == 0.0)
    {
        _commandForTurning = intToStr(_curAlpha) + _dotZero + _space + _zeroDotZero + _space + _zeroDotZero + _space;
        _commandForTurning += intToStr(_curAlpha) + _dotZero + _space + _zeroDotZero + _space + _zeroDotZero + _space;
        _commandForTurning += intToStr(_curAlpha) + _dotZero + _space + _zeroDotZero + _space + _zeroDotZero;
    }
    else
    {
        _commandForTurning =
            intToStr(_arrayForTurningAllTelega.at(0)) + _dotZero + _space + intToStr(_speedForRiding) + _dotZero +
            _space + intToStr(arcLength) + _dotZero + _space;
        _commandForTurning +=
            intToStr(_arrayForTurningAllTelega.at(1)) + _dotZero + _space + intToStr(-_speedForRiding) + _dotZero +
            _space + intToStr(arcLength) + _dotZero + _space;
        _commandForTurning +=
            intToStr(_arrayForTurningAllTelega.at(2)) + _dotZero + _space + intToStr(_speedForRiding) + _dotZero +
            _space + intToStr(arcLength) + _dotZero;
    }
}

void CardModel::goOnLineForFirstMovingType()
{
    double hypotenuse = sqrt(pow((_deltaX), 2) + pow((_deltaY), 2));
    if (hypotenuse == 0)
    {
        _commandForMoving = "ok";  // we do not ride anywhere
    }
    else
    {
        _commandForMoving =
            _zeroDotZero + _space + intToStr(_speedForRiding) + _dotZero + _space + intToStr(hypotenuse) +
            _dotZero + _space;
        _commandForMoving +=
            _zeroDotZero + _space + intToStr(_speedForRiding) + _dotZero + _space + intToStr(hypotenuse) +
            _dotZero + _space;
        _commandForMoving +=
            _zeroDotZero + _space + intToStr(_speedForRiding) + _dotZero + _space + intToStr(hypotenuse) + _dotZero;
    }
}

void CardModel::firstTypeOfMoving(double curX, double curY, double gotoX, double gotoY)
{
    setDeltas(curX, curY, gotoX, gotoY);

    turnOnAngleForFirstMovingType();
    goOnLineForFirstMovingType();

    std::cout << getCommandTurn() << '\n' << getCommandMove() << '\n';
}

void CardModel::angleForSecondTypeOfMoving()
{
    double alpha = getCourseAngle();
    if (abs(static_cast<int>(alpha)) <= 90.0)
    {
        _gotoAlpha = alpha;
        _spdKoef = 1.0;
    }
    else if (alpha < 180.0 && alpha > 90.0)
    {
        _gotoAlpha = alpha - 180;
        _spdKoef = -1;
    }
    else if (alpha > -180.0 && alpha < -90.0)
    {
        _gotoAlpha = 180 + alpha;
        _spdKoef = -1.0;
    }
    else if (abs(static_cast<int>(alpha)) == 180.0)
    {
        _gotoAlpha = 0.0;
        _spdKoef = -1.0;
    }
}

std::array<double, 9> CardModel::secondTypeOfMoving(double curX, double curY, double gotoX, double gotoY)
{
    setDeltas(curX, curY, gotoX, gotoY);
    angleForSecondTypeOfMoving();


    double hypotenuse = static_cast<int>(sqrt(pow((_deltaX), 2) + pow((_deltaY), 2)));
    _speedForRiding = _maxSpeed * _spdKoef;

    /*_commandForMovingBySecondTypeOfMoving =
        intToStr(_gotoAlpha) + _dotZero + _space + intToStr(_speedForRiding) + _dotZero + _space +
        intToStr(hypotenuse) + _dotZero + _space;
    _commandForMovingBySecondTypeOfMoving +=
        intToStr(_gotoAlpha) + _dotZero + _space + intToStr(_speedForRiding) + _dotZero + _space +
        intToStr(hypotenuse) + _dotZero + _space;
    _commandForMovingBySecondTypeOfMoving +=
        intToStr(_gotoAlpha) + _dotZero + _space + intToStr(_speedForRiding) + _dotZero + _space +
        intToStr(hypotenuse) + _dotZero;*/

    std::array<double, 9> coordinates = { { _gotoAlpha, -_speedForRiding, hypotenuse,
        _gotoAlpha,-_speedForRiding, hypotenuse,  _gotoAlpha, -_speedForRiding, hypotenuse} };

    return coordinates;
}


std::string CardModel::getCommandMoveBySecondType()
{
    return _commandForMovingBySecondTypeOfMoving;
}

std::string CardModel::getCommandTurn()
{
    return _commandForTurning;
}

std::string CardModel::getCommandMove()
{
    return _commandForMoving;
}

double CardModel::getCourseAngle()
{
    return rad2deg(atan2(_deltaY, _deltaX));
}