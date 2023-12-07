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

#pragma once
#include "../mmg-utils.h"
#include "mmg-number-display.h"
#include "mmg-string-display.h"

#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QScrollArea>
#include <QLayout>
#include <QToolButton>
#include <QFont>
#include <QTextEdit>
#include <QScopedPointer>
#include <QFileDialog>

#define HAS_BOOLEAN_FIELD if (!!boolean_field)

struct MMGOBSFieldInit {
	obs_property_t *prop;
	QJsonObject current_json;
	QJsonObject default_json;
};
Q_DECLARE_METATYPE(MMGOBSFieldInit);

class MMGOBSField : public QWidget {
	Q_OBJECT

public:
	virtual ~MMGOBSField() = default;

	const QString &name() const { return _name; };

	virtual void jsonUpdate(QJsonObject &json_obj) const = 0;
	virtual void jsonData(QJsonObject &json_obj) const = 0;
	virtual void apply(const QJsonObject &json_obj) = 0;
	virtual void refresh(const MMGOBSFieldInit &init) = 0;
	virtual void update(obs_property_t *prop) = 0;

signals:
	void triggerUpdate();
	void saveUpdates();
	void saveData();

protected:
	QString _name;

	QLabel *label = nullptr;

	MMGOBSField(QWidget *widget = nullptr);

	const QSignalBlocker updateAndBlock(obs_property_t *prop);

	static const QString frame_style;
};

class MMGOBSNumberField : public MMGOBSField {
	Q_OBJECT

public:
	MMGOBSNumberField(QWidget *parent, const MMGOBSFieldInit &init);

	void jsonUpdate(QJsonObject &json_obj) const override { json_obj[_name] = number.value(); };
	void jsonData(QJsonObject &json_obj) const override;
	void apply(const QJsonObject &json_obj) override;
	void refresh(const MMGOBSFieldInit &init) override;
	void update(obs_property_t *prop) override;

private:
	MMGUtils::MMGNumber number;
	MMGNumberDisplay *num_display;

	static const MMGUtils::MMGNumber propertyBounds(obs_property_t *prop);
	static const MMGUtils::MMGNumber propertyBounds(obs_source_t *source, const QString &field);
};

class MMGOBSStringField : public MMGOBSField {
	Q_OBJECT

public:
	MMGOBSStringField(QWidget *parent, const MMGOBSFieldInit &init);

	void jsonUpdate(QJsonObject &json_obj) const override;
	void jsonData(QJsonObject &json_obj) const override;
	void apply(const QJsonObject &json_obj) override;
	void refresh(const MMGOBSFieldInit &init) override;
	void update(obs_property_t *prop) override;

	static const QVariantList propertyValues(obs_source_t *source, const QString &field);

private:
	MMGUtils::MMGString string;
	MMGStringDisplay *str_display;
	QStringList options;
	QVariantList values;

	static const QVariantList propertyValues(obs_property_t *prop);
	static const QStringList propertyDescriptions(obs_property_t *prop);
};

class MMGOBSBooleanField : public MMGOBSField {
	Q_OBJECT

public:
	MMGOBSBooleanField(QWidget *parent, const MMGOBSFieldInit &init);

	void jsonUpdate(QJsonObject &json_obj) const override { json_obj[_name] = boolean; };
	void jsonData(QJsonObject &json_obj) const override;
	void apply(const QJsonObject &json_obj) override;
	void refresh(const MMGOBSFieldInit &init) override;
	void update(obs_property_t *prop) override { updateAndBlock(prop); };

private slots:
	void changeBoolean(bool value);
	void changeState(int state);

private:
	bool boolean;
	MMGUtils::ValueState state = MMGUtils::STATE_FIXED;

	QPushButton *button;
	MMGMIDIButtons *midi_buttons;

	friend class MMGOBSGroupField;
};

class MMGOBSButtonField : public MMGOBSField {
	Q_OBJECT

public:
	MMGOBSButtonField(QWidget *parent, const MMGOBSFieldInit &init);

	void callback(obs_property_t *prop, void *source);
	void jsonUpdate(QJsonObject &) const override{};
	void jsonData(QJsonObject &) const override{};
	void apply(const QJsonObject &) override{};
	void refresh(const MMGOBSFieldInit &init) override;
	void update(obs_property_t *prop) override;

private:
	QPushButton *button;

	static const QString style_sheet;

signals:
	void clicked();
};

class MMGOBSColorField : public MMGOBSField {
	Q_OBJECT

public:
	MMGOBSColorField(QWidget *parent, const MMGOBSFieldInit &init, bool use_alpha);

	void callback();
	void jsonUpdate(QJsonObject &json_obj) const override;
	void jsonData(QJsonObject &json_obj) const override;
	void apply(const QJsonObject &json_obj) override;
	void refresh(const MMGOBSFieldInit &init) override;
	void update(obs_property_t *prop) override;

private:
	QColor color;
	bool alpha;

	QFrame *frame;
	QPushButton *button;

	static double convertColor(double rgb);
};

class MMGOBSFontField : public MMGOBSField {
	Q_OBJECT

public:
	MMGOBSFontField(QWidget *parent, const MMGOBSFieldInit &init);

	void callback();
	void jsonUpdate(QJsonObject &json_obj) const override;
	void jsonData(QJsonObject &json_obj) const override;
	void apply(const QJsonObject &json_obj) override;
	void refresh(const MMGOBSFieldInit &init) override;
	void update(obs_property_t *prop) override;

private:
	QFont font;

	QFrame *frame;
	QPushButton *button;
	QLabel *inner_label;
};

class MMGOBSGroupField : public MMGOBSField {
	Q_OBJECT

public:
	MMGOBSGroupField(QWidget *parent, const MMGOBSFieldInit &init);

	void jsonUpdate(QJsonObject &json_obj) const override;
	void jsonData(QJsonObject &json_obj) const override;
	void apply(const QJsonObject &) override{};
	void refresh(const MMGOBSFieldInit &init) override;
	void update(obs_property_t *prop) override;

	MMGOBSBooleanField *booleanField() const { return boolean_field; };

private:
	MMGOBSBooleanField *boolean_field = nullptr;

	~MMGOBSGroupField() { HAS_BOOLEAN_FIELD delete boolean_field; }
};

class MMGOBSPathField : public MMGOBSField {
	Q_OBJECT

public:
	MMGOBSPathField(QWidget *parent, const MMGOBSFieldInit &init);

	void callback();
	void jsonUpdate(QJsonObject &json_obj) const override { json_obj[_name] = path; };
	void jsonData(QJsonObject &json_obj) const override;
	void apply(const QJsonObject &json_obj) override;
	void refresh(const MMGOBSFieldInit &init) override;
	void update(obs_property_t *prop) override;

private:
	QString path;
	MMGUtils::ValueState state = MMGUtils::STATE_FIXED;

	short dialog_type;
	QString filters;
	QString default_path;

	QFrame *frame;
	QPushButton *button;
	QLabel *path_display;
};

class MMGOBSTextField : public MMGOBSField {
	Q_OBJECT

public:
	MMGOBSTextField(QWidget *parent, const MMGOBSFieldInit &init);

	void callback(const QString &str);
	void jsonUpdate(QJsonObject &json_obj) const override { json_obj[_name] = text; };
	void jsonData(QJsonObject &json_obj) const override;
	void apply(const QJsonObject &json_obj) override;
	void refresh(const MMGOBSFieldInit &init) override;
	void update(obs_property_t *prop) override;

	bool checkMIDI() const;

private:
	QString text;
	MMGUtils::ValueState state = MMGUtils::STATE_FIXED;

	QTextEdit *text_edit;
	QLineEdit *line_edit;
};

class MMGOBSFields : public QWidget {
	Q_OBJECT

public:
	MMGOBSFields(obs_source_t *source);
	~MMGOBSFields() { obs_properties_destroy(props); }

	void changeSource(obs_source_t *source);
	bool match(obs_source_t *source) const { return identifier == obs_source_get_id(source); };
	void setJsonDestination(MMGUtils::MMGJsonObject *json);

	void addFields(QWidget *parent, obs_properties_t *props, const MMGOBSFieldInit &json_init);

public slots:
	void updateField();
	void jsonUpdate();
	void jsonData();

private slots:
	void buttonFieldClicked(); // For when an MMGOBSButtonField is clicked

private:
	QString identifier;
	QString source_name;
	obs_properties_t *props = nullptr;
	QJsonObject json_src;

	MMGUtils::MMGJsonObject *json_des = nullptr;
	QList<MMGOBSField *> fields;

	MMGOBSFieldInit getSourceData(obs_source_t *source);
	void addGroupContent(MMGOBSGroupField *field, const MMGOBSFieldInit &init);
};

using MMGOBSFieldsList = QList<MMGOBSFields *>;
