#include <MesonProject/ninjaparser.h>
#include <iostream>
#include <QDir>
#include <QObject>
#include <QString>
#include <QTemporaryDir>
#include <QtTest/QtTest>

using namespace MesonProjectManager::Internal;

struct TestData
{
    QString ninjaSTDO;
    std::vector<std::size_t> steps;
    std::size_t warnings;
    std::size_t errors;
};
Q_DECLARE_METATYPE(TestData);

static const TestData sample1{
    R"([1/2] Compiling C object 'SimpleCProject@exe/main.c.o'
../../qt-creator/src/plugins/mesonprojectmanager/Tests/resources/SimpleCProject/main.c: In function ‘main’:
../../qt-creator/src/plugins/mesonprojectmanager/Tests/resources/SimpleCProject/main.c:1:14: warning: unused parameter ‘argc’ [-Wunused-parameter]
    1 | int main(int argc, char** argv)
      |          ~~~~^~~~
../../qt-creator/src/plugins/mesonprojectmanager/Tests/resources/SimpleCProject/main.c:1:27: warning: unused parameter ‘argv’ [-Wunused-parameter]
    1 | int main(int argc, char** argv)
      |                    ~~~~~~~^~~~
[2/2] Linking target SimpleCProject)",
    {50,100},
    2Ul,
    0Ul};

void feedParser(NinjaParser& parser, const TestData& data)
{
    auto lines = data.ninjaSTDO.split('\n');
    std::for_each(std::cbegin(lines), std::cend(lines), [&](const auto &line) {
        parser.stdOutput(line);
    });
}

class ANinjaParser : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {}
    void extractProgress_data()
    {
        QTest::addColumn<TestData>("testData");
        QTest::newRow("") << sample1;
    }
    void extractProgress()
    {
        QFETCH(TestData, testData);
        std::vector<std::size_t> steps;
        NinjaParser parser;
        connect(&parser, &NinjaParser::reportProgress, [&](int progress) {
            steps.push_back(progress);
        });
        feedParser(parser,testData);
        QVERIFY( steps == testData.steps);
    }
    void cleanupTestCase() {}
};

QTEST_MAIN(ANinjaParser)
#include "testninjaparser.moc"
