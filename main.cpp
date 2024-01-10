#include <random>
#include <QApplication>
#include "circularprogressbar.h"

class Demo : public QObject {

    Q_OBJECT

public:
    Demo() : bar_{ new CircularProgressBar }
    { }

    void start() {
        bar_->setRange(0, gen(100, 10000));
        bar_->setValue(0);
        bar_->show();
        startUpdateTimer();
    }

protected:
    void timerEvent(QTimerEvent *te) override {
        Q_UNUSED(te);

        auto value = bar_->value();
        if (value == bar_->maximum()) {
            bar_->close();
            return;
        }

        value += gen(1, std::max(bar_->maximum() / 20, 1));
        bar_->setValue(value);
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
        startTimer(gen(1000, 5000));
    }

private:
    CircularProgressBar *bar_;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Demo demo;
    demo.start();
    return a.exec();
}
#include "main.moc"


