#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "plugin.h"

class FileDownloader : public mssm::Plugin
{
    Q_OBJECT
private:
    QNetworkAccessManager m_WebCtrl;
    QByteArray            m_DownloadedData;
    bool                  downloadComplete;
public:
    explicit FileDownloader(QObject *parent, const std::string& url);
    virtual ~FileDownloader();
    bool shouldDelete() override;
    void update(std::function<void(const std::string&, int, int, int, const std::string&)> sendEvent) override;
private slots:
    void fileDownloaded(QNetworkReply* pReply);
};

namespace mssm
{
    class Graphics;

    void download(Graphics& g, const std::string& url);
}

#endif // FILEDOWNLOADER_H
