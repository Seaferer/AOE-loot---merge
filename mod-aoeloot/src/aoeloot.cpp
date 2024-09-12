#include "aoeloot.h"

class AOEloot_player : public PlayerScript {
public:
    AOEloot_player() : PlayerScript("AOEloot_player") {}

    void OnLogin(Player* player) override
    {
        if (sConfigMgr->GetOption<bool>("AOELt.enable", true))        
            ChatHandler(player->GetSession()).PSendSysMessage("This server is running the |cff4CFF00AOE-loot by Dev. Seaferer|r module.");        
    }
};

class AOEloot_sscript : public ServerScript {
public:
    AOEloot_sscript() : ServerScript("AOEloot_sscript") {}
   
    bool CanPacketReceive(WorldSession* session, WorldPacket& packet) override
    {
        if (packet.GetOpcode() == CMSG_LOOT)
        {
            Player* player = session->GetPlayer();         

            if (!sConfigMgr->GetOption<bool>("AOELt.enable", true))
                return true;

            if (player->GetGroup() && !sConfigMgr->GetOption<bool>("AOELt.group", true))
                return true;

            std::list<Creature*> lootcreature; player->GetDeadCreatureListInGrid(lootcreature, 55.f);
            ObjectGuid guid; packet >> guid;
            Loot* mainloot = &(player->GetMap()->GetCreature(guid))->loot;   

            for (auto itr = lootcreature.begin(); itr != lootcreature.end(); ++itr)
            {
                Creature* creature = *itr;

                //Prevent infiny add items
                if (creature->GetGUID() == guid)
                    continue;

                //Prevent steal loot
                if (!creature->loot.hasItemFor(player))
                    continue;

                //Max 15 items per creature
                if (mainloot->items.size() + mainloot->quest_items.size() > 15)
                    break;

                Loot* loot = &(*itr)->loot;    

                //FILL QITEMS
                if (!loot->quest_items.empty())
                {
                    mainloot->items.insert(mainloot->items.end(), loot->quest_items.begin(), loot->quest_items.end());
                    loot->quest_items.clear();
                }

                //FILL GOLD
                if (loot->gold != 0)
                {
                    mainloot->gold += loot->gold;
                    loot->gold = 0;
                }
                
                //FILL ITEMS
                if (!loot->items.empty())
                {                
                    mainloot->items.insert(mainloot->items.end(), loot->items.begin(), loot->items.end());
                    loot->items.clear();                    
                }

                //Set flag for skinning
                if (loot->items.empty() && loot->quest_items.empty())
                {
                    creature->AllLootRemovedFromCorpse();
                    creature->RemoveDynamicFlag(UNIT_DYNFLAG_LOOTABLE);
                }                
            }            
            player->SendLoot(guid, LOOT_CORPSE);
            return false;
        }
        return true;
    }  
};

void AddSC_aoeloot()
{
    new AOEloot_player();
    new AOEloot_sscript();
}
