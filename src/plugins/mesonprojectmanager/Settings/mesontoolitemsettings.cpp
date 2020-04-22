#include "mesontoolitemsettings.h"
#include "mesonetooltreeitem.h"
#include "ui_mesontoolitemsettings.h"

namespace MesonProjectManager {
namespace Internal {
MesonToolItemSettings::MesonToolItemSettings(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MesonToolItemSettings)
{
    ui->setupUi(this);
    ui->mesonPathChooser->setExpectedKind(Utils::PathChooser::ExistingCommand);
    ui->mesonPathChooser->setHistoryCompleter(QLatin1String("Meson.Command.History"));
    connect(ui->mesonPathChooser, &Utils::PathChooser::rawPathChanged, this, &MesonToolItemSettings::store);
    connect(ui->mesonNameLineEdit, &QLineEdit::textChanged, this, &MesonToolItemSettings::store);
}

MesonToolItemSettings::~MesonToolItemSettings()
{
    delete ui;
}

void MesonToolItemSettings::load(MesoneToolTreeItem *item)
{
    if (item) {
        m_currentId = Utils::nullopt;
        ui->mesonNameLineEdit->setDisabled(item->isAutoDetected());
        ui->mesonNameLineEdit->setText(item->name());
        ui->mesonPathChooser->setDisabled(item->isAutoDetected());
        ui->mesonPathChooser->setFileName(item->executable());
        m_currentId = item->id();
    } else {
        m_currentId = Utils::nullopt;
    }
}

void MesonToolItemSettings::store()
{
    if (m_currentId)
        emit applyChanges(*m_currentId,
                          ui->mesonNameLineEdit->text(),
                          ui->mesonPathChooser->fileName());
}

} // namespace Internal
} // namespace MesonProjectManager
