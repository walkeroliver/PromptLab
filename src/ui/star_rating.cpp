#include "star_rating.h"

#include <QPainter>
#include <QMouseEvent>
#include <QPen>
#include <QBrush>
#include <QtMath>

namespace StarRatingText {
QString toDisplayString(double rating, int starCount)
{
    QString text;
    const int fullStars = qFloor(rating);
    for (int i = 0; i < starCount; ++i) {
        text += (i < fullStars) ? QString::fromUtf8("★") : QString::fromUtf8("☆");
    }
    return text;
}
}

StarRating::StarRating(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumSize(m_starSize * m_starCount, m_starSize);
}

void StarRating::setRating(double rating)
{
    if (qFuzzyCompare(m_rating, rating)) {
        return;
    }
    m_rating = qBound(0.0, rating, static_cast<double>(m_starCount));
    update();
    emit ratingChanged(m_rating);
}

void StarRating::setEditable(bool editable)
{
    m_editable = editable;
    setMouseTracking(editable);
}

void StarRating::setStarSize(int size)
{
    m_starSize = size;
    setMinimumSize(m_starSize * m_starCount, m_starSize);
    update();
}

QSize StarRating::sizeHint() const
{
    return QSize(m_starSize * m_starCount, m_starSize);
}

QSize StarRating::minimumSizeHint() const
{
    return QSize(m_starSize * m_starCount, m_starSize);
}

void StarRating::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor activeColor(245, 158, 11);
    QColor hoverColor(251, 191, 36);
    QColor inactiveColor(203, 213, 225);
    QColor borderColor(217, 119, 6);

    for (int i = 1; i <= m_starCount; ++i) {
        QRect starRect((i - 1) * m_starSize, 0, m_starSize, m_starSize);

        QColor fillColor;
        QColor strokeColor;

        if (i <= m_hoverStar && m_editable) {
            fillColor = hoverColor;
            strokeColor = borderColor;
        } else if (i <= m_rating) {
            fillColor = activeColor;
            strokeColor = borderColor;
        } else if (i - 0.5 <= m_rating) {
            fillColor = QColor(253, 224, 71);
            strokeColor = borderColor;
        } else {
            fillColor = inactiveColor;
            strokeColor = QColor(148, 163, 184);
        }

        painter.setPen(QPen(strokeColor, 1));
        painter.setBrush(QBrush(fillColor));

        QPolygonF star;
        qreal cx = starRect.center().x();
        qreal cy = starRect.center().y();
        qreal outerRadius = m_starSize / 2.0 - 2;
        qreal innerRadius = outerRadius * 0.4;

        for (int j = 0; j < 10; ++j) {
            qreal radius = (j % 2 == 0) ? outerRadius : innerRadius;
            qreal angle = j * 36.0 - 90.0;
            qreal rad = qDegreesToRadians(angle);
            star.append(QPointF(cx + radius * qCos(rad), cy + radius * qSin(rad)));
        }

        painter.drawPolygon(star);
    }
}

void StarRating::mousePressEvent(QMouseEvent* event)
{
    if (!m_editable) {
        return;
    }

    int star = starAtPosition(event->position().x());
    if (star > 0) {
        setRating(star);
        emit editingFinished();
    }
}

void StarRating::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_editable) {
        return;
    }

    int star = starAtPosition(event->position().x());
    if (star != m_hoverStar) {
        m_hoverStar = star;
        update();
    }
}

void StarRating::leaveEvent(QEvent* event)
{
    Q_UNUSED(event)
    m_hoverStar = 0;
    update();
}

int StarRating::starAtPosition(int x) const
{
    int star = x / m_starSize + 1;
    return qBound(0, star, m_starCount);
}
