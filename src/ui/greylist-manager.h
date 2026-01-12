#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include "../models/greylist-entry.h"

class SettingsManager;

class GreylistManager : public QDialog {
    Q_OBJECT

public:
    explicit GreylistManager(QWidget* parent = nullptr);
    ~GreylistManager() override = default;

signals:
    void greylistChanged();

private slots:
    void onRemoveClicked();
    void onClearAllClicked();
    void onSelectionChanged();

private:
    void setupUi();
    void setupConnections();
    void loadGreylist();
    void updateButtonStates();

    // UI Components
    QVBoxLayout* m_mainLayout = nullptr;
    QListWidget* m_listWidget = nullptr;
    QLabel* m_emptyLabel = nullptr;
    QPushButton* m_removeButton = nullptr;
    QPushButton* m_clearAllButton = nullptr;
    QPushButton* m_closeButton = nullptr;
};
