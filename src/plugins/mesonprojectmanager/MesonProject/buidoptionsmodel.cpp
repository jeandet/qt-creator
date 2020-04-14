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
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QStyledItemDelegate>

namespace MesonProjectManager {
namespace Internal {

inline Utils::TreeItem *makeBuildOptionTreeItem(CancellableOption *buildOption)
{
    return new BuildOptionTreeItem(buildOption);
}

BuidOptionsModel::BuidOptionsModel(QObject *parent)
    : Utils::TreeModel<>(parent)
{
    setHeader({tr("Key"), tr("Value")});
}

inline void sortPerSubprojectAndSection(
    const CancellableOptionsList &options,
    QMap<QString, QMap<QString, std::vector<CancellableOption *>>> &subprojectOptions,
    QMap<QString, std::vector<CancellableOption *>> &perSectionOptions)
{
    std::for_each(std::cbegin(options),
                  std::cend(options),
                  [&subprojectOptions,
                   &perSectionOptions](const std::unique_ptr<CancellableOption> &option) {
                      if (option->subproject()) {
                          subprojectOptions[*option->subproject()][option->section()].push_back(
                              option.get());
                      } else {
                          perSectionOptions[option->section()].push_back(option.get());
                      }
                  });
}

void makeTree(Utils::TreeItem *root,
              const QMap<QString, std::vector<CancellableOption *>> &perSectioOptions)
{
    std::for_each(perSectioOptions.constKeyValueBegin(),
                  perSectioOptions.constKeyValueEnd(),
                  [root](const std::pair<QString, std::vector<CancellableOption *>> kv) {
                      const auto &options = kv.second;
                      auto sectionNode = new Utils::StaticTreeItem(kv.first);
                      std::for_each(std::cbegin(options),
                                    std::cend(options),
                                    [sectionNode](CancellableOption *option) {
                                        sectionNode->appendChild(makeBuildOptionTreeItem(option));
                                    });
                      root->appendChild(sectionNode);
                  });
}

void BuidOptionsModel::setConfiguration(const BuildOptionsList &options)
{
    clear();
    m_options = decltype(m_options)();
    std::for_each(std::cbegin(options),
                  std::cend(options),
                  [this](const BuildOptionsList::value_type &option) {
                      m_options.emplace_back(std::make_unique<CancellableOption>(option.get()));
                  });
    {
        QMap<QString, QMap<QString, std::vector<CancellableOption *>>> subprojectOptions;
        QMap<QString, std::vector<CancellableOption *>> perSectionOptions;
        sortPerSubprojectAndSection(m_options, subprojectOptions, perSectionOptions);
        auto root = new Utils::TreeItem;
        makeTree(root, perSectionOptions);
        auto subProjects = new Utils::StaticTreeItem{"Subprojects"};
        std::for_each(subprojectOptions.constKeyValueBegin(),
                      subprojectOptions.constKeyValueEnd(),
                      [subProjects](
                          const std::pair<QString, QMap<QString, std::vector<CancellableOption *>>>
                              kv) {
                          auto subProject = new Utils::StaticTreeItem{kv.first};
                          makeTree(subProject, kv.second);
                          subProjects->appendChild(subProject);
                      });
        root->appendChild(subProjects);
        setRootItem(root);
    }
}

QWidget *BuildOptionDelegate::makeWidget(QWidget *parent, const QVariant &data)
{
    auto type = data.userType();
    switch (type) {
    case QVariant::Int: {
        auto w = new QSpinBox{parent};
        w->setValue(data.toInt());
        return w;
    }
    case QVariant::Bool: {
        auto w = new QComboBox{parent};
        w->addItems({"false", "true"});
        w->setCurrentIndex(data.toBool());
        return w;
    }
    case QVariant::StringList: {
        auto w = new QLineEdit{parent};
        w->setText(data.toString());
        return w;
    }
    case QVariant::String: {
        auto w = new QLineEdit{parent};
        w->setText(data.toString());
        return w;
    }
    default: {
        if (type == qMetaTypeId<ComboData>()) {
            auto w = new QComboBox{parent};
            auto value = data.value<ComboData>();
            w->addItems(value.choices());
            w->setCurrentIndex(value.currentIndex());
            return w;
        }
        if (type == qMetaTypeId<FeatureData>()) {
            auto w = new QComboBox{parent};
            auto value = data.value<FeatureData>();
            w->addItems(value.choices());
            w->setCurrentIndex(value.currentIndex());
            return w;
        }
        return nullptr;
    }
    }
}

BuildOptionDelegate::BuildOptionDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}

QWidget *BuildOptionDelegate::createEditor(QWidget *parent,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
    auto data = index.data(Qt::EditRole);
    auto widget = makeWidget(parent, data);
    if (widget) {
        widget->setFocusPolicy(Qt::StrongFocus);
        return widget;
    }
    return QStyledItemDelegate::createEditor(parent, option, index);
}

} // namespace Internal
} // namespace MesonProjectManager
