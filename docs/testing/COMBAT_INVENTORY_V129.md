# Crédito imediato de manuais e livros — v129, resolvido

Em03/11/2026, usuário delimitou o sintoma: após terminar a partida, manuais/livros não eram utilizáveis; comprar na loja atualizava os saldos. Depois de instalar v128 QA, relatou crédito imediato correto ao terminar e aprovou o encerramento desta etapa. Esta conclusão supersede a hipótese genérica de tela desatualizada nas notas v127.

## Causa e correção

`persist_local_clear` já gravava os totais em seed_items.json. `apply_local_reward_items` chamava AddLocalItem por pilha, mas a rota original1.9.1 é legada: ItemManager.AddLocalItem0x280C2C8 → ProtocolManager.AddItem0x2A8C368 → AddItemArray0x2A8B304 → TriggerProtocolEvent com S2C_Item_Add_Data/S2C_PlayerItemData. Invocação sem exceção não provava atualização do ItemManager atual. A compra recarrega o inventário usando a API ET `ItemManager.OnItemInit(G2C_ItemInfo)`, explicando por que o saldo salvo aparecia depois.

Conferidos apenas os métodos necessários no binário já recuperado `libil2cpp.stripes.so`, SHA256 `A2A4510A6650A6BFC9C23949E466392A098F9A1C992E21194B531C1E45BABDBE`, com índice/decoder existentes do acervo. REA provider nativo indisponível já conhecido; não repetida configuração, extração ou mapeamento. `ItemManager.OnItemInit`0x280A82C possui evidência anotada preexistente no acervo. Skill reverse-engineer-anything aplicada à inspeção delimitada. Arquivos externos somente lidos.

A correção remove a chamada legada desse ponto e desserializa o **saldo absoluto completo já salvo** em G2C_ItemInfo; chama OnItemInit original na thread Unity e confere cada item recompensado por GetOwnItemNum. Registra saldo vivo antes/depois e recusa callback/readback divergente. Confere que a atualização não alterou os bytes salvos. Não soma a recompensa outra vez; repetir a atualização aplica o mesmo total. Nenhuma nova fórmula, taxa, concessão, hook/prologue ou offset gerenciado. Itens de capítulo também passam por esse ponto; pilha vazia continua sem atualização.

## Validação v128 QA

Package `com.habby.kinjarun.offline.qa.heroes`, modo forte v127 preservado, sem refazer seeds. Usuário acabou conduzindo a partida antes do teste automatizado planejado; atribuir a execução a ele. Logs PID15257: Chapter6/region7 iniciado19:23:33; EndGame stage11/score24428 às19:28:40. Recompensas persistidas e readbacks na mesma thread:

| Item | Ganho | Vivo antes | Salvo/após callback |
|---|---:|---:|---:|
|6001 Manual do Herói|198|26|224|
|20001 livro|3|0|3|
|20005 livro|2|1|3|
|20003 livro|1|4|5|
|20002 livro|1|0|1|

OnItemInit completou às19:28:40.854; todos os getters conferiram os totais até.855. Sem callback de compra/chest refresh entre o início dessa partida e a evolução seguinte no recorte observado. Às19:29:08, HeroLevelUp4002 consumiu90manuais,224→134, e340ouro,3407→3067; save/live/readback de heróis confirmaram o commit. Isso fecha **drop natural → vitória → inventário utilizável → consumo**, incluindo os livros pelo mesmo manager. Usuário confirmou que o crédito caiu direto e considerou a etapa finalizada.

[Síntese dos logs/hashes](COMBAT_INVENTORY_V129.json). Não executados fault injection, repetição da callback sob falha, partida longa com interrupção ou restart após este consumo. Não inferir essas coberturas da aprovação. Não testado consumo de cada tipo de livro na forja; foram conferidos os quatro saldos no getter usado pelo inventário atual.

## Build normal e retenção

Normal `v129-combat-inventory-refresh`, audit1, assinatura de desenvolvimento existente. APK SHA256 `E0E58B2D2B19E88D15254EA1185F7B90B4CE69B54C5E22B2A2A758ECAE0BAA7D`; biblioteca normal SHA256 `AEDF3550218B5C6F5DD09AFD84E004B665D903BA982B347F8DC1A737170C2CC3`. Compilação ARM64 e empacotamento/assinatura passaram; warning preexistente equipment_rewards. ZIP conferido: biblioteca normal exata, seeds normais/um herói, strings da assistência QA ausentes. QA usa biblioteca separada57A21B90520A95B193F6B691BAD989FE0CF4AEF6FCF53E91C04B701230CBBDBD. Não declarar bibliotecas QA/normal idênticas.

V129 instalada pelo comando install-r no audit1. Metadados após instalação: firstInstallTime e lastUpdateTime ambos2026-11-03 19:31:06; portanto instalação nova, não uma migração preservada do antigo perfil normal. O recorte prévio não apresentou firstInstallTime; não atribuir a remoção anterior a uma causa sem evidência. O agente não executou uninstall/clear. Instalação validada; não houve abertura/partida instrumental da normal nesta rodada. QA forte v128 permanece instalada separadamente para testes. Celular, saves de outros packages e acervo externo preservados. Nenhuma nova partida da automação depois da aprovação do usuário.

Depois da síntese, limpeza autorizada mantém somente APK normal v129 e relatório de assinatura; versões/IDs/hashes pequenos preservados no histórico. Evidências brutas desta pasta build são históricas após essa limpeza. Guia e auditoria atualizados; nível/reciclagem/heróis e crédito de materiais aceitos como resolvidos. Tutorial aguarda nova etapa.

Limpeza concluída:1891844076→375970459bytes;1515873617bytes liberados. Só APK v129 e relatório de assinatura permanecem em build; hash reconferido.
