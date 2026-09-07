# Claude Code — le dossier `db/`

## La règle, sans exception

- **`db/import/` est le dossier de TRAVAIL.** Toute modification de données va ici, et c'est la
  première chose à lire. Il est chargé en DERNIER, donc il écrase tout le reste.
- **`db/pre-re/` est le FALLBACK.** Lecture seule : il fournit ce que `import` ne redéfinit pas,
  et seulement quand le point d'entrée le liste (voir plus bas — souvent il ne le liste pas).
- **`db/re/` est de la RÉFÉRENCE.** Ce serveur est pre-renewal. Les entrées `db/re/` sont
  listées avec `Mode: Renewal`, donc **jamais chargées à l'exécution**. Ne jamais en tirer un
  chiffre, ne jamais y écrire.

## 🔴 Avant tout comptage ou toute recherche : lire le `Footer:` du point d'entrée

L'autorité est le bloc `Footer: Imports:` de `db/<nom>.yml`. Lui seul dit quels dossiers sont
réellement chargés, et **le chaînage n'est PAS le même d'une DB à l'autre** : dans plusieurs
d'entre elles, la ligne `db/pre-re/…` est commentée.

```sh
sed -n '/^Footer:/,$p' db/item_db.yml
```

**Un comptage qui ne regarde qu'un seul dossier est faux.** Exemple vécu : chercher les cartes
dans `db/pre-re/item_db_etc.yml` donne 538 — un chiffre qui ne correspond à rien, puisque
`db/item_db.yml` a `db/pre-re/item_db.yml` en commentaire. La vraie réponse est 912, dans
`db/import/items/item_db_card.yml`, seule source chargée.

### DB où `import` est la SEULE source (pre-re commenté)

`item_db` · `mob_db` · `skill_tree` · `achievement_db` · `instance_db` · `item_group_db` ·
`item_combos` · `item_cash` · `mob_item_ratio` · `mob_chat_db` · `mob_summon` · `map_index` ·
`statpoint` · `create_arrow_db` · `battleground_db` · `captcha_db` · `const` · `job_stats`

### DB où `pre-re` est réellement chargé sous `import`

`skill_db` · `abra_db` · `status` · `refine` · `size_fix` · `attr_fix` · `quest_db` · `pet_db` ·
`mercenary_db` · `homunculus_db` · `castle_db` · `exp_guild` · `exp_homun`

## `db/import/items/` est découpé par famille

`item_db_card.yml` (cartes de monstre) · `item_db_enchant.yml` (enchantements) ·
`item_db_weapon.yml` · `item_db_armor.yml` · `item_db_costumes.yml` · `item_db_cash.yml` ·
`item_db_etc.yml` · `item_db_usable.yml` · `item_db_healing.yml` · `item_db_ammo.yml` ·
`item_db_pet.yml`. Le chaînage est dans le `Footer:` de `db/import/item_db.yml`.

⚠ **Une carte de monstre n'est pas un enchantement.** Les deux portent `Type: Card` ; les
enchantements ont en plus `SubType: Enchant` (`CARD_ENCHANT`, `src/map/pc.hpp:1139`). Compter
`Type: Card` sans exclure `SubType: Enchant` mélange 912 cartes et 485 enchantements.

## Même logique côté `conf/`

`conf/import/` l'emporte sur `conf/`. La valeur effective d'un réglage est celle de
`conf/import/`, jamais celle du fichier de base.
