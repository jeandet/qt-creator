#include "mesoninfoparser.h"
#include "../mesonpluginconstants.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace MesonProjectManager {
namespace Internal {

class AbstractParser
{
public:
    AbstractParser(const QString &jsonFile)
        : m_jsonFile{jsonFile}
    {
        QFile js(m_jsonFile);
        js.open(QIODevice::ReadOnly | QIODevice::Text);
        if (js.isOpen()) {
            auto data = js.readAll();
            m_json = QJsonDocument::fromJson(data);
        }
    }

    const QString m_jsonFile;
    QJsonDocument m_json;
};

class TargetParser final: AbstractParser
{
public:
    TargetParser(const QString &buildDir)
        : AbstractParser(QString("%1/%2/%3")
                             .arg(buildDir)
                             .arg(Constants::MESON_INFO_DIR)
                             .arg(Constants::MESON_INTRO_TARGETS))
    {}

    inline QStringList targetList()
    {
        QStringList targets;
        if (!m_json.isEmpty()) {
            for (const auto &target : m_json.array()) {
                targets.append(target.toObject()["name"].toString());
            }
        }
        return targets;
    }
};

class MesonInfoParserPrivate
{
public:
    MesonInfoParserPrivate(const QString &buildDir)
        : targets(buildDir)
    {}
    inline QStringList targetList() { return targets.targetList(); }

private:
    TargetParser targets;
};

MesonInfoParser::MesonInfoParser(const QString &buildDir)
{
    d_ptr = new MesonInfoParserPrivate(buildDir);
}

MesonInfoParser::~MesonInfoParser()
{
    delete d_ptr;
}

QStringList MesonInfoParser::targetList()
{
    return d_ptr->targetList();
}
} // namespace Internal
} // namespace MesonProjectManager
