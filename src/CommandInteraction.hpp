#pragma once

#include "types.hpp"
#include "Singleton.hpp"
#include "PawnDispatcher.hpp"
#include "Callback.hpp"
#include "Command.hpp"

#include <json.hpp>
#include <map>
#include <string>
#include <vector>

using json = nlohmann::json;
static CommandInteractionOption_t InvalidCommandInteractionChildOption;

struct CommandInteractionOption
{
	COMMAND_OPTION_TYPE m_Type;
	std::string m_Name;
	std::string m_Value;
};

class CommandInteraction
{
public:
	CommandInteraction(CommandInteractionId_t id, UserId_t user, nlohmann::json const& interaction_json);
	~CommandInteraction() = default;

	void ParseOptions(nlohmann::json const& interaction_json, std::string const& guildid);
	void AddInteractionOption(CommandInteractionOption_t &option);

	CommandInteractionId_t GetPawnId() const { return m_ID; }
	GuildId_t GetGuildID() const { return m_Guild; }
	ChannelId_t GetChannelID() const { return m_Channel; }
	UserId_t GetUserID() const { return m_InteractionUser; }
	int GetType() const { return m_Type; }
	int GetComponentType() const { return m_ComponentType; }
	std::string const& GetCustomId() const { return m_CustomId; }
	bool HasResponded() const { return m_Responded; }

	std::vector<UserId_t> const& GetMentions() const { return m_Mentions; }
	std::vector<CommandInteractionOption_t> const& GetOptions() const { return m_InteractionOptions; }
	std::vector<std::string> const& GetValues() const { return m_Values; }

	bool GetModalValue(std::string const& custom_id, std::string& value) const;

	void SendEmbed(EmbedId_t embedid, const std::string message = "");
	void SendInteractionMessage(const std::string message, bool ephemeral = false);
	void UpdateInteractionMessage(const std::string message);
	bool ShowModal(ModalId_t modalid);
	bool Acknowledge();

private:
	void ParseComponentData(nlohmann::json const& interaction_json);
	void ParseModalComponents(nlohmann::json const& components);
	bool SendInitialResponse(int response_type, nlohmann::json const* data = nullptr);

	CommandInteractionId_t m_ID;
	Snowflake_t m_IDSnowflake;
	Snowflake_t m_Token;
	ChannelId_t m_Channel = INVALID_CHANNEL_ID;
	GuildId_t m_Guild = INVALID_GUILD_ID;
	UserId_t m_InteractionUser = INVALID_USER_ID;
	int m_Type = 0;
	int m_ComponentType = 0;
	std::string m_CustomId;
	bool m_Responded = false;

	std::vector<CommandInteractionOption_t> m_InteractionOptions;
	std::vector<UserId_t> m_Mentions;
	std::vector<std::string> m_Values;
	std::map<std::string, std::string> m_ModalValues;
};

class CommandInteractionManager : public Singleton<CommandInteractionManager>
{
	friend class Singleton<CommandInteractionManager>;
private:
	CommandInteractionManager() = default;
	~CommandInteractionManager() = default;

	std::map<CommandInteractionId_t, CommandInteraction_t> m_Interactions;

public:
	CommandInteraction_t const& FindCommandInteraction(CommandInteractionId_t interaction);
	CommandInteractionId_t AddCommandInteraction(UserId_t user, nlohmann::json const& interaction_json);
	bool DeleteCommandInteraction(CommandInteractionId_t interaction);

	CommandInteractionId_t m_CurrentInteractionID = 0;
};
