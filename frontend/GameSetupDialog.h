#pragma once

#include "GameTypes.h"

#include <QDialog>
#include <QVector>

class QSpinBox;
class QTableWidget;

class GameSetupDialog : public QDialog {
public:
    explicit GameSetupDialog(int gridSize, QWidget *parent = nullptr);

    QVector<BotSetup> botSetups() const;
    int gameMonths() const;
    int maxBuildings() const;

private:
    void rebuildBotRows(int count);

    QSpinBox *botCount_ = nullptr;
    QSpinBox *months_ = nullptr;
    QSpinBox *maxBuildings_ = nullptr;
    QTableWidget *table_ = nullptr;
};
