#include "preset-selector.h"
#include "../plugin.h"
#include "../storage/settings-manager.h"
#include <obs-module.h>

PresetSelector::PresetSelector(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    loadPresets();
}

void PresetSelector::setupUi()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(4);

    m_comboBox = new QComboBox(this);
    m_comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_layout->addWidget(m_comboBox);

    m_editButton = new QPushButton(obs_module_text("Preset.EditPreset"), this);
    m_editButton->setMaximumWidth(60);
    m_editButton->hide(); // Only show for custom presets
    m_layout->addWidget(m_editButton);

    m_deleteButton = new QPushButton(obs_module_text("Preset.DeletePreset"), this);
    m_deleteButton->setMaximumWidth(60);
    m_deleteButton->hide(); // Only show for custom presets
    m_layout->addWidget(m_deleteButton);

    connect(m_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PresetSelector::onComboIndexChanged);
    connect(m_editButton, &QPushButton::clicked,
            this, &PresetSelector::onEditClicked);
    connect(m_deleteButton, &QPushButton::clicked,
            this, &PresetSelector::onDeleteClicked);
}

void PresetSelector::loadPresets()
{
    m_presets.clear();
    m_comboBox->clear();

    // Add built-in presets
    for (const FilterPreset& preset : FilterPreset::builtInPresets()) {
        m_presets.append(preset);
        m_comboBox->addItem(obs_module_text(QString("Preset.%1").arg(preset.name).toUtf8().constData()),
                           preset.id);
    }

    // Add custom presets from settings
    SettingsManager* settings = HooRaid::getSettingsManager();
    if (settings) {
        for (const FilterPreset& preset : settings->customPresets()) {
            m_presets.append(preset);
            m_comboBox->addItem(preset.name, preset.id);
        }
    }

    // Select first preset by default
    if (!m_presets.isEmpty()) {
        m_comboBox->setCurrentIndex(0);
    }
}

FilterPreset PresetSelector::currentPreset() const
{
    int index = m_comboBox->currentIndex();
    if (index >= 0 && index < m_presets.size()) {
        return m_presets[index];
    }
    return FilterPreset();
}

void PresetSelector::setCurrentPreset(const QString& presetId)
{
    for (int i = 0; i < m_presets.size(); ++i) {
        if (m_presets[i].id == presetId) {
            m_comboBox->setCurrentIndex(i);
            return;
        }
    }
}

void PresetSelector::addCustomPreset(const FilterPreset& preset)
{
    m_presets.append(preset);
    m_comboBox->addItem(preset.name, preset.id);
    m_comboBox->setCurrentIndex(m_comboBox->count() - 1);
}

void PresetSelector::updateCustomPreset(const FilterPreset& preset)
{
    for (int i = 0; i < m_presets.size(); ++i) {
        if (m_presets[i].id == preset.id) {
            m_presets[i] = preset;
            m_comboBox->setItemText(i, preset.name);
            if (m_comboBox->currentIndex() == i) {
                emit presetChanged(preset);
            }
            return;
        }
    }
}

void PresetSelector::removeCustomPreset(const QString& presetId)
{
    for (int i = 0; i < m_presets.size(); ++i) {
        if (m_presets[i].id == presetId) {
            m_presets.removeAt(i);
            m_comboBox->removeItem(i);
            return;
        }
    }
}

void PresetSelector::refreshPresets()
{
    QString currentId = currentPreset().id;
    loadPresets();
    setCurrentPreset(currentId);
}

void PresetSelector::onComboIndexChanged(int index)
{
    if (index < 0 || index >= m_presets.size()) {
        return;
    }

    const FilterPreset& preset = m_presets[index];

    // Show/hide edit/delete buttons for custom presets
    bool isCustom = !preset.isBuiltIn;
    m_editButton->setVisible(isCustom);
    m_deleteButton->setVisible(isCustom);

    emit presetChanged(preset);
}

void PresetSelector::onEditClicked()
{
    FilterPreset preset = currentPreset();
    if (!preset.isBuiltIn) {
        emit editPresetClicked(preset);
    }
}

void PresetSelector::onDeleteClicked()
{
    FilterPreset preset = currentPreset();
    if (!preset.isBuiltIn) {
        emit deletePresetClicked(preset.id);
    }
}
