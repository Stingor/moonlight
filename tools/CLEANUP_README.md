# NPC Script Cleanup Tool

Outil pour nettoyer et formater les fichiers NPC/scripts rAthena avec encodage ANSI (latin-1).

## Utilisation rapide

### Option 1 : Drag & Drop (Windows - le plus simple)
1. **Double-cliquez** sur `cleanup-npc.ps1`
2. **Glissez-déposez** vos fichiers `.npc`, `.txt` ou `.cpp` sur la fenêtre PowerShell
3. Les fichiers sont nettoyés automatiquement

### Option 2 : Ligne de commande (PowerShell)
```powershell
.\cleanup-npc.ps1 mon_fichier.npc
.\cleanup-npc.ps1 fichier1.npc fichier2.npc fichier3.txt
```

### Option 3 : Python (multiplateforme)
```bash
python cleanup-npc.py mon_fichier.npc
python cleanup-npc.py fichier1.npc fichier2.npc
```

## Ce que le script fait

✓ **Normalise les espaces** après les contrôles :
- `if(condition)` → `if (condition)`
- `for(i=0; ...)` → `for (i=0; ...)`
- `while(test)` → `while (test)`
- `switch(value)` → `switch (value)`

✓ **Supprime les espaces de fin de ligne** (trailing whitespace)

✓ **Corrige l'indentation des labels** : les labels comme `L_Menu:` sont placés à la colonne 0

✓ **Valide l'encodage ANSI (latin-1)** : s'assure que le fichier est bien encodé pour rAthena

## Formats supportés

- `.npc` - Scripts NPC rAthena
- `.txt` - Fichiers texte rAthena (configurations, imports)
- `.cpp` - Scripts C++ rAthena

## Notes importantes

- **L'encodage latin-1 (ANSI/Windows-1252) est obligatoire** pour que les caractères accentués s'affichent correctement en jeu
- Les modifications sont appliquées directement au fichier (pas de sauvegarde de l'original)
- Le script vérifie qu'aucune séquence UTF-8 invalide (U+FFFD) ne reste dans le fichier

## Exemple

**Avant :**
```c
if( BaseLevel == 999 && !(base3rd || base4th) ) {
	.@menu$ = "Couleur des vêtements";   
	for( .@i = 0; .@i < 10; .@i++ )
```

**Après :**
```c
if (BaseLevel == 999 && !(base3rd || base4th)) {
	.@menu$ = "Couleur des vêtements";
	for (.@i = 0; .@i < 10; .@i++)
```

## Dépannage

### PowerShell refuse d'exécuter le script
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### Le script Python n'est pas trouvé
Assure-toi que `python` ou `python3` est installé et dans le PATH.

### Le fichier contient des caractères invalides
Les caractères non-compatibles avec latin-1 doivent être remplacés avant d'utiliser le script.
