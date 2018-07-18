#include <QTest>

class ExecutorTests : public QObject {
    Q_OBJECT
private slots:
    void t1()
    {
        QVERIFY(true);
    }
};

QTEST_MAIN(ExecutorTests)
#include "ExecutorTest.moc"