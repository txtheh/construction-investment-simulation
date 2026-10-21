#include "GameSetupDialog.h"

#include <QComboBox>
#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace {
QStringList strategyNames() {
    return {
        QStringLiteral("Строитель"),
        QStringLiteral("Рыночная"),
        QStringLiteral("Сбалансированная"),
        QStringLiteral("Случайная")
    };
}

QStringList strategyIds() {
    return {
        QStringLiteral("BuilderPlayer"),
        QStringLiteral("MarketPlayer"),
        QStringLiteral("BalancedPlayer"),
        QStringLiteral("RandomPlayer")
    };
}
}

GameSetupDialog::GameSetupDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle(QStringLiteral("Новая игра"));
    setMinimumWidth(610);
    setModal(true);
    setStyleSheet(R"(
        QDialog { background: #111b25; color: #e6edf3; }
        QLabel { color: #dbe5ee; }
        QSpinBox, QComboBox, QTableWidget {
            background: #1b2936; color: #eef4f8; border: 1px solid #354657;
            border-radius: 5px; padding: 5px;
        }
        QHeaderView::section { background: #243442; color: #c8d5df; padding: 7px; border: none; }
        QTableWidget { gridline-color: #30404d; }
        QPushButton { background: #2878e5; color: white; border: none; border-radius: 6px; padding: 8px 15px; }
        QPushButton:hover { background: #3988f3; }
    )");

    auto *root = new QVBoxLayout(this);
    root->setSpacing(14);
    auto *title = new QLabel(QStringLiteral("Настройки симуляции"), this);
    title->setStyleSheet("font-size: 22px; font-weight: 700;");
    root->addWidget(title);

    auto *form = new QFormLayout;
    botCount_ = new QSpinBox(this);
    botCount_->setRange(2, 5);
    botCount_->setValue(4);
    months_ = new QComboBox(this);
    for (int month = 6; month <= 24; ++month) {
        QString label;
        if (month % 10 == 1 && month % 100 != 11) {
            label = QStringLiteral("%1 месяц").arg(month);
        } else if (month % 10 >= 2 && month % 10 <= 4
                   && (month % 100 < 12 || month % 100 > 14)) {
            label = QStringLiteral("%1 месяца").arg(month);
        } else {
            label = QStringLiteral("%1 месяцев").arg(month);
        }
        months_->addItem(label, month);
    }
    months_->setCurrentIndex(6);
    form->addRow(QStringLiteral("Количество ботов"), botCount_);
    form->addRow(QStringLiteral("Длительность"), months_);
    root->addLayout(form);

    auto *participantsTitle = new QLabel(QStringLiteral("Участники"), this);
    participantsTitle->setStyleSheet("font-size: 15px; font-weight: 600;");
    root->addWidget(participantsTitle);

    table_ = new QTableWidget(this);
    table_->setColumnCount(2);
    table_->setHorizontalHeaderLabels({QStringLiteral("Имя бота"), QStringLiteral("Стратегия")});
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table_->verticalHeader()->setVisible(false);
    table_->setSelectionMode(QAbstractItemView::NoSelection);
    table_->setMinimumHeight(205);
    root->addWidget(table_);

    auto *note = new QLabel(QStringLiteral("Цвет участника на городской карте назначается автоматически."), this);
    note->setStyleSheet("color: #91a4b5; font-size: 11px;");
    root->addWidget(note);

    auto *buttons = new QDialogButtonBox(this);
    auto *cancel = buttons->addButton(QStringLiteral("Отмена"), QDialogButtonBox::RejectRole);
    auto *start = buttons->addButton(QStringLiteral("Начать игру"), QDialogButtonBox::AcceptRole);
    cancel->setStyleSheet("background: #293947;");
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(start, &QPushButton::clicked, this, &QDialog::accept);
    root->addWidget(buttons);

    connect(botCount_, qOverload<int>(&QSpinBox::valueChanged), this,
            [this](int count) { rebuildBotRows(count); });
    rebuildBotRows(botCount_->value());
}

void GameSetupDialog::rebuildBotRows(int count) {
    QStringList previousNames;
    QStringList previousStrategies;
    for (int row = 0; row < table_->rowCount(); ++row) {
        QTableWidgetItem *name = table_->item(row, 0);
        previousNames.append(name ? name->text() : QString());
        auto *combo = qobject_cast<QComboBox *>(table_->cellWidget(row, 1));
        previousStrategies.append(combo ? combo->currentData().toString() : QStringLiteral("BuilderPlayer"));
    }

    table_->setRowCount(count);
    const QStringList labels = strategyNames();
    const QStringList ids = strategyIds();
    for (int row = 0; row < count; ++row) {
        QString name = row < previousNames.size() ? previousNames[row] : QString();
        if (name.trimmed().isEmpty()) {
            name = QStringLiteral("Бот %1").arg(row + 1);
        }
        table_->setItem(row, 0, new QTableWidgetItem(name));

        auto *strategy = new QComboBox(table_);
        for (int i = 0; i < labels.size(); ++i) {
            strategy->addItem(labels[i], ids[i]);
        }
        if (row < previousStrategies.size()) {
            int index = strategy->findData(previousStrategies[row]);
            if (index >= 0) {
                strategy->setCurrentIndex(index);
            }
        }
        table_->setCellWidget(row, 1, strategy);
    }
}

QVector<BotSetup> GameSetupDialog::botSetups() const {
    QVector<BotSetup> result;
    for (int row = 0; row < table_->rowCount(); ++row) {
        BotSetup setup;
        QTableWidgetItem *name = table_->item(row, 0);
        setup.name = name ? name->text().trimmed() : QString();
        if (setup.name.isEmpty()) {
            setup.name = QStringLiteral("Бот %1").arg(row + 1);
        }
        auto *strategy = qobject_cast<QComboBox *>(table_->cellWidget(row, 1));
        setup.strategy = strategy ? strategy->currentData().toString() : QStringLiteral("BalancedPlayer");
        result.append(setup);
    }
    return result;
}

int GameSetupDialog::gameMonths() const {
    return months_->currentData().toInt();
}
