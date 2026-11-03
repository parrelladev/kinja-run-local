# Auditoria de prontidão offline — 14/10/2026

## Pausa solicitada pelo usuário — checkpoint

Trabalho pausado a pedido do usuário por limite de tokens. APK instalável do package principal já assinado: `build/kinja-run-offline-lab-v110-readiness-main-fresh-login.apk` (376627234 bytes, SHA256501B9AD44EC90DAE4B0070BAAC38E2CFD15B582014F82BFC8895F0CB9704FF91). Inclui fusão v107 e correção de primeira abertura v109. Não foi instalado no lab6, que permanece v107; pode atualizar por cima no mesmo package/keystore, sem desinstalar.

Audit1 está na v111-readiness-menu-refresh, APK7456C7566259F69D5F9808B89055B639F25E385067F5A051B4A55EDFA80EA56D / biblioteca57F41A24F7652411EF8754CD282DC6141AD5485F7790F5CF448EA996FF3196C4. Ela acrescenta refresh de FuncManager.SetOpeningFunc após vitória (setter original0x276D2F0, disassembly anotado; ainda sem vitória de gameplay nesta build). Source atual contém ambas correções; v110 usa a biblioteca v109, sem esse último refresh. Não promover v111 como gameplay validado nem confundir package audit1 com lab6.

Primeira vitória natural audit1/v109, totalmente sem rede: region2/Capítulo1, passou às21:30:02; payout625/259 mais callback tardio5/2, final630ouro/261gemas/0EXP. Saldo1500→2130ouro, gemas0→261, EXP0; capítulo2 desbloqueado, itens100001×1/40001×2. Resultado visual final não foi capturado; home/callback/readbacks foram. Menu Talentos ainda recusou “Desafie o Capítulo1” na mesma sessão; após restart abriu, comprovando cache stale. Recompensas/saldos persistiram no PID11111. Uma compra controlada de talento1001/Poder ATQ+5 por500ouro às21:35:09 mudou2130→1630 e TalentLevel0→1; atualização/restart v111 preservou1630/261/0 e os2cristais. Logs/capturas `kinja-audit1-first-win-*`, `kinja-audit1-after-first-win-restart-logcat.txt`, `kinja-audit1-first-talent-purchase-logcat.txt`, `kinja-audit1-v111-startup-logcat.txt`.

Rede restaurada ao estado original: avião0/Wi-Fi1/dados0. Última ação no audit1/PID13571 foi tocar a seta esquerda na home para selecionar Capítulo1; captura `kinja-audit1-repeat-chapter1-selection.png` ainda não inspecionada. Nenhuma segunda partida foi iniciada por nós. Próximo: validar efeito ATQ+5 em combate sem assistência do Capítulo2, abandono sem payout, depois vitória do Capítulo2 para conferir refresh116 imediato na v111 e progressão/EXP; não gastar recursos do lab6. V110 é a entrega principal já existente; v111 é somente auditoria. Lab6 preservado (57 itens e saldos7490720/938307/1000640 na observação inicial), sem consumo nosso nesta bateria.

## Escopo autorizado

O usuário autorizou iniciar a bateria proposta: progressão natural em perfil separado, funcionamento sem internet, transações, persistência, capítulos e acabamento. O save principal lab6 é preservado; testes consumidores devem usar o perfil isolado com recursos obtidos naturalmente e baseline prévia. Sem seeding QA, alteração de saldos, limpeza ou desinstalação. Estado original de rede: modo avião0/Wi-Fi1/dados móveis0; restaurar ao concluir.

## Primeiros resultados

- **Abertura offline lab6/v107:** cold start em modo avião, Wi-Fi0/dados0 e `Active default network: none`; processo2495 chegou a initialized/home. Saldos7490720/938307/1000640 preservados. Log `build/kinja-readiness-airplane-main-logcat.txt`, captura `kinja-readiness-airplane-main.png`. O inventário observado agora é57 e não há trios elegíveis; isso difere da baseline195 anterior, por atividade entre sessões. Não foi executada fusão no save principal durante esta auditoria e não atribuir a alteração a teste nosso. Falta log consumidor para reconciliar a origem exata.
- **Defeito confirmado:** `ET.RecoverPowerHelper.RequestNextRecoverPowerTime` continua gerando NullReferenceException repetida sem rede. Não impediu chegar à home, mas a dependência remota/contador ainda precisa de correção. Timer de energia exibe duração excessiva.
- **Perfil separado audit1:** buildv108 usa a biblioteca exata v107 (SHA2563973702C...) e os seeds normais: nível1/ouro1500/gemas0/EXP0/itens e equipamentos vazios; mantém os perks offline normais8/9/10 e assistência do Capítulo2 existentes. Não é uma reprodução vanilla sem essas adaptações. Package `com.habby.kinjarun.offline.audit1`, keystore de laboratório existente. O lab6 não foi atualizado nem apagado.
- **Primeira abertura limpa falhou:** v108/PID4645 parou em67%, `Cannot read chapter completion for menu unlocks`. Seed inicial só tem regiões1/2; `set_local_opening_functions` exigia registro3, que ainda não deveria existir. Reprodução e tela em `kinja-audit1-first-start-logcat.txt`/`kinja-audit1-first-start.png`.
- **Correção fundamentada v109:** `read_local_menu_progress` aceita registro3 ausente somente se MaxRegion<3, mantendo flags false e menus bloqueados. Registro3 alcançado ausente ou existente malformado continua recusado. Não cria registros, não conclui capítulos, não concede recursos. Regressão ARM64 com helpers reais em `kinja-v109-menu-regression.txt` passou.
- **Reabertura corrigida:** atualização audit1 por adb install -r, PID9720/initialized/home nível1,1500/0/0, inventário0. `kinja-audit1-v109-startup-logcat.txt`/`kinja-audit1-v109-home.png`. Menus continuam bloqueados corretamente. Google Play Games ofereceu login opcional na primeira abertura; foi cancelado pelo botão Cancelar, sem autenticação. SDKs ainda tentam serviços externos.

## Artefatos

| Build | Package | APK SHA256 |
|---|---|---|
| v108-readiness-clean-profile | audit1 | 235BEFCF6DA6B0DB536F4947ECF214D6EC38DE1F3441351DB8291F00A51872E1 |
| v109-readiness-fresh-login | audit1 | AB45DD0F731A83BB0A8571BDC24F9CFA4AA108E230E6BAD67071BFBF7EBAFF60 |

Biblioteca v109: DB0D85D480A8C8BA2070EEBFF64B5C6D9A2E9BF3DD541F455337D61247E7C79B. Backup anterior: build/offline_bridge.before-readiness-v109.cpp. O APK original mantém SHA2567953EB369BA9B8C814C4736DAC4A7E3AA116A5583837F6F12D795EA70C20595E.

## Em execução / ainda sem conclusão

Reabertura fria offline do audit1 corrigido, primeira partida natural, progressão/recompensa/reinício. Não considerar a bateria completa nem declarar gameplay/persistência apenas pela instalação ou initialized. Fusão coletiva consumidora, melhoria automática, chaves, saldo insuficiente, recuperação real após interrupção, capítulos posteriores e heróis continuam dependendo de casos seguros e recursos naturais.
