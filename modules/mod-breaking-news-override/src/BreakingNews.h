#ifndef MODULE_BREAKINGNEWS_H
#define MODULE_BREAKINGNEWS_H

#include "Config.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "WardenWin.h"

#include <algorithm>
#include <fstream>
#include <iostream>

bool bn_Enabled;

std::string bn_Title;
std::string bn_Body;
std::string bn_Formatted;

const std::string _prePayload = "wlbuf = '';";
const std::string _postPayload = "local a,b=loadstring(wlbuf)if not a then message(b)else a()end";
const std::string _midPayloadFmt = "local a=ServerAlertFrame;local b=ServerAlertText;local c=ServerAlertTitle;local d=CharacterSelect;local sf=_G[\'ServerAlertScrollFrame\'] or _G[\'ServerAlertTextScrollFrame\'];if a~=nil and b~=nil and c~=nil and d~=nil then a:SetParent(d);a:ClearAllPoints();a:SetPoint(\'TOPLEFT\',d,\'TOPLEFT\',{},{})a:SetWidth({})a:SetHeight({});c:ClearAllPoints();c:SetPoint(\'TOP\',a,\'TOP\',0,{});if c.SetTextColor then c:SetTextColor(1,0.82,0)end;if b.SetJustifyH then b:SetJustifyH(\'LEFT\')end;if b.SetJustifyV then b:SetJustifyV(\'TOP\')end;if b.SetSpacing then b:SetSpacing({})end;if sf~=nil then sf:ClearAllPoints();sf:SetPoint(\'TOPLEFT\',a,\'TOPLEFT\',{},{})sf:SetPoint(\'BOTTOMRIGHT\',a,\'BOTTOMRIGHT\',{},{})end;b:SetWidth({})b:SetHeight({})ServerAlertTitle:SetText(\'{}\')ServerAlertText:SetText(\'{}\')a:Show()else message(\'ServerAlert Frame is nil.\')end";
uint16 _prePayloadId = 9500;
uint16 _postPayloadId = 9501;
uint16 _tmpPayloadId = 9502;

std::vector<std::string> GetChunks(std::string s, uint8_t chunkSize);
void SendChunkedPayload(Warden* warden, std::string payload, uint32 chunkSize);

class BreakingNewsServerScript : ServerScript
{
public:
    BreakingNewsServerScript() : ServerScript("BreakingNewsServerScript", {
        SERVERHOOK_CAN_PACKET_SEND
    }) { }

private:
    bool CanPacketSend(WorldSession* session, WorldPacket const& packet) override;
    std::vector<std::string> GetChunks(std::string s, uint8_t chunkSize);
    void SendChunkedPayload(Warden* warden, WardenPayloadMgr* payloadMgr, std::string payload, uint32 chunkSize);
};

class BreakingNewsWorldScript : public WorldScript
{
public:
    BreakingNewsWorldScript() : WorldScript("BreakingNewsWorldScript", {
        WORLDHOOK_ON_AFTER_CONFIG_LOAD
    }) { }

private:
    void OnAfterConfigLoad(bool reload) override;
};

#endif //MODULE_BREAKINGNEWS_H
