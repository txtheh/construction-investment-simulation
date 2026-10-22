#include "MainWindow.h"

#include "GameSetupDialog.h"
#include "ResultsDialog.h"

#include "../backend/player/BalancedPlayer.h"
#include "../backend/player/BuilderPlayer.h"
#include "../backend/player/MarketPlayer.h"
#include "../backend/player/RandomPlayer.h"

#include <QApplication>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPoint>
#include <QPushButton>
#include <QScrollArea>
#include <QSet>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QPainter>
#include <QPolygonF>
#include <QHeaderView>
#include <QGraphicsDropShadowEffect>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QComboBox>
#include <QStyle>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QStringList>
#include <QtMath>
#include <algorithm>
#include <random>
#include <stdexcept>

namespace {
const QVector<QColor> botColors = {
    QColor("#2878e5"), QColor("#ff8a1f"), QColor("#19aa68"),
    QColor("#8c5be8"), QColor("#e04b4b")
};

QString moneyText(double value) {
    return QLocale(QLocale::Russian).toString(value / 1000000.0, 'f', 1) + QStringLiteral(" млн");
}

QLabel *makeValueLabel(const QString &value, QWidget *parent) {
    auto *label = new QLabel(value, parent);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    label->setStyleSheet("color: #edf3f7; font-weight: 600;");
    return label;
}

void addStatRow(QGridLayout *layout, int row, const QString &name,
                const QString &value, QWidget *parent) {
    auto *nameLabel = new QLabel(name, parent);
    nameLabel->setStyleSheet("color: #9dafbd; font-size: 11px;");
    layout->addWidget(nameLabel, row, 0);
    layout->addWidget(makeValueLabel(value, parent), row, 1);
}

class StoreLegendIcon : public QWidget {
public:
    explicit StoreLegendIcon(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedSize(26, 22);
        setToolTip(QStringLiteral("Магазин: низкое здание с витринами и козырьком"));
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event)
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(QColor("#d7e1e8"), 1.0));
        painter.setBrush(QColor("#d7e1e8"));
        painter.drawRect(QRectF(4.0, 9.0, 18.0, 11.0));

        QPolygonF awning;
        awning << QPointF(3.0, 5.0) << QPointF(23.0, 5.0)
               << QPointF(21.0, 10.0) << QPointF(5.0, 10.0);
        painter.setPen(QPen(QColor("#b96e19"), 0.7));
        painter.setBrush(QColor("#ffb547"));
        painter.drawPolygon(awning);
        painter.setPen(QPen(QColor("#fff0cd"), 1.2));
        painter.drawLine(QPointF(8.0, 6.0), QPointF(8.0, 9.0));
        painter.drawLine(QPointF(13.0, 6.0), QPointF(13.0, 9.0));
        painter.drawLine(QPointF(18.0, 6.0), QPointF(18.0, 9.0));

        painter.setPen(QPen(QColor("#486271"), 0.7));
        painter.setBrush(QColor("#8cc6d9"));
        painter.drawRect(QRectF(6.0, 12.0, 8.0, 5.0));
        painter.setBrush(QColor("#a3b1bd"));
        painter.drawRect(QRectF(16.0, 12.0, 4.0, 8.0));
    }
};

class MiniMapWidget : public QWidget {
public:
    explicit MiniMapWidget(QWidget *parent = nullptr)
        : QWidget(parent) {
        setMinimumHeight(78);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    }

    void setGridSize(int size) {
        gridSize_ = qBound(12, size, 20);
        update();
    }

    void setBuildings(const QVector<BuildingVisual> &buildings, const QVector<QColor> &colors) {
        buildings_ = buildings;
        colors_ = colors;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event)
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillRect(rect(), QColor("#14202a"));
        const qreal cellWidth = (width() - 14.0) / gridSize_;
        const qreal cellHeight = (height() - 12.0) / gridSize_;
        for (int row = 0; row < gridSize_; ++row) {
            for (int column = 0; column < gridSize_; ++column) {
                QRectF cell(7 + column * cellWidth, 6 + row * cellHeight,
                            cellWidth - 1.2, cellHeight - 1.2);
                if (row % 4 == 0 || column % 4 == 0) {
                    painter.setBrush(QColor("#475563"));
                } else {
                    painter.setBrush(QColor("#63834d"));
                }
                painter.setPen(Qt::NoPen);
                painter.drawRect(cell);
            }
        }
        for (const BuildingVisual &building : buildings_) {
            if (building.row < 0 || building.row >= gridSize_ ||
                building.column < 0 || building.column >= gridSize_) {
                continue;
            }
            QColor color("#9aa7b1");
            if (building.owner >= 0 && building.owner < colors_.size()) {
                color = colors_[building.owner];
            }
            const qreal x = 7 + building.column * cellWidth;
            const qreal y = 6 + building.row * cellHeight;
            painter.setPen(Qt::NoPen);
            painter.setBrush(color);
            if (building.type == BuildingVisual::Type::Shop) {
                QRectF storefront(x + cellWidth * 0.16, y + cellHeight * 0.35,
                                  cellWidth * 0.68, cellHeight * 0.42);
                painter.drawRect(storefront);
                painter.setBrush(QColor("#f1e5c8"));
                painter.drawRect(QRectF(storefront.x(), storefront.y(), storefront.width(), cellHeight * 0.13));
                painter.setPen(QPen(QColor("#f1e5c8"), 1.0));
                painter.drawLine(QPointF(storefront.center().x(), storefront.y() + cellHeight * 0.13),
                                 QPointF(storefront.center().x(), storefront.bottom()));
            } else {
                QPolygonF roof;
                roof << QPointF(x + cellWidth * 0.18, y + cellHeight * 0.48)
                     << QPointF(x + cellWidth * 0.50, y + cellHeight * 0.18)
                     << QPointF(x + cellWidth * 0.82, y + cellHeight * 0.48);
                painter.drawPolygon(roof);
                painter.drawRect(QRectF(x + cellWidth * 0.27, y + cellHeight * 0.46,
                                        cellWidth * 0.46, cellHeight * 0.35));
            }
        }
    }

private:
    QVector<BuildingVisual> buildings_;
    QVector<QColor> colors_;
    int gridSize_ = 16;
};

QWidget *makeBottomPanel(const QString &titleText, QWidget *content) {
    auto *panel = new QFrame;
    panel->setObjectName("bottomPanel");
    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(10, 7, 10, 8);
    layout->setSpacing(6);
    auto *title = new QLabel(titleText, panel);
    title->setStyleSheet("font-size: 12px; font-weight: 700; color: #dce6ed;");
    layout->addWidget(title);
    layout->addWidget(content, 1);
    return panel;
}

std::unique_ptr<Player> makeBot(const QString &strategy, int id,
                                 const std::string &name, const Config &config) {
    if (strategy == QLatin1String("BuilderPlayer")) {
        return std::make_unique<BuilderPlayer>(id, name, config);
    }
    if (strategy == QLatin1String("MarketPlayer")) {
        return std::make_unique<MarketPlayer>(id, name, config);
    }
    if (strategy == QLatin1String("BalancedPlayer")) {
        return std::make_unique<BalancedPlayer>(id, name, config);
    }
    return std::make_unique<RandomPlayer>(id, name, config);
}

BuildingVisual toVisual(const BuildingView &bv, int owner,
                        int gridRow, int gridCol) {
    BuildingVisual v;
    v.owner = owner;
    v.row = gridRow;
    v.column = gridCol;
    v.type = (bv.buildingType == BuildingType::Supermarket)
                 ? BuildingVisual::Type::Shop
                 : BuildingVisual::Type::House;
    v.houseType = static_cast<int>(bv.houseType);

    if (bv.isBuilt) {
        v.stage = 4;
    } else if (bv.constructionMonths <= 0) {
        v.stage = 0;
    } else {
        const qreal progress =
            static_cast<qreal>(bv.monthsBuilt) / bv.constructionMonths;
        v.stage = (progress < 0.25) ? 0
                : (progress < 0.50) ? 1
                : (progress < 0.75) ? 2
                                    : 3;
    }
    v.apartmentsSold = bv.soldApartments;
    return v;
}
}  // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("Мегаполис: инвестиционное моделирование"));
    setMinimumSize(1280, 800);
    resize(1600, 960);
    setStyleSheet(R"(
        QMainWindow, QWidget { background: #111b25; color: #e6edf3; font-family: Arial; }
        QFrame#topBar { background: #17232e; border: 1px solid #2b3d4b; border-radius: 8px; }
        QFrame#statPanel { background: #1b2936; border: 1px solid #314352; border-radius: 7px; }
        QFrame#bottomPanel { background: #17232e; border: 1px solid #2b3d4b; border-radius: 7px; }
        QPushButton { background: #263846; color: #e7eef4; border: 1px solid #3a4c5a; border-radius: 5px; padding: 7px 11px; }
        QPushButton:hover { background: #344b5c; }
        QPushButton:pressed { background: #1b2a36; }
        QPushButton#primaryButton { background: #2878e5; border-color: #3988f3; color: white; font-weight: 600; }
        QPushButton#primaryButton:hover { background: #3988f3; }
        QProgressBar { background: #293a48; border: none; border-radius: 4px; height: 8px; text-align: center; }
        QProgressBar::chunk { background: #2878e5; border-radius: 4px; }
        QScrollArea { border: none; background: transparent; }
        QPlainTextEdit { background: transparent; color: #9dafbd; border: none; font-size: 11px; }
        QScrollBar:vertical { background: #17232e; width: 8px; margin: 0; }
        QScrollBar::handle:vertical { background: #3b4e5c; border-radius: 4px; min-height: 24px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    )");

    pages_ = new QStackedWidget(this);
    setCentralWidget(pages_);
    menuPage_ = createMenuPage();
    gamePage_ = createGamePage();
    pages_->addWidget(menuPage_);
    pages_->addWidget(gamePage_);
    pages_->setCurrentWidget(menuPage_);

    autoplayTimer_ = new QTimer(this);
    autoplayTimer_->setInterval(650);
    connect(autoplayTimer_, &QTimer::timeout, this, [this]() { advanceMonth(); });
}

QWidget *MainWindow::createMenuPage() {
    auto *page = new QWidget(this);
    page->setObjectName("menuPage");
    page->setStyleSheet("QWidget#menuPage { background: #101923; }");
    auto *outer = new QVBoxLayout(page);
    outer->setContentsMargins(30, 25, 30, 25);
    outer->addStretch(2);

    auto *center = new QWidget(page);
    center->setMinimumWidth(340);
    center->setMaximumWidth(570);
    auto *centerLayout = new QVBoxLayout(center);
    centerLayout->setSpacing(18);

    auto *eyebrow = new QLabel(QStringLiteral("ГОРОДСКАЯ ЭКОНОМИЧЕСКАЯ СИМУЛЯЦИЯ"), center);
    eyebrow->setAlignment(Qt::AlignCenter);
    eyebrow->setStyleSheet("color: #6f8da3; font-size: 11px; font-weight: 600; letter-spacing: 3px;");
    centerLayout->addWidget(eyebrow);

    auto *accent = new QFrame(center);
    accent->setFixedSize(74, 3);
    accent->setStyleSheet("background: #2878e5; border: none; border-radius: 1px;");
    centerLayout->addWidget(accent, 0, Qt::AlignHCenter);

    auto *newGame = new QPushButton(QStringLiteral("НОВАЯ ИГРА"), center);
    newGame->setObjectName("primaryButton");
    newGame->setMinimumHeight(46);
    auto *settings = new QPushButton(QStringLiteral("НАСТРОЙКИ"), center);
    settings->setMinimumHeight(42);
    auto *exit = new QPushButton(QStringLiteral("ВЫХОД"), center);
    exit->setMinimumHeight(42);
    centerLayout->addSpacing(12);
    centerLayout->addWidget(newGame);
    centerLayout->addWidget(settings);
    centerLayout->addWidget(exit);

    outer->addWidget(center, 0, Qt::AlignHCenter);
    outer->addStretch(3);

    connect(newGame, &QPushButton::clicked, this, [this]() { openSetupDialog(); });
    connect(settings, &QPushButton::clicked, this, [this]() { openSettingsDialog(); });
    connect(exit, &QPushButton::clicked, this, &QWidget::close);
    return page;
}

QWidget *MainWindow::createGamePage() {
    auto *page = new QWidget(this);
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(8, 7, 8, 8);
    root->setSpacing(7);

    auto *topBar = new QFrame(page);
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(52);
    auto *top = new QHBoxLayout(topBar);
    top->setContentsMargins(12, 4, 10, 4);
    top->setSpacing(8);

    top->addSpacing(8);
    monthLabel_ = new QLabel(QStringLiteral("Месяц: 0 / 12"), topBar);
    monthLabel_->setStyleSheet("font-weight: 600; min-width: 92px;");
    top->addWidget(monthLabel_);
    monthProgress_ = new QProgressBar(topBar);
    monthProgress_->setRange(0, 12);
    monthProgress_->setValue(0);
    monthProgress_->setFixedWidth(135);
    top->addWidget(monthProgress_);
    top->addStretch();

    auto *nextMonthButton = new QPushButton(QStringLiteral("▶  Следующий месяц"), topBar);
    nextMonthButton->setObjectName("primaryButton");
    autoplayButton_ = new QPushButton(QStringLiteral("⏩  Промотка"), topBar);
    pauseButton_ = new QPushButton(QStringLiteral("Ⅱ  Пауза"), topBar);
    top->addWidget(nextMonthButton);
    top->addWidget(autoplayButton_);
    top->addWidget(pauseButton_);
    root->addWidget(topBar);

    auto *body = new QHBoxLayout;
    body->setSpacing(7);

    auto *leftScroll = new QScrollArea(page);
    leftScroll->setWidgetResizable(true);
    leftScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    leftScroll->setFixedWidth(190);
    leftBotsHost_ = new QWidget(leftScroll);
    leftBotsLayout_ = new QVBoxLayout(leftBotsHost_);
    leftBotsLayout_->setContentsMargins(0, 0, 3, 0);
    leftBotsLayout_->setSpacing(7);
    leftBotsLayout_->addStretch();
    leftScroll->setWidget(leftBotsHost_);
    body->addWidget(leftScroll);

    cityMap_ = new CityMapWidget(page);
    cityMap_->setMinimumWidth(600);
    body->addWidget(cityMap_, 1);

    auto *rightScroll = new QScrollArea(page);
    rightScroll->setWidgetResizable(true);
    rightScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rightScroll->setFixedWidth(340);
    auto *rightAgencyHost = new QWidget(rightScroll);
    auto *rightAgencyLayout = new QVBoxLayout(rightAgencyHost);
    rightAgencyLayout->setContentsMargins(0, 0, 3, 0);
    rightAgencyLayout->setSpacing(7);
    rightAgencyLayout->addWidget(createRealtorPanel());
    rightAgencyLayout->addStretch();
    rightScroll->setWidget(rightAgencyHost);
    body->addWidget(rightScroll);
    root->addLayout(body, 1);

    auto *bottom = new QHBoxLayout;
    bottom->setSpacing(7);

    auto *eventsContent = new QWidget(page);
    auto *eventsLayout = new QVBoxLayout(eventsContent);
    eventsLayout->setContentsMargins(0, 0, 0, 0);
    eventsLog_ = new QPlainTextEdit(eventsContent);
    eventsLog_->setReadOnly(true);
    eventsLog_->setMaximumBlockCount(8);
    eventsLog_->setPlaceholderText(QStringLiteral("Здесь будут отображаться события симуляции"));
    eventsLayout->addWidget(eventsLog_);
    QWidget *eventsPanel = makeBottomPanel(QStringLiteral("СОБЫТИЯ"), eventsContent);
    bottom->addWidget(eventsPanel, 6);
    root->addLayout(bottom);

    connect(nextMonthButton, &QPushButton::clicked, this, [this]() { advanceMonth(); });
    connect(autoplayButton_, &QPushButton::clicked, this, [this]() { toggleAutoplay(); });
    connect(pauseButton_, &QPushButton::clicked, this, [this]() {
        autoplayTimer_->stop();
        autoplayRunning_ = false;
        autoplayButton_->setText(QStringLiteral("⏩  Промотка"));
        pauseButton_->setText(QStringLiteral("Ⅱ  Пауза"));
    });
    return page;
}

QWidget *MainWindow::createStatPanel(const BotInfo &bot) {
    auto *panel = new QFrame;
    panel->setObjectName("statPanel");
    panel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    panel->setMinimumHeight(152);
    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(10, 9, 10, 10);
    layout->setSpacing(6);

    auto *header = new QHBoxLayout;
    auto *swatch = new QFrame(panel);
    swatch->setFixedSize(12, 12);
    swatch->setStyleSheet(QStringLiteral("background: %1; border-radius: 3px;").arg(bot.color.name()));
    header->addWidget(swatch);
    auto *name = new QLabel(bot.name, panel);
    name->setStyleSheet("font-size: 14px; font-weight: 700;");
    header->addWidget(name, 1);
    layout->addLayout(header);

    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(6);
    grid->setVerticalSpacing(4);
    addStatRow(grid, 0, QStringLiteral("Деньги"), moneyText(bot.cash), panel);
    addStatRow(grid, 1, QStringLiteral("Капитал"), moneyText(bot.totalCapital), panel);
    layout->addLayout(grid);

    auto *line = new QFrame(panel);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #344653;");
    layout->addWidget(line);

    auto *houses = new QHBoxLayout;
    auto *houseLabel = new QLabel(QStringLiteral("⌂ Дома"), panel);
    houseLabel->setStyleSheet("color: #9dafbd; font-size: 11px;");
    houses->addWidget(houseLabel);
    houses->addStretch();
    houses->addWidget(makeValueLabel(QString::number(bot.houses), panel));
    layout->addLayout(houses);

    auto *shops = new QHBoxLayout;
    auto *shopLabel = new QLabel(QStringLiteral("▦ Магазины"), panel);
    shopLabel->setStyleSheet("color: #9dafbd; font-size: 11px;");
    shops->addWidget(shopLabel);
    shops->addStretch();
    shops->addWidget(makeValueLabel(QString::number(bot.shops), panel));
    layout->addLayout(shops);

    auto *construction = new QHBoxLayout;
    auto *constructionLabel = new QLabel(QStringLiteral("⚒ Строится"), panel);
    constructionLabel->setStyleSheet("color: #9dafbd; font-size: 11px;");
    construction->addWidget(constructionLabel);
    construction->addStretch();
    construction->addWidget(makeValueLabel(QString::number(bot.underConstruction), panel));
    layout->addLayout(construction);

    return panel;
}

QWidget *MainWindow::createLegendPanel() {
    auto *content = new QWidget;
    auto *grid = new QGridLayout(content);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(5);

    const QStringList labels = {
        QStringLiteral("Жилой дом"), QStringLiteral("Магазин"),
        QStringLiteral("Строится"), QStringLiteral("Фундамент")
    };
    const QStringList symbols = {
        QStringLiteral("⌂"), QString(), QStringLiteral("⚒"), QStringLiteral("▱")
    };
    const QStringList colors = {"#9bb8cf", "#ffb547", "#ffb547", "#7e8d99"};

    for (int i = 0; i < labels.size(); ++i) {
        QWidget *symbolWidget = nullptr;
        if (i == 1) {
            symbolWidget = new StoreLegendIcon(content);
        } else {
            auto *symbol = new QLabel(symbols[i], content);
            symbol->setFixedSize(26, 22);
            symbol->setAlignment(Qt::AlignCenter);
            symbol->setStyleSheet(QStringLiteral("color: %1; font-size: 16px; font-weight: 700;").arg(colors[i]));
            symbolWidget = symbol;
        }

        auto *label = new QLabel(labels[i], content);
        label->setStyleSheet("font-size: 10px; color: #9dafbd;");
        const int row = i % 2;
        const int column = (i / 2) * 2;
        grid->addWidget(symbolWidget, row, column);
        grid->addWidget(label, row, column + 1);
    }

    return makeBottomPanel(QStringLiteral("УСЛОВНЫЕ ОБОЗНАЧЕНИЯ"), content);
}

QWidget *MainWindow::createRealtorPanel() {
    auto *content = new QWidget;
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    auto *subtitle = new QLabel(QStringLiteral("Последние сделки (месяц, участник, тип, кв., цена/м²)"), content);
    subtitle->setStyleSheet("color: #9dafbd; font-size: 11px;");
    layout->addWidget(subtitle);

    salesTable_ = new QTableWidget(content);
    salesTable_->setColumnCount(5);
    salesTable_->setHorizontalHeaderLabels({
        QStringLiteral("Мес."), QStringLiteral("Участник"),
        QStringLiteral("Тип"), QStringLiteral("Кв."), QStringLiteral("₽/м²")
    });
    salesTable_->verticalHeader()->setVisible(false);
    salesTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    salesTable_->horizontalHeader()->setStretchLastSection(true);
    salesTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    salesTable_->setSelectionMode(QAbstractItemView::NoSelection);
    salesTable_->setStyleSheet(
        "QTableWidget { background: #1b2936; color: #e6edf3; "
        "gridline-color: #30404d; border: 1px solid #354657; }"
        "QHeaderView::section { background: #253442; color: #c7d4df; "
        "border: none; padding: 4px; }"
    );
    layout->addWidget(salesTable_, 1);

    QWidget *panel = makeBottomPanel(QStringLiteral("РИЕЛТОРСКОЕ АГЕНТСТВО"), content);
    panel->setMinimumWidth(305);
    panel->setMinimumHeight(430);
    return panel;
}

QWidget *MainWindow::createMiniMapPanel() {
    auto *content = new MiniMapWidget;
    miniMap_ = content;
    return makeBottomPanel(QStringLiteral("МИНИ-КАРТА"), content);
}

void MainWindow::openSettingsDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("Настройки"));
    dialog.setMinimumWidth(380);
    dialog.setStyleSheet("QDialog { background: #111b25; color: #e6edf3; } QLabel { color: #dbe5ee; } QComboBox { background: #1b2936; color: #eef4f8; border: 1px solid #354657; border-radius: 5px; padding: 6px; } QPushButton { background: #2878e5; color: white; border: none; border-radius: 6px; padding: 8px 15px; }");
    auto *layout = new QVBoxLayout(&dialog);
    auto *title = new QLabel(QStringLiteral("Настройки карты"), &dialog);
    title->setStyleSheet("font-size: 20px; font-weight: 700;");
    layout->addWidget(title);
    auto *form = new QFormLayout;
    auto *grid = new QComboBox(&dialog);
    grid->addItem(QStringLiteral("12 × 12 — компактный город"), 12);
    grid->addItem(QStringLiteral("16 × 16 — мегаполис"), 16);
    grid->addItem(QStringLiteral("20 × 20 — большая карта"), 20);
    int currentIndex = grid->findData(preferredGridSize_);
    if (currentIndex >= 0) grid->setCurrentIndex(currentIndex);
    form->addRow(QStringLiteral("Размер сетки"), grid);
    layout->addLayout(form);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Сохранить"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Отмена"));
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (dialog.exec() == QDialog::Accepted) {
        preferredGridSize_ = grid->currentData().toInt();
    }
}

void MainWindow::openSetupDialog() {
    GameSetupDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        startGame(dialog.botSetups(), dialog.gameMonths(), preferredGridSize_);
    }
}

void MainWindow::startGame(const QVector<BotSetup> &setups, int months, int gridSize) {
    autoplayTimer_->stop();
    autoplayRunning_ = false;
    autoplayButton_->setText(QStringLiteral("⏩  Промотка"));
    pauseButton_->setText(QStringLiteral("Ⅱ  Пауза"));

    try {
        Config config;
        config.playerCount = setups.size();
        config.gameMonths = months;

        engine_ = std::make_unique<GameEngine>(config);

        for (int i = 0; i < setups.size(); ++i) {
            engine_->addPlayer(makeBot(setups[i].strategy, i,
                                       setups[i].name.toStdString(), config));
        }
        engine_->startGame();
    } catch (const std::exception &e) {
        QMessageBox::critical(this, QStringLiteral("Ошибка запуска"),
                              QString::fromUtf8(e.what()));
        engine_.reset();
        return;
    }

    currentMonth_ = engine_->getCurrentMonth();
    totalMonths_ = months;
    preferredGridSize_ = gridSize;

    bots_.clear();
    buildings_.clear();
    cellById_.clear();
    buildAvailableSites(gridSize);

    cityMap_->setGridSize(gridSize);
    if (auto *miniMap = dynamic_cast<MiniMapWidget *>(miniMap_)) {
        miniMap->setGridSize(gridSize);
    }

    eventsLog_->clear();
    refreshFromEngine();

    if (buildings_.isEmpty()) {
        addEvent(QStringLiteral("Новая игра: город пока пуст."));
        addEvent(QStringLiteral("Первые строительные площадки появятся после перехода к следующему месяцу."));
    } else {
        addEvent(QStringLiteral("Новая игра: первые стройки уже начались."));
    }

    pages_->setCurrentWidget(gamePage_);
}

void MainWindow::advanceMonth() {
    if (!engine_ || engine_->isFinished()) {
        return;
    }

    try {
        if (!engine_->advanceMonth()) {
            autoplayTimer_->stop();
            autoplayRunning_ = false;
            showResults();
            return;
        }
    } catch (const std::exception &e) {
        autoplayTimer_->stop();
        autoplayRunning_ = false;
        QMessageBox::critical(this, QStringLiteral("Ошибка хода"),
                              QString::fromUtf8(e.what()));
        return;
    }

    refreshFromEngine();

    const auto &history = engine_->getState().getSalesHistory();
    int salesCount = 0;
    for (auto it = history.rbegin(); it != history.rend(); ++it) {
        if (it->month != currentMonth_) break;
        ++salesCount;
    }
    if (salesCount > 0) {
        addEvent(QStringLiteral("Месяц %1: агентство закрыло %2 сделок.")
                     .arg(currentMonth_).arg(salesCount));
    } else {
        addEvent(QStringLiteral("Месяц %1: продаж не было.").arg(currentMonth_));
    }

    if (engine_->isFinished()) {
        autoplayTimer_->stop();
        autoplayRunning_ = false;
        autoplayButton_->setText(QStringLiteral("⏩  Промотка"));
        QTimer::singleShot(250, this, [this]() { showResults(); });
    }
}

void MainWindow::toggleAutoplay() {
    if (!engine_ || engine_->isFinished()) {
        return;
    }
    if (autoplayRunning_) {
        autoplayTimer_->stop();
        autoplayRunning_ = false;
        autoplayButton_->setText(QStringLiteral("⏩  Промотка"));
    } else {
        autoplayTimer_->start();
        autoplayRunning_ = true;
        autoplayButton_->setText(QStringLiteral("⏸  Стоп"));
    }
}

void MainWindow::buildAvailableSites(int gridSize) {
    availableSites_.clear();

    const int blocksPerSide = (gridSize + 3) / 4;
    QVector<QVector<QPoint>> blocks(blocksPerSide * blocksPerSide);

    for (int row = 0; row < gridSize; ++row) {
        for (int column = 0; column < gridSize; ++column) {
            if (row % 4 != 0 && column % 4 != 0) {
                const int blockIndex = (row / 4) * blocksPerSide + column / 4;
                blocks[blockIndex].append(QPoint(row, column));
            }
        }
    }

    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    for (QVector<QPoint> &block : blocks) {
        std::shuffle(block.begin(), block.end(), generator);
    }
    std::shuffle(blocks.begin(), blocks.end(), generator);

    for (int cellIndex = 0; ; ++cellIndex) {
        bool addedCell = false;
        for (const QVector<QPoint> &block : blocks) {
            if (cellIndex < block.size()) {
                availableSites_.append(block[cellIndex]);
                addedCell = true;
            }
        }
        if (!addedCell) {
            break;
        }
    }
}

void MainWindow::rebuildBuildingVisuals()
{
    buildings_.clear();
    if (!engine_) {
        return;
    }

    const auto &players = engine_->getState().getPlayers();

    std::vector<PublicView> views;
    views.reserve(players.size());
    for (std::size_t pi = 0; pi < players.size(); ++pi) {
        views.push_back(engine_->getState().createPublicView(players[pi]->getId()));
    }

    QSet<QPoint> usedCells;
    QHash<int, QPoint> newCellById;

    for (const PublicView &view : views) {
        for (const BuildingView &bv : view.getOwnBuildings()) {
            if (bv.id < 0) continue;
            const auto it = cellById_.constFind(bv.id);
            if (it != cellById_.constEnd()) {
                newCellById.insert(bv.id, it.value());
                usedCells.insert(it.value());
            }
        }
    }

    for (const PublicView &view : views) {
        for (const BuildingView &bv : view.getOwnBuildings()) {
            if (bv.id < 0) continue;
            if (newCellById.contains(bv.id)) continue;

            QPoint cell(-1, -1);
            for (const QPoint &site : availableSites_) {
                if (!usedCells.contains(site)) {
                    cell = site;
                    break;
                }
            }
            if (cell.x() < 0) continue;

            usedCells.insert(cell);
            newCellById.insert(bv.id, cell);
        }
    }

    for (std::size_t pi = 0; pi < views.size(); ++pi) {
        for (const BuildingView &bv : views[pi].getOwnBuildings()) {
            if (bv.id < 0) continue;
            const auto it = newCellById.constFind(bv.id);
            if (it == newCellById.constEnd()) continue;
            buildings_.append(toVisual(bv, static_cast<int>(pi),
                                       it->x(), it->y()));
        }
    }

    cellById_ = newCellById;
}

void MainWindow::refreshFromEngine() {
    if (!engine_) {
        return;
    }

    bots_.clear();
    const auto &players = engine_->getState().getPlayers();
    for (std::size_t i = 0; i < players.size(); ++i) {
        const Player &p = *players[i];
        BotInfo b;
        b.name = QString::fromStdString(p.getName());
        b.color = botColors[i % botColors.size()];
        b.cash = p.getCapital();
        b.totalCapital = p.getTotalCapital();

        int queuedHouses = 0;
        int queuedShops = 0;
        for (const auto &bld : p.getBuildingQueue()) {
            if (dynamic_cast<const House *>(bld.get())) {
                ++queuedHouses;
            } else if (dynamic_cast<const Supermarket *>(bld.get())) {
                ++queuedShops;
            }
        }

        b.houses = static_cast<int>(p.getBuiltHouses().size()) + queuedHouses;
        b.shops = static_cast<int>(p.getBuiltSupermarkets().size()) + queuedShops;
        b.underConstruction = static_cast<int>(p.getBuildingQueue().size());
        b.completed = static_cast<int>(p.getBuiltHouses().size()
                                       + p.getBuiltSupermarkets().size());

        bots_.append(b);
    }

    rebuildBuildingVisuals();

    currentMonth_ = engine_->getCurrentMonth();
    if (monthLabel_) {
        monthLabel_->setText(QStringLiteral("Месяц: %1 / %2")
                                 .arg(currentMonth_).arg(totalMonths_));
    }
    if (monthProgress_) {
        monthProgress_->setRange(0, totalMonths_);
        monthProgress_->setValue(currentMonth_);
    }

    rebuildBotPanels();

    cityMap_->setOwnerColors(botColors.mid(0, bots_.size()));

    if (engine_->getCurrentMonth() != 0) {
        cityMap_->setBuildings(buildings_);
    } else {
        cityMap_->setBuildings(QVector<BuildingVisual>());
    }

    if (auto *miniMap = dynamic_cast<MiniMapWidget *>(miniMap_)) {
        miniMap->setBuildings(buildings_, botColors.mid(0, bots_.size()));
    }

    refreshSalesTable();
}

void MainWindow::refreshSalesTable() {
    if (!salesTable_ || !engine_) {
        return;
    }

    salesTable_->clearSpans();
    salesTable_->setRowCount(0);

    const auto &history = engine_->getState().getSalesHistory();
    const int count = qMin(4, static_cast<int>(history.size()));

    if (count == 0) {
        salesTable_->setRowCount(1);
        auto *item = new QTableWidgetItem(
            QStringLiteral("Продаж пока нет — они появятся по ходу игры"));
        item->setForeground(QColor("#8297a7"));
        salesTable_->setItem(0, 0, item);
        salesTable_->setSpan(0, 0, 1, 5);
        salesTable_->setRowHeight(0, 23);
        return;
    }

    const auto &players = engine_->getState().getPlayers();
    const int first = static_cast<int>(history.size()) - count;

    for (int row = 0; row < count; ++row) {
        const SaleRecord &sale = history[first + row];

        QString botName = QStringLiteral("—");
        int botIndex = -1;
        for (std::size_t i = 0; i < players.size(); ++i) {
            if (players[i]->getId() == sale.playerId) {
                botName = QString::fromStdString(players[i]->getName());
                botIndex = static_cast<int>(i);
                break;
            }
        }

        const QString typeName =
            (sale.houseType == HouseType::Panel) ? QStringLiteral("Панельный")
            : (sale.houseType == HouseType::Monolithic) ? QStringLiteral("Монолитный")
            : QStringLiteral("Кирпичный");

        const QStringList values = {
            QString::number(sale.month),
            botName,
            typeName,
            QString::number(sale.apartmentsSold),
            QLocale(QLocale::Russian).toString(sale.pricePerSqm, 'f', 0)
        };

        for (int col = 0; col < values.size(); ++col) {
            auto *item = new QTableWidgetItem(values[col]);
            if (col == 1 && botIndex >= 0) {
                item->setForeground(botColors[botIndex % botColors.size()]);
            }
            salesTable_->setItem(row, col, item);
        }
        salesTable_->setRowHeight(row, 20);
    }
}

void MainWindow::rebuildBotPanels() {
    if (!leftBotsLayout_) {
        return;
    }

    clearLayout(leftBotsLayout_);

    for (int i = 0; i < bots_.size(); ++i) {
        leftBotsLayout_->addWidget(createStatPanel(bots_[i]));
    }

    leftBotsLayout_->addStretch();
}

void MainWindow::clearLayout(QLayout *layout) {
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (item->widget()) {
            delete item->widget();
        }
        if (item->layout()) {
            clearLayout(item->layout());
            delete item->layout();
        }
        delete item;
    }
}

void MainWindow::addEvent(const QString &message) {
    if (eventsLog_) {
        eventsLog_->appendPlainText(message);
    }
}

void MainWindow::showResults() {
    autoplayTimer_->stop();
    autoplayRunning_ = false;
    refreshFromEngine();
    ResultsDialog dialog(bots_, totalMonths_, this);
    if (dialog.exec() == QDialog::Accepted) {
        openSetupDialog();
    } else {
        pages_->setCurrentWidget(menuPage_);
    }
}
