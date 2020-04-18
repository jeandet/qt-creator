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
#include "projecttree.h"
namespace MesonProjectManager {
namespace Internal {
ProjectTree::ProjectTree()
{

}

std::unique_ptr<MesonProjectNode> ProjectTree::buildTree(const Utils::FilePath &srcDir, const TargetsList &targets)
{
    using namespace ProjectExplorer;
    auto root = std::make_unique<MesonProjectNode>(srcDir);
    std::for_each(std::cbegin(targets),std::cend(targets),[&root](const Target& target)
                  {
                      auto targetNode = std::make_unique<MesonTargetNode>(Utils::FilePath::fromString(target.definedIn),target.name);
                      targetNode->setDisplayName(target.name);
                      std::for_each(std::cbegin(target.sources),std::cend(target.sources),[&targetNode](const Target::SourceGroup& source)
                                    {
                                        std::for_each(std::cbegin(source.sources),std::cend(source.sources),[&targetNode](const QString& file)
                                                      {
                                                          auto sourceNode = std::make_unique<FileNode>(Utils::FilePath::fromString(file),FileType::Source);
                                                          targetNode->addNode(std::move(sourceNode));
                                                      });
                                    });
                      root->addNode(std::move(targetNode));
                  });
    return root;
}
} // namespace Internal
} // nam
