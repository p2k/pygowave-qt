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
#include "addgadgetwindow.h"
#include "js_api.h"
#include "waveletnetworkaccessmanager.h"

#include <PyGoWaveApi/model.h>
#include <PyGoWaveApi/controller.h>

#include <QtCore/QTimer>
#include <QtWebKit/QWebFrame>
#include <QtGui/QMessageBox>
#include <QtGui/QInputDialog>
#include <QtGui/QScrollBar>
#include <QtGui/QMenu>
#include <QtGui/QResizeEvent>

WaveletWidget::WaveletWidget(PyGoWave::Wavelet * wavelet, PyGoWave::Controller * controller, QWidget *parent) : QWidget(parent), ui(new Ui::WaveletWidget)
{
	this->m_wavelet = wavelet;
	this->m_controller = controller;
	connect(wavelet, SIGNAL(statusChange(QByteArray)), this, SLOT(wavelet_statusChange(QByteArray)));
	connect(wavelet, SIGNAL(participantsChanged()), this, SLOT(wavelet_participantsChanged()));
	connect(wavelet->waveModel(), SIGNAL(waveletAboutToBeRemoved(QByteArray)), this, SLOT(wave_waveletAboutToBeRemoved(QByteArray)));

	this->m_wnam = new WaveletNetworkAccessManager(controller->hostName(), this);

	this->m_jsApi = new PyGoWave::JavaScriptAPI(controller, wavelet->waveId(), wavelet->id(), this);
	this->m_jsApi->setObjectName("jsApi");
	this->m_installedApi = false;

	this->m_scrollTimer = new QTimer(this);
	this->m_scrollTimer->setObjectName("scrollTimer");
	this->m_scrollTimer->setInterval(30);

	this->m_waveletOptions = new QMenu(this);
	this->m_waveletOptions->setObjectName("waveletOptions");

	this->ui->setupUi(this);

	connect(this->ui->participantsContainer->horizontalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(updateScrollers()));

	this->m_waveletOptions->addAction(this->ui->actLeaveWave);
	connect(this->ui->actAttachTab, SIGNAL(triggered()), this, SIGNAL(attachTabRequest()));
	connect(this->ui->actDetachTab, SIGNAL(triggered()), this, SIGNAL(detachTabRequest()));

	this->ui->participantsLayout->removeWidget(this->ui->leftScroller);
	this->ui->leftScroller->move(0,0);
	this->ui->leftScroller->installEventFilter(this);
	this->ui->participantsLayout->removeWidget(this->ui->rightScroller);
	this->ui->rightScroller->move(0,0);
	this->ui->rightScroller->installEventFilter(this);
	this->ui->participantsList->installEventFilter(this);

	ErrorLoggingWebPage * elwp = new ErrorLoggingWebPage(this->ui->content);
	elwp->setNetworkAccessManager(this->m_wnam);
	this->ui->content->setPage(elwp);
	elwp->mainFrame()->setUrl(QUrl("wavelet:load?rc=main"));

	this->setWindowTitle(tr("%1 - PyGoWave").arg(wavelet->title()));

	this->wavelet_statusChange(wavelet->status());
	this->wavelet_participantsChanged();
	this->m_addPWindow = NULL;
	this->m_addGWindow = NULL;

	this->ui->content->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
}

WaveletWidget::~WaveletWidget()
{
	this->ui->participantsList->removeEventFilter(this);
	this->ui->leftScroller->removeEventFilter(this);
	this->ui->rightScroller->removeEventFilter(this);
	delete this->ui;
}

PyGoWave::Wavelet * WaveletWidget::wavelet()
{
	return this->m_wavelet;
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
	if (this->m_addGWindow)
		delete this->m_addGWindow;
	QWidget::closeEvent(e);
}

void WaveletWidget::on_cmdAddParticipant_clicked()
{
	if (!this->m_addPWindow) {
		this->m_addPWindow = new AddParticipantWindow(this->m_controller);
		connect(this->m_addPWindow, SIGNAL(participantSelected(QByteArray)), this, SLOT(participantSelected(QByteArray)));
		connect(this->m_addPWindow, SIGNAL(finished(int)), this, SLOT(participantAddFinished()));
		this->m_addPWindow->move(this->mapToGlobal(QPoint(0,0)) + QPoint(50, 50));
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
		this->ui->content->setFocus();
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
	this->m_currentTextRange.first = start;
	this->m_currentTextRange.second = end;
	if (start == -1 || end == -1)
		this->ui->statusMessage->setText("");
	else if (start == end)
		this->ui->statusMessage->setText(tr("Position: %1").arg(start));
	else
		this->ui->statusMessage->setText(tr("Selected: %1-%2").arg(start).arg(end));
}

void WaveletWidget::on_jsApi_blipEditing(const QByteArray &blipId)
{
	this->m_currentBlipId = blipId;
	this->ui->cmdAddGadget->setEnabled(!blipId.isEmpty());
}

void WaveletWidget::wavelet_participantsChanged()
{
	QSet<QByteArray> newSet;
	foreach (PyGoWave::Participant * p, this->m_wavelet->allParticipants()) {
		QByteArray id = p->id();
		newSet.insert(id);
		if (!this->m_participants.contains(id)) {
			ParticipantWidget * pwgt = new ParticipantWidget(p, ParticipantWidget::StyleNormal, this->ui->participantsList);
			this->m_participants[id] = pwgt;
			this->ui->participantsLayout->addWidget(pwgt);
		}
	}

	QSet<QByteArray> oldSet = QSet<QByteArray>::fromList(this->m_participants.keys());
	foreach (QByteArray id, oldSet.subtract(newSet)) {
		ParticipantWidget * pwgt = this->m_participants.take(id);
		this->ui->participantsLayout->removeWidget(pwgt);
		delete pwgt;
	}

	this->ui->participantsList->adjustSize();
	this->ui->participantsList->updateGeometry();
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
	this->ui->content->setFocus();
	this->m_controller->appendBlip(this->m_wavelet->id());
}

void WaveletWidget::resizeEvent(QResizeEvent *e)
{
	QWidget::resizeEvent(e);
	this->updateScrollers();
}

void WaveletWidget::updateScrollers()
{
	QScrollBar * sc = this->ui->participantsContainer->horizontalScrollBar();
	int scOffset = sc->value();

	if (sc->value() > 0) {
		this->ui->leftScroller->raise();
		this->ui->leftScroller->move(scOffset, 0);
		this->ui->leftScroller->setVisible(true);
	}
	else
		this->ui->leftScroller->setVisible(false);

	if (sc->value() != sc->maximum()) {
		this->ui->rightScroller->raise();
		this->ui->rightScroller->move(this->ui->participantsContainer->width() + scOffset - this->ui->rightScroller->width(), 0);
		this->ui->rightScroller->setVisible(true);
	}
	else
		this->ui->rightScroller->setVisible(false);
}

bool WaveletWidget::eventFilter(QObject * watched, QEvent * event)
{
	if (watched == this->ui->participantsList) {
		if (event->type() == QEvent::Resize)
			this->ui->participantsContainer->setMaximumWidth(this->ui->participantsContainer->sizeHint().width());
	}
	else if (watched == this->ui->leftScroller) {
		if (event->type() == QEvent::Enter) {
			this->m_scrollLeft = true;
			this->m_scrollSpeed = 4;
			this->m_scrollTimer->start();
		}
		else if (event->type() == QEvent::Leave)
			this->m_scrollTimer->stop();
		else if (event->type() == QEvent::MouseButtonPress)
			this->m_scrollSpeed = 8;
		else if (event->type() == QEvent::MouseButtonRelease)
			this->m_scrollSpeed = 4;
	}
	else if (watched == this->ui->rightScroller) {
		if (event->type() == QEvent::Enter) {
			this->m_scrollLeft = false;
			this->m_scrollSpeed = 4;
			this->m_scrollTimer->start();
		}
		else if (event->type() == QEvent::Leave)
			this->m_scrollTimer->stop();
		else if (event->type() == QEvent::MouseButtonPress)
			this->m_scrollSpeed = 8;
		else if (event->type() == QEvent::MouseButtonRelease)
			this->m_scrollSpeed = 4;
	}
	return QWidget::eventFilter(watched, event);
}

void WaveletWidget::on_scrollTimer_timeout()
{
	QScrollBar * sc = this->ui->participantsContainer->horizontalScrollBar();
	sc->setSliderPosition(sc->value() + (this->m_scrollLeft ? -this->m_scrollSpeed : this->m_scrollSpeed));
	this->updateScrollers();
}

void WaveletWidget::on_cmdAddGadget_clicked()
{
	if (!this->m_addGWindow) {
		this->m_addGWindow = new AddGadgetWindow(this->m_controller);
		connect(this->m_addGWindow, SIGNAL(gadgetSelected(QString)), this, SLOT(gadgetSelected(QString)));
		connect(this->m_addGWindow, SIGNAL(finished(int)), this, SLOT(gadgetAddFinished()));
		this->m_addGWindow->move(this->mapToGlobal(QPoint(0,0)) + QPoint(50, 50));
		this->m_addGWindow->show();
#ifdef Q_WS_X11
		QTimer * workaroundTimer = new QTimer(this);
		workaroundTimer->setSingleShot(true);
		connect(workaroundTimer, SIGNAL(timeout()), this, SLOT(on_cmdAddGadget_clicked()));
		connect(workaroundTimer, SIGNAL(timeout()), workaroundTimer, SLOT(deleteLater())); // Self-destruct
		workaroundTimer->start(25);
#endif /*Q_WS_X11*/
	}
	this->m_addGWindow->activateWindow();
	this->m_addGWindow->setFocus();
}

void WaveletWidget::gadgetSelected(const QString &url)
{
	if (this->m_currentBlipId.isEmpty()) {
		QMessageBox::warning(this, tr("Sorry, but..."), tr("You must be inside of a Blip to add a Gadget."), tr("I see"));
		return;
	}
	int index = this->m_currentTextRange.first;
	if (index == -1) {
		QMessageBox::warning(this, tr("Sorry, but..."), tr("You must place your cursor inside the text to mark the Gadget's insertion point."), tr("I see"));
		return;
	}

	// Add a newline if not at start of a line
	if (!this->m_jsApi->checkOrAddNewline(index))
		index++;
	/* Doesn't work until better handling of model events is implemented
	PyGoWave::Blip * currentBlip = this->m_wavelet->blipById(this->m_currentBlipId);
	QString content = currentBlip->content();
	if (index-1 < 0 || content.at(index-1) != '\n')
		this->m_controller->textInserted(this->wavelet()->id(), this->m_currentBlipId, index, "\n");*/

	QVariantMap properties;
	properties["url"] = url;
	this->m_controller->elementInsert(this->m_wavelet->id(), this->m_currentBlipId, index, PyGoWave::Element::GADGET, properties);
}

void WaveletWidget::gadgetAddFinished()
{
	this->m_addGWindow->deleteLater();
	this->m_addGWindow = NULL;
}

void WaveletWidget::on_cmdWaveletOptions_clicked()
{
	this->m_waveletOptions->popup(this->ui->cmdWaveletOptions->mapToGlobal(QPoint(-this->m_waveletOptions->width() + this->ui->cmdWaveletOptions->width(), this->ui->cmdWaveletOptions->height())));
}

void WaveletWidget::setTabState(bool tabsEnabled, bool attached)
{
	QList<QAction*> acts = this->m_waveletOptions->actions();
	if (tabsEnabled) {
		this->m_waveletOptions->removeAction(this->ui->actAttachTab);
		this->m_waveletOptions->removeAction(this->ui->actDetachTab);
		QAction * top = NULL;
		if (!this->m_waveletOptions->actions().isEmpty())
			top = this->m_waveletOptions->actions().first();
		if (attached)
			this->m_waveletOptions->insertAction(top, this->ui->actDetachTab);
		else
			this->m_waveletOptions->insertAction(top, this->ui->actAttachTab);
	}
	else {
		if (acts.contains(this->ui->actAttachTab))
			this->m_waveletOptions->removeAction(this->ui->actAttachTab);
		if (acts.contains(this->ui->actDetachTab))
			this->m_waveletOptions->removeAction(this->ui->actDetachTab);
	}
	this->checkOptionsSeperator();
}

void WaveletWidget::checkOptionsSeperator()
{
	int idx = this->m_waveletOptions->actions().indexOf(this->ui->actLeaveWave);
	if (idx == 0)
		return;
	QAction * before = this->m_waveletOptions->actions().at(idx-1);
	if (before->isSeparator() && idx-1 == 0) {
		this->m_waveletOptions->removeAction(before);
		delete before;
	}
	else if (!before->isSeparator())
		this->m_waveletOptions->insertSeparator(this->ui->actLeaveWave);
}

void WaveletWidget::on_waveletOptions_aboutToHide()
{
	this->ui->cmdWaveletOptions->update();
}

void WaveletWidget::on_actLeaveWave_triggered()
{
	if (QMessageBox::question(this, tr("Please confirm"), tr("Do you really want to leave the Wave?\n\nWarning: If you do so, you cannot come back to this wave unless someone adds you again."), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
		return;
	this->m_controller->leaveWavelet(this->m_wavelet->id());
}
