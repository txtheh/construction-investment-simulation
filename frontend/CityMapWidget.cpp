#include "CityMapWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPolygonF>
#include <QTimer>
#include <QtMath>

CityMapWidget::CityMapWidget(QWidget *parent)
    : QWidget(parent) {
    setMinimumSize(600, 460);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_OpaquePaintEvent);

    ownerColors_ = {
        QColor("#2878e5"), QColor("#ff8a1f"), QColor("#19aa68"),
        QColor("#8c5be8"), QColor("#e04b4b")
    };

    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        animationFrame_ = (animationFrame_ + 1) % 1000;
        update();
    });
    timer->start(70);
}

QSize CityMapWidget::sizeHint() const {
    return QSize(1120, 740);
}

void CityMapWidget::setBuildings(const QVector<BuildingVisual> &buildings) {
    buildings_ = buildings;
    update();
}

void CityMapWidget::setOwnerColors(const QVector<QColor> &colors) {
    ownerColors_ = colors;
    update();
}

void CityMapWidget::setGridSize(int size) {
    mapRows_ = qBound(12, size, 35);
    mapColumns_ = mapRows_;
    update();
}

int CityMapWidget::gridSize() const {
    return mapRows_;
}

void CityMapWidget::advanceConstruction() {
    for (BuildingVisual &building : buildings_) {
        if (building.stage < 4) {
            ++building.stage;
        }
    }
    update();
}

QPointF CityMapWidget::tileCenter(int row, int column, qreal tileWidth,
                                  qreal tileHeight, qreal originX,
                                  qreal originY) const {
    return QPointF(originX + (column - row) * tileWidth / 2.0,
                   originY + (row + column) * tileHeight / 2.0);
}

QColor CityMapWidget::ownerColor(int owner) const {
    if (owner >= 0 && owner < ownerColors_.size()) {
        return ownerColors_[owner];
    }
    return QColor("#8594a6");
}

void CityMapWidget::drawTile(QPainter &painter, const QPointF &center,
                             qreal tileWidth, qreal tileHeight, bool road,
                             int row, int column) {
    QPolygonF tile;
    tile << QPointF(center.x(), center.y() - tileHeight / 2.0)
         << QPointF(center.x() + tileWidth / 2.0, center.y())
         << QPointF(center.x(), center.y() + tileHeight / 2.0)
         << QPointF(center.x() - tileWidth / 2.0, center.y());

    if (road) {
        painter.setBrush(QColor("#394653"));
        painter.setPen(QPen(QColor("#53616c"), 1.0));
    } else {
        QColor grass = ((row + column) % 2 == 0)
                           ? QColor("#789b58")
                           : QColor("#719150");
        painter.setBrush(grass);
        painter.setPen(QPen(QColor("#4e6d43"), 1.0));
    }
    painter.drawPolygon(tile);

    if (!road) {
        QPolygonF plot;
        plot << QPointF(center.x(), center.y() - tileHeight * 0.36)
             << QPointF(center.x() + tileWidth * 0.36, center.y())
             << QPointF(center.x(), center.y() + tileHeight * 0.36)
             << QPointF(center.x() - tileWidth * 0.36, center.y());
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor(220, 235, 196, 40), 0.8));
        painter.drawPolygon(plot);
    }
}

void CityMapWidget::drawTree(QPainter &painter, const QPointF &center, qreal size) {
    const qreal sway = qSin(animationFrame_ * 0.035 + center.x() * 0.01) * size * 0.08;
    QPointF trunkBase(center.x(), center.y() + size * 0.28);

    painter.setPen(QPen(QColor("#493e32"), size * 0.12));
    painter.drawLine(trunkBase, QPointF(center.x() + sway, center.y() - size * 0.15));

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#2d6344"));
    painter.drawEllipse(QPointF(center.x() - size * 0.12, center.y() - size * 0.10),
                        size * 0.34, size * 0.27);
    painter.setBrush(QColor("#428553"));
    painter.drawEllipse(QPointF(center.x() + sway, center.y() - size * 0.27),
                        size * 0.31, size * 0.29);
    painter.setBrush(QColor("#65a45f"));
    painter.drawEllipse(QPointF(center.x() - size * 0.11 + sway,
                                center.y() - size * 0.38),
                        size * 0.19, size * 0.18);
}

void CityMapWidget::drawBuilding(QPainter &painter, const QPointF &center,
                                 qreal tileWidth, qreal tileHeight,
                                 const BuildingVisual &building) {
    const QColor accent = ownerColor(building.owner);
    const bool shop = building.type == BuildingVisual::Type::Shop;
    const bool office = building.type == BuildingVisual::Type::Office;
    const bool finished = building.stage >= 4;
    const qreal width = tileWidth * (shop ? 0.88 : 0.66);
    const qreal depth = tileHeight * (shop ? 0.86 : 0.62);
    const qreal baseY = center.y() + tileHeight * 0.06;
    qreal targetHeight = shop ? 27.0 : office ? 92.0 : 52.0;
    targetHeight *= tileWidth / 58.0;
    targetHeight = qMax<qreal>(targetHeight, shop ? 18.0 : 8.0);
    qreal height = targetHeight;
    if (!finished) {
        height = targetHeight * (0.20 + building.stage * 0.20);
    }

    const QPointF left(center.x() - width / 2.0, baseY);
    const QPointF front(center.x(), baseY + depth / 2.0);
    const QPointF right(center.x() + width / 2.0, baseY);
    const QPointF back(center.x(), baseY - depth / 2.0);
    const QPointF topLeft(left.x(), left.y() - height);
    const QPointF topFront(front.x(), front.y() - height);
    const QPointF topRight(right.x(), right.y() - height);
    const QPointF topBack(back.x(), back.y() - height);

    QPolygonF site;
    site << QPointF(center.x(), baseY + depth * 0.28)
         << QPointF(center.x() + width * 0.62, baseY)
         << QPointF(center.x(), baseY - depth * 0.28)
         << QPointF(center.x() - width * 0.62, baseY);
    painter.setPen(QPen(QColor("#c4bba4"), 1.0));
    painter.setBrush(finished ? QColor("#8e9c83") : QColor("#b3a17c"));
    painter.drawPolygon(site);

    if (!finished) {
        QPolygonF foundation;
        foundation << left << front << right << back;
        painter.setPen(QPen(QColor("#59636a"), 1.0));
        painter.setBrush(QColor("#6b7277"));
        painter.drawPolygon(foundation);

        if (building.stage >= 2) {
            QPolygonF leftWall;
            leftWall << left << front << topFront << topLeft;
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(accent.red(), accent.green(), accent.blue(), 165));
            painter.drawPolygon(leftWall);

            QPolygonF rightWall;
            rightWall << front << right << topRight << topFront;
            painter.setBrush(accent.darker(120));
            painter.drawPolygon(rightWall);
        }

        painter.setPen(QPen(accent.lighter(125), shop ? 2.4 : 2.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawLine(left, front);
        painter.drawLine(front, right);
        painter.drawLine(right, back);
        painter.drawLine(back, left);
        painter.drawLine(left, topLeft);
        painter.drawLine(front, topFront);
        painter.drawLine(right, topRight);
        painter.drawLine(back, topBack);
        painter.drawLine(topLeft, topFront);
        painter.drawLine(topFront, topRight);
        painter.drawLine(topRight, topBack);
        painter.drawLine(topBack, topLeft);

        if (building.stage >= 1) {
            painter.setPen(QPen(accent.lighter(135), 1.4));
            painter.drawLine(QPointF(left.x(), baseY - height * 0.48),
                             QPointF(front.x(), front.y() - height * 0.48));
            painter.drawLine(QPointF(front.x(), front.y() - height * 0.48),
                             QPointF(right.x(), baseY - height * 0.48));
        }

        if (building.stage >= 3) {
            painter.setPen(QPen(QColor("#d5dce0"), 1.0));
            painter.drawLine(topLeft, topFront);
            painter.drawLine(topFront, topRight);
            painter.drawLine(topRight, topBack);
            painter.drawLine(topBack, topLeft);
        }

        if (building.stage >= 1) {
            const qreal craneX = center.x() + width * 0.28;
            const qreal craneTop = baseY - height - 13.0;
            const qreal sway = qSin(animationFrame_ * 0.05 + building.column) * 2.2;
            painter.setPen(QPen(QColor("#ef9c24"), 2.2));
            painter.drawLine(QPointF(craneX, baseY), QPointF(craneX, craneTop));
            painter.drawLine(QPointF(craneX - width * 0.32, craneTop + sway),
                             QPointF(craneX + width * 0.48, craneTop + sway));
            painter.setPen(QPen(QColor("#f4c768"), 1.0));
            painter.drawLine(QPointF(craneX + width * 0.27, craneTop + sway),
                             QPointF(craneX + width * 0.27, craneTop + 12.0));
        }

        return;
    }

    const QColor frontColor = accent.lighter(shop ? 128 : 122);
    const QColor sideColor = accent.darker(shop ? 112 : 118);
    const QColor roofColor = shop ? QColor("#667785") : accent.darker(108);

    QPolygonF leftFace;
    leftFace << left << front << topFront << topLeft;
    painter.setPen(QPen(accent.darker(155), 1.1));
    painter.setBrush(frontColor);
    painter.drawPolygon(leftFace);

    QPolygonF rightFace;
    rightFace << front << right << topRight << topFront;
    painter.setBrush(sideColor);
    painter.drawPolygon(rightFace);

    QPolygonF roof;
    roof << topLeft << topFront << topRight << topBack;
    painter.setBrush(roofColor);
    painter.drawPolygon(roof);

    if (shop) {
        // Wide storefront: glass windows, a striped awning and a readable sign.
        auto facePoint = [](const QPointF &a, const QPointF &b, qreal along, qreal up, qreal h) {
            return QPointF(a.x() + (b.x() - a.x()) * along,
                           a.y() + (b.y() - a.y()) * along - h * up);
        };

        QPolygonF awning;
        awning << facePoint(left, front, 0.10, 0.60, height)
               << facePoint(left, front, 0.92, 0.60, height)
               << facePoint(left, front, 0.92, 0.43, height)
               << facePoint(left, front, 0.10, 0.43, height);
        painter.setPen(QPen(QColor("#e7e0ce"), 0.7));
        painter.setBrush(accent);
        painter.drawPolygon(awning);
        painter.setPen(QPen(QColor("#f7f0df"), 1.0));
        for (int stripe = 1; stripe < 6; ++stripe) {
            const qreal along = stripe / 6.0;
            painter.drawLine(facePoint(left, front, along, 0.60, height),
                             facePoint(left, front, along, 0.43, height));
        }

        QPolygonF shopWindow;
        shopWindow << facePoint(left, front, 0.13, 0.36, height)
                   << facePoint(left, front, 0.88, 0.36, height)
                   << facePoint(left, front, 0.88, 0.13, height)
                   << facePoint(left, front, 0.13, 0.13, height);
        painter.setBrush(QColor("#bde4ee"));
        painter.setPen(QPen(QColor("#354a58"), 0.8));
        painter.drawPolygon(shopWindow);
        painter.setPen(QPen(QColor("#577988"), 0.8));
        painter.drawLine(facePoint(left, front, 0.50, 0.36, height),
                         facePoint(left, front, 0.50, 0.13, height));

        QPolygonF sideWindow;
        sideWindow << facePoint(front, right, 0.14, 0.34, height)
                   << facePoint(front, right, 0.80, 0.34, height)
                   << facePoint(front, right, 0.80, 0.14, height)
                   << facePoint(front, right, 0.14, 0.14, height);
        painter.setBrush(QColor("#a8d5e2"));
        painter.drawPolygon(sideWindow);

    } else if (office) {
        // Keep windows on office buildings, but not on residential high-rises.
        const int rows = 5;
        for (int floor = 0; floor < rows; ++floor) {
            const qreal y = baseY - height * (floor + 0.55) / rows;
            for (int window = 0; window < 2; ++window) {
                const qreal xLeft = center.x() - width * 0.37 + window * width * 0.23;
                const qreal xRight = center.x() + width * 0.08 + window * width * 0.19;
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor("#d6e7ec"));
                painter.drawRect(QRectF(xLeft, y, width * 0.10, qMax<qreal>(3.0, height * 0.075)));
                painter.drawRect(QRectF(xRight, y, width * 0.085, qMax<qreal>(3.0, height * 0.075)));
            }
        }
    }
}

void CityMapWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.fillRect(rect(), QColor("#18232d"));

    const qreal horizontalTileLimit = (width() - 48.0) * 2.0 /
                                      (mapRows_ + mapColumns_);
    const qreal verticalTileLimit = (height() - 150.0) * 2.0 /
                                    (0.48 * (mapRows_ + mapColumns_));
    const qreal tileWidth = qMax<qreal>(24.0, qMin(horizontalTileLimit, verticalTileLimit) * 0.96);
    const qreal tileHeight = tileWidth * 0.48;
    const qreal originX = width() / 2.0;
    const qreal mapHeight = (mapRows_ + mapColumns_ - 2) * tileHeight / 2.0 + tileHeight;
    const qreal originY = qMax<qreal>(54.0, (height() - mapHeight) / 2.0 + tileHeight / 2.0);

    painter.setPen(QPen(QColor("#253642"), 1.0));
    painter.setBrush(QColor("#263540"));
    painter.drawRoundedRect(QRectF(8, 8, width() - 16, height() - 16), 8, 8);

    for (int depth = 0; depth < mapRows_ + mapColumns_ - 1; ++depth) {
        for (int row = 0; row < mapRows_; ++row) {
            int column = depth - row;
            if (column < 0 || column >= mapColumns_) {
                continue;
            }
            QPointF center = tileCenter(row, column, tileWidth, tileHeight,
                                        originX, originY);
            bool road = row % 4 == 0 || column % 4 == 0;
            drawTile(painter, center, tileWidth, tileHeight, road, row, column);
        }
    }

    painter.setPen(QPen(QColor(231, 235, 228, 130), 1.0, Qt::DashLine));
    for (int row = 0; row < mapRows_; ++row) {
        if (row % 4 == 0) {
            for (int column = 0; column < mapColumns_ - 1; ++column) {
                QPointF first = tileCenter(row, column, tileWidth, tileHeight,
                                           originX, originY);
                QPointF second = tileCenter(row, column + 1, tileWidth, tileHeight,
                                            originX, originY);
                painter.drawLine(first, second);
            }
        }
    }
    for (int column = 0; column < mapColumns_; ++column) {
        if (column % 4 == 0) {
            for (int row = 0; row < mapRows_ - 1; ++row) {
                QPointF first = tileCenter(row, column, tileWidth, tileHeight,
                                           originX, originY);
                QPointF second = tileCenter(row + 1, column, tileWidth, tileHeight,
                                            originX, originY);
                painter.drawLine(first, second);
            }
        }
    }

    for (int depth = 0; depth < mapRows_ + mapColumns_ - 1; ++depth) {
        for (int row = 0; row < mapRows_; ++row) {
            int column = depth - row;
            if (column < 0 || column >= mapColumns_) {
                continue;
            }
            QPointF center = tileCenter(row, column, tileWidth, tileHeight,
                                        originX, originY);
            bool road = row % 4 == 0 || column % 4 == 0;
            if (!road && (row * 7 + column * 11) % 13 == 0) {
                drawTree(painter, center, tileWidth * 0.22);
            }
            for (const BuildingVisual &building : buildings_) {
                if (building.row == row && building.column == column) {
                    drawBuilding(painter, center, tileWidth, tileHeight, building);
                }
            }
        }
    }

    if (buildings_.isEmpty()) {
        const QRectF notice(width() / 2.0 - 190.0, height() * 0.48 - 31.0, 380.0, 62.0);
        painter.setPen(QPen(QColor("#365063"), 1.0));
        painter.setBrush(QColor(13, 27, 38, 225));
        painter.drawRoundedRect(notice, 8.0, 8.0);
        painter.setPen(QColor("#e6edf3"));
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.drawText(QRectF(notice.x() + 8.0, notice.y() + 7.0,
                                notice.width() - 16.0, 23.0),
                         Qt::AlignCenter, QStringLiteral("ГОРОД ПОКА ПУСТ"));
        painter.setPen(QColor("#9dafbd"));
        painter.setFont(QFont("Arial", 9));
        painter.drawText(QRectF(notice.x() + 8.0, notice.y() + 29.0,
                                notice.width() - 16.0, 23.0),
                         Qt::AlignCenter, QStringLiteral("Строительство начнётся после первого месяца"));
    }

    QLinearGradient fade(0, height() - 52, 0, height());
    fade.setColorAt(0, QColor(24, 35, 45, 0));
    fade.setColorAt(1, QColor(24, 35, 45, 190));
    painter.fillRect(QRectF(8, height() - 52, width() - 16, 44), fade);

    painter.setPen(QColor("#a8bac5"));
    painter.setFont(QFont("Arial", 9));
    const QString footerText = buildings_.isEmpty()
        ? QStringLiteral("ГОРОДСКАЯ КАРТА  •  СВОБОДНЫЕ УЧАСТКИ")
        : QStringLiteral("ГОРОДСКАЯ КАРТА  •  ОБЪЕКТЫ ОКРАШЕНЫ ПО ВЛАДЕЛЬЦУ");
    painter.drawText(QRectF(20, height() - 34, width() - 40, 20),
                     Qt::AlignLeft | Qt::AlignVCenter, footerText);
}
