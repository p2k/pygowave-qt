/*
 * This file is part of the PyGoWave Desktop Client
 *
 * Copyright (C) 2009 Patrick Schneider <patrick.p2k.schneider@googlemail.com>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; see the file COPYING.  If not,
 * see <http://www.gnu.org/licenses/>.
 */

#include "waveletwidget.h"
#include "ui_waveletwidget.h"

#include "participantwidget.h"
#include "addparticipantwindow.h"
#include "js_api.h"

#include "PyGoWaveApi/model.h"
#include "PyGoWaveApi/controller.h"

#include <QtCore/QTimer>
#include <QtWebKit/QWebFrame>
#include <QtGui/QMessageBox>
#include <QtGui/QInputDialog>

WaveletWidget::WaveletWidget(PyGoWave::Wavelet * wavelet, PyGoWave::Controller * controller, QWidget *parent) : QWidget(parent), ui(new Ui::WaveletWidget)
{
	this->m_wavelet = wavelet;
	this->m_controller = controller;
	connect(wavelet, SIGNAL(statusChange(QByteArray)), this, SLOT(wavelet_statusChange(QByteArray)));
	connect(wavelet, SIGNAL(participantsChanged()), this, SLOT(wavelet_participantsChanged()));
	connect(wavelet->waveModel(), SIGNAL(waveletAboutToBeRemoved(QByteArray)), this, SLOT(wave_waveletAboutToBeRemoved(QByteArray)));

	this->m_jsApi = new PyGoWave::JavaScriptAPI(controller, wavelet->waveId(), wavelet->id(), this);
	this->m_jsApi->setObjectName("jsApi");
	this->m_installedApi = false;

	this->ui->setupUi(this);

	ErrorLoggingWebPage * elwp = new ErrorLoggingWebPage(this->ui->content);
	this->ui->content->setPage(elwp);
	elwp->mainFrame()->setUrl(QUrl("qrc:/gui/html/wave.html"));
	//elwp->mainFrame()->setUrl(QUrl("../src/gui/html/wave.html"));

	this->setWindowTitle(tr("%1 - PyGoWave").arg(wavelet->title()));

	this->wavelet_statusChange(wavelet->status());
	this->wavelet_participantsChanged();
	this->m_addPWindow = NULL;

	this->ui->content->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
}

WaveletWidget::~WaveletWidget()
{
	delete this->ui;
}

void WaveletWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
		this->ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void WaveletWidget::closeEvent(QCloseEvent *e)
{
	emit closing(this->m_wavelet->id());
	if (this->m_addPWindow)
		delete this->m_addPWindow;
	QWidget::closeEvent(e);
}

void WaveletWidget::on_cmdAddParticipant_clicked()
{
	if (!this->m_addPWindow) {
		this->m_addPWindow = new AddParticipantWindow(this->m_controller);
		connect(this->m_addPWindow, SIGNAL(participantSelected(QByteArray)), this, SLOT(participantSelected(QByteArray)));
		connect(this->m_addPWindow, SIGNAL(finished(int)), this, SLOT(participantAddFinished()));
		this->m_addPWindow->move(this->pos() + QPoint(50, 50));
		this->m_addPWindow->show();
#ifdef Q_WS_X11
		QTimer * workaroundTimer = new QTimer(this);
		workaroundTimer->setSingleShot(true);
		connect(workaroundTimer, SIGNAL(timeout()), this, SLOT(on_cmdAddParticipant_clicked()));
		connect(workaroundTimer, SIGNAL(timeout()), workaroundTimer, SLOT(deleteLater())); // Self-destruct
		workaroundTimer->start(25);
#endif /*Q_WS_X11*/
	}
	this->m_addPWindow->activateWindow();
	this->m_addPWindow->setFocus();
}

void WaveletWidget::on_content_loadFinished(bool ok)
{
	if (!ok)
		return;

	QVariant check = this->ui->content->page()->mainFrame()->evaluateJavaScript("$defined(window.__wave_view__)");
	if (!check.toBool())
		this->m_installedApi = false;

	if (!this->m_installedApi) {
		this->m_installedApi = true;
		QWebFrame * frm = this->ui->content->page()->mainFrame();
		this->m_jsApi->install(frm);
	}
	else {
		QTimer * workaroundTimer = new QTimer(this);
		workaroundTimer->setSingleShot(true);
		connect(workaroundTimer, SIGNAL(timeout()), this, SLOT(delay_loadFinished()));
		connect(workaroundTimer, SIGNAL(timeout()), workaroundTimer, SLOT(deleteLater()));
		workaroundTimer->start(500);
	}
}

void WaveletWidget::delay_loadFinished()
{
	this->ui->content->page()->mainFrame()->evaluateJavaScript("__wave_view__.fireEvent('loadFinished');");
}


ErrorLoggingWebPage::ErrorLoggingWebPage(QObject *parent) : QWebPage(parent) {}

void ErrorLoggingWebPage::javaScriptConsoleMessage(const QString & message, int lineNumber, const QString & sourceID)
{
	qDebug("QWebPage: %s@%d: %s", qPrintable(sourceID), lineNumber, qPrintable(message));
}

void WaveletWidget::wavelet_statusChange(const QByteArray & status)
{
	if (status == "dirty") {
		this->ui->documentStatus->setPixmap(QPixmap(":/gui/icons/document_status_pending.png"));
		this->ui->documentStatus->setToolTip(tr("Status: Waiting for acknowledgement"));
	}
	else if (status == "clean") {
		this->ui->documentStatus->setPixmap(QPixmap(":/gui/icons/document_status_valid.png"));
		this->ui->documentStatus->setToolTip(tr("Status: Document in sync with server"));
	}
	else if (status == "invalid") {
		this->ui->documentStatus->setPixmap(QPixmap(":/gui/icons/document_status_pending.png"));
		this->ui->documentStatus->setToolTip(tr("Status: Document not in sync with server"));
	}
	else {
		this->ui->documentStatus->setText("?");
		this->ui->documentStatus->setToolTip(QString::fromAscii(status));
	}
}

void WaveletWidget::on_jsApi_currentTextRangeChanged(int start, int end)
{
	if (start == -1 || end == -1)
		this->ui->statusMessage->setText("");
	else if (start == end)
		this->ui->statusMessage->setText(tr("Position: %1").arg(start));
	else
		this->ui->statusMessage->setText(tr("Selected: %1-%2").arg(start).arg(end));
}

void WaveletWidget::wavelet_participantsChanged()
{
	QSet<QByteArray> newSet;
	foreach (PyGoWave::Participant * p, this->m_wavelet->allParticipants()) {
		QByteArray id = p->id();
		newSet.insert(id);
		if (!this->m_participants.contains(id)) {
			ParticipantWidget * pwgt = new ParticipantWidget(p, ParticipantWidget::StyleNormal, this->ui->participants);
			this->m_participants[id] = pwgt;
			this->ui->participantsLayout->insertWidget(this->ui->participantsLayout->count()-2, pwgt);
		}
	}

	QSet<QByteArray> oldSet = QSet<QByteArray>::fromList(this->m_participants.keys());
	foreach (QByteArray id, oldSet.subtract(newSet)) {
		ParticipantWidget * pwgt = this->m_participants.take(id);
		this->ui->participantsLayout->removeWidget(pwgt);
		delete pwgt;
	}
}

void WaveletWidget::participantAddFinished()
{
	this->m_addPWindow->deleteLater();
	this->m_addPWindow = NULL;
}

void WaveletWidget::participantSelected(const QByteArray &id)
{
	if (this->m_participants.contains(id))
		QMessageBox::warning(this, tr("Cannot add participant"), tr("This participant is already on the wave!"));
	else
		this->m_controller->addParticipant(this->m_wavelet->id(), id);
}

void WaveletWidget::wave_waveletAboutToBeRemoved(const QByteArray & waveletId)
{
	if (waveletId == this->m_wavelet->id()) // They killed me!
		this->close();
}

void WaveletWidget::on_cmdReply_clicked()
{
	this->m_controller->appendBlip(this->m_wavelet->id());
	this->ui->content->setFocus();
}
