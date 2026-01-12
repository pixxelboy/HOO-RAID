#include "greylist-manager.h"
#include "../plugin.h"
#include "../storage/settings-manager.h"
#include <obs-module.h>
#include <QHBoxLayout>
#include <QMessageBox>

GreylistManager::GreylistManager(QWidget* parent)
    : QDialog(parent)
{
    setupUi();
    setupConnections();
    loadGreylist();
}

void GreylistManager::setupUi()
{
    setWindowTitle(obs_module_text("Greylist.Title"));
    setMinimumSize(400, 300);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(12);

    // List widget
    m_listWidget = new QListWidget(this);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_mainLayout->addWidget(m_listWidget);

    // Empty state label
    m_emptyLabel = new QLabel(obs_module_text("Greylist.Empty"), this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("color: #888;");
    m_mainLayout->addWidget(m_emptyLabel);

    // Action buttons
    QHBoxLayout* actionLayout = new QHBoxLayout();

    m_removeButton = new QPushButton(obs_module_text("Greylist.Remove"), this);
    m_removeButton->setEnabled(false);
    actionLayout->addWidget(m_removeButton);

    m_clearAllButton = new QPushButton(tr("Clear All"), this);
    actionLayout->addWidget(m_clearAllButton);

    actionLayout->addStretch();

    m_closeButton = new QPushButton(tr("Close"), this);
    actionLayout->addWidget(m_closeButton);

    m_mainLayout->addLayout(actionLayout);
}

void GreylistManager::setupConnections()
{
    connect(m_listWidget, &QListWidget::itemSelectionChanged,
            this, &GreylistManager::onSelectionChanged);

    connect(m_removeButton, &QPushButton::clicked,
            this, &GreylistManager::onRemoveClicked);

    connect(m_clearAllButton, &QPushButton::clicked,
            this, &GreylistManager::onClearAllClicked);

    connect(m_closeButton, &QPushButton::clicked,
            this, &QDialog::accept);

    // Listen for greylist changes from settings
    SettingsManager* settings = HooRaid::getSettingsManager();
    if (settings) {
        connect(settings, &SettingsManager::greylistChanged,
                this, &GreylistManager::loadGreylist);
    }
}

void GreylistManager::loadGreylist()
{
    m_listWidget->clear();

    SettingsManager* settings = HooRaid::getSettingsManager();
    if (!settings) return;

    QList<GreylistEntry> entries = settings->greylist();

    for (const GreylistEntry& entry : entries) {
        QListWidgetItem* item = new QListWidgetItem(m_listWidget);

        QString displayText = entry.userName;
        if (entry.addedAt.isValid()) {
            displayText += QString(" (%1)").arg(entry.addedAt.toString("yyyy-MM-dd"));
        }
        if (!entry.reason.isEmpty()) {
            displayText += QString(" - %1").arg(entry.reason);
        }

        item->setText(displayText);
        item->setData(Qt::UserRole, entry.userId);
    }

    updateButtonStates();
}

void GreylistManager::updateButtonStates()
{
    bool hasSelection = !m_listWidget->selectedItems().isEmpty();
    bool hasEntries = m_listWidget->count() > 0;

    m_removeButton->setEnabled(hasSelection);
    m_clearAllButton->setEnabled(hasEntries);

    m_listWidget->setVisible(hasEntries);
    m_emptyLabel->setVisible(!hasEntries);
}

void GreylistManager::onRemoveClicked()
{
    QListWidgetItem* selected = m_listWidget->currentItem();
    if (!selected) return;

    QString userId = selected->data(Qt::UserRole).toString();
    QString displayName = selected->text().split(" (").first();

    SettingsManager* settings = HooRaid::getSettingsManager();
    if (settings) {
        settings->removeFromGreylist(userId);
        emit greylistChanged();
    }
}

void GreylistManager::onClearAllClicked()
{
    int result = QMessageBox::question(
        this,
        obs_module_text("Greylist.Title"),
        tr("Are you sure you want to clear the entire greylist?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (result != QMessageBox::Yes) return;

    SettingsManager* settings = HooRaid::getSettingsManager();
    if (!settings) return;

    // Remove all entries
    QList<GreylistEntry> entries = settings->greylist();
    for (const GreylistEntry& entry : entries) {
        settings->removeFromGreylist(entry.userId);
    }

    emit greylistChanged();
}

void GreylistManager::onSelectionChanged()
{
    updateButtonStates();
}
