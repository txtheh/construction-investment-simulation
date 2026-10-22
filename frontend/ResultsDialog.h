#pragma once

#include "GameTypes.h"

#include <QDialog>
#include <QVector>

class ResultsDialog : public QDialog {
public:
    ResultsDialog(const QVector<BotInfo> &bots, int months, QWidget *parent = nullptr);
};
