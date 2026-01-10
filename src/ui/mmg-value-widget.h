/*
obs-midi-mg
Copyright (C) 2022-2024 nhielost <nhielost@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#ifndef MMG_VALUE_WIDGET_H
#define MMG_VALUE_WIDGET_H

#include "../mmg-params.h"
#include "../mmg-value.h"

#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include <QScrollBar>

namespace MMGWidgets {

class MMGValueWidget : public QWidget {
	Q_OBJECT

public:
	MMGValueWidget(QWidget *parent = nullptr);
	virtual ~MMGValueWidget() = default;

	bool isModifiable() const { return !pixmap_view || pixmap_view->isHidden(); };
	void setModifiable(bool modify);

	virtual void reset() = 0;

protected:
	void focusInEvent(QFocusEvent *) final override;
	void focusOutEvent(QFocusEvent *) final override;
	void resizeEvent(QResizeEvent *) override;

	void resizePixmap(const QSize &size);

	virtual bool focusAcquired() { return false; };
	virtual bool focusLost() { return false; };

signals:
	void valueChanged() const;

public slots:
	virtual void display() const = 0;
	void update() const;

protected:
	QHBoxLayout *main_layout;

	QLabel *pixmap_view = nullptr;
};

void clearLayout(QLayout *layout);
void createScrollBarOverlay(QAbstractScrollArea *scroller);

const char *getStateName(ValueState state);
void setStateIconAndToolTip(QWidget *icon_display, ValueState state);

#define icon_url(name) ":/icons/" name ".svg"
#define mmgicon(name) QIcon(icon_url(name))

#define prompt_question(name)                                                                                    \
	(QMessageBox::question(nullptr, mmgtr(jointr("UI.MessageBox.Title", name)),                              \
			       mmgtr(jointr("UI.MessageBox.Text", name)), QMessageBox::Ok | QMessageBox::Cancel, \
			       QMessageBox::Ok) == QMessageBox::Ok)

#define prompt_info(name)                                                              \
	(QMessageBox::information(nullptr, mmgtr(jointr("UI.MessageBox.Title", name)), \
				  mmgtr(jointr("UI.MessageBox.Text", name))))

} // namespace MMGWidgets

#endif // MMG_VALUE_WIDGET_H
