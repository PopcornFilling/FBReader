/*
 * Copyright (C) 2004-2012 Geometer Plus <contact@geometerplus.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QScrollBar>
#include <QtGui/QGraphicsDropShadowEffect>

#include <QtCore/QDebug>

#include <ZLNetworkManager.h>
#include <ZLStringUtil.h>

#include "../image/ZLQtImageUtils.h"

#include "ZLQtPreviewWidget.h"

static const int PREVIEW_WIDTH = 300;


ZLQtButtonAction::ZLQtButtonAction(shared_ptr<ZLTreeAction> action,QWidget *parent) :
	QPushButton(parent), myAction(action) {
	connect(this, SIGNAL(clicked()), this, SLOT(onClicked()));
}

void ZLQtButtonAction::onClicked() {
	if (myAction.isNull() || !myAction->makesSense()) {
		return;
	}
	myAction->run();
}

ZLQtPreviewWidget::ZLQtPreviewWidget(QWidget *parent) : QWidget(parent), myWidget(0) {
	setMinimumWidth(PREVIEW_WIDTH);
	setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	QHBoxLayout *layout = new QHBoxLayout;
	layout->setSizeConstraint(QLayout::SetMinimumSize);
	layout->setContentsMargins(0,0,0,0);
	setLayout(layout);
}

void ZLQtPreviewWidget::fill(const ZLTreePageInfo &info) {
	clear();
	myWidget = new ZLQtPageWidget(info);
	myWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	layout()->addWidget(myWidget);
}

void ZLQtPreviewWidget::fillCatalog(const ZLTreeTitledNode *node) {
	clear();
	myWidget = new ZLQtCatalogPageWidget(node);
	myWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	layout()->addWidget(myWidget);
}

void ZLQtPreviewWidget::clear() {
	if (myWidget) {
		delete myWidget;
		myWidget = 0;
//		delete myLayout;
//		qDeleteAll(this->children());
	}
}

ZLQtPageWidget::ZLQtPageWidget(const ZLTreePageInfo &info, QWidget *parent) : QWidget(parent) {
	//TODO fix it: if element is absent, there's a empty space instead it. Looks bad.
	createElements();
	setInfo(info);
}

void ZLQtPageWidget::createElements() {
	myPicLabel = new QLabel;

	QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect;
	effect->setBlurRadius(12);
	effect->setOffset(3);
	myPicLabel->setGraphicsEffect(effect);
//	myPicLabel->setMaximumSize(300,300);
//	myPicLabel->setMinimumSize(77,77);

	myTitleLabel = new QLabel;
	myAuthorLabel = new QLabel;
	myCategoriesLabel = new QLabel;
	mySummaryTitleLabel = new QLabel;
	mySummaryLabel = new QLabel;

	mySummaryScrollArea = new QScrollArea;
	mySummaryScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	mySummaryScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mySummaryScrollArea->setWidgetResizable(true);
	mySummaryScrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	mySummaryScrollArea->setWidget(mySummaryLabel);
	mySummaryLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	mySummaryLabel->setMargin(3);
	//mySummaryLabel->setMaximumHeight(200);
	mySummaryLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	//mySummaryLabel->setIndent(3);

	myActionsWidget = new QWidget;

	QVBoxLayout *previewLayout = new QVBoxLayout;
	QHBoxLayout *actionsLayout = new QHBoxLayout;
	myActionsWidget->setLayout(actionsLayout);
	previewLayout->setSizeConstraint(QLayout::SetMinimumSize);

	QHBoxLayout *picLayout = new QHBoxLayout;
	picLayout->addStretch();
	picLayout->addWidget(myPicLabel);
	picLayout->addStretch();

	previewLayout->addLayout(picLayout);
	previewLayout->addWidget(myTitleLabel);
	previewLayout->addWidget(myAuthorLabel);
	previewLayout->addWidget(myCategoriesLabel);
	previewLayout->addWidget(mySummaryTitleLabel);
	previewLayout->addWidget(mySummaryScrollArea);
	previewLayout->addWidget(myActionsWidget);
	previewLayout->addStretch();

	setLayout(previewLayout);

	mySummaryScrollArea->hide();
}

void ZLQtPageWidget::setInfo(const ZLTreePageInfo &info) {
	shared_ptr<const ZLImage> image = info.image();
	if (!image.isNull()) {
		ZLNetworkManager::Instance().perform(image->synchronizationData());
		QPixmap pixmap = ZLQtImageUtils::ZLImageToQPixmap(image);
		if (pixmap.height() > PREVIEW_WIDTH || pixmap.width() > PREVIEW_WIDTH) {
			pixmap = pixmap.scaled(PREVIEW_WIDTH, PREVIEW_WIDTH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}
		myPicLabel->setPixmap(pixmap);
	}

	//TODO implement 'condition'-like attributes support for ZLResource
	static const ZLResource &resource = ZLResource::resource("bookInfo");
	static std::string VALUE_1 = "value 1";
	static QString colon = "<b>%1</b> %2";

	if (!info.title().empty()) {
		static QString title = QString::fromStdString(resource["title"].value());
		myTitleLabel->setText(colon.arg(title, QString::fromStdString(info.title())));
	}

	if (info.authors().size() > 0) {
		static QString authors = QString::fromStdString(resource["authors"].value());
		static QString author = QString::fromStdString(resource["authors"][VALUE_1].value());
		QString labelText = info.authors().size() == 1 ? author : authors;
		myAuthorLabel->setText(colon.arg(labelText, QString::fromStdString(ZLStringUtil::join(info.authors(), ", "))));
	}

	if (info.tags().size() > 0) {
		static QString tags = QString::fromStdString(resource["tags"].value());
		static QString tag = QString::fromStdString(resource["tags"][VALUE_1].value());
		QString labelText = info.tags().size() == 1 ? tag : tags;
		myCategoriesLabel->setText(colon.arg(labelText, QString::fromStdString(ZLStringUtil::join(info.tags(), ", "))));
	}

	if (!info.summary().empty()) {
		static QString annotation = QString::fromStdString(resource["annotation"].value());
		mySummaryLabel->setText(QString::fromStdString(info.summary()));
		mySummaryTitleLabel->setText(QString("<b>%1</b>").arg(annotation));
		mySummaryScrollArea->show();
	}

	myTitleLabel->setWordWrap(true);
	myAuthorLabel->setWordWrap(true);
	myCategoriesLabel->setWordWrap(true);
	mySummaryLabel->setWordWrap(true);

	foreach(shared_ptr<ZLTreeAction> action, info.actions()) {
		if (!action->makesSense()) {
			continue;
		}
		QPushButton *actionButton = new ZLQtButtonAction(action);
		QString text = QString::fromStdString(info.actionText(action));
		actionButton->setText(text);
		myButtons.push_back(actionButton);
		myActionsWidget->layout()->addWidget(actionButton);
	}
}

ZLQtCatalogPageWidget::ZLQtCatalogPageWidget(const ZLTreeTitledNode *node, QWidget *parent) : QWidget(parent) {
	createElements();
	setInfo(node);
}

void ZLQtCatalogPageWidget::createElements() {
	myPicLabel = new QLabel;
	myTitleLabel = new QLabel;
	mySubtitleLabel = new QLabel;

	mySubtitleLabel->setWordWrap(true);
	myTitleLabel->setAlignment(Qt::AlignCenter);
	mySubtitleLabel->setAlignment(Qt::AlignCenter);

	QVBoxLayout *previewLayout = new QVBoxLayout;

	QHBoxLayout *picLayout = new QHBoxLayout;
	picLayout->addStretch();
	picLayout->addWidget(myPicLabel);
	picLayout->addStretch();

	previewLayout->addStretch();
	previewLayout->addLayout(picLayout);
	previewLayout->addWidget(myTitleLabel);
	previewLayout->addWidget(mySubtitleLabel);
	previewLayout->addStretch();

	setLayout(previewLayout);
}

void ZLQtCatalogPageWidget::setInfo(const ZLTreeTitledNode *node) {
	shared_ptr<const ZLImage> image = node->image();
	if (!image.isNull()) {
		ZLNetworkManager::Instance().perform(image->synchronizationData());
		QPixmap pixmap = ZLQtImageUtils::ZLImageToQPixmap(image);
		if (pixmap.height() > PREVIEW_WIDTH || pixmap.width() > PREVIEW_WIDTH) {
			pixmap = pixmap.scaled(PREVIEW_WIDTH, PREVIEW_WIDTH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}
		myPicLabel->setPixmap(pixmap);
	}

	if (!node->title().empty()) {
		myTitleLabel->setText(QString("<b>%1</b>").arg(QString::fromStdString(node->title())));
	}
	if (!node->subtitle().empty()) {
		mySubtitleLabel->setText(QString::fromStdString(node->subtitle()));
	}

}

//void ZLQtPreviewWidget::clear() {
//	myPicLabel->setPixmap(QPixmap());
//	myTitleLabel->clear();
//	myAuthorLabel->clear();
//	myCategoriesLabel->clear();
//	mySummaryLabel->clear();
//	mySummaryTitleLabel->clear();
//	mySummaryScrollArea->hide();
//	foreach(QPushButton *button, myButtons) {
//		delete button;
//	}
//	myButtons.clear();
//}
