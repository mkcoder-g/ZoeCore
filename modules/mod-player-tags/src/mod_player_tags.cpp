/*
 * ZoeCore - Player Tags FREE / VIP / VIP MASTER
 *
 * A tag é decidida no QueryHandler.cpp por item na bag:
 *   sem item VIP        = <FREE>Nome
 *   com VipItemId       = <VIP>Nome
 *   com VipMasterItemId = <VIP MASTER>Nome
 */

#include "Config.h"
#include "Log.h"
#include "ScriptMgr.h"

class mod_player_tags_worldscript : public WorldScript
{
public:
    mod_player_tags_worldscript() : WorldScript("mod_player_tags_worldscript") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        if (!sConfigMgr->GetOption<bool>("PlayerTags.Enable", true))
        {
            LOG_INFO("module", "mod-player-tags: disabled");
            return;
        }

        LOG_INFO("module", "mod-player-tags: enabled. VIP item: {}, VIP MASTER item: {}",
            sConfigMgr->GetOption<uint32>("PlayerTags.VipItemId", 90000),
            sConfigMgr->GetOption<uint32>("PlayerTags.VipMasterItemId", 90001));
    }
};

void Addmod_player_tagsScripts()
{
    new mod_player_tags_worldscript();
}
