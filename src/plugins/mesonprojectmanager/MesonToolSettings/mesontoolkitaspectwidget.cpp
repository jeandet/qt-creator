#include "mesontoolkitaspectwidget.h"
#include "coreplugin/icore.h"
#include "mesonpluginconstants.h"
#include "mesontoolkitaspect.h"
#include "utils/qtcassert.h"

namespace MesonProjectManager {
namespace Internal {
MesonToolKitAspectWidget::MesonToolKitAspectWidget(std::shared_ptr<MesonTools> tools,
                                                   ProjectExplorer::Kit *kit,
                                                   const ProjectExplorer::KitAspect *ki)
    : ProjectExplorer::KitAspectWidget(kit, ki)
    , m_toolsComboBox{new QComboBox}
    , m_manageButton(new QPushButton(KitAspectWidget::msgManage()))
    , m_tools{tools}
{
    m_toolsComboBox->setSizePolicy(QSizePolicy::Ignored,
                                   m_toolsComboBox->sizePolicy().verticalPolicy());
    m_toolsComboBox->setEnabled(false);
    m_toolsComboBox->setToolTip(ki->description());
    loadTools();

    m_manageButton->setContentsMargins(0, 0, 0, 0);
    connect(m_manageButton, &QPushButton::clicked, this, [this]() {
        Core::ICore::showOptionsDialog(Constants::MESON_SETTINGSPAGE_ID, buttonWidget());
    });

    connect(m_tools.get(),
            &MesonTools::mesonToolAdded,
            this,
            &MesonToolKitAspectWidget::addMesonTool);
    connect(m_tools.get(),
            &MesonTools::mesonToolRemoved,
            this,
            &MesonToolKitAspectWidget::removeMesonTool);
    connect(m_toolsComboBox,
            qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            &MesonToolKitAspectWidget::setCurrentToolIndex);
}

MesonToolKitAspectWidget::~MesonToolKitAspectWidget()
{
    delete m_toolsComboBox;
    delete m_manageButton;
}

void MesonToolKitAspectWidget::addMesonTool(const MesonWrapper &tool)
{
    this->m_toolsComboBox->addItem(tool.name(), tool.id().toSetting());
}

void MesonToolKitAspectWidget::removeMesonTool(const MesonWrapper &tool)
{
    const int index = indexOf(tool.id());
    QTC_ASSERT(index >= 0, return );
    if (index == m_toolsComboBox->currentIndex())
        setToDefault();
    m_toolsComboBox->removeItem(index);
}

void MesonToolKitAspectWidget::setCurrentToolIndex(int index)
{
    const Core::Id id = Core::Id::fromSetting(m_toolsComboBox->itemData(index));
    MesonToolKitAspect::setMesonTool(m_kit, id);
}

int MesonToolKitAspectWidget::indexOf(const Core::Id &id)
{
    for (int i = 0; i < m_toolsComboBox->count(); ++i) {
        if (id == Core::Id::fromSetting(m_toolsComboBox->itemData(i)))
            return i;
    }
    return -1;
}

void MesonToolKitAspectWidget::loadTools()
{
    std::for_each(std::cbegin(m_tools->tools()),
                  std::cend(m_tools->tools()),
                  [this](const MesonWrapper &tool) { addMesonTool(tool); });
    refresh();
    m_toolsComboBox->setEnabled(m_toolsComboBox->count());
}

void MesonToolKitAspectWidget::setToDefault()
{
    const auto autoDetected = m_tools->autoDetected();
    if (autoDetected) {
        const auto index = indexOf(autoDetected->id());
        m_toolsComboBox->setCurrentIndex(index);
        setCurrentToolIndex(index);
    } else {
        m_toolsComboBox->setCurrentIndex(0);
        setCurrentToolIndex(0);
    }
}
} // namespace Internal
} // namespace MesonProjectManager
