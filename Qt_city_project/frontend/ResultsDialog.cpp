#include "ResultsDialog.h"

#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLocale>
#include <QStringList>
#include <QTableWidgetItem>
#include <QtMath>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <algorithm>

namespace {
QString moneyText(double value) {
    return QLocale(QLocale::Russian).toString(value / 1000000.0, 'f', 1) + QStringLiteral(" млн");
}
}

ResultsDialog::ResultsDialog(const QVector<BotInfo> &bots, int months, QWidget *parent)
    : QDialog(parent) {
    setWindowTitle(QStringLiteral("Итоги симуляции"));
    setMinimumSize(720, 520);
    setStyleSheet(R"(
        QDialog { background: #111b25; color: #e6edf3; }
        QLabel { color: #dbe5ee; }
        QTableWidget { background: #1b2936; color: #e6edf3; gridline-color: #30404d; border: 1px solid #354657; }
        QHeaderView::section { background: #253442; color: #c7d4df; border: none; padding: 8px; }
        QPushButton { background: #2878e5; color: white; border: none; border-radius: 6px; padding: 8px 15px; }
        QPushButton:hover { background: #3988f3; }
    )");

    QVector<BotInfo> sortedBots = bots;
    std::sort(sortedBots.begin(), sortedBots.end(), [](const BotInfo &left, const BotInfo &right) {
        return left.totalCapital > right.totalCapital;
    });

    auto *root = new QVBoxLayout(this);
    auto *title = new QLabel(QStringLiteral("ИТОГИ СИМУЛЯЦИИ"), this);
    title->setStyleSheet("font-size: 24px; font-weight: 700;");
    root->addWidget(title);

    auto *duration = new QLabel(QStringLiteral("Симуляция завершена · %1 месяцев").arg(months), this);
    duration->setStyleSheet("color: #94a7b7;");
    root->addWidget(duration);

    QString winnerText = QStringLiteral("Победитель не определён");
    if (!sortedBots.isEmpty()) {
        QStringList winners;
        double bestCapital = sortedBots.first().totalCapital;
        for (const BotInfo &bot : sortedBots) {
            if (qAbs(bot.totalCapital - bestCapital) < 0.01) {
                winners.append(bot.name);
            }
        }
        winnerText = winners.size() > 1
                         ? QStringLiteral("Ничья: %1").arg(winners.join(QStringLiteral(", ")))
                         : QStringLiteral("Лидер по капиталу: %1").arg(winners.first());
    }

    auto *winner = new QLabel(winnerText, this);
    winner->setStyleSheet("font-size: 18px; font-weight: 600; color: #f1b84b; background: #1b2936; border: 1px solid #334756; border-radius: 7px; padding: 13px;");
    root->addWidget(winner);

    auto *table = new QTableWidget(this);
    table->setColumnCount(7);
    table->setHorizontalHeaderLabels({
        QStringLiteral("Место"), QStringLiteral("Участник"), QStringLiteral("Деньги"),
        QStringLiteral("Итоговый капитал"), QStringLiteral("Дома"),
        QStringLiteral("Магазины"), QStringLiteral("Завершено")
    });
    table->setRowCount(sortedBots.size());
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);

    for (int row = 0; row < sortedBots.size(); ++row) {
        const BotInfo &bot = sortedBots[row];
        QStringList values = {
            QString::number(row + 1), bot.name, moneyText(bot.cash),
            moneyText(bot.totalCapital), QString::number(bot.houses),
            QString::number(bot.shops), QString::number(bot.completed)
        };
        for (int column = 0; column < values.size(); ++column) {
            auto *item = new QTableWidgetItem(values[column]);
            if (column == 1) {
                item->setForeground(bot.color);
            }
            table->setItem(row, column, item);
        }
    }
    root->addWidget(table, 1);

    auto *buttons = new QDialogButtonBox(this);
    auto *newGame = buttons->addButton(QStringLiteral("Новая игра"), QDialogButtonBox::AcceptRole);
    auto *menu = buttons->addButton(QStringLiteral("В главное меню"), QDialogButtonBox::RejectRole);
    connect(newGame, &QPushButton::clicked, this, &QDialog::accept);
    connect(menu, &QPushButton::clicked, this, &QDialog::reject);
    root->addWidget(buttons);
}
