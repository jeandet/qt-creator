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
#pragma once

#include "utils/fileutils.h"
#include "utils/optional.h"
#include <QString>
#include <QTemporaryFile>
#include <QVariant>

namespace MesonProjectManager {
namespace Internal {

struct BuildOption
{
    enum class Type {integer, string, feature, combo,array, boolean, unknown};
    const QString name;
    const QString section;
    const QString description;
    const Utils::optional<QString> subproject;
    virtual QString value() const = 0;
    virtual void setValue(const QString &) = 0;
    virtual Type type() const =0;
    virtual BuildOption* copy()const =0;
    BuildOption(const QString &name, const QString &section, const QString &description)
        : name{name.contains(":") ? name.split(":").last() : name}
        , section{section}
        , description{description}
        , subproject{name.contains(":") ? Utils::optional<QString>(name.split(":").first())
                                        : Utils::nullopt}
    {}
};

struct IntegerBuildOption  : BuildOption
{
    QString value() const override{ return QString::number(m_currentValue); }
    void setValue(const QString &value) override{ m_currentValue = value.toInt(); }
    Type type() const override {return Type::integer;}
    BuildOption* copy()const override {return new IntegerBuildOption{*this};}
    IntegerBuildOption(const QString &name,
                       const QString &section,
                       const QString &description,
                       int value)
        : BuildOption(name, section, description)
        , m_currentValue{value}
    {}

protected:
    int m_currentValue;
};

struct StringBuildOption  : BuildOption
{
    QString value() const override{ return m_currentValue; }
    void setValue(const QString &value) override { m_currentValue = value; }
    Type type() const override {return Type::string;}
    BuildOption* copy()const override {return new StringBuildOption{*this};}
    StringBuildOption(const QString &name,
                      const QString &section,
                      const QString &description,
                      const QString &value)
        : BuildOption(name, section, description)
        , m_currentValue{value}
    {}

protected:
    QString m_currentValue;
};

struct FeatureBuildOption : BuildOption
{
    enum class state_t { Enabled, Auto, Disabled };
    QString value() const override
    {
        switch (m_currentValue) {
        case state_t::Enabled:
            return "enabled";

        case state_t::Disabled:
            return "disabled";
        default:
            return "auto";
        }
    }
    void setValue(const QString &value) override
    {
        if (value.toLower() == "enabled")
            m_currentValue = state_t::Enabled;
        else if (value.toLower() == "disabled")
            m_currentValue = state_t::Disabled;
        else if (value.toLower() == "auto")
            m_currentValue = state_t::Auto;
    }
    Type type() const override {return Type::feature;}
    BuildOption* copy()const override {return new FeatureBuildOption{*this};}
    FeatureBuildOption(const QString &name,
                       const QString &section,
                       const QString &description,
                       const QString &value)
        : BuildOption(name, section, description)
    {
        setValue(value);
    }

protected:
    state_t m_currentValue;
};

struct ComboBuildOption  : BuildOption
{
    const QStringList choices;
    QString value() const override { return m_currentValue; }
    void setValue(const QString &value)override
    {
        if (choices.contains(value))
            m_currentValue = value;
    }
    Type type() const override {return Type::combo;}
    BuildOption* copy()const override {return new ComboBuildOption{*this};}
    ComboBuildOption(const QString &name,
                     const QString &section,
                     const QString &description,
                     const QStringList &choices,
                     const QString &value)
        : BuildOption(name, section, description)
        , choices{choices}
        , m_currentValue{value}
    {}

protected:
    QString m_currentValue;
};

struct ArrayBuildOption  : BuildOption
{
    QString value() const override{ return m_currentValue.join(":"); }
    void setValue(const QString &value) override{ m_currentValue = value.split(":"); }
    Type type() const override {return Type::array;}
    BuildOption* copy()const override {return new ArrayBuildOption{*this};}
    ArrayBuildOption(const QString &name,
                     const QString &section,
                     const QString &description,
                     const QStringList &value)
        : BuildOption(name, section, description)
        , m_currentValue{value}
    {}

protected:
    QStringList m_currentValue;
};

struct BooleanBuildOption  : BuildOption
{
    QString value() const override
    {
        if (m_currentValue)
            return "True";
        return "False";
    }
    void setValue(const QString &value) override
    {
        if (value.toLower() == "true")
            m_currentValue = true;
        else
            m_currentValue = false;
    }
    Type type() const override {return Type::boolean;}
    BuildOption* copy()const override {return new BooleanBuildOption{*this};}
    BooleanBuildOption(const QString &name,
                       const QString &section,
                       const QString &description,
                       const QString &value)
        : BuildOption(name, section, description)
    {
        setValue(value);
    }

protected:
    bool m_currentValue;
};

struct UnknownBuildOption  : BuildOption
{
    QString value() const override { return "Unknown option"; }
    void setValue(const QString &) override{}
    Type type() const override {return Type::unknown;}
    BuildOption* copy()const override {return new UnknownBuildOption{*this};}
    UnknownBuildOption(const QString &name, const QString &section, const QString &description)
        : BuildOption(name, section, description)
    {}
};

struct Target
{
    enum class Type {
        executable,
        run,
        custom,
        sharedLibrary,
        sharedModule,
        staticLibrary,
        jar,
        unknown
    };
    struct Source
    {
        const QString langage;
        const QStringList compiler;
        const QStringList parameters;
        const QStringList sources;
        const QStringList generatedSources;
        Source(QString &&langage,
               QStringList &&compiler,
               QStringList &&parameters,
               QStringList &&sources,
               QStringList &&generatedSources)
            : langage{std::move(langage)}
            , compiler{std::move(compiler)}
            , parameters{std::move(parameters)}
            , sources{std::move(sources)}
            , generatedSources{std::move(generatedSources)}
        {}
    };
    using SourcesList = std::vector<Source>;
    const Type type;
    const QString name;
    const QString id;
    const QString definedIn;
    const QString fileName;
    const Utils::optional<QString> subproject;
    const SourcesList sources;

    static Type toType(const QString &typeStr)
    {
        if (typeStr == "executable")
            return Type::executable;
        if (typeStr == "static library")
            return Type::staticLibrary;
        if (typeStr == "shared library")
            return Type::sharedLibrary;
        if (typeStr == "shared module")
            return Type::sharedModule;
        if (typeStr == "custom")
            return Type::custom;
        if (typeStr == "run")
            return Type::run;
        if (typeStr == "jar")
            return Type::jar;
        return Type::unknown;
    }

    Target(const QString &type,
           QString &&name,
           QString &&id,
           QString &&definedIn,
           QString &&fileName,
           QString &&subproject,
           SourcesList &&sources)
        : type{toType(type)}
        , name{std::move(name)}
        , id{std::move(id)}
        , definedIn{std::move(definedIn)}
        , fileName{std::move(fileName)}
        , subproject{subproject.isNull() ? Utils::nullopt
                                         : Utils::optional<QString>{std::move(subproject)}}
        , sources{std::move(sources)}
    {}
};

using BuildOptionsList = std::vector<std::unique_ptr<BuildOption>>;
using TargetsList = std::vector<Target>;

class MesonInfoParserPrivate;
class MesonInfoParser
{
public:
    MesonInfoParser(const QString &buildDir);
    MesonInfoParser(QIODevice *introFile);
    ~MesonInfoParser();
    TargetsList targets();
    BuildOptionsList buildOptions();

private:
    MesonInfoParserPrivate *d_ptr;
};

} // namespace Internal
} // namespace MesonProjectManager
