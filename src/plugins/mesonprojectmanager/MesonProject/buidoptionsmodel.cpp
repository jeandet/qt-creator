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

#include "buidoptionsmodel.h"
#include <QMap>
namespace MesonProjectManager {
namespace Internal {

Utils::TreeItem *makeBuildOptionTreeItem(BuildOption *buildOption)
{
    switch (buildOption->type()) {
    case BuildOption::Type::integer:
        return new BuildOptionTreeItem<IntegerBuildOption>(
            dynamic_cast<IntegerBuildOption *>(buildOption));
    case BuildOption::Type::string:
        return new BuildOptionTreeItem<StringBuildOption>(
            dynamic_cast<StringBuildOption *>(buildOption));
    case BuildOption::Type::feature:
        return new BuildOptionTreeItem<FeatureBuildOption>(
            dynamic_cast<FeatureBuildOption *>(buildOption));
    case BuildOption::Type::combo:
        return new BuildOptionTreeItem<ComboBuildOption>(
            dynamic_cast<ComboBuildOption *>(buildOption));
    case BuildOption::Type::array:
        return new BuildOptionTreeItem<ArrayBuildOption>(
            dynamic_cast<ArrayBuildOption *>(buildOption));
    default:
        return new BuildOptionTreeItem<UnknownBuildOption>(
            dynamic_cast<UnknownBuildOption *>(buildOption));
    }
}

BuidOptionsModel::BuidOptionsModel(QObject *parent)
    : Utils::TreeModel<>(parent)
{}

inline void sortPerSubprojectAndSection(
    const BuildOptionsList &options,
    QMap<QString, QMap<QString, std::vector<BuildOption *>>> subprojectOptions,
    QMap<QString, std::vector<BuildOption *>> perSectionOptions)
{
    std::for_each(std::cbegin(options),
                  std::cend(options),
                  [&subprojectOptions,
                   &perSectionOptions](const std::unique_ptr<BuildOption> &option) {
                      if (option->subproject) {
                          subprojectOptions[*option->subproject][option->section].push_back(
                              option.get());
                      } else {
                          perSectionOptions[option->section].push_back(option.get());
                      }
                  });
}

void makeTree(Utils::TreeItem *root, QMap<QString, std::vector<BuildOption *>> perSectioOptions)
{
    std::for_each(perSectioOptions.constKeyValueBegin(),
                  perSectioOptions.constKeyValueEnd(),
                  [root](const QPair<QString, std::vector<BuildOption *>> kv) {
                      const auto &options = kv.second;
                      auto sectionNode = new Utils::StaticTreeItem(kv.first);
                      std::for_each(std::cbegin(options),
                                    std::cend(options),
                                    [sectionNode](BuildOption *option) {
                                        sectionNode->appendChild(makeBuildOptionTreeItem(option));
                                    });
                      root->appendChild(sectionNode);
                  });
}

void BuidOptionsModel::setConfiguration(const BuildOptionsList &options)
{
    clear();
    m_options = options;
    {
        QMap<QString, QMap<QString, std::vector<BuildOption *>>> subprojectOptions;
        QMap<QString, std::vector<BuildOption *>> perSectionOptions;
        sortPerSubprojectAndSection(m_options, subprojectOptions, perSectionOptions);
        auto root = new Utils::TreeItem;
        makeTree(root, perSectionOptions);
        std::for_each(subprojectOptions.constKeyValueBegin(),
                      subprojectOptions.constKeyValueEnd(),
                      [root](const QPair<QString, QMap<QString, std::vector<BuildOption *>>> kv) {
                          auto subProject = new Utils::StaticTreeItem{kv.first};
                          makeTree(subProject, kv.second);
                          root->appendChild(subProject);
                      });
        setRootItem(root);
    }
}

} // namespace Internal
} // namespace MesonProjectManager
