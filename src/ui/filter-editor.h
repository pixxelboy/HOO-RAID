#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QCompleter>
#include "../models/filter-preset.h"

class TwitchApi;

class FilterEditor : public QDialog {
    Q_OBJECT

public:
    explicit FilterEditor(QWidget* parent = nullptr);
    explicit FilterEditor(const FilterPreset& preset, QWidget* parent = nullptr);
    ~FilterEditor() override = default;

    FilterPreset preset() const;
    void setPreset(const FilterPreset& preset);

signals:
    void presetSaved(const FilterPreset& preset);

private slots:
    void onCategoryTextChanged(const QString& text);
    void onCategorySelected(const QString& text);
    void onSaveClicked();
    void onCancelClicked();
    void validateInputs();

private:
    void setupUi();
    void setupConnections();
    void populateLanguages();
    bool isValid() const;
    QString validationError() const;

    // UI Components
    QVBoxLayout* m_mainLayout = nullptr;
    QFormLayout* m_formLayout = nullptr;

    QLineEdit* m_nameEdit = nullptr;
    QComboBox* m_languageCombo = nullptr;
    QLineEdit* m_categoryEdit = nullptr;
    QCompleter* m_categoryCompleter = nullptr;
    QSpinBox* m_viewersMinSpin = nullptr;
    QSpinBox* m_viewersMaxSpin = nullptr;
    QSpinBox* m_followersMinSpin = nullptr;
    QSpinBox* m_followersMaxSpin = nullptr;

    QLabel* m_errorLabel = nullptr;
    QPushButton* m_saveButton = nullptr;
    QPushButton* m_cancelButton = nullptr;

    // State
    QString m_presetId;
    bool m_isEditing = false;
    QString m_selectedGameId;
    QStringList m_categorySearchResults;
    QMap<QString, QString> m_gameIdMap; // name -> id
};
