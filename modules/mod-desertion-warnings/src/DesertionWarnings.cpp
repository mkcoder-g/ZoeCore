/*
 * This file is part of the AzerothCore Project.
 * ZoeCore customization: PT-BR, top-screen notification, standardized config,
 * and economy penalty for battleground desertion.
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "Log.h"
#include "ObjectMgr.h"

#include <algorithm>
#include <string>

namespace
{
    bool ZoeEnabled()
    {
        return sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.Enable", true);
    }

    bool ModuleEnabled()
    {
        return sConfigMgr->GetOption<bool>("DesertionWarnings.Enabled", true) && ZoeEnabled();
    }

    std::string Prefix()
    {
        return sConfigMgr->GetOption<std::string>("DesertionWarnings.Zoe.MessagePrefix", "[ZoeCore BG]");
    }

    void ReplaceAll(std::string& text, std::string const& search, std::string const& replace)
    {
        if (search.empty())
            return;

        size_t pos = 0;
        while ((pos = text.find(search, pos)) != std::string::npos)
        {
            text.replace(pos, search.length(), replace);
            pos += replace.length();
        }
    }

    bool IsQueueWarningType(BattlegroundDesertionType type)
    {
        switch (type)
        {
            case BG_DESERTION_TYPE_LEAVE_QUEUE:
            case BG_DESERTION_TYPE_NO_ENTER_BUTTON:
                return true;
            default:
                return false;
        }
    }

    bool ShouldWarn(BattlegroundDesertionType type)
    {
        switch (type)
        {
            case BG_DESERTION_TYPE_LEAVE_QUEUE:
                return sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.Warn.LeaveQueue", true);
            case BG_DESERTION_TYPE_NO_ENTER_BUTTON:
                return sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.Warn.NoEnterButton", true);
            default:
                return false;
        }
    }

    bool IsPotentialLeaveBgType(BattlegroundDesertionType type)
    {
        if (IsQueueWarningType(type))
            return false;

        return sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.LeaveBG.Penalty.UnknownTypeAsLeaveBG", true);
    }

    std::string GetWarningText(BattlegroundDesertionType type)
    {
        switch (type)
        {
            case BG_DESERTION_TYPE_LEAVE_QUEUE:
                return sConfigMgr->GetOption<std::string>(
                    "DesertionWarnings.Zoe.LeaveQueueText",
                    "|cffff2020Voce saiu da fila da BG apos ser chamado.|r"
                );
            case BG_DESERTION_TYPE_NO_ENTER_BUTTON:
                return sConfigMgr->GetOption<std::string>(
                    "DesertionWarnings.Zoe.NoEnterText",
                    "|cffff2020Voce nao entrou na BG apos o convite.|r"
                );
            default:
                return sConfigMgr->GetOption<std::string>(
                    "DesertionWarnings.Zoe.WarningText",
                    "|cffff2020Voce recusou a entrada da BG.|r"
                );
        }
    }

    void SendTopScreen(Player* player, std::string const& message)
    {
        if (!player || !player->GetSession())
            return;

        player->GetSession()->SendAreaTriggerMessage(message.c_str());
    }

    void SendChat(Player* player, std::string const& message)
    {
        if (!player || !player->GetSession())
            return;

        ChatHandler(player->GetSession()).PSendSysMessage("{} {}", Prefix().c_str(), message.c_str());
    }

    void SendWarning(Player* player, BattlegroundDesertionType type)
    {
        if (!player)
            return;

        std::string message = GetWarningText(type);

        if (sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.TopScreen.Enable", true))
            SendTopScreen(player, message);

        if (sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.Chat.Enable", false))
            SendChat(player, message);

        if (sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.AbuseReminder.Enable", true))
        {
            std::string reminder = sConfigMgr->GetOption<std::string>(
                "DesertionWarnings.Zoe.AbuseReminderText",
                "|cffffcc00Aviso registrado. Repetir isso pode resultar em punicao pela staff.|r"
            );

            if (sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.TopScreen.Enable", true))
                SendTopScreen(player, reminder);

            if (sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.Chat.Enable", false))
                SendChat(player, reminder);
        }

        if (sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.Log.Enable", true))
        {
            LOG_INFO("module", "ZoeCore DesertionWarnings: queue warning player='{}' guid={} type={} zone={} area={}",
                player->GetName(),
                player->GetGUID().GetCounter(),
                uint32(type),
                player->GetZoneId(),
                player->GetAreaId());
        }
    }

    std::string BuildEconomyPenaltyText(uint32 honorLost, uint32 arenaLost, uint32 itemLost, uint32 itemEntry)
    {
        std::string message = sConfigMgr->GetOption<std::string>(
            "DesertionWarnings.Zoe.LeaveBG.Penalty.Message",
            "|cffff2020Voce abandonou a BG.|r |cffffff00Punicao aplicada: {penalty}.|r"
        );

        std::string penaltyText;

        if (honorLost > 0)
            penaltyText += std::to_string(honorLost) + " Honor";

        if (arenaLost > 0)
        {
            if (!penaltyText.empty())
                penaltyText += ", ";

            penaltyText += std::to_string(arenaLost) + " Arena Points";
        }

        if (itemLost > 0)
        {
            if (!penaltyText.empty())
                penaltyText += ", ";

            std::string itemName = "item " + std::to_string(itemEntry);

            if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemEntry))
                itemName = proto->Name1;

            penaltyText += std::to_string(itemLost) + "x " + itemName;
        }

        if (penaltyText.empty())
            penaltyText = "nenhum recurso removido";

        ReplaceAll(message, "{penalty}", penaltyText);
        ReplaceAll(message, "{honor}", std::to_string(honorLost));
        ReplaceAll(message, "{arena}", std::to_string(arenaLost));
        ReplaceAll(message, "{item}", std::to_string(itemLost));

        return message;
    }

    void ApplyDeserterFallback(Player* player, BattlegroundDesertionType type)
    {
        if (!player)
            return;

        if (!sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.LeaveBG.DeserterFallback.Enable", true))
            return;

        if (!IsPotentialLeaveBgType(type))
            return;

        uint32 spellId = sConfigMgr->GetOption<uint32>("DesertionWarnings.Zoe.LeaveBG.DeserterFallback.SpellId", 26013);
        if (!spellId)
            return;

        bool onlyIfMissing = sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.LeaveBG.DeserterFallback.OnlyIfMissing", true);
        if (onlyIfMissing && player->HasAura(spellId))
            return;

        player->AddAura(spellId, player);

        if (sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.LeaveBG.DeserterFallback.Log", true))
        {
            LOG_INFO("module", "ZoeCore DesertionWarnings: deserter fallback applied player='{}' guid={} type={} spell={} zone={} area={}",
                player->GetName(),
                player->GetGUID().GetCounter(),
                uint32(type),
                spellId,
                player->GetZoneId(),
                player->GetAreaId());
        }
    }

    void ApplyEconomyPenalty(Player* player, BattlegroundDesertionType type)
    {
        if (!player)
            return;

        if (!sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.LeaveBG.Penalty.Enable", true))
            return;

        if (!IsPotentialLeaveBgType(type))
            return;

        if (sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.LeaveBG.Penalty.RequireBattleground", false) && !player->GetBattleground())
            return;

        uint32 honorLost = 0;
        uint32 arenaLost = 0;
        uint32 itemLost = 0;

        if (sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.LeaveBG.Penalty.RemoveHonor", true))
        {
            uint32 requestedHonor = sConfigMgr->GetOption<uint32>("DesertionWarnings.Zoe.LeaveBG.Penalty.HonorAmount", 250);
            honorLost = std::min<uint32>(requestedHonor, player->GetHonorPoints());

            if (honorLost > 0)
                player->ModifyHonorPoints(-int32(honorLost));
        }

        if (sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.LeaveBG.Penalty.RemoveArenaPoints", false))
        {
            uint32 requestedArena = sConfigMgr->GetOption<uint32>("DesertionWarnings.Zoe.LeaveBG.Penalty.ArenaPointsAmount", 10);
            arenaLost = std::min<uint32>(requestedArena, player->GetArenaPoints());

            if (arenaLost > 0)
                player->ModifyArenaPoints(-int32(arenaLost));
        }

        uint32 itemEntry = sConfigMgr->GetOption<uint32>("DesertionWarnings.Zoe.LeaveBG.Penalty.ItemEntry", 900001);

        if (sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.LeaveBG.Penalty.RemoveItem", true) && itemEntry > 0)
        {
            uint32 requestedItem = sConfigMgr->GetOption<uint32>("DesertionWarnings.Zoe.LeaveBG.Penalty.ItemCount", 2);
            uint32 availableItem = player->GetItemCount(itemEntry, true);
            itemLost = std::min<uint32>(requestedItem, availableItem);

            if (itemLost > 0)
                player->DestroyItemCount(itemEntry, itemLost, true);
        }

        std::string message = BuildEconomyPenaltyText(honorLost, arenaLost, itemLost, itemEntry);

        if (sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.LeaveBG.Penalty.Announce", true))
        {
            if (sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.TopScreen.Enable", true))
                SendTopScreen(player, message);

            if (sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.Chat.Enable", false))
                SendChat(player, message);
        }

        if (sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.LeaveBG.Penalty.Log", true)
            || sConfigMgr->GetOption<bool>("DesertionWarnings.Zoe.Log.Enable", true))
        {
            LOG_INFO("module", "ZoeCore DesertionWarnings: BG economy penalty player='{}' guid={} type={} honorLost={} arenaLost={} itemEntry={} itemLost={} zone={} area={}",
                player->GetName(),
                player->GetGUID().GetCounter(),
                uint32(type),
                honorLost,
                arenaLost,
                itemEntry,
                itemLost,
                player->GetZoneId(),
                player->GetAreaId());
        }
    }
}

class DesertionWarnings : public PlayerScript
{
public:
    DesertionWarnings() : PlayerScript("DesertionWarnings", {
        PLAYERHOOK_ON_BATTLEGROUND_DESERTION
    }) { }

    void OnPlayerBattlegroundDesertion(Player* player, const BattlegroundDesertionType type) override
    {
        if (!ModuleEnabled())
            return;

        if (ShouldWarn(type))
        {
            SendWarning(player, type);
            return;
        }

        ApplyDeserterFallback(player, type);
        ApplyEconomyPenalty(player, type);
    }
};

void AddDesertionWarningsScripts()
{
    new DesertionWarnings();
}
