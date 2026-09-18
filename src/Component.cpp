#include "Component.hpp"

#include "Logger.hpp"

#include <algorithm>

Component::Component(ComponentId_t id, json data) :
	m_Id(id),
	m_Data(std::move(data))
{
}

DiscordComponentType Component::GetType() const
{
	return static_cast<DiscordComponentType>(m_Data.value("type", 0));
}

bool Component::AddChild(Component const& child)
{
	if (GetType() != DiscordComponentType::ACTION_ROW)
		return false;

	auto child_type = child.GetType();
	if (child_type == DiscordComponentType::ACTION_ROW || child_type == DiscordComponentType::TEXT_INPUT)
		return false;

	if (m_Data.find("components") == m_Data.end())
		m_Data["components"] = json::array();

	auto& children = m_Data["components"];
	if (!children.is_array() || children.size() >= 5)
		return false;

	bool child_is_button = child_type == DiscordComponentType::BUTTON;
	for (auto const& existing : children)
	{
		bool existing_is_button = existing.value("type", 0) == static_cast<int>(DiscordComponentType::BUTTON);
		if (existing_is_button != child_is_button)
			return false;
	}

	if (!child_is_button && !children.empty())
		return false;

	children.push_back(child.GetData());
	return true;
}

bool Component::AddSelectOption(std::string const& label, std::string const& value,
	std::string const& description, std::string const& emoji, bool is_default)
{
	auto type = GetType();
	if (type != DiscordComponentType::STRING_SELECT &&
		type != DiscordComponentType::RADIO_GROUP &&
		type != DiscordComponentType::CHECKBOX_GROUP)
		return false;

	if (m_Data.find("options") == m_Data.end())
		m_Data["options"] = json::array();

	auto& options = m_Data["options"];
	size_t max_options = type == DiscordComponentType::STRING_SELECT ? 25 : 10;
	if (!options.is_array() || options.size() >= max_options)
		return false;

	json option = {
		{ "label", label },
		{ "value", value },
		{ "default", is_default }
	};

	if (!description.empty())
		option["description"] = description;

	if (!emoji.empty() && type == DiscordComponentType::STRING_SELECT)
		option["emoji"] = { { "name", emoji } };

	options.push_back(std::move(option));
	return true;
}

ComponentId_t ComponentManager::CreateActionRow()
{
	ComponentId_t id = 1;
	while (m_Components.find(id) != m_Components.end())
		++id;

	json data = {
		{ "type", static_cast<int>(DiscordComponentType::ACTION_ROW) },
		{ "components", json::array() }
	};

	m_Components.emplace(id, Component_t(new Component(id, std::move(data))));
	return id;
}

ComponentId_t ComponentManager::CreateButton(std::string const& custom_id, std::string const& label,
	int style, bool disabled, std::string const& emoji, std::string const& url)
{
	if (style < static_cast<int>(DiscordButtonStyle::PRIMARY) ||
		style > static_cast<int>(DiscordButtonStyle::LINK))
		return INVALID_COMPONENT_ID;

	bool is_link = style == static_cast<int>(DiscordButtonStyle::LINK);
	if ((is_link && url.empty()) || (!is_link && custom_id.empty()))
		return INVALID_COMPONENT_ID;

	ComponentId_t id = 1;
	while (m_Components.find(id) != m_Components.end())
		++id;

	json data = {
		{ "type", static_cast<int>(DiscordComponentType::BUTTON) },
		{ "style", style },
		{ "label", label },
		{ "disabled", disabled }
	};

	if (is_link)
		data["url"] = url;
	else
		data["custom_id"] = custom_id;

	if (!emoji.empty())
		data["emoji"] = { { "name", emoji } };

	m_Components.emplace(id, Component_t(new Component(id, std::move(data))));
	return id;
}

ComponentId_t ComponentManager::CreateSelect(int type, std::string const& custom_id,
	std::string const& placeholder, int min_values, int max_values, bool disabled)
{
	if (type != static_cast<int>(DiscordComponentType::STRING_SELECT) &&
		type != static_cast<int>(DiscordComponentType::USER_SELECT) &&
		type != static_cast<int>(DiscordComponentType::ROLE_SELECT) &&
		type != static_cast<int>(DiscordComponentType::MENTIONABLE_SELECT) &&
		type != static_cast<int>(DiscordComponentType::CHANNEL_SELECT))
		return INVALID_COMPONENT_ID;

	if (custom_id.empty() || min_values < 0 || max_values < 1 || min_values > max_values || max_values > 25)
		return INVALID_COMPONENT_ID;

	ComponentId_t id = 1;
	while (m_Components.find(id) != m_Components.end())
		++id;

	json data = {
		{ "type", type },
		{ "custom_id", custom_id },
		{ "min_values", min_values },
		{ "max_values", max_values },
		{ "disabled", disabled }
	};

	if (!placeholder.empty())
		data["placeholder"] = placeholder;

	if (type == static_cast<int>(DiscordComponentType::STRING_SELECT))
		data["options"] = json::array();

	m_Components.emplace(id, Component_t(new Component(id, std::move(data))));
	return id;
}

ComponentId_t ComponentManager::CreateFileUpload(std::string const& custom_id,
	int min_values, int max_values, bool required)
{
	if (custom_id.empty() || custom_id.length() > 100 ||
		min_values < 0 || max_values < 1 || min_values > max_values || max_values > 10)
		return INVALID_COMPONENT_ID;

	ComponentId_t id = 1;
	while (m_Components.find(id) != m_Components.end())
		++id;

	json data = {
		{ "type", static_cast<int>(DiscordComponentType::FILE_UPLOAD) },
		{ "custom_id", custom_id },
		{ "min_values", min_values },
		{ "max_values", max_values },
		{ "required", required }
	};

	m_Components.emplace(id, Component_t(new Component(id, std::move(data))));
	return id;
}

ComponentId_t ComponentManager::CreateChoiceGroup(int type, std::string const& custom_id,
	int min_values, int max_values, bool required)
{
	if (type != static_cast<int>(DiscordComponentType::RADIO_GROUP) &&
		type != static_cast<int>(DiscordComponentType::CHECKBOX_GROUP))
		return INVALID_COMPONENT_ID;

	if (custom_id.empty() || custom_id.length() > 100)
		return INVALID_COMPONENT_ID;

	if (type == static_cast<int>(DiscordComponentType::RADIO_GROUP))
	{
		min_values = 1;
		max_values = 1;
	}
	else if (min_values < 0 || max_values < 1 || min_values > max_values || max_values > 10)
	{
		return INVALID_COMPONENT_ID;
	}

	ComponentId_t id = 1;
	while (m_Components.find(id) != m_Components.end())
		++id;

	json data = {
		{ "type", type },
		{ "custom_id", custom_id },
		{ "options", json::array() },
		{ "required", required }
	};

	if (type == static_cast<int>(DiscordComponentType::CHECKBOX_GROUP))
	{
		data["min_values"] = min_values;
		data["max_values"] = max_values;
	}

	m_Components.emplace(id, Component_t(new Component(id, std::move(data))));
	return id;
}

ComponentId_t ComponentManager::CreateCheckbox(std::string const& custom_id, bool is_default)
{
	if (custom_id.empty() || custom_id.length() > 100)
		return INVALID_COMPONENT_ID;

	ComponentId_t id = 1;
	while (m_Components.find(id) != m_Components.end())
		++id;

	json data = {
		{ "type", static_cast<int>(DiscordComponentType::CHECKBOX) },
		{ "custom_id", custom_id },
		{ "default", is_default }
	};

	m_Components.emplace(id, Component_t(new Component(id, std::move(data))));
	return id;
}

bool ComponentManager::AddFileType(ComponentId_t id, std::string const& file_type)
{
	auto const& component = Find(id);
	if (!component || component->GetType() != DiscordComponentType::FILE_UPLOAD ||
		file_type.empty())
		return false;

	json data = component->GetData();
	if (data.find("file_types") == data.end())
		data["file_types"] = json::array();

	auto& types = data["file_types"];
	if (!types.is_array() || types.size() >= 10)
		return false;

	types.push_back(file_type);

	// Replace the object while keeping the same Pawn id.
	m_Components[id].reset(new Component(id, std::move(data)));
	return true;
}

Component_t const& ComponentManager::Find(ComponentId_t id)
{
	static Component_t invalid_component;
	auto it = m_Components.find(id);
	if (it == m_Components.end())
		return invalid_component;
	return it->second;
}

bool ComponentManager::Delete(ComponentId_t id)
{
	return m_Components.erase(id) != 0;
}

Modal::Modal(ModalId_t id, std::string custom_id, std::string title) :
	m_Id(id)
{
	m_Data = {
		{ "custom_id", std::move(custom_id) },
		{ "title", std::move(title) },
		{ "components", json::array() }
	};
}

bool Modal::AddTextInput(std::string const& custom_id, std::string const& label, int style,
	std::string const& placeholder, bool required, int min_length, int max_length,
	std::string const& value, std::string const& description)
{
	if (m_Data["components"].size() >= 5)
		return false;

	if (custom_id.empty() || custom_id.length() > 100 || label.empty() || label.length() > 45 ||
		description.length() > 100 ||
		(style != static_cast<int>(DiscordTextInputStyle::SHORT) &&
		 style != static_cast<int>(DiscordTextInputStyle::PARAGRAPH)) ||
		min_length < 0 || max_length < 1 || min_length > max_length || max_length > 4000)
		return false;

	json input = {
		{ "type", static_cast<int>(DiscordComponentType::TEXT_INPUT) },
		{ "custom_id", custom_id },
		{ "style", style },
		{ "required", required },
		{ "min_length", min_length },
		{ "max_length", max_length }
	};

	if (!placeholder.empty())
		input["placeholder"] = placeholder;
	if (!value.empty())
		input["value"] = value;

	json label_component = {
		{ "type", static_cast<int>(DiscordComponentType::LABEL) },
		{ "label", label },
		{ "component", std::move(input) }
	};

	if (!description.empty())
		label_component["description"] = description;

	m_Data["components"].push_back(std::move(label_component));
	return true;
}

bool Modal::AddSelect(Component const& select, std::string const& label,
	std::string const& description, bool required)
{
	if (m_Data["components"].size() >= 5 || label.empty() || label.length() > 45 ||
		description.length() > 100)
		return false;

	auto type = select.GetType();
	if (type != DiscordComponentType::STRING_SELECT &&
		type != DiscordComponentType::USER_SELECT &&
		type != DiscordComponentType::ROLE_SELECT &&
		type != DiscordComponentType::MENTIONABLE_SELECT &&
		type != DiscordComponentType::CHANNEL_SELECT)
		return false;

	json select_data = select.GetData();
	select_data.erase("disabled");
	select_data["required"] = required;

	json label_component = {
		{ "type", static_cast<int>(DiscordComponentType::LABEL) },
		{ "label", label },
		{ "component", std::move(select_data) }
	};

	if (!description.empty())
		label_component["description"] = description;

	m_Data["components"].push_back(std::move(label_component));
	return true;
}

bool Modal::AddInputComponent(Component const& component, std::string const& label,
	std::string const& description)
{
	if (m_Data["components"].size() >= 5 || label.empty() || label.length() > 45 ||
		description.length() > 100)
		return false;

	auto type = component.GetType();
	if (type != DiscordComponentType::FILE_UPLOAD &&
		type != DiscordComponentType::RADIO_GROUP &&
		type != DiscordComponentType::CHECKBOX_GROUP &&
		type != DiscordComponentType::CHECKBOX)
		return false;

	auto const& component_data = component.GetData();
	if (type == DiscordComponentType::RADIO_GROUP)
	{
		auto it = component_data.find("options");
		if (it == component_data.end() || !it->is_array() || it->size() < 2)
			return false;
	}
	else if (type == DiscordComponentType::CHECKBOX_GROUP)
	{
		auto it = component_data.find("options");
		if (it == component_data.end() || !it->is_array() || it->empty())
			return false;
	}

	json label_component = {
		{ "type", static_cast<int>(DiscordComponentType::LABEL) },
		{ "label", label },
		{ "component", component.GetData() }
	};

	if (!description.empty())
		label_component["description"] = description;

	m_Data["components"].push_back(std::move(label_component));
	return true;
}

bool Modal::AddTextDisplay(std::string const& content)
{
	if (m_Data["components"].size() >= 5 || content.empty())
		return false;

	m_Data["components"].push_back({
		{ "type", static_cast<int>(DiscordComponentType::TEXT_DISPLAY) },
		{ "content", content }
	});
	return true;
}

ModalId_t ModalManager::Create(std::string const& custom_id, std::string const& title)
{
	if (custom_id.empty() || title.empty())
		return INVALID_MODAL_ID;

	ModalId_t id = 1;
	while (m_Modals.find(id) != m_Modals.end())
		++id;

	m_Modals.emplace(id, Modal_t(new Modal(id, custom_id, title)));
	return id;
}

Modal_t const& ModalManager::Find(ModalId_t id)
{
	static Modal_t invalid_modal;
	auto it = m_Modals.find(id);
	if (it == m_Modals.end())
		return invalid_modal;
	return it->second;
}

bool ModalManager::Delete(ModalId_t id)
{
	return m_Modals.erase(id) != 0;
}
