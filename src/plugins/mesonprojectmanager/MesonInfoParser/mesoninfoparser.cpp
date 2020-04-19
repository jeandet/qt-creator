/****************************************************************************
**
** Copyright (C) 2020 Alexis Jeandet.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "mesoninfoparser.h"
#include "../mesonpluginconstants.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace MesonProjectManager {
namespace Internal {

template<typename T>
T load(QJsonDocument &&doc);

template<>
QJsonArray load<QJsonArray>(QJsonDocument &&doc)
{
    return doc.array();
}

template<>
QJsonObject load<QJsonObject>(QJsonDocument &&doc)
{
    return doc.object();
}

template<typename T>
T load(const QJsonValueRef &ref);

template<>
QJsonArray load<QJsonArray>(const QJsonValueRef &ref)
{
    return ref.toArray();
}

template<>
QJsonObject load<QJsonObject>(const QJsonValueRef &ref)
{
    return ref.toObject();
}

template<typename root_type>
class AbstractParser
{
public:
    AbstractParser(const QString &jsonFile)
    {
        QFile js(jsonFile);
        js.open(QIODevice::ReadOnly | QIODevice::Text);
        if (js.isOpen()) {
            auto data = js.readAll();
            m_json = load<root_type>(QJsonDocument::fromJson(data));
        }
    }
    AbstractParser(const QJsonValueRef &js) { m_json = load<root_type>(js); }
    root_type m_json;
};

class TargetParser final : AbstractParser<QJsonArray>
{
public:
    TargetParser(const QString &buildDir)
        : AbstractParser(QString("%1/%2/%3")
                             .arg(buildDir)
                             .arg(Constants::MESON_INFO_DIR)
                             .arg(Constants::MESON_INTRO_TARGETS))
    {}

    TargetParser(const QJsonDocument &js)
        : AbstractParser{js.object()["targets"]}
    {}

    static inline Target::SourceGroup extract_source(const QJsonValue &source)
    {
        const auto srcObj = source.toObject();
        return {srcObj["language"].toString(),
                srcObj["compiler"].toVariant().toStringList(),
                srcObj["parameters"].toVariant().toStringList(),
                srcObj["sources"].toVariant().toStringList(),
                srcObj["generated_sources"].toVariant().toStringList()};
    }

    static inline Target::SourceGroupList extract_sources(const QJsonArray &sources)
    {
        Target::SourceGroupList res;
        std::transform(std::cbegin(sources),
                       std::cend(sources),
                       std::back_inserter(res),
                       extract_source);
        return res;
    }

    static inline Target extract_target(const QJsonValue &target)
    {
        auto targetObj = target.toObject();
        Target t{targetObj["type"].toString(),
                 targetObj["name"].toString(),
                 targetObj["id"].toString(),
                 targetObj["defined_in"].toString(),
                 targetObj["filename"].toVariant().toStringList(),
                 targetObj["subproject"].toString(),
                 extract_sources(targetObj["target_sources"].toArray())};
        return t;
    }

    inline TargetsList targetList()
    {
        TargetsList targets;
        std::transform(std::cbegin(m_json),
                       std::cend(m_json),
                       std::back_inserter(targets),
                       extract_target);
        return targets;
    }
};

class BuildOptionsParser final : AbstractParser<QJsonArray>
{
public:
    BuildOptionsParser(const QString &buildDir)
        : AbstractParser(QString("%1/%2/%3")
                             .arg(buildDir)
                             .arg(Constants::MESON_INFO_DIR)
                             .arg(Constants::MESON_INTRO_BUIDOPTIONS))
    {}
    BuildOptionsParser(const QJsonDocument &js)
        : AbstractParser{js.object()["buildoptions"]}
    {}

    std::unique_ptr<BuildOption> parse_option(const QJsonObject &option)
    {
        const auto type = option["type"].toString();
        if (type == "string")
            return std::make_unique<StringBuildOption>(option["name"].toString(),
                                                       option["section"].toString(),
                                                       option["description"].toString(),
                                                       option["value"]);
        if (type == "boolean")
            return std::make_unique<BooleanBuildOption>(option["name"].toString(),
                                                        option["section"].toString(),
                                                        option["description"].toString(),
                                                        option["value"]);
        if (type == "combo")
            return std::make_unique<ComboBuildOption>(option["name"].toString(),
                                                      option["section"].toString(),
                                                      option["description"].toString(),
                                                      option["choices"].toVariant().toStringList(),
                                                      option["value"]);
        if (type == "integer")
            return std::make_unique<IntegerBuildOption>(option["name"].toString(),
                                                        option["section"].toString(),
                                                        option["description"].toString(),
                                                        option["value"]);
        if (type == "array")
            return std::make_unique<ArrayBuildOption>(option["name"].toString(),
                                                      option["section"].toString(),
                                                      option["description"].toString(),
                                                      option["value"]);
        if (type == "feature")
            return std::make_unique<FeatureBuildOption>(option["name"].toString(),
                                                        option["section"].toString(),
                                                        option["description"].toString(),
                                                        option["value"]);
        return std::make_unique<UnknownBuildOption>(option["name"].toString(),
                                                    option["section"].toString(),
                                                    option["description"].toString());
    }

    std::vector<std::unique_ptr<BuildOption>> buildOptions()
    {
        std::vector<std::unique_ptr<BuildOption>> options;
        if (!m_json.isEmpty()) {
            for (const auto &option : m_json) {
                options.emplace_back(parse_option(option.toObject()));
            }
        }
        return options;
    }
};

class InfoParser final : AbstractParser<QJsonObject>
{
public:
    InfoParser(const QString &buildDir)
        : AbstractParser(jsonFile(buildDir))
    {}
    static inline QString jsonFile(const QString &buildDir)
    {
        return QString("%1/%2/%3")
            .arg(buildDir)
            .arg(Constants::MESON_INFO_DIR)
            .arg(Constants::MESON_INFO);
    }
    MesonInfo info()
    {
        MesonInfo info;
        auto version = m_json["meson_version"].toObject();
        info.mesonVersion = MesonVersion{version["major"].toInt(),
                                         version["minor"].toInt(),
                                         version["patch"].toInt()};
        return info;
    }
};

class MesonInfoParserPrivate
{
public:
    MesonInfoParserPrivate(const QString &buildDir)
        : m_targets{buildDir}
        , m_buildOptions{buildDir}
    {}
    MesonInfoParserPrivate(const QJsonDocument &introDoc)
        : m_targets{introDoc}
        , m_buildOptions{introDoc}
    {}
    inline auto targets() { return m_targets.targetList(); }
    inline auto buildOptions() { return m_buildOptions.buildOptions(); }

    static inline Utils::optional<MesonInfo> mesonInfo(const QString &buildDir)
    {
        if(QFile::exists(InfoParser::jsonFile(buildDir)))
        {
            InfoParser mesonInfo{buildDir};
            return mesonInfo.info();
        }
        return Utils::nullopt;
    }

private:
    TargetParser m_targets;
    BuildOptionsParser m_buildOptions;
};

MesonInfoParser::MesonInfoParser(const QString &buildDir)
{
    d_ptr = new MesonInfoParserPrivate(buildDir);
}

MesonInfoParser::MesonInfoParser(QIODevice *introFile)
{
    if (introFile) {
        if (!introFile->isOpen())
            introFile->open(QIODevice::ReadOnly | QIODevice::Text);
        introFile->seek(0);
        auto data = introFile->readAll();
        d_ptr = new MesonInfoParserPrivate(QJsonDocument::fromJson(data));
    }
}

MesonInfoParser::MesonInfoParser(const QByteArray &data)
{
    d_ptr = new MesonInfoParserPrivate(QJsonDocument::fromJson(data));
}

MesonInfoParser::~MesonInfoParser()
{
    delete d_ptr;
}

TargetsList MesonInfoParser::targets()
{
    return d_ptr->targets();
}

BuildOptionsList MesonInfoParser::buildOptions()
{
    return d_ptr->buildOptions();
}

Utils::optional<MesonInfo> MesonInfoParser::mesonInfo(const QString &buildDir)
{
    return MesonInfoParserPrivate::mesonInfo(buildDir);
}
} // namespace Internal
} // namespace MesonProjectManager
