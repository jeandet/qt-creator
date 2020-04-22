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
#include "mesonwrapper.h"
#include "ninjawrapper.h"
#include "toolwrapper.h"
#include "utils/algorithm.h"
#include <memory>
#include <mesonpluginconstants.h>
#include <QObject>
namespace MesonProjectManager {
namespace Internal {

class MesonTools : public QObject
{
    Q_OBJECT
    MesonTools() { setObjectName(Constants::MESON_TOOL_MANAGER); }
    ~MesonTools() {}

    template<typename T>
    void fixAutoDetected()
    {
        auto autoDetectedTool = autoDetected<T>();
        if (!autoDetectedTool) {
            auto path = T::find();
            if (path)
                this->m_tools.emplace_back(std::make_shared<T>(QString("System %1 at %2")
                                                                   .arg(T::toolName())
                                                                   .arg(path->toString()),
                                                               *path,
                                                               true));
        }
    }

public:
    using Tool_t = std::shared_ptr<ToolWrapper>;

    template<typename T>
    static inline bool is(const Tool_t &tool)
    {
        return bool(std::dynamic_pointer_cast<T>(tool));
    }
    static inline bool isMesonWrapper(const Tool_t &tool) { return is<MesonWrapper>(tool); }
    static inline bool isNinjaWrapper(const Tool_t &tool) { return is<NinjaWrapper>(tool); }

    static inline void addTool(const Core::Id &itemId,
                               const QString &name,
                               const Utils::FilePath &exe)
    {
        // TODO improve this
        if (exe.fileName().contains("ninja"))
            addTool(std::make_shared<NinjaWrapper>(name, exe, itemId));
        else
            addTool(std::make_shared<MesonWrapper>(name, exe, itemId));
    }

    static inline void addTool(Tool_t meson)
    {
        auto self = instance();
        self->m_tools.emplace_back(std::move(meson));
        emit self->mesonToolAdded(self->m_tools.back());
    }

    static inline void setTools(std::vector<Tool_t> &&tools)
    {
        auto self = instance();
        std::swap(self->m_tools, tools);
        self->fixAutoDetected<MesonWrapper>();
        self->fixAutoDetected<NinjaWrapper>();
    }

    static inline const std::vector<Tool_t> &tools() { return instance()->m_tools; }

    static inline void updateTool(const Core::Id &itemId,
                                  const QString &name,
                                  const Utils::FilePath &exe)
    {
        auto self = instance();
        auto item = std::find_if(std::begin(self->m_tools),
                                 std::end(self->m_tools),
                                 [&itemId](const Tool_t &tool) { return tool->id() == itemId; });
        if (item != std::end(self->m_tools)) {
            (*item)->setExe(exe);
            (*item)->setName(name);
        } else {
            addTool(itemId, name, exe);
        }
    }
    static void removeTool(const Core::Id &id)
    {
        auto self = instance();
        auto item = Utils::take(self->m_tools, [&id](const auto &item) { return item->id() == id; });
        QTC_ASSERT(item, return );
        emit self->mesonToolRemoved(*item);
    }

    template<typename T>
    static std::shared_ptr<T> autoDetected()
    {
        static_assert(std::is_base_of<ToolWrapper, T>::value, "Type must derive from ToolWrapper");
        auto self = instance();
        for (const auto &tool : self->m_tools) {
            if (tool->autoDetected() && std::dynamic_pointer_cast<T>(tool)) {
                return std::dynamic_pointer_cast<T>(tool);
            }
        }
        return nullptr;
    }

    template<typename T>
    static std::shared_ptr<T> tool(const Core::Id &id)
    {
        static_assert(std::is_base_of<ToolWrapper, T>::value, "Type must derive from ToolWrapper");
        auto self = instance();
        const auto tool = std::find_if(std::cbegin(self->m_tools),
                                       std::cend(self->m_tools),
                                       [&id](const Tool_t &tool) { return tool->id() == id; });
        if (tool != std::cend(self->m_tools) && std::dynamic_pointer_cast<T>(*tool))
            return std::dynamic_pointer_cast<T>(*tool);
        return nullptr;
    }

    Q_SIGNAL void mesonToolAdded(const Tool_t &tool);
    Q_SIGNAL void mesonToolRemoved(const Tool_t &tool);

    static MesonTools *instance()
    {
        static MesonTools inst;
        return &inst;
    }

private:
    std::vector<Tool_t> m_tools;
};

} // namespace Internal
} // namespace MesonProjectManager
