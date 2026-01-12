#include "streamer-preview.h"
#include <obs-module.h>
#include <QHBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPixmap>

StreamerPreview::StreamerPreview(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void StreamerPreview::setupUi()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(8);

    // Thumbnail
    m_thumbnailLabel = new QLabel(this);
    m_thumbnailLabel->setFixedSize(320, 180);
    m_thumbnailLabel->setScaledContents(true);
    m_thumbnailLabel->setStyleSheet("background-color: #1a1a1a; border-radius: 4px;");
    m_layout->addWidget(m_thumbnailLabel, 0, Qt::AlignCenter);

    // Streamer name
    m_nameLabel = new QLabel(this);
    m_nameLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    m_layout->addWidget(m_nameLabel);

    // Stream title
    m_titleLabel = new QLabel(this);
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setStyleSheet("color: #888;");
    m_layout->addWidget(m_titleLabel);

    // Category
    m_categoryLabel = new QLabel(this);
    m_layout->addWidget(m_categoryLabel);

    // Stats row
    QHBoxLayout* statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(16);

    m_viewersLabel = new QLabel(this);
    statsLayout->addWidget(m_viewersLabel);

    m_followersLabel = new QLabel(this);
    statsLayout->addWidget(m_followersLabel);

    m_durationLabel = new QLabel(this);
    statsLayout->addWidget(m_durationLabel);

    statsLayout->addStretch();
    m_layout->addLayout(statsLayout);

    // Action buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    m_pickAnotherButton = new QPushButton(obs_module_text("Action.PickAnother"), this);
    connect(m_pickAnotherButton, &QPushButton::clicked,
            this, &StreamerPreview::pickAnotherClicked);
    buttonLayout->addWidget(m_pickAnotherButton);

    m_greylistButton = new QPushButton(obs_module_text("Action.AddToGreylist"), this);
    connect(m_greylistButton, &QPushButton::clicked,
            this, &StreamerPreview::addToGreylistClicked);
    buttonLayout->addWidget(m_greylistButton);

    m_layout->addLayout(buttonLayout);

    // Launch raid button (full width)
    m_launchRaidButton = new QPushButton(obs_module_text("Action.LaunchRaid"), this);
    m_launchRaidButton->setMinimumHeight(40);
    m_launchRaidButton->setStyleSheet("background-color: #9147ff; color: white; font-weight: bold;");
    connect(m_launchRaidButton, &QPushButton::clicked,
            this, &StreamerPreview::launchRaidClicked);
    m_layout->addWidget(m_launchRaidButton);
}

void StreamerPreview::setStreamer(const Streamer& streamer)
{
    m_streamer = streamer;
    updateDisplay();
}

void StreamerPreview::clear()
{
    m_streamer = Streamer();
    m_thumbnailLabel->clear();
    m_nameLabel->clear();
    m_titleLabel->clear();
    m_categoryLabel->clear();
    m_viewersLabel->clear();
    m_followersLabel->clear();
    m_durationLabel->clear();
}

void StreamerPreview::updateDisplay()
{
    if (!m_streamer.isValid()) {
        clear();
        return;
    }

    m_nameLabel->setText(m_streamer.userName);
    m_titleLabel->setText(m_streamer.title);
    m_categoryLabel->setText(QString("%1: %2")
                                 .arg(obs_module_text("Preview.Category"))
                                 .arg(m_streamer.gameName));
    m_viewersLabel->setText(QString("%1: %2")
                                .arg(obs_module_text("Preview.Viewers"))
                                .arg(m_streamer.viewerCount));
    m_followersLabel->setText(QString("%1: %2")
                                  .arg(obs_module_text("Preview.Followers"))
                                  .arg(m_streamer.followerCount));
    m_durationLabel->setText(QString(obs_module_text("Preview.StreamingFor"))
                                 .arg(m_streamer.formattedDuration()));

    // Load thumbnail asynchronously
    QString thumbnailUrl = m_streamer.thumbnailUrlWithSize(320, 180);
    if (!thumbnailUrl.isEmpty()) {
        QNetworkAccessManager* manager = new QNetworkAccessManager(this);
        QNetworkRequest request(QUrl(thumbnailUrl));
        QNetworkReply* reply = manager->get(request);

        connect(reply, &QNetworkReply::finished, this, [this, reply, manager]() {
            reply->deleteLater();
            manager->deleteLater();

            if (reply->error() == QNetworkReply::NoError) {
                QPixmap pixmap;
                pixmap.loadFromData(reply->readAll());
                m_thumbnailLabel->setPixmap(pixmap);
            }
        });
    }
}
