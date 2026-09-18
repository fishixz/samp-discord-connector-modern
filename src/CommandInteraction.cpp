#include "Command.hpp"
#include "Network.hpp"
#include "PawnDispatcher.hpp"
#include "Callback.hpp"
#include "Logger.hpp"
#include "utils.hpp"
#include "Bot.hpp"
#include "Guild.hpp"
#include "CommandInteraction.hpp"
#include "Channel.hpp"
#include "User.hpp"
#include "Embed.hpp"
#include "Component.hpp"
#include "Http.hpp"

#include <json.hpp>
#include <regex>

CommandInteraction::CommandInteraction(CommandInteractionId_t id, UserId_t user,
	nlohmann::json const& interaction_json) :
	m_ID(id),
	m_InteractionUser(user)
{
	m_IDSnowflake = interaction_json.at("id").get<std::string>();
	m_Token = interaction_json.at("token").get<std::string>();
	m_Type = interaction_json.value("type", 0);

	std::string guild_str;
	bool has_guild = utils::TryGetJsonValue(interaction_json, guild_str, "guild_id");
	if (has_guild)
	{
		auto const& guild = GuildManager::Get()->FindGuildById(guild_str);
		if (guild)
			m_Guild = guild->GetPawnId();
	}

	std::string channel_str;
	if (utils::TryGetJsonValue(interaction_json, channel_str, "channel_id"))
	{
		Channel_t const& channel = ChannelManager::Get()->FindChannelById(channel_str);
		if (!channel && !has_guild)
		{
			ChannelId_t cid = ChannelManager::Get()->AddDMChannel(interaction_json);
			auto const& dm = ChannelManager::Get()->FindChannel(cid);
			m_Channel = dm ? dm->GetPawnId() : INVALID_CHANNEL_ID;
		}
		else
		{
			m_Channel = channel ? channel->GetPawnId() : INVALID_CHANNEL_ID;
		}
	}

	if (m_Type == 2)
	{
		std::string name;
		if (!utils::TryGetJsonValue(interaction_json, name, "data", "name"))
			return;

		if (has_guild)
		{
			auto const& command = CommandManager::Get()->FindCommand(
				CommandManager::Get()->FindCommandIdByName(name, m_Guild));
			if (!command || command->GetGuild() != m_Guild)
			{
				Logger::Get()->Log(samplog_LogLevel::WARNING,
					"received a command interaction for command {} (guild {}) but the callee guild doesn't match",
					name, guild_str);
				return;
			}
		}
		else
		{
			auto const& command = CommandManager::Get()->FindCommand(
				CommandManager::Get()->FindCommandIdByName(name));
			if (!command)
			{
				Logger::Get()->Log(samplog_LogLevel::WARNING,
					"received a command interaction for command {} but no command exists", name);
				return;
			}
		}

		ParseOptions(interaction_json, has_guild ? guild_str : "");
		// CommandManager defers application commands before constructing this object.
		m_Responded = true;
	}
	else if (m_Type == 3 || m_Type == 5)
	{
		ParseComponentData(interaction_json);
	}
}

void CommandInteraction::ParseOptions(nlohmann::json const& interaction_json, std::string const& guildid)
{
	CommandInteractionOption tmpoption;

	if (interaction_json.at("data").find("options") == interaction_json.at("data").end())
	{
		tmpoption.m_Type = COMMAND_OPTION_TYPE::OPTION_STRING;
		tmpoption.m_Name = "";
		tmpoption.m_Value = "";
	}
	else
	{
		auto first = interaction_json.at("data").at("options")[0];
		tmpoption.m_Type = static_cast<COMMAND_OPTION_TYPE>(first.at("type").get<int>());
		tmpoption.m_Name = first.at("name").get<std::string>();
		utils::TryGetJsonValue(first, tmpoption.m_Value, "value");

		if (tmpoption.m_Type == COMMAND_OPTION_TYPE::OPTION_STRING && guildid.length())
		{
			std::regex mentions("<@!\\s*(\\d+)\\s*>");
			auto mentions_begin = std::sregex_iterator(
				tmpoption.m_Value.begin(), tmpoption.m_Value.end(), mentions);
			auto mentions_end = std::sregex_iterator();

			for (std::sregex_iterator i = mentions_begin; i != mentions_end; ++i)
			{
				std::string match = (*i)[1].str();
				const User_t& mentioned_user = UserManager::Get()->FindUserById(match);
				if (mentioned_user)
					m_Mentions.push_back(mentioned_user->GetPawnId());
			}
		}
	}

	CommandInteractionOption_t option(
		new CommandInteractionOption(std::move(tmpoption)));
	AddInteractionOption(option);
}

void CommandInteraction::ParseComponentData(nlohmann::json const& interaction_json)
{
	if (interaction_json.find("data") == interaction_json.end())
		return;

	auto const& data = interaction_json.at("data");
	utils::TryGetJsonValue(data, m_CustomId, "custom_id");

	if (m_Type == 3)
	{
		utils::TryGetJsonValue(data, m_ComponentType, "component_type");
		if (data.find("values") != data.end() && data.at("values").is_array())
		{
			for (auto const& value : data.at("values"))
			{
				if (value.is_string())
					m_Values.push_back(value.get<std::string>());
			}
		}
	}
	else if (m_Type == 5 && data.find("components") != data.end())
	{
		ParseModalComponents(data.at("components"));
	}
}

void CommandInteraction::ParseModalComponents(nlohmann::json const& components)
{
	if (components.is_array())
	{
		for (auto const& component : components)
			ParseModalComponents(component);
		return;
	}

	if (!components.is_object())
		return;

	if (components.find("components") != components.end())
		ParseModalComponents(components.at("components"));

	if (components.find("component") != components.end())
		ParseModalComponents(components.at("component"));

	std::string custom_id;
	if (!utils::TryGetJsonValue(components, custom_id, "custom_id"))
		return;

	std::string value;
	if (utils::TryGetJsonValue(components, value, "value"))
	{
		m_ModalValues[custom_id] = value;
		return;
	}

	if (components.find("values") != components.end() && components.at("values").is_array())
	{
		std::string joined;
		for (auto const& entry : components.at("values"))
		{
			if (!entry.is_string())
				continue;
			if (!joined.empty())
				joined += ",";
			joined += entry.get<std::string>();
		}
		m_ModalValues[custom_id] = joined;
	}
}

void CommandInteraction::AddInteractionOption(CommandInteractionOption_t &option)
{
	m_InteractionOptions.push_back(std::move(option));
}

bool CommandInteraction::GetModalValue(std::string const& custom_id, std::string& value) const
{
	auto it = m_ModalValues.find(custom_id);
	if (it == m_ModalValues.end())
		return false;
	value = it->second;
	return true;
}

bool CommandInteraction::SendInitialResponse(int response_type, nlohmann::json const* data)
{
	if (m_Responded)
		return false;

	json payload = { { "type", response_type } };
	if (data != nullptr)
		payload["data"] = *data;

	std::string json_str;
	if (!utils::TryDumpJson(payload, json_str))
	{
		Logger::Get()->Log(samplog_LogLevel::ERROR, "can't serialize interaction response JSON: {}", json_str);
		return false;
	}

	Network::Get()->Http().Post(
		fmt::format("/interactions/{:s}/{:s}/callback", m_IDSnowflake, m_Token),
		json_str);

	m_Responded = true;
	return true;
}

void CommandInteraction::SendEmbed(EmbedId_t embedid, const std::string message)
{
	auto& embed = EmbedManager::Get()->FindEmbed(embedid);
	if (!embed)
		return;

	json data = {
		{ "content", message },
		{ "embeds", { json::object() } }
	};

	data["embeds"][0] = json::object({
		{ "title", embed->GetTitle() },
		{ "description", embed->GetDescription() },
		{ "url", embed->GetUrl() },
		{ "timestamp", embed->GetTimestamp() },
		{ "color", embed->GetColor() },
		{ "footer", {
			{"text", embed->GetFooterText()},
			{"icon_url", embed->GetFooterIconUrl()},
		} },
		{ "thumbnail", json::object() },
		{ "image", json::object() }
	});

	if (!embed->GetThumbnailUrl().empty())
		data["embeds"][0]["thumbnail"]["url"] = embed->GetThumbnailUrl();
	if (!embed->GetImageUrl().empty())
		data["embeds"][0]["image"]["url"] = embed->GetImageUrl();

	if (embed->GetFields().size())
	{
		json field_array = json::array();
		for (const auto& i : embed->GetFields())
		{
			field_array.push_back({
				{"name", i._name},
				{"value", i._value},
				{"inline", i._inline_}
			});
		}
		data["embeds"][0]["fields"] = field_array;
	}

	if (!m_Responded)
	{
		SendInitialResponse(4, &data);
		return;
	}

	std::string json_str;
	if (!utils::TryDumpJson(data, json_str))
	{
		Logger::Get()->Log(samplog_LogLevel::ERROR, "can't serialize JSON: {}", json_str);
		return;
	}

	Network::Get()->Http().Patch(
		fmt::format("/webhooks/{:s}/{:s}/messages/@original",
			ThisBot::Get()->GetApplicationID(), m_Token),
		json_str);
}

void CommandInteraction::SendInteractionMessage(const std::string message, bool ephemeral)
{
	json data = { { "content", message } };
	if (ephemeral)
		data["flags"] = 64;

	if (!m_Responded)
	{
		SendInitialResponse(4, &data);
		return;
	}

	// Ephemeral flags cannot be changed after the initial response.
	data.erase("flags");

	std::string json_str;
	if (!utils::TryDumpJson(data, json_str))
	{
		Logger::Get()->Log(samplog_LogLevel::ERROR, "can't serialize JSON: {}", json_str);
		return;
	}

	Network::Get()->Http().Patch(
		fmt::format("/webhooks/{:s}/{:s}/messages/@original",
			ThisBot::Get()->GetApplicationID(), m_Token),
		json_str);
}

void CommandInteraction::UpdateInteractionMessage(const std::string message)
{
	json data = { { "content", message } };

	if (!m_Responded)
	{
		SendInitialResponse(7, &data);
		return;
	}

	std::string json_str;
	if (!utils::TryDumpJson(data, json_str))
	{
		Logger::Get()->Log(samplog_LogLevel::ERROR, "can't serialize JSON: {}", json_str);
		return;
	}

	Network::Get()->Http().Patch(
		fmt::format("/webhooks/{:s}/{:s}/messages/@original",
			ThisBot::Get()->GetApplicationID(), m_Token),
		json_str);
}

bool CommandInteraction::ShowModal(ModalId_t modalid)
{
	if (m_Responded)
		return false;

	auto const& modal = ModalManager::Get()->Find(modalid);
	if (!modal)
		return false;

	return SendInitialResponse(9, &modal->GetData());
}

bool CommandInteraction::Acknowledge()
{
	if (m_Responded)
		return true;

	if (m_Type == 3)
		return SendInitialResponse(6);

	// Modal submissions can be deferred as a channel response.
	return SendInitialResponse(5);
}

CommandInteraction_t const &CommandInteractionManager::FindCommandInteraction(CommandInteractionId_t interaction)
{
	static CommandInteraction_t invalid_command_interaction;
	auto it = m_Interactions.find(interaction);
	if (it == m_Interactions.end())
		return invalid_command_interaction;
	return it->second;
}

CommandInteractionId_t CommandInteractionManager::AddCommandInteraction(
	UserId_t user, nlohmann::json const& interaction_json)
{
	CommandInteractionId_t id = 1;
	while (m_Interactions.find(id) != m_Interactions.end())
		++id;

	if (!m_Interactions.emplace(id,
		CommandInteraction_t(new CommandInteraction(id, user, interaction_json))).first->second)
	{
		Logger::Get()->Log(samplog_LogLevel::ERROR,
			"can't create command interaction: duplicate key '{}'", id);
		return INVALID_COMMAND_INTERACTION_ID;
	}

	Logger::Get()->Log(samplog_LogLevel::DEBUG,
		"successfully created interaction with id '{}'", id);
	return id;
}

bool CommandInteractionManager::DeleteCommandInteraction(CommandInteractionId_t interaction)
{
	if (m_Interactions.find(interaction) == m_Interactions.end())
	{
		Logger::Get()->Log(samplog_LogLevel::WARNING,
			"attempted to delete interaction with id '{}' but it does not exist", interaction);
		return false;
	}

	m_Interactions.erase(interaction);
	Logger::Get()->Log(samplog_LogLevel::DEBUG,
		"successfully deleted interaction with id '{}'", interaction);
	return true;
}
