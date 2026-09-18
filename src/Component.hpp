#pragma once

#include "types.hpp"
#include "Singleton.hpp"

#include <json.hpp>
#include <map>
#include <string>
#include <vector>

using json = nlohmann::json;

enum class DiscordComponentType : int
{
	ACTION_ROW = 1,
	BUTTON = 2,
	STRING_SELECT = 3,
	TEXT_INPUT = 4,
	USER_SELECT = 5,
	ROLE_SELECT = 6,
	MENTIONABLE_SELECT = 7,
	CHANNEL_SELECT = 8,
	TEXT_DISPLAY = 10,
	LABEL = 18,
	FILE_UPLOAD = 19,
	RADIO_GROUP = 21,
	CHECKBOX_GROUP = 22,
	CHECKBOX = 23
};

enum class DiscordButtonStyle : int
{
	PRIMARY = 1,
	SECONDARY = 2,
	SUCCESS = 3,
	DANGER = 4,
	LINK = 5
};

enum class DiscordTextInputStyle : int
{
	SHORT = 1,
	PARAGRAPH = 2
};

class Component
{
public:
	Component(ComponentId_t id, json data);
	~Component() = default;

	ComponentId_t GetPawnId() const { return m_Id; }
	DiscordComponentType GetType() const;
	json const& GetData() const { return m_Data; }

	bool AddChild(Component const& child);
	bool AddSelectOption(std::string const& label, std::string const& value,
		std::string const& description, std::string const& emoji, bool is_default);

private:
	ComponentId_t m_Id;
	json m_Data;
};

class ComponentManager : public Singleton<ComponentManager>
{
	friend class Singleton<ComponentManager>;
private:
	ComponentManager() = default;
	~ComponentManager() = default;

	std::map<ComponentId_t, Component_t> m_Components;

public:
	ComponentId_t CreateActionRow();
	ComponentId_t CreateButton(std::string const& custom_id, std::string const& label,
		int style, bool disabled, std::string const& emoji, std::string const& url);
	ComponentId_t CreateSelect(int type, std::string const& custom_id, std::string const& placeholder,
		int min_values, int max_values, bool disabled);
	ComponentId_t CreateFileUpload(std::string const& custom_id, int min_values, int max_values, bool required);
	ComponentId_t CreateChoiceGroup(int type, std::string const& custom_id,
		int min_values, int max_values, bool required);
	ComponentId_t CreateCheckbox(std::string const& custom_id, bool is_default);
	bool AddFileType(ComponentId_t id, std::string const& file_type);

	Component_t const& Find(ComponentId_t id);
	bool Delete(ComponentId_t id);
};

class Modal
{
public:
	Modal(ModalId_t id, std::string custom_id, std::string title);
	~Modal() = default;

	ModalId_t GetPawnId() const { return m_Id; }
	json const& GetData() const { return m_Data; }

	bool AddTextInput(std::string const& custom_id, std::string const& label, int style,
		std::string const& placeholder, bool required, int min_length, int max_length,
		std::string const& value, std::string const& description);
	bool AddSelect(Component const& select, std::string const& label,
		std::string const& description, bool required);
	bool AddInputComponent(Component const& component, std::string const& label,
		std::string const& description);
	bool AddTextDisplay(std::string const& content);

private:
	ModalId_t m_Id;
	json m_Data;
};

class ModalManager : public Singleton<ModalManager>
{
	friend class Singleton<ModalManager>;
private:
	ModalManager() = default;
	~ModalManager() = default;

	std::map<ModalId_t, Modal_t> m_Modals;

public:
	ModalId_t Create(std::string const& custom_id, std::string const& title);
	Modal_t const& Find(ModalId_t id);
	bool Delete(ModalId_t id);
};
