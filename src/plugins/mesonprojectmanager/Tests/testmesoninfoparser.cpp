#include "MesonInfoParser/mesoninfoparser.h"
#include "MesonWrapper/mesonwrapper.h"
#include <iostream>
#include <QDir>
#include <QObject>
#include <QString>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QtTest/QtTest>

using namespace MesonProjectManager::Internal;

struct projectData
{
    const char *name;
    QString path;
    QStringList targets;
};

namespace {
static const QList<projectData> projectList{
    {"Simple C Project", "SimpleCProject", {"SimpleCProject"}}};
} // namespace

#define WITH_CONFIGURED_PROJECT(_source_dir, _build_dir, ...) \
    { \
        QTemporaryDir _build_dir{"test-meson"}; \
        const auto _meson = MesonWrapper("name", *findMeson()); \
        _meson.setup(Utils::FilePath::fromString(_source_dir), \
                     Utils::FilePath::fromString(_build_dir.path())); \
        QVERIFY(isSetup(Utils::FilePath::fromString(_build_dir.path()))); \
        __VA_ARGS__ \
    }

#define WITH_UNCONFIGURED_PROJECT(_source_dir, _intro_file, ...) \
    { \
        QTemporaryFile _intro_file; \
        _intro_file.open(); \
        const auto _meson = MesonWrapper("name", *findMeson()); \
        _meson.introspect(Utils::FilePath::fromString(_source_dir), &_intro_file); \
        __VA_ARGS__ \
    }

class AMesonInfoParser : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {}

    void shouldListTargets_data()
    {
        QTest::addColumn<QString>("src_dir");
        QTest::addColumn<QStringList>("expectedTargets");
        for (const auto &project : projectList) {
            QTest::newRow(project.name)
                << QString("%1/%2").arg(MESON_SAMPLES_DIR).arg(project.path) << project.targets;
        }
    }

    void shouldListTargets()
    {
        QFETCH(QString, src_dir);
        QFETCH(QStringList, expectedTargets);
        WITH_CONFIGURED_PROJECT(src_dir, build_dir, {
            MesonInfoParser parser{build_dir.path()};
            QStringList targetsNames;
            const auto targets = parser.targets();
            std::transform(std::cbegin(targets),
                           std::cend(targets),
                           std::back_inserter(targetsNames),
                           [](const auto &target) { return target.name; });
            QVERIFY(targetsNames == expectedTargets);
        })

        WITH_UNCONFIGURED_PROJECT(
            src_dir, introFile, MesonInfoParser parser{&introFile}; {
                QStringList targetsNames;
                const auto targets = parser.targets();
                volatile auto options = parser.buildOptions();
                std::transform(std::cbegin(targets),
                               std::cend(targets),
                               std::back_inserter(targetsNames),
                               [](const auto &target) { return target.name; });
                QVERIFY(targetsNames == expectedTargets);
            })
    }

    void cleanupTestCase() {}
};

QTEST_MAIN(AMesonInfoParser)
#include "testmesoninfoparser.moc"
