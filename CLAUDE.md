# Claude Code — Instructions projet

## Git
- Travailler directement sur la branche `main`.
- Ne jamais créer de branche séparée, de pull request, ni de worktree.
- Ne jamais utiliser `isolation: "worktree"` dans les appels à l'Agent tool.
- Ne pas committer ou pusher, proposer éventuellement de le faire mais toujours attendre une validation

## Encodage des fichiers NPC / scripts rAthena
- Tous les fichiers `.npc`, `.txt` et scripts rAthena doivent être écrits en **ANSI (latin-1 / Windows-1252)**.
- Les outils d'édition écrivent en UTF-8 par défaut : toujours forcer l'encodage `latin-1` lors de l'écriture via Python (`open(..., encoding='latin-1')`).
- Ne jamais laisser des séquences UTF-8 ou des U+FFFD (`0xEF 0xBF 0xBD`) dans ces fichiers — les accents s'afficheraient incorrectement en jeu.
- Après toute modification d'un fichier NPC, vérifier l'absence de bytes `0xEF 0xBF 0xBD` avant de committer.
