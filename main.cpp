#include <random>
#include <QApplication>
#include <QTimer>
#include "circularprogressbar.h"

class Demo : public QObject {

    Q_OBJECT

public:
    Demo() {
        bar_ = new CircularProgressBar;
        updateTimer_ = new QTimer(this);
        updateTimer_->setTimerType(Qt::TimerType::CoarseTimer);
        connect(updateTimer_, &QTimer::timeout, this, [&]() {
            auto value = bar_->value();
            if (value == bar_->maximum())
                return;
            value += gen(1, std::max(bar_->maximum() / 15, 1));
            bar_->setValue(value);
            updateTimer_->start(gen(100, 10000));
            startUpdateTimer();
        });
    }

    void start() {
        bar_->setRange(0, gen(100, 10000));
        bar_->setValue(0);
        bar_->show();
        startUpdateTimer();
    }

private:
    static int gen(int min, int max) {
        static std::random_device rd;
        static std::mt19937 generator{ rd() };
        std::uniform_int_distribution dist{ min, max };
        return dist(generator);
    }

    void startUpdateTimer() {
        updateTimer_->start(gen(1000, 5000));
    }

private:
    QTimer *updateTimer_;
    CircularProgressBar *bar_;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    auto demo = new Demo;
    demo->start();
    return a.exec();
}
#include "main.moc"


