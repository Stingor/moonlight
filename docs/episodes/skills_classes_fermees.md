# Compétences de classes fermées : table de remplacement

Table validée et appliquée le 2026-11-07. Les comptes d'objets et de combos sont ceux d'avant la conversion.

## Règles

- **Remplacement :** même classe jouable, même valeur de bonus.
- **`bSkillCooldown` :** même règle que pour 16.2, il devient `bDelayrate` (1 s = 5 %), plafonné à -25 par bonus. Un seul délai converti reste par script, le plus fort.
- **Bonus calculés :** ils s'écrivent `max(-25, ...)` et peuvent s'additionner sur un même objet (`Racing_Cap_GS`).
- **Exceptions :** `Highness_Heal_3sec` perd son bonus, et `Naqsh` garde seulement son propre `bDelayRate,-100`.
- **Doublons :** quand deux compétences d'un objet tombent sur le même remplacement, seule la plus forte valeur reste, sans addition.
- **`-` :** le bonus est retiré.
- **garder :** l'objet ne change pas (voir plus bas).

## Crusader / Paladin (Royal Guard)

| Compétence | Remplacement | Objets | Combos | Exemple |
|---|---|---:|---:|---|
| `LG_BANISHINGPOINT` | `CR_HOLYCROSS` | 3 | 3 | Hero_Plate, Imperial_Spear |
| `LG_CANNONSPEAR` | `PA_SHIELDCHAIN` | 4 | 5 | Imperial_Spear, Cannon_Spear |
| `LG_EARTHDRIVE` | `CR_SHIELDBOOMERANG` | 1 | 2 | Racing_Cap_RG |
| `LG_INSPIRATION` | `PA_GOSPEL` | 1 | 0 | Spearfish_ |
| `LG_OVERBRAND` | `KN_BRANDISHSPEAR` | 3 | 2 | Velum_Glaive, Metal_Stick |
| `LG_PINPOINTATTACK` | `CR_HOLYCROSS` | 1 | 0 | Velum_Spear |
| `LG_RAGEBURST` | `PA_SACRIFICE` | 1 | 0 | Racing_Cap_RG |
| `LG_RAYOFGENESIS` | `CR_GRANDCROSS` | 2 | 3 | Divine_Guard_Hat, Farthezan |
| `LG_SHIELDPRESS` | `CR_SHIELDCHARGE` | 2 | 7 | Imperial_Guard, Harve |

## Knight / Lord Knight (Rune Knight)

| Compétence | Remplacement | Objets | Combos | Exemple |
|---|---|---:|---:|---|
| `RK_ABUNDANCE` | garder | 1 | 0 | Runstone_Urj |
| `RK_CRUSHSTRIKE` | garder | 1 | 0 | Runstone_Rhydo |
| `RK_DRAGONBREATH` | `KN_BOWLINGBASH` | 3 | 0 | Red_Baby_Dragon, LoVA_Bahamut_Card |
| `RK_DRAGONBREATH_WATER` | **retiré** | 3 | 0 | Red_Baby_Dragon, LoVA_Bahamut_Card |
| `RK_FIGHTINGSPIRIT` | garder | 1 | 0 | Runstone_Asir |
| `RK_GIANTGROWTH` | garder | 1 | 0 | Runstone_Turisus |
| `RK_HUNDREDSPEAR` | `KN_PIERCE` | 4 | 1 | Ur_Seal, Hero_Plate |
| `RK_IGNITIONBREAK` | `KN_BOWLINGBASH` | 1 | 1 | Volar |
| `RK_MILLENNIUMSHIELD` | garder | 1 | 0 | Runstone_Verkana |
| `RK_REFRESH` | garder | 1 | 0 | Runstone_Nosiege |
| `RK_SONICWAVE` | `LK_SPIRALPIERCE` | 1 | 2 | Vernan |
| `RK_STONEHARDSKIN` | garder | 1 | 0 | Runstone_Hagalas |
| `RK_STORMBLAST` | `KN_BOWLINGBASH` | 1 | 0 | Runstone_Pertz |
| `RK_VITALITYACTIVATION` | garder | 1 | 0 | Runstone_Isia |

## Priest / High Priest (Arch Bishop)

| Compétence | Remplacement | Objets | Combos | Exemple |
|---|---|---:|---:|---|
| `AB_ADORAMUS` | `PR_MAGNUS` | 4 | 6 | Hero_Judgement_Shawl, Racing_Cap_AB |
| `AB_CHEAL` | `AL_HEAL` | 6 | 3 | Medical_Boots, Coluceo_Heal30 |
| `AB_CLEARANCE` | `PR_LEXAETERNA` | 1 | 1 | Velum_Bible |
| `AB_CLEMENTIA` | `AL_BLESSING` | 1 | 0 | Robe_Of_Affection |
| `AB_DUPLELIGHT` | `RC_Undead (bonus de race)` | 2 | 1 | Racing_Cap_AB, Lucis_Flail |
| `AB_HIGHNESSHEAL` | `AL_HEAL` | 5 | 3 | Highness_Heal_3sec, Recovery_Drop |
| `AB_JUDEX` | `AL_HOLYLIGHT` | 4 | 3 | Shoes_Of_Judgement, Hero_Judgement_Shawl |
| `AB_LAUDAAGNUS` | `PR_STRECOVERY` | 1 | 2 | Velum_Bible |
| `AB_LAUDARAMUS` | `PR_SLOWPOISON` | 1 | 2 | Velum_Bible |
| `AB_ORATIO` | `PR_LEXAETERNA` | 1 | 0 | Shawl_Of_Judgement |
| `AB_SECRAMENT` | `HP_ASSUMPTIO` | 0 | 1 |  |
| `AB_SILENTIUM` | `PR_LEXDIVINA` | 1 | 1 | Ganbantein |

## Wizard / High Wizard (Warlock)

| Compétence | Remplacement | Objets | Combos | Exemple |
|---|---|---:|---:|---|
| `WL_CHAINLIGHTNING` | `WZ_VERMILION` | 1 | 2 | GodOfWinds_Fan |
| `WL_COMET` | `WZ_METEOR` | 1 | 1 | Astro_Circle |
| `WL_CRIMSONROCK` | `WZ_METEOR` | 3 | 1 | Hero_Magic_Coat, Astro_Circle |
| `WL_DRAINLIFE` | `MG_SOULSTRIKE` | 1 | 2 | Gravitation_Staff |
| `WL_EARTHSTRAIN` | `WZ_HEAVENDRIVE` | 0 | 2 |  |
| `WL_FROSTMISTY` | `WZ_STORMGUST` | 0 | 4 |  |
| `WL_HELLINFERNO` | `WZ_FIREPILLAR` | 5 | 1 | EL_A17T_Card, Shadow_Staff |
| `WL_JACKFROST` | `WZ_FROSTNOVA` | 0 | 3 |  |
| `WL_RECOGNIZEDSPELL` | `HW_MAGICPOWER` | 1 | 0 | Naqsh |
| `WL_SOULEXPANSION` | `MG_SOULSTRIKE` | 3 | 1 | Telekinetic_Orb, Staff_of_Miracle |
| `WL_TETRAVORTEX` | `WZ_VERMILION` | 1 | 0 | Racing_Cap_WL |

## Sage / Scholar (Sorcerer)

| Compétence | Remplacement | Objets | Combos | Exemple |
|---|---|---:|---:|---|
| `SO_DIAMONDDUST` | `MG_COLDBOLT` | 2 | 1 | Chilly_Spell_Book, Dust_Grave |
| `SO_EARTHGRAVE` | `WZ_EARTHSPIKE` | 1 | 1 | Dust_Grave |
| `SO_PSYCHIC_WAVE` | `PF_SOULBURN` | 5 | 2 | Telekinetic_Orb, Evilspirit_Gloves |
| `SO_SPELLFIST` | `SA_AUTOSPELL` | 1 | 0 | Sorc_Night_Cap |
| `SO_STRIKING` | `PF_DOUBLECASTING` | 0 | 2 |  |
| `SO_VARETYR_SPEAR` | `MG_THUNDERSTORM` | 1 | 0 | Psychic_Spear_Rod |

## Hunter / Sniper (Ranger)

| Compétence | Remplacement | Objets | Combos | Exemple |
|---|---|---:|---:|---|
| `RA_AIMEDBOLT` | `SN_SHARPSHOOTING` | 3 | 2 | White_Wing_Boots, Racing_Cap_RA |
| `RA_ARROWSTORM` | `AC_SHOWER` | 5 | 5 | Gambler_Seal, Upd_Bow_Guardian_Card |
| `RA_CLUSTERBOMB` | `HT_BLASTMINE` | 0 | 1 |  |
| `RA_FEARBREEZE` | `HT_FALCON` | 0 | 3 |  |
| `RA_UNLIMIT` | `SN_WINDWALK` | 0 | 1 |  |

## Bard, Dancer / Clown, Gypsy (Minstrel, Wanderer)

| Compétence | Remplacement | Objets | Combos | Exemple |
|---|---|---:|---:|---|
| `MI_ECHOSONG` | `BA_POEMBRAGI` | 1 | 0 | Minstrel_Song_Hat |
| `MI_RUSH_WINDMILL` | `BA_WHISTLE` | 3 | 2 | Minstrel_Song_Hat, Green_Whistle |
| `WA_MOONLIT_SERENADE` | `DC_SERVICEFORYOU` | 0 | 2 |  |
| `WA_SWING_DANCE` | `DC_HUMMING` | 3 | 0 | Dance_Shoes, Dying_Swan |
| `WA_SYMPHONY_OF_LOVER` | `DC_FORTUNEKISS` | 1 | 0 | Dying_Swan |
| `WM_LESSON` | `BA_MUSICALLESSON` | 1 | 0 | Lyrica_Hat |
| `WM_LULLABY_DEEPSLEEP` | `BD_LULLABY` | 3 | 0 | Miming_Card, Green_Whistle |
| `WM_METALICSOUND` | `BA_MUSICALSTRIKE` | 4 | 3 | Sound_Amplifier, Emerald_Earring |
| `WM_RANDOMIZESPELL` | `BD_ENCORE` | 1 | 0 | Contabass |
| `WM_REVERBERATION` | `BA_MUSICALSTRIKE` | 3 | 1 | Racing_Cap_MI, Racing_Cap_WA |
| `WM_SEVERE_RAINSTORM` | `CG_ARROWVULCAN` | 7 | 3 | Lyrica_Hat, Mystic_Bow |
| `WM_VOICEOFSIREN` | `DC_WINKCHARM` | 1 | 0 | Rosebine |

## Blacksmith / Whitesmith (Mechanic)

| Compétence | Remplacement | Objets | Combos | Exemple |
|---|---|---:|---:|---|
| `NC_ARMSCANNON` | `WS_CARTTERMINATION` | 1 | 1 | Engine_Pilebuncker |
| `NC_AXEBOOMERANG` | `MC_CARTREVOLUTION` | 4 | 2 | Supplement_Part_Dex, Sea_Captain_Hat |
| `NC_AXETORNADO` | `MC_MAMMONITE` | 3 | 2 | Sea_Captain_Hat, Tornado_Axe |
| `NC_BOOSTKNUCKLE` | `WS_CARTTERMINATION` | 2 | 1 | Upgrade_Part_Gun_Barrel, Engine_Pilebuncker |
| `NC_HOVERING` | `WS_CARTBOOST` | 1 | 0 | Upgrade_Part_Booster |
| `NC_MAGMA_ERUPTION` | `BS_MAXIMIZE` | 1 | 0 | Maxi_Spanner |
| `NC_POWERSWING` | `MC_MAMMONITE` | 3 | 1 | Golden_Wrench, Guardian_Knight_Axe |
| `NC_TRAININGAXE` | `BS_AXE` | 1 | 0 | Sea_Captain_Hat |
| `NC_VULCANARM` | `WS_CARTTERMINATION` | 1 | 0 | Engine_Pilebuncker |

## Alchemist / Creator (Genetic)

| Compétence | Remplacement | Objets | Combos | Exemple |
|---|---|---:|---:|---|
| `GN_CARTBOOST` | `WS_CARTBOOST` | 0 | 3 |  |
| `GN_CARTCANNON` | `AM_ACIDTERROR` | 3 | 1 | Harvester_Hat, Sapphire_Mace-OS |
| `GN_CART_TORNADO` | `WS_CARTTERMINATION` | 1 | 0 | Racing_Cap_GN |
| `GN_CRAZYWEED` | `AM_DEMONSTRATION` | 2 | 1 | Red_Ether_Bag, Gene_Rod |
| `GN_DEMONIC_FIRE` | `AM_DEMONSTRATION` | 1 | 0 | Red_Ether_Bag |
| `GN_MANDRAGORA` | `AM_BERSERKPITCHER` | 0 | 2 |  |
| `GN_SPORE_EXPLOSION` | `AM_ACIDTERROR` | 2 | 1 | Sapphire_Wrist, Estal |

## Assassin / Assassin Cross (Guillotine Cross)

| Compétence | Remplacement | Objets | Combos | Exemple |
|---|---|---:|---:|---|
| `GC_COUNTERSLASH` | `AS_SONICBLOW` | 1 | 2 | Oni_Horns_ |
| `GC_CROSSIMPACT` | `AS_SONICBLOW` | 6 | 2 | Muffler_Of_Roki, Oni_Horns_ |
| `GC_CROSSRIPPERSLASHER` | `ASC_METEORASSAULT` | 3 | 1 | Hero_Hidden_Cloth, Meuchler-OS |
| `GC_DARKCROW` | `ASC_BREAKER` | 0 | 1 |  |
| `GC_ROLLINGCUTTER` | `AS_GRIMTOOTH` | 1 | 1 | Ripper_Cross |
| `GC_WEAPONBLOCKING` | `AS_LEFT` | 1 | 0 | Oni_Horns_ |

## Rogue / Stalker (Shadow Chaser)

| Compétence | Remplacement | Objets | Combos | Exemple |
|---|---|---:|---:|---|
| `SC_AUTOSHADOWSPELL` | `RG_INTIMIDATE` | 0 | 3 |  |
| `SC_ENERVATION` | `RG_RAID` | 0 | 1 |  |
| `SC_FATALMENACE` | `RG_BACKSTAP` | 3 | 1 | Hero_Hidden_Cloth, Black_Wing |
| `SC_GROOMY` | `RG_RAID` | 0 | 1 |  |
| `SC_MAELSTROM` | `ST_REJECTSWORD` | 1 | 1 | Pendant_Of_Maelstorm |
| `SC_TRIANGLESHOT` | `AC_DOUBLE` | 5 | 2 | Hero_Hidden_Cloth, Vigilante_Suit |

## Monk / Champion (Sura)

| Compétence | Remplacement | Objets | Combos | Exemple |
|---|---|---:|---:|---|
| `SR_DRAGONCOMBO` | `MO_TRIPLEATTACK` | 3 | 0 | Focus_Beret, Hero_Judgement_Shawl |
| `SR_EARTHSHAKER` | `MO_INVESTIGATE` | 2 | 0 | Hero_Judgement_Shawl, Sura_Rampage |
| `SR_FALLENEMPIRE` | `MO_CHAINCOMBO` | 1 | 0 | Focus_Beret |
| `SR_GATEOFHELL` | `MO_EXTREMITYFIST` | 1 | 0 | Gambler_Seal |
| `SR_KNUCKLEARROW` | `MO_FINGEROFFENSIVE` | 3 | 4 | Racing_Cap_SR, Fallen_Monk_Rosary |
| `SR_RAMPAGEBLASTER` | `MO_EXTREMITYFIST` | 1 | 0 | Burning_Knuckle-OS |
| `SR_RIDEINLIGHTNING` | `MO_BODYRELOCATION` | 0 | 1 |  |
| `SR_SKYNETBLOW` | `MO_CHAINCOMBO` | 3 | 1 | Hero_Judgement_Shawl, Fallen_Monk_Rosary |

## Ninja (Kagerou, Oboro)

| Compétence | Remplacement | Objets | Combos | Exemple |
|---|---|---:|---:|---|
| `KO_BAKURETSU` | `NJ_BAKUENRYU` | 1 | 1 | Monokage |
| `KO_HAPPOKUNAI` | `NJ_KUNAI` | 2 | 1 | Golden_Ninja_Suit, Kuroiro-OS |
| `KO_HUUMARANKA` | `NJ_HUUMA` | 4 | 1 | Racing_Cap_NJ, Huuma_Swirling_Petal |
| `KO_JYUMONJIKIRI` | `NJ_ISSEN` | 0 | 1 |  |
| `KO_ZANZOU` | `NJ_UTSUSEMI` | 1 | 0 | Tenebris_Latitantes |
| `OB_OBOROGENSOU` | `NJ_NEN` | 0 | 3 |  |
| `OB_ZANGETSU` | `NJ_ZENYNAGE` | 0 | 1 |  |

## Gunslinger (Rebellion)

| Compétence | Remplacement | Objets | Combos | Exemple |
|---|---|---:|---:|---|
| `RL_AM_BLAST` | `GS_TRACKING` | 1 | 0 | Finisher |
| `RL_BANISHING_BUSTER` | `GS_FULLBUSTER` | 2 | 3 | Dustfire, Calf_Anaconda |
| `RL_C_MARKER` | `GS_ADJUSTMENT` | 1 | 0 | Racing_Cap_GS |
| `RL_D_TAIL` | `GS_DESPERADO` | 4 | 0 | Racing_Cap_GS, Avenger |
| `RL_FALLEN_ANGEL` | `GS_MADNESSCANCEL` | 1 | 0 | H_FEATHER_H_FIRE |
| `RL_FIREDANCE` | `GS_RAPIDSHOWER` | 2 | 0 | Bellare_Card, Calf_Deathadder |
| `RL_FIRE_RAIN` | `GS_SPREADATTACK` | 1 | 0 | Burning_Rose |
| `RL_HAMMER_OF_GOD` | `GS_DUST` | 2 | 0 | Racing_Cap_GS, HR-S55-OS |
| `RL_HEAT_BARREL` | `GS_GATLINGFEVER` | 1 | 0 | Crimson_Rose |
| `RL_H_MINE` | `GS_GROUNDDRIFT` | 1 | 0 | Avenger |
| `RL_MASS_SPIRAL` | `GS_PIERCINGSHOT` | 1 | 0 | Finisher |
| `RL_P_ALTER` | `GS_ADJUSTMENT` | 2 | 0 | Demon_Slayer_Shot, The_Black |
| `RL_R_TRIP` | `GS_FULLBUSTER` | 3 | 0 | Greater_Bellare_Card, Calf_Diamondback |
| `RL_S_STORM` | `GS_SPREADATTACK` | 1 | 3 | Dustfire |

## Doram (Summoner) : classe absente

| Compétence | Remplacement | Objets | Combos | Exemple |
|---|---|---:|---:|---|
| `SU_ARCLOUSEDASH` | **retiré** | 1 | 0 | Magical_Yellow_Foxtail_Staff |
| `SU_BUNCHOFSHRIMP` | **retiré** | 1 | 0 | Racing_Cap_SU |
| `SU_CN_METEOR` | **retiré** | 3 | 6 | Racing_Cap_SU, Charm_Grass_Necklace |
| `SU_FRESHSHRIMP` | **retiré** | 3 | 0 | Racing_Cap_SU, Magical_Yellow_Foxtail_Staff |
| `SU_LOPE` | **retiré** | 1 | 0 | Elegant_Doram_Shoes |
| `SU_LUNATICCARROTBEAT` | **retiré** | 2 | 6 | Racing_Cap_SU, Meowmeow_Foxtail |
| `SU_NYANGGRASS` | **retiré** | 1 | 0 | Racing_Cap_SU |
| `SU_PICKYPECK` | **retiré** | 3 | 0 | Chubby_Worm_Talisman, Exquisite_Yellow_Foxtail_Model |
| `SU_SCAROFTAROU` | **retiré** | 1 | 0 | Fresh_Grass_Necklace |
| `SU_SVG_SPIRIT` | **retiré** | 1 | 0 | Racing_Cap_SU |
| `SU_SV_STEMSPEAR` | **retiré** | 3 | 0 | Shiny_Branch_Talisman, Magical_Yellow_Foxtail_Staff |
| `SU_TUNAPARTY` | **retiré** | 2 | 0 | Cute_Grass_Necklace, Fresh_Tuna_Talisman |

## Star Gladiator, Soul Linker (Star Emperor, Soul Reaper)

| Compétence | Remplacement | Objets | Combos | Exemple |
|---|---|---:|---:|---|
| `SJ_DOCUMENT` | **retiré** | 0 | 2 |  |
| `SJ_FALLINGSTAR` | `TK_COUNTER` | 0 | 3 |  |
| `SJ_FULLMOONKICK` | `TK_TURNKICK` | 0 | 3 |  |
| `SJ_PROMINENCEKICK` | `TK_DOWNKICK` | 1 | 0 | Racing_Cap_SG |
| `SJ_SOLARBURST` | `TK_STORMKICK` | 1 | 3 | Racing_Cap_SG |
| `SP_SOULREAPER` | `SL_SMA` | 0 | 2 |  |
| `SP_SPA` | `SL_STIN` | 1 | 3 | Racing_Cap_SL |
| `SP_SWHOO` | `SL_STUN` | 1 | 3 | Racing_Cap_SL |

## Ajouts du 2026‑11‑08 (portage de 17.1), validés

Trois compétences des combos Grace manquaient à la table. Remplacées par analogie avec
les lignes existantes, validées par l'utilisateur le 2026‑11‑08 :

| Compétence | Remplacement | Analogie |
|---|---|---|
| `AB_DUPLELIGHT_MAGIC` | `bonus2 bMagicAddRace,RC_Undead,N` | `AB_DUPLELIGHT` devient un bonus de race Mort-vivant, ici en magique |
| `SR_TIGERCANNON` | `MO_EXTREMITYFIST` | comme `SR_GATEOFHELL` et `SR_RAMPAGEBLASTER` |
| `WM_GLOOMYDAY` (condition niveau 5) | `BA_MUSICALLESSON` ou `DC_DANCINGLESSON` niveau 10 | comme `WM_LESSON` → `BA_MUSICALLESSON` |

`WM_SEVERE_RAINSTORM_MELEE` suit `WM_SEVERE_RAINSTORM` (`CG_ARROWVULCAN`).

## Ajouts du 2026‑11‑10 (Circlet of Time du mode défi), validés

Cinq compétences des Circlet of Time manquaient à la table. Validées par l'utilisateur le 2026‑11‑10 :

| Compétence | Remplacement | Analogie |
|---|---|---|
| `WL_CHAINLIGHTNING_ATK` | `WZ_VERMILION` | variante d'attaque de `WL_CHAINLIGHTNING`, même ligne |
| `SJ_FALLINGSTAR_ATK`, `SJ_FALLINGSTAR_ATK2` | `TK_COUNTER` (une seule ligne) | variantes de `SJ_FALLINGSTAR` ; règle des doublons |
| `SC_FEINTBOMB` | `RG_RAID` | comme `SC_ENERVATION` et `SC_GROOMY` (**nouveau**) |
| `SP_CURSEEXPLOSION` | `SL_SMA` | comme `SP_SOULREAPER` ; le Circlet porte déjà `SL_STUN` (**nouveau**) |

## Objets touchés par la règle des doublons

`GodOfWinds_Fan`, `Astro_Circle`, `Emerald_Earring`, `Chilly_Spell_Book`, `Recovery_Light`, `Staff_of_Miracle`, `Red_Ether_Bag`, `Engine_Pilebuncker`, `Volar`, `Vernan`, `Master_Soul_Rifle`, `Jack_The_Knife`.

`Recovery_Drop`, `Virgo` et `Libra` listent déjà `AL_HEAL` dans leur tableau : `AB_CHEAL` et `AB_HIGHNESSHEAL` en sortent simplement.

## À trancher

- **`Runstone_*` et compétences `RK_*` en « garder » :** ces consommables vérifient la classe Rune Knight, ou fabriquent des runes. Personne ne peut s'en servir ici, ils restent inertes.
- **`Racing_Cap_SU` :** classe Doram absente, la coiffe n'a plus d'usage. Les autres `Racing_Cap_*` sont rattachées à leur classe jouable, dont `_SG` et `_SL`.
- **Objets Doram (`Foxtail`, `Talisman`) :** une fois le bonus retiré, certains deviennent vides. Il faudra décider s'ils restent obtenables.
- **Baisse de délai globale :** `Highness_Heal_3sec` passerait de -3 s sur une seule compétence à -15 % sur toutes. C'est la règle de 16.2, mais elle pèse plus lourd sur les petits enchantements.
