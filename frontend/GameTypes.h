#pragma once

#include <QColor>
#include <QString>
#include <QVector>

struct BotInfo {
    QString name;
    QString strategy;
    QColor color;
    double cash = 37000000.0;
    double totalCapital = 37000000.0;
    int houses = 0;
    int shops = 0;
    int underConstruction = 0;
    int completed = 0;
};

struct BotSetup {
    QString name;
    QString strategy;
};
