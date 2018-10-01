#include "CardModel.h"

std::string intToStr(double n)
{
    return std::to_string(static_cast<int>(n));
}


CardModel::CardModel(double curX, double curY)
    :
    _curX(curX),
    _curY(curY),
    _curAlpha(0.0),
    _gotoAlpha(0.0),
    _spdKoef(0.0),
    _commandForTurning(""),
    _commandForMoving(""),
    _radius(280.0)
{

}

QVector<double> CardModel::linerMoving(QVector<double> params)
{
    _curX = params[0];
    _curY = params[1];
    _gotoX = params[2];
    _gotoY = params[3];

    angleForsecondTypeOfMoving();

    double hypotenuse = sqrt(pow((_gotoX - _curX), 2) + pow((_gotoY - _curY), 2));
    _spdKoef *= 255.0;

    return { _gotoAlpha, _spdKoef, hypotenuse, _gotoAlpha, _spdKoef, hypotenuse,
        _gotoAlpha, _spdKoef, hypotenuse };
}

QVector<double> CardModel::RotationMoving(QVector<double> params)
{
    _curAlpha = params[0];
    _gotoAlpha = params[1];

    double alpha = atan2(_gotoY - _curY, _gotoX - _curX) * 180.0 / PI;
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

    double arcLength = round(PI * _radius * _gotoAlpha / 180.0 * 10.0) / 10.0;
    _spdKoef = _spdKoef * 255.0;

    _curAlpha = alpha;
    std::cout << alpha << " alpha _gotoAlpha " << _gotoAlpha << "\n";
    if (arcLength == 0.0)
    {
        return { _curAlpha, 0., 0. , _curAlpha, 0. ,0. , _curAlpha, 0., 0.};
    }
    else
    {
        return { 30., _spdKoef, arcLength, -30., -_spdKoef, arcLength, -90., _spdKoef, arcLength };
    }
}

void CardModel::setPoint(double  gotoX, double gotoY)
{
    _gotoX = gotoX;
    _gotoY = gotoY;
}

void CardModel::turnOnAngle()
{
    double alpha = atan2(_gotoY - _curY, _gotoX - _curX) * 180.0 / PI;
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

    double arcLength = round(PI * _radius * _gotoAlpha / 180.0 * 10.0) / 10.0;
    _spdKoef = _spdKoef * 255.0;

    _curAlpha = alpha;
    std::cout << alpha << " alpha _gotoAlpha " << _gotoAlpha << "\n";
    if (arcLength == 0.0)
    {
        _commandForTurning = intToStr(_curAlpha) + ".0 0.0 0.0 ";
        _commandForTurning += intToStr(_curAlpha) + ".0 0.0 0.0 ";
        _commandForTurning += intToStr(_curAlpha) + ".0 0.0 0.0";
    }
    else
    {
        _commandForTurning = "30.0 " + intToStr(_spdKoef) + ".0 " + intToStr(arcLength) + ".0 ";
        _commandForTurning += "-30.0 " + intToStr(-_spdKoef) + ".0 " + intToStr(arcLength) + ".0 ";
        _commandForTurning += "-90.0 " + intToStr(_spdKoef) + ".0 " + intToStr(arcLength) + ".0";

    }
}

void CardModel::goOnLine()
{
    double hypotenuse = sqrt(pow((_gotoX - _curX), 2) + pow((_gotoY - _curY), 2));
    if (hypotenuse == 0)
    {
        _commandForMoving = "ok";
    }
    else
    {
        _commandForMoving = "0.0 -255.0 " + intToStr(hypotenuse) + ".0 ";
        _commandForMoving += "0.0 -255.0 " + intToStr(hypotenuse) + ".0 ";
        _commandForMoving += "0.0 -255.0 " + intToStr(hypotenuse) + ".0";
    }
}

void CardModel::pognali(double gotoX, double gotoY)
{
    setPoint(gotoX, gotoY);

    turnOnAngle();
    goOnLine();

    _curX = _gotoX;
    _curY = _gotoY;

    std::string command1 = getCommandTurn();
    std::string command2 = getCommandMove();


    std::cout << command1 << "\n";//отправляем дальше

    std::cout << command2 << "\n";//отправляем дальше
}

void CardModel::angleForsecondTypeOfMoving()
{
    double alpha = atan2(_gotoY - _curY, _gotoX - _curX) * 180.0 / PI;

    if (abs(alpha) <= 90.0)
    {
        _spdKoef = 1.0;
    }
    if (alpha < 180.0 && alpha > 90.0)
    {
        _gotoAlpha = alpha - 180;
        _spdKoef = -1;
    }
    if (alpha > -180.0 && alpha < -90.0)
    {
        _gotoAlpha = 180 + alpha;
        _spdKoef = -1.0;
    }
    if (abs(alpha) == 180.0)
    {
        _gotoAlpha = 0.0;
        _spdKoef = -1.0;
    }
}

void CardModel::secondTypeOfMoving(double gotoX, double gotoY)
{
    setPoint(gotoX, gotoY);
    angleForsecondTypeOfMoving();


    double hypotenuse = sqrt(pow((_gotoX - _curX), 2) + pow((_gotoY - _curY), 2));
    _spdKoef *= 255.0;
    _commandForMovingBySecondTypeOfMoving = intToStr(_gotoAlpha) + ".0 " + intToStr(_spdKoef) + ".0 " + intToStr(hypotenuse) + ".0 ";
    _commandForMovingBySecondTypeOfMoving += intToStr(_gotoAlpha) + ".0 " + intToStr(_spdKoef) + ".0 " + intToStr(hypotenuse) + ".0 ";
    _commandForMovingBySecondTypeOfMoving += intToStr(_gotoAlpha) + ".0 " + intToStr(_spdKoef) + ".0 " + intToStr(hypotenuse) + ".0";

    std::cout << grtCommandMoveBySecondType() << "\n";

}

std::string CardModel::grtCommandMoveBySecondType()
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