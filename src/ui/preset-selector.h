#pragma once

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include "../models/filter-preset.h"

class PresetSelector : public QWidget {
    Q_OBJECT

public:
    explicit PresetSelector(QWidget* parent = nullptr);
    ~PresetSelector() override = default;

    // Get current selection
    FilterPreset currentPreset() const;
    void setCurrentPreset(const QString& presetId);

    // Preset management
    void addCustomPreset(const FilterPreset& preset);
    void updateCustomPreset(const FilterPreset& preset);
    void removeCustomPreset(const QString& presetId);
    void refreshPresets();

signals:
    void presetChanged(const FilterPreset& preset);
    void editPresetClicked(const FilterPreset& preset);
    void deletePresetClicked(const QString& presetId);

private slots:
    void onComboIndexChanged(int index);
    void onEditClicked();
    void onDeleteClicked();

private:
    void setupUi();
    void loadPresets();

    QHBoxLayout* m_layout = nullptr;
    QComboBox* m_comboBox = nullptr;
    QPushButton* m_editButton = nullptr;
    QPushButton* m_deleteButton = nullptr;

    QList<FilterPreset> m_presets;
};
