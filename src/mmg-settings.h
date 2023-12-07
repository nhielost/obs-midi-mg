/*
obs-midi-mg
Copyright (C) 2022-2023 nhielost <nhielost@gmail.com>

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

#ifndef MMG_SETTINGS_H
#define MMG_SETTINGS_H

#include "mmg-utils.h"
#include "mmg-manager.h"

class MMGBinding;
class MMGSettingsManager;

class MMGSettings : public QObject {

public:
	MMGSettings(MMGSettingsManager *parent);

	virtual QString name() const = 0;
	void setName(const QString &){};

	virtual void json(QJsonObject &json_obj) const = 0;
	virtual void copy(MMGSettings *){};
	bool isEditable() const { return *editable; };

	virtual void createDisplay(QWidget *parent) { _display = new QWidget(parent); };
	QWidget *display() const { return _display; };

private:
	const bool *editable;
	QWidget *_display = nullptr;
};

class MMGGeneralSettings : public MMGSettings {
	Q_OBJECT

public:
	MMGGeneralSettings(MMGSettingsManager *parent, const QJsonObject &json_obj = QJsonObject());

	QString name() const override { return mmgtr("Preferences.General"); };
	void json(QJsonObject &json_obj) const override;
	void createDisplay(QWidget *parent) override;
};

class MMGBindingSettings : public MMGSettings {
	Q_OBJECT

public:
	MMGBindingSettings(MMGSettingsManager *parent, const QJsonObject &json_obj = QJsonObject());

	enum ResetBehavior { BEHAVIOR_RESET, BEHAVIOR_NO_RESET };

	QString name() const override { return mmgtr("Preferences.Binding"); };
	void json(QJsonObject &json_obj) const override;
	void copy(MMGSettings *dest) override;
	void createDisplay(QWidget *parent) override;

	ResetBehavior resetBehavior() const { return (ResetBehavior)resetBehaviorOptions().indexOf(reset_behavior); };

	static const QStringList resetBehaviorOptions();

signals:
	void resetBehaviorChanged();

private slots:
	void setLabel();

private:
	MMGUtils::MMGString reset_behavior;

	QLabel *reset_behavior_desc;
};

class MMGMessageLogSettings : public MMGSettings {
	Q_OBJECT

public:
	MMGMessageLogSettings(MMGSettingsManager *parent) : MMGSettings(parent){};

	QString name() const override { return mmgtr("Preferences.Log"); };
	void json(QJsonObject &) const override;
	void createDisplay(QWidget *parent) override;
};

class MMGSettingsManager : public MMGManager<MMGSettings> {

public:
	MMGSettingsManager(QObject *parent) : MMGManager(parent){};

	const bool &isEditable() const { return editable; };
	void setEditable(bool edit) { editable = edit; };
	bool filter(MMGUtils::DeviceType type, MMGSettings *) const override { return type == MMGUtils::TYPE_NONE; };

	MMGSettings *add(const QJsonObject &json_obj = QJsonObject()) override;

	static uint visiblePanes() { return 2; };

private:
	bool editable = true;
};

#endif // MMG_SETTINGS_H
