#include "filter-editor.h"
#include "../plugin.h"
#include "../services/twitch-api.h"
#include "../storage/settings-manager.h"
#include <obs-module.h>
#include <QHBoxLayout>
#include <QUuid>
#include <QTimer>
#include <QStringListModel>

FilterEditor::FilterEditor(QWidget* parent)
    : QDialog(parent)
{
    setupUi();
    setupConnections();
    populateLanguages();
}

FilterEditor::FilterEditor(const FilterPreset& preset, QWidget* parent)
    : QDialog(parent)
    , m_presetId(preset.id)
    , m_isEditing(true)
{
    setupUi();
    setupConnections();
    populateLanguages();
    setPreset(preset);
}

void FilterEditor::setupUi()
{
    setWindowTitle(m_isEditing
                       ? obs_module_text("Preset.EditPreset")
                       : obs_module_text("Preset.SaveAsPreset"));
    setMinimumWidth(350);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(12);

    m_formLayout = new QFormLayout();
    m_formLayout->setSpacing(8);

    // Preset name
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(obs_module_text("Preset.PresetName"));
    m_formLayout->addRow(obs_module_text("Preset.PresetName"), m_nameEdit);

    // Language
    m_languageCombo = new QComboBox(this);
    m_languageCombo->setEditable(false);
    m_formLayout->addRow(obs_module_text("Filter.Language"), m_languageCombo);

    // Category/Game with autocomplete
    m_categoryEdit = new QLineEdit(this);
    m_categoryEdit->setPlaceholderText(obs_module_text("Filter.AnyCategory"));
    m_categoryCompleter = new QCompleter(this);
    m_categoryCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_categoryEdit->setCompleter(m_categoryCompleter);
    m_formLayout->addRow(obs_module_text("Filter.Category"), m_categoryEdit);

    // Viewers range
    QHBoxLayout* viewersLayout = new QHBoxLayout();
    m_viewersMinSpin = new QSpinBox(this);
    m_viewersMinSpin->setRange(0, 999999);
    m_viewersMinSpin->setSpecialValueText(obs_module_text("Filter.NoLimit"));
    viewersLayout->addWidget(m_viewersMinSpin);

    viewersLayout->addWidget(new QLabel("-", this));

    m_viewersMaxSpin = new QSpinBox(this);
    m_viewersMaxSpin->setRange(0, 999999);
    m_viewersMaxSpin->setSpecialValueText(obs_module_text("Filter.NoLimit"));
    viewersLayout->addWidget(m_viewersMaxSpin);

    m_formLayout->addRow(obs_module_text("Filter.Viewers"), viewersLayout);

    // Followers range
    QHBoxLayout* followersLayout = new QHBoxLayout();
    m_followersMinSpin = new QSpinBox(this);
    m_followersMinSpin->setRange(0, 999999);
    m_followersMinSpin->setSpecialValueText(obs_module_text("Filter.NoLimit"));
    followersLayout->addWidget(m_followersMinSpin);

    followersLayout->addWidget(new QLabel("-", this));

    m_followersMaxSpin = new QSpinBox(this);
    m_followersMaxSpin->setRange(0, 999999);
    m_followersMaxSpin->setSpecialValueText(obs_module_text("Filter.NoLimit"));
    followersLayout->addWidget(m_followersMaxSpin);

    m_formLayout->addRow(obs_module_text("Filter.Followers"), followersLayout);

    m_mainLayout->addLayout(m_formLayout);

    // Error label
    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: red;");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();
    m_mainLayout->addWidget(m_errorLabel);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_cancelButton = new QPushButton(tr("Cancel"), this);
    buttonLayout->addWidget(m_cancelButton);

    m_saveButton = new QPushButton(tr("Save"), this);
    m_saveButton->setDefault(true);
    buttonLayout->addWidget(m_saveButton);

    m_mainLayout->addLayout(buttonLayout);
}

void FilterEditor::setupConnections()
{
    connect(m_nameEdit, &QLineEdit::textChanged, this, &FilterEditor::validateInputs);
    connect(m_viewersMinSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &FilterEditor::validateInputs);
    connect(m_viewersMaxSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &FilterEditor::validateInputs);
    connect(m_followersMinSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &FilterEditor::validateInputs);
    connect(m_followersMaxSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &FilterEditor::validateInputs);

    connect(m_categoryEdit, &QLineEdit::textChanged, this, &FilterEditor::onCategoryTextChanged);

    connect(m_categoryCompleter, QOverload<const QString&>::of(&QCompleter::activated),
            this, &FilterEditor::onCategorySelected);

    connect(m_saveButton, &QPushButton::clicked, this, &FilterEditor::onSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &FilterEditor::onCancelClicked);
}

void FilterEditor::populateLanguages()
{
    m_languageCombo->addItem(obs_module_text("Filter.AnyLanguage"), QString());

    // Common streaming languages
    QStringList languages = {
        "en", "es", "pt", "fr", "de", "it", "ru", "ja", "ko", "zh",
        "ar", "tr", "pl", "nl", "sv", "cs", "hu", "th", "vi", "id"
    };

    QMap<QString, QString> languageNames = {
        {"en", "English"}, {"es", "Spanish"}, {"pt", "Portuguese"},
        {"fr", "French"}, {"de", "German"}, {"it", "Italian"},
        {"ru", "Russian"}, {"ja", "Japanese"}, {"ko", "Korean"},
        {"zh", "Chinese"}, {"ar", "Arabic"}, {"tr", "Turkish"},
        {"pl", "Polish"}, {"nl", "Dutch"}, {"sv", "Swedish"},
        {"cs", "Czech"}, {"hu", "Hungarian"}, {"th", "Thai"},
        {"vi", "Vietnamese"}, {"id", "Indonesian"}
    };

    for (const QString& lang : languages) {
        m_languageCombo->addItem(languageNames.value(lang, lang), lang);
    }
}

void FilterEditor::setPreset(const FilterPreset& preset)
{
    m_presetId = preset.id;
    m_nameEdit->setText(preset.name);

    // Set language
    int langIndex = m_languageCombo->findData(preset.language);
    if (langIndex >= 0) {
        m_languageCombo->setCurrentIndex(langIndex);
    }

    // Set category
    m_categoryEdit->setText(preset.gameName);
    m_selectedGameId = preset.gameId;

    // Set ranges
    m_viewersMinSpin->setValue(preset.viewersMin);
    m_viewersMaxSpin->setValue(preset.viewersMax);
    m_followersMinSpin->setValue(preset.followersMin);
    m_followersMaxSpin->setValue(preset.followersMax);
}

FilterPreset FilterEditor::preset() const
{
    FilterPreset p;
    p.id = m_presetId.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : m_presetId;
    p.name = m_nameEdit->text().trimmed();
    p.isBuiltIn = false;
    p.language = m_languageCombo->currentData().toString();
    p.gameId = m_selectedGameId;
    p.gameName = m_categoryEdit->text().trimmed();
    p.viewersMin = m_viewersMinSpin->value();
    p.viewersMax = m_viewersMaxSpin->value();
    p.followersMin = m_followersMinSpin->value();
    p.followersMax = m_followersMaxSpin->value();
    return p;
}

void FilterEditor::onCategoryTextChanged(const QString& text)
{
    if (text.length() < 2) {
        return;
    }

    // Clear game ID if text changed (user is typing new search)
    m_selectedGameId.clear();

    // Search for games
    TwitchApi* api = HooRaid::getTwitchApi();
    if (!api) return;

    api->searchGames(text, 10, [this](const ApiResponse<QList<Game>>& response) {
        if (!response.isSuccess()) return;

        m_categorySearchResults.clear();
        m_gameIdMap.clear();

        for (const Game& game : response.data) {
            m_categorySearchResults.append(game.name);
            m_gameIdMap[game.name] = game.id;
        }

        QStringListModel* model = new QStringListModel(m_categorySearchResults, m_categoryCompleter);
        m_categoryCompleter->setModel(model);
        m_categoryCompleter->complete();
    });
}

void FilterEditor::onCategorySelected(const QString& text)
{
    m_selectedGameId = m_gameIdMap.value(text);
    blog(LOG_DEBUG, "[HOO-Raid] Selected game: %s (ID: %s)",
         text.toUtf8().constData(), m_selectedGameId.toUtf8().constData());
}

void FilterEditor::onSaveClicked()
{
    if (!isValid()) {
        m_errorLabel->setText(validationError());
        m_errorLabel->show();
        return;
    }

    FilterPreset p = preset();

    // Save to settings
    SettingsManager* settings = HooRaid::getSettingsManager();
    if (settings) {
        settings->saveCustomPreset(p);
    }

    emit presetSaved(p);
    accept();
}

void FilterEditor::onCancelClicked()
{
    reject();
}

void FilterEditor::validateInputs()
{
    if (isValid()) {
        m_errorLabel->hide();
        m_saveButton->setEnabled(true);
    } else {
        m_errorLabel->setText(validationError());
        m_errorLabel->show();
        m_saveButton->setEnabled(false);
    }
}

bool FilterEditor::isValid() const
{
    return validationError().isEmpty();
}

QString FilterEditor::validationError() const
{
    QString name = m_nameEdit->text().trimmed();

    // Name required
    if (name.isEmpty()) {
        return obs_module_text("Validation.NameRequired");
    }

    // Check for duplicate name (only for new presets or if name changed)
    SettingsManager* settings = HooRaid::getSettingsManager();
    if (settings) {
        for (const FilterPreset& existing : settings->customPresets()) {
            if (existing.name == name && existing.id != m_presetId) {
                return obs_module_text("Validation.NameExists");
            }
        }
    }

    // Min cannot exceed max
    if (m_viewersMaxSpin->value() > 0 && m_viewersMinSpin->value() > m_viewersMaxSpin->value()) {
        return obs_module_text("Validation.MinExceedsMax");
    }
    if (m_followersMaxSpin->value() > 0 && m_followersMinSpin->value() > m_followersMaxSpin->value()) {
        return obs_module_text("Validation.MinExceedsMax");
    }

    // At least one filter (language, category, or ranges)
    bool hasFilter = !m_languageCombo->currentData().toString().isEmpty() ||
                     !m_categoryEdit->text().trimmed().isEmpty() ||
                     m_viewersMinSpin->value() > 0 ||
                     m_viewersMaxSpin->value() > 0 ||
                     m_followersMinSpin->value() > 0 ||
                     m_followersMaxSpin->value() > 0;

    if (!hasFilter) {
        return obs_module_text("Validation.AtLeastOneFilter");
    }

    return QString();
}
