/*
obs-midi-mg
Copyright (C) 2022-2026 nhielost <nhielost@gmail.com>

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

#ifndef MMG_OBS_FIELDS_H
#define MMG_OBS_FIELDS_H

#include "actions/mmg-action.h"
#include "mmg-manager.h"

class QPushButton;
class QLabel;

namespace MMGOBSFields {

#define ENUMERATE_PROPS(f)                   \
	for (MMGOBSProperty * prop : _props) \
	prop->f

class MMGOBSPropertyManager;
class MMGOBSObject;

void addRefreshCallback(MMGWidgets::MMGValueManager *display, QObject *binder, const MMGCallback &cb);

class MMGOBSProperty : public QObject {
	Q_OBJECT

public:
	MMGOBSProperty(MMGOBSPropertyManager *parent, obs_property_t *prop);
	virtual ~MMGOBSProperty() = default;

	const char *name() const { return _name; };

	virtual void loadProperty(const QJsonObject &data) = 0;
	virtual void loadData(const QJsonObject &data) = 0;
	virtual void setDefaultValue(const QJsonObject &) {};

	virtual void jsonProperty(QJsonObject &data) const = 0;
	virtual void jsonData(QJsonObject &data) const = 0;

	void copy(MMGOBSProperty *) {};

	virtual void createDisplay(MMGWidgets::MMGValueManager *display) = 0;

	virtual void execute(const MMGMappingTest &test, QJsonObject &data) const = 0;
	virtual void processEvent(MMGMappingTest &test, const QJsonObject &data) const = 0;

signals:
	void valueChanged() const;

protected:
	const char *_name;
	obs_property_t *_prop;
};

template <typename T> class MMGOBSField : public MMGOBSProperty {

public:
	MMGOBSField(MMGOBSPropertyManager *parent, obs_property_t *prop) : MMGOBSProperty(parent, prop)
	{
		_storage.template changeTo<STATE_IGNORE>();
	};

	// LOAD
	void loadProperty(const QJsonObject &data) override { _storage = getPropertyValue(data); };
	void loadData(const QJsonObject &data) override { _storage = MMGValue<T>(data, _name); };
	void setDefaultValue(const QJsonObject &data) override { default_value = getPropertyValue(data); };

	// SAVE
	void jsonProperty(QJsonObject &data) const override { setPropertyValue(data, storageValue()); };
	void jsonData(QJsonObject &data) const override { _storage->json(data, _name); };

	// DISPLAY
	void createDisplay(MMGWidgets::MMGValueManager *display) override
	{
		if (!widget_params) widget_params.reset(new MMGParams<T>(widgetParams()));
		addRefreshCallback(display, this, std::bind(&MMGOBSField<T>::displayChanged, this));
		MMGParameters::Construct<T>::createField(display, &_storage, widget_params.get(),
							 std::bind(&MMGOBSProperty::valueChanged, this));
	};

	void displayChanged() const
	{
		widget_params->desc = nontr(obs_property_description(_prop));
		widget_params->options.setFlag(OPTION_DISABLED,
					       !(obs_property_visible(_prop) && obs_property_enabled(_prop)));
	};

	// RUN
	void execute(const MMGMappingTest &test, QJsonObject &data) const override
	{
		T prop_value = getPropertyValue(data);
		ACTION_ASSERT(test.applicable(_storage, prop_value),
			      "An OBS property could not be selected. Check all OBS fields "
			      "and try again.");
		setPropertyValue(data, prop_value);
	};

	void processEvent(MMGMappingTest &test, const QJsonObject &data) const override
	{
		test.addAcceptable(_storage, getPropertyValue(data));
	};

private:
	T getPropertyValue(const QJsonObject &data) const { return MMGJson::getValue<T>(data, _name); };
	void setPropertyValue(QJsonObject &data, const T &value) const { MMGJson::setValue(data, _name, value); };

	T storageValue() const { return _storage.converts() ? _storage : default_value; };
	MMGParams<T> widgetParams() const;

private:
	MMGValue<T> _storage;
	T default_value;

	std::unique_ptr<MMGParams<T>> widget_params;
};

class MMGOBSGroupField : public MMGOBSProperty {
	Q_OBJECT

public:
	MMGOBSGroupField(MMGOBSPropertyManager *parent, obs_property_t *prop);
	~MMGOBSGroupField();

	void loadProperty(const QJsonObject &json_obj) override;
	void loadData(const QJsonObject &json_obj) override;

	void jsonProperty(QJsonObject &json_obj) const override;
	void jsonData(QJsonObject &json_obj) const override;

	void createDisplay(MMGWidgets::MMGValueManager *display) override;
	void displayNormalChanged(QLabel *label) const;
	void displayCheckableChanged(MMGWidgets::MMGValueManager *display) const;

	void execute(const MMGMappingTest &test, QJsonObject &data) const override;
	void processEvent(MMGMappingTest &test, const QJsonObject &data) const override;

private:
	MMGParams<bool> controllerParams() const;

private:
	MMGBoolean controller;
	std::unique_ptr<MMGParams<bool>> controller_params;

	MMGOBSPropertyManager *group_props;
};

class MMGOBSPropertyManager : public QObject {
	Q_OBJECT

public:
	MMGOBSPropertyManager(MMGOBSObject *parent, obs_properties_t *props);

	const char *sourceId() const;
	qsizetype size() const { return _props.size(); };

	void loadProperties(const QJsonObject &current_json, const QJsonObject &default_json = {}) const;
	void loadData(const QJsonObject &data) const { ENUMERATE_PROPS(loadData(data)); };

	void jsonProperties(QJsonObject &data) const { ENUMERATE_PROPS(jsonProperty(data)); };
	void jsonData(QJsonObject &data) const { ENUMERATE_PROPS(jsonData(data)); };

	void createDisplays(MMGWidgets::MMGValueManager *display) const { ENUMERATE_PROPS(createDisplay(display)); };

	void execute(const MMGMappingTest &test, QJsonObject &data) const { ENUMERATE_PROPS(execute(test, data)); };
	void processEvent(MMGMappingTest &test, const QJsonObject &data) const
	{
		ENUMERATE_PROPS(processEvent(test, data));
	};

signals:
	void propertyChanged();

private:
	QList<MMGOBSProperty *> _props;
};

class MMGOBSObject : public QObject {
	Q_OBJECT

public:
	MMGOBSObject(QObject *parent, const MMGString &source_id, const QJsonObject &json_obj);
	~MMGOBSObject()
	{
		if (!!props) obs_properties_destroy(props);
	};

	const char *sourceId() const { return source_uuid; };
	void changeSource(const MMGString &source_id, const QJsonObject &json_obj = {});

	void json(QJsonObject &json_obj) const;
	void copy(MMGOBSObject *dest) const;

	void createDisplay(MMGWidgets::MMGActionDisplay *display);

	void execute(const MMGMappingTest &test) const;
	void processEvent(MMGMappingTest &test) const;

private:
	void init() { changeSource(source_uuid, property_json); };
	void loadProperties();

signals:
	void sourceChanged();
	void propertyChanged();

private slots:
	void updateProperties();

private:
	MMGString source_uuid;

	obs_properties_t *props = nullptr;
	MMGOBSPropertyManager *props_manager = nullptr;
	mutable QJsonObject property_json;

	MMGWidgets::MMGActionDisplay *created_display = nullptr;
	int32_t front_widgets_num = 0;
};

#undef ENUMERATE_PROPS

template <> MMGParams<int32_t> MMGOBSField<int32_t>::widgetParams() const;
template <> MMGParams<float> MMGOBSField<float>::widgetParams() const;
template <> MMGParams<MMGString> MMGOBSField<MMGString>::widgetParams() const;
template <> MMGParams<bool> MMGOBSField<bool>::widgetParams() const;
template <> MMGParams<QColor> MMGOBSField<QColor>::widgetParams() const;
template <> MMGParams<QFont> MMGOBSField<QFont>::widgetParams() const;
template <> MMGParams<QDir> MMGOBSField<QDir>::widgetParams() const;
template <> MMGParams<QString> MMGOBSField<QString>::widgetParams() const;

} // namespace MMGOBSFields

#endif // MMG_OBS_FIELDS_H
