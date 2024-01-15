#include "circularprogressbar.h"

CircularProgressBar::CircularProgressBar(QWidget *parent)
    : QWidget(parent)
{
    animationTimer_ = new QTimer(this);
    animationTimer_->setTimerType(Qt::TimerType::VeryCoarseTimer);
    animationTimer_->setInterval(1000);
    connect(animationTimer_, &QTimer::timeout, this, [&](){
        if (!animationEnabled_) {
            animationEnabled_ = true;
            animationMovieTimer_->start();
        }
    });

    animationMovieTimer_ = new QTimer(this);
    animationMovieTimer_->setTimerType(Qt::TimerType::PreciseTimer);
    animationMovieTimer_->setInterval(5);
    connect(animationMovieTimer_, &QTimer::timeout, this, [&]() {
        animationMovieToDraw_ = true;
        repaint();
        animationMovieToDraw_ = false;
    });

    animationTimer_->start();
}

int CircularProgressBar::minimum() const noexcept {
    return min_;
}

int CircularProgressBar::maximum() const noexcept {
    return max_;
}

int CircularProgressBar::value() const noexcept {
    return value_;
}

void CircularProgressBar::setDisplayState(bool flag) {
    displayState_ = flag;
    update();
}

QSize CircularProgressBar::minimumSizeHint() const {
    return QSize(30, 30);
}

QSize CircularProgressBar::sizeHint() const {
    return QSize(250, 250);
}

void CircularProgressBar::reset() {
    value_ = min_;
    update();
}

void CircularProgressBar::setValue(int nval) {
    value_ = std::clamp(nval, min_, max_);
    update();
    emit valueChanged(value_);
}

void CircularProgressBar::setMinimum(int nmin) {
    min_ = nmin;
    validateRange();
    update();
}

void CircularProgressBar::setMaximum(int nmax) {
    max_ = nmax;
    validateRange();
    update();
}

void CircularProgressBar::setRange(int nmin, int nmax) {
    min_ = nmin;
    max_ = nmax;
    validateRange();
    update();
}

CircularProgressBar::~CircularProgressBar()
{ }

void CircularProgressBar::paintEvent(QPaintEvent *pe) {
    Q_UNUSED(pe);

    QPainter p{this};
    p.setRenderHint(QPainter::Antialiasing);

    QPen
        circlePen(QBrush(Qt::black), 2),
        textPen(QBrush(Qt::black), 1);
    QBrush
        progressBrush(Qt::darkGreen),
        movieBrush(QColor::fromRgba(qRgba(0, 255, 0, 50)));

    QRect
        exRect = getExternalRect(),
        inRect = getInternalRect(exRect),
        textRect = getTextRect(inRect);

    // draw bar markup
    p.setPen(circlePen);
    p.drawEllipse(exRect);
    p.setBrush(QColor::fromRgb(230, 230, 230));
    p.drawEllipse(inRect);

    // draw main annular
    auto share = (qreal)value_ / (max_ - min_);
    auto fi = 360.0 * share;
    auto path = markUpAnnularSpace(exRect, inRect, 0, fi);
    p.fillPath(path, progressBrush);

    // display state
    if (displayState_) {
        auto text = QString("%1%").arg(qRound(100 * share));
        p.setFont(adjustFontToRect(text, p.font(), textRect));
        p.setPen(textPen);
        p.drawText(textRect, Qt::AlignCenter, text);
    }

    // handle animation
    if (animationEnabled_ && animationMovieToDraw_) {
        if (animationOffsetAngle_ <= fi) {
            path = markUpAnnularSpace(exRect, inRect, 0, animationOffsetAngle_);
            p.fillPath(path, createMovieBrush(exRect, animationOffsetAngle_));
            animationOffsetAngle_ += 2;
            animationMovieTimer_->start();
        }
        else {
            animationOffsetAngle_ = 0;
            animationEnabled_ = false;
            animationTimer_->start();
        }
    }
}

void CircularProgressBar::validateRange() const {
    if (max_ - min_ < 1)
        throw std::invalid_argument("invalid range");
}

QRect CircularProgressBar::getExternalRect() const {
    return getInnerRect(rect());
}

QRect CircularProgressBar::getInternalRect(const QRect& external) const {
    return getInnerRect(external, 0.6);
}

QRect CircularProgressBar::getTextRect(const QRect& external) const {
    return getInnerRect(external, sqrt(2) / 2);
}

QRect CircularProgressBar::getInnerRect(const QRect &external, qreal factor) const {
    auto side = factor * qMin(external.width(), external.height());
    return QRect(
        external.x() + (external.width()  - side) / 2,
        external.y() + (external.height() - side) / 2,
        side,
        side
    );
}

QPoint CircularProgressBar::rotateCircular(const QRect& rect, const QPoint& target, qreal angle) {
    QPoint center = rect.center();
    return QTransform()
            .translate(center.x(), center.y())
            .rotate(angle)
            .translate(-center.x(), -center.y())
            .map(target);
}

QPainterPath CircularProgressBar::markUpAnnularSpace(
    const QRect& exRect,
    const QRect& inRect,
    qreal offsetAngle,
    qreal spanAngle
) {
    QPoint
        exCenter = exRect.center(),
        inCenter = inRect.center();

    QPoint
        exCtrl = exRect.topLeft(),
        inCtrl = inRect.topLeft();
    exCtrl.rx() += exRect.width() / 2;
    inCtrl.rx() += inRect.width() / 2;

    QPoint
        exCtrlOffset = rotateCircular(exRect, exCtrl, offsetAngle),
        inCtrlOffset = rotateCircular(inRect, inCtrl, offsetAngle),
        exCtrlDest   = rotateCircular(exRect, exCtrlOffset, spanAngle),
        inCtrlDest   = rotateCircular(inRect, inCtrlOffset, spanAngle);

    QPainterPath path;
    path.moveTo(exCtrlOffset);
    path.lineTo(inCtrlOffset);
    path.arcTo(inRect, 90 - offsetAngle, -spanAngle);
    path.lineTo(exCtrlDest);
    path.arcTo(exRect, 90 - offsetAngle - spanAngle, spanAngle);

    return path;
}

QBrush CircularProgressBar::createMovieBrush(const QRect &rect, qreal angle) {
    QPoint ctrl = rect.topLeft();
    ctrl.rx() += rect.width() / 2;

    QPoint ctrlRot = rotateCircular(rect, ctrl, angle);
    QLinearGradient lg;
    lg.setStart(ctrl);
    lg.setFinalStop(ctrlRot);
    lg.setColorAt(0.25, QColor::fromRgba(qRgba(136, 203, 8,  70)));
    lg.setColorAt(0.70, QColor::fromRgba(qRgba(0,   255, 46, 70)));

    return QBrush{lg};
}

QFont CircularProgressBar::adjustFontToRect(
    const QString &text,
    const QFont &src,
    const QRect &rect
) {
    int l{1}, r{500};
    QRect sRect{0, 0, rect.width(), rect.height()};
    QFont cur{src};
    while (r - l > 1) {
       auto mid = (r + l) / 2;
       cur.setPointSize(mid);
       QFontMetrics fm{cur};

       auto fmRect = fm.boundingRect(text);
       fmRect.moveTo(0, 0);

       if (sRect.contains(fmRect))
           l = mid;
       else
           r = mid;
    }
    cur.setPointSize(l);
    return cur;
}
