#pragma once

#include "GameTypes.h"

#include <QDialog>
#include <QVector>

class QSpinBox;
class QTableWidget;

class GameSetupDialog : public QDialog {
public:
    explicit GameSetupDialog(QWidget *parent = nullptr);

    QVector<BotSetup> botSetups() const;
    int gameMonths() const;

private:
    void rebuildBotRows(int count);

    QSpinBox *botCount_ = nullptr;
    QSpinBox *months_ = nullptr;
    QTableWidget *table_ = nullptr;
};
