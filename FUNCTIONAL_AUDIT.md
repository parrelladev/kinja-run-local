# Auditoria funcional — build offline v31

## EXP de combate — v150

[Evidência e limites](docs/testing/COMBAT_EXP_V150.md). Regra offline autorizada de 5/25/50 EXP por comum/elite/boss; a fórmula remota original não foi recuperada. O bridge informa `KillEnemy.Exp`, usa `EndGame.Exp` como total da run e credita apenas o delta no saldo da conta após vitória. QA v149 no BlueStacks: Capítulo 1 vencido, 620 EXP, nível 2/1990 → nível 3/610, home e save persistentes após relançar. Testes isolados de taxa/delta/limites de nível passaram. Normal v150 gerada e assinada, **não instalada**; derrota e callback tardio não tiveram gameplay observado. Registros antigos abaixo que indicam EXP de combate zero são históricos.

## Baús v148

[Resultado e limites](docs/testing/CHESTS_V148.md). Teste QA BlueStacks confirmou bronze a 10.000 moedas na tela e compra com saldo exato: débito único 10.000→0, gemas 0→0 e um equipamento. Sorteio Divino de 2% verificado pela configuração e enumeração determinística da categoria; faltam testes de gameplay de insuficiência, saldo acima, reinício e x10. APK normal assinado, não instalado. Registros antigos de 60 gemas para bronze e taxas divergentes são históricos.

## Backup manual nativo — v141, validado em QA Android

[Contrato, evidências e limites](docs/testing/MANUAL_BACKUP_V141.md). Relato de v131 reproduzido: crash no trampoline UISetting.DoOpen em UnityMain/Houdini. Hooks UISetting retirados; Backup agora usa widgets NGUI originais no StoragePanel real, acima de Configurações. Formato2 inclui todo o perfil local inventariado, ledgers e preferências de progresso/som/música/sensibilidade/idioma. Sem escolha explícita, idioma segue o sistema do destino. Exportação conferida e importação integral no cold start com recuperação de diretório e primeiro checkpoint lógico imutável.

**Passou no jogo, somente QA Source/Destination:** posição do botão, janela, X/fora/Voltar, Configurações sem crash, seletor/exportação, cancelamento de confirmação sem mudança, arquivo inválido, versão999 com hash válido recusada, restore, sucesso após bootstrap, reinício e navegação de equipamento. Destino nível1/1500/0 passou a nível3/321EXP/6543/432/capítulo6; herói e equipamento restaurados. Reexportação dos dez registros e arquivo completo idênticos à origem. Host e Android ART com arquivos artificiais:1943 verificações/80 cenários/40 pontos, fsync/rename reais no ART com falhas injetadas. Não equivale a corte de energia ou falha real de um provedor SAF.

Normalv141 compilada/assinada, sem fixture e com mesmos bridge/DEX da QA destino; **não instalada no perfil do usuário**. Saves pessoais/QA Heróis/acervo externo preservados. Dois celulares físicos, reinstalação, run completa, preferências cifradas não zero e transferência com idioma explicitamente escolhido não tiveram validação visual específica nesta rodada; tutorial continua na limitação anterior. A auditoria v131 antiga não comprova execução Android; o usuário instalou aquela versão depois do seu registro.

## Heróis e crédito de materiais — RESOLVIDOS, v129

[Correção e evidências](docs/testing/COMBAT_INVENTORY_V129.md). Usuário identificou que comprar na loja liberava saldos de manuais/livros ganhos. Fim de partida salvava itens, mas chamava AddLocalItem/rota legada; loja recarregava saldo ET por OnItemInit. Correção aplica o inventário absoluto salvo por G2C_ItemInfo/OnItemInit e confere GetOwnItemNum, sem novo crédito. QA128/partida do usuário no Chapter6 confirmou198manuais: vivo26/salvo224→vivo224, mais quatro tipos de livros reconciliados; evolução4002 posterior consumiu90manuais224→134 sem compra intermediária observada. Usuário relatou crédito imediato correto e aprovou etapa como finalizada. Normalv129 compilada/assinada/instalada no audit1, sem assistência QA. FirstInstallTime/lastUpdateTime iguais19:31:06 indicam instalação nova, não migração; agente não executou uninstall/clear. Sem abertura/gameplay instrumental da normal nesta rodada. QA forte128 preservada. Nível/reciclagem/heróis e crédito de materiais resolvidos; tutorial permanece para próxima etapa.


## QA Heróis forte — v127; vitória com366 manuais

[Build QA e evidências](docs/testing/HERO_QA_COMBAT_V127.md). Usuário autorizou assistência exclusiva QA: dano mínimo1000000 e dano comum recebido0, por define opt-in + filesDir exato do qa.heroes, sem alterar atributos/saves/economia. Update preservou perfil; normalv126 e biblioteca padrão intactos. Partida do usuário no Chapter13/region14 confirmou assistência, vitória,366manuais6001 e três pilhas de livros persistidos. Após relato de material ausente em Melhorar, reabertura do treino mostrou366/50 e4112ouro. Não reproduzida a ausência anterior; hipótese de refresh não confirmada, sem crédito duplicado/correção especulativa. Ainda falta consumo do manual ganho; tutorial não iniciado. Evidência atual supersede a ausência de drop observado na auditoria anterior.


## Manual do Herói — obtenção natural ainda pendente

[Auditoria de fontes v126](docs/testing/HERO_MATERIAL_V126.md). Usuário relatou combate positivo com novos heróis. Item6001 é Manual do Herói; nível exige ouro+manual (primeira melhoria170+50), estrela usa fragmentos. Pools comuns incluem manual a partir do capítulo6; bridge enfileira drops originais e credita somente em vitória. HangUp offline atual só fornece ouro/EXP104, não manuais. Logs do teste do usuário mostram vitória no capítulo1 sem itens, conforme pool100010. Tentativa instrumental QA no capítulo6/region7 terminou em derrota, sem drops6001 nem gasto de revive;1950ouro/257gemas preservados. Não valida ganho natural: falta drop→vitória→inventário→restart→consumo do material ganho. Nenhuma economia/código/APK alterados; v126 permanece atual. Tutorial não iniciado.


## Heróis — v126 em avaliação

[Contrato e testes](docs/testing/HEROES_V126.md), [síntese/hashes](docs/testing/HEROES_V126.json). Usuário confirmou falha tanto em nível quanto estrelas. Corrigidos Star0 legado (mínimo original1, sem custos), desbloqueio107 conforme Chapter5 e operações locais de seleção/obtenção por fragmentos/nível/estrela. Tabelas e handler IL2CPP originais, journal/fsync/readback/rollback; nenhuma escrita de offsets gerenciados. QA BlueStacks validou seleção4002,1001 nível1→2 (170ouro+50materiais), estrela1→2 (120fragmentos/teto30), obtenção4003 (80fragmentos) e persistência/reseleção após update/restart. Biblioteca final v125 QA idêntica à v126 normal; contagem dos três registros conferida. V126 install-r preservou firstInstallTime e home normal1500/0/0/Nv1/Chapter1, menus bloqueados; sem consumo no perfil normal. Combate, máximos/talentos e falhas de journal injetadas não validados. Nível da conta/reciclagem individual aprovados pelo usuário; tutorial aguarda aprovação de heróis. Não repetir extração do acervo externo.


## Reciclagem no detalhe — v119 QA/v120 normal, 03/11/2026

[Relatório](docs/testing/EQUIPMENT_RECYCLE_V120.md). Bug de nível aprovado pelo usuário como resolvido; foco somente no segundo bug individual. V118 reproduziu NRE no helper ET BreakEquip. V119 confirmou devolução original400ouro/4livros e peça5→1, depois200/2 e peça equipada3→1 com slot preservado. Tudo com peças artificiais em package separado qa.equipbreak. Restart confirmou2100ouro/6livros/3peças nível1 e botão de reciclar ausente; callbacks/readbacks concordantes. V120 normal usa biblioteca idêntica e seeds normais. Sem mudança no celular, itens reais ou outros bugs. Recuperação sob falha, outras faixas e lote não têm teste positivo; correção individual aguarda avaliação do usuário.

## Nível da conta — validação com saves sintéticos v113–v117

[Relatório QA](docs/testing/ACCOUNT_LEVEL_QA_V116.md). Autorização explícita para save editado, somente em dois packages separados no BlueStacks. Migração 2/5838 → 4/838 e idempotência no restart confirmadas. Nova coleta de151 EXP elevou2/1990 →3/141, com ouro1651/gemas0, home/readback e restart preservados. Defeito de leitura de nível descoberto na v112 corrigido por receiver unboxed na v116 normal, atualizada no audit1. Telefone/acervo não alterados. Isso valida a passagem com EXP acumulada/HangUp em QA, sem validar fórmula de EXP por combate ou save real do celular. Histórico v112 abaixo foi supersedido por esses resultados.

## Nível da conta — v112, 03/11/2026

Imagem da v111 no celular: nível 2, 5838/2000 EXP e Capítulo 6. Confirmado no bridge: crédito de EXP não alterava nível. [Correção, hashes, testes e limitações](docs/testing/ACCOUNT_LEVEL_V112.md). Planejador com tabela original passou caso 2/5838 → 4/838 e os 199 limites. V112 instalada somente no audit1 do BlueStacks com `install -r`, mesmo certificado e firstInstallTime; curva original lida com sucesso, initialized/home preservados em 1500/0/0, nível 1. Não houve save avançado disponível como arquivo, concessão artificial ou teste consumidor. Subida visual e persistência de nível elevado permanecem pendentes; não equivale a validação completa de gameplay. Sem correção dos outros bugs nesta rodada.

## BlueStacks — execução da v111 relatada pelo usuário

Coleta posterior em 03/11/2026: identidade do APK audit1/v111 conferida por SHA256 via ADB; bootstrap/home observados. Reinício PID3724→4487 recarregou1500ouro/0gemas/0EXP/inventário0. Sem transações ou partida nesta coleta. Rede padrão via Ethernet virtual ainda ativa; offline completo e refresh após vitória não validados. [Ambiente, evidências e limites](docs/testing/BLUESTACKS.md). O relato abaixo antecede esta observação instrumental.

Em 03/11/2026, o usuário informou que conseguiu executar normalmente o APK v111 no BlueStacks. Evidência por relato do usuário, sem captura/logs ou inspeção por este agente. Não promover esse resultado a validação do refresh pós-vitória, persistência, execução sem rede ou bateria completa. Detalhes e pendências em [docs/testing/BLUESTACKS.md](docs/testing/BLUESTACKS.md). Os registros anteriores de BlueStacks não testado referem-se ao momento da migração.


## Auditoria de prontidão iniciada — v108/v109, 14/10/2026

[Relatório da bateria](analysis/native/OFFLINE_READINESS_AUDIT.md). O usuário autorizou testes de progressão natural em perfil separado. Lab6/v107 foi aberto em modo avião com Wi-Fi/dados desligados e sem rede padrão: initialized/home/saldos preservados; RecoverPowerHelper.RequestNextRecoverPowerTime ainda lança NRE repetida. Não houve consumo no save principal. Seu inventário observado é57, diferente da baseline195 anterior por atividade entre sessões; sem log consumidor nesta coleta para atribuir origem exata.

Criado package separado audit1/v108 com a biblioteca exata v107, seeds normais (Nv1/1500ouro/0gemas/0EXP/bolsa vazia), sem QA e sem copiar/limpar lab6. **Falha reproduzida:** primeira abertura fica em67%, pois SetOpeningFunctions exigia registro de região3 que não existe numa conta nova. V109 corrige a leitura: ausência antes de alcançar o capítulo significa bloqueado; registro alcançado ausente ou registro existente inválido continua recusado. Não cria progresso/recursos. Teste ARM64 `kinja-v109-menu-regression.txt` passou; adb install -r no audit1 preservou a conta e a home abriu com valores iniciais/menus bloqueados. V109 está somente no audit1; lab6 ainda v107. APKv109 `AB45DD0F731A83BB0A8571BDC24F9CFA4AA108E230E6BAD67071BFBF7EBAFF60`, .so `DB0D85D480A8C8BA2070EEBFF64B5C6D9A2E9BF3DD541F455337D61247E7C79B`. Primeira partida do audit1 iniciada sem rede; resultados/persistência ainda em observação.

## Unir tudo — implementação atual v107, 14/10/2026

Implementada a **adaptação offline expressamente autorizada** no chat: trios de mesmo EquipId/qualidade1–3, cascata até4/roxo, raiz de maior nível/UID preservado, sobras mantidas e equipados excluídos. Não afirmar equivalência à seleção remota desconhecida. Hook próprio ET.OneKeyCompoundEquip0x2C2D50C, prólogo `FF0301D1 F31300F9 FD7B03A9 FDC30091`, resposta G2C_OneKeyCompoundEquip/OnGetOneKeyCompoundEquip originais; nenhuma chamada à transação individual/legado. Validação integral save/live/config/cache, modelos preparados antes do consumo, commit único com fsync/journal, readback e rollback dos managers; journal conservado se a restauração em memória falhar. Não cobra/fabrica moeda ou peças. Relatório [ONEKEY_OFFLINE_V107.md](analysis/native/ONEKEY_OFFLINE_V107.md).

Build instalada `v107-onekey-atomic-durable`, lab6/1.9.1/code51, keystore existente e `adb install -r`; firstInstallTime preservado19:43:33 de13/10, lastUpdateTime20:56:02 de14/10. APK SHA256 `5F244C374A26EAA44558E8F61C93C3A88830E1EC403EB7BBD420043E3A0437F0`; .so `3973702C547BE9C709EF62EA0AA1E82CFBFD7C159D1FBEC05C462ADE068CEB2A`. Original/v57/builds anteriores preservados, sem defines QA ou seeding novo. Loja sem alterações.

**Estática:** contrato ET e receitas conferidos no APK exato; seleção local autorizada. **Bridge:** hook validado e dry-run v107/PID20496 confirmou195 itens,69 passos projetados,138 custos,31 UIDs finais/57 sobreviventes; desserialização e contagens das listas originais verificadas. Diagnóstico completo de32900 bytes somente em filesDir privado. **Testes isolados:** planejador ARM64 e helpers JSON/journal passam; fsync/destino inexistente/recuperação em fixtures fora do save, logs `kinja-v104-plan-regression.txt` e `kinja-v107-json-journal-regression.txt`. **Visual/save sem consumo:** v106 foi reiniciada,195 registros completos iguais e saldos7490720/938307/1000640 preservados (`kinja-v106-validation.json`), botão/bolsa capturados. **Gameplay coletivo:** não validado com consumo, callback visual de sucesso ou persistência de lote aplicado; grupos incluem peças reais e o teste consumidor continua fora do permitido. Ver relatório para confirmação posterior do reinício da build final.

A v105 foi somente compilada: revisão automática bloqueou instalação/lançamento pelo diagnóstico externo contendo saves completos. Essa exportação foi removida na v106/v107; diagnóstico privado, sem solicitar autorização adicional. Abort de inicialização original IL2CPP na primeira abertura v103 foi preservado como ocorrência não explicada; segunda abertura/v104/v106/v107 inicializaram. Não confundir esse histórico ou ações individuais do jogador com teste coletivo positivo.

**Reinício final v107:** PID20496→22535, preflight/modelos repetidos com195 itens/mesmas projeções/saldos; bolsa capturada em `kinja-v107-bag-after-restart.png`. `kinja-v107-validation.json` compara195 linhas antes com138 disponíveis depois, todas iguais;57 ausentes no recorte por supressão de logd. Não confundir validação interna195 com dump externo integral. SHA256 instalado confere com APK (`kinja-v107-installed-sha256.txt`). Nenhum `OneKey committed`/callback de sucesso durante testes; lote aplicado e sua persistência permanecem não validados.

## Investigação adicional de Unir tudo — antes da autorização, 14/10/2026

[Contrato coletivo e divergência do legado](analysis/native/ONEKEY_1_9_1_INVESTIGATION.md). O fluxo ET/assinatura/resposta/callback foram confirmados no APK1.9.1; o pedido vazio transfere a seleção para o servidor. O handler legado é outro protocolo: reúne todas as cópias retornadas por CheckCompound, não limita materiais à receita, gera UID novo e possui devoluções próprias. Não foi transplantado para ET. Nenhuma correção coletiva/build nova foi produzida; v102 permanece instalada. Aparelho conectado, captura/logs não destrutivos `build/kinja-onekey-current.*`; nenhuma fusão real coletiva foi executada. Ações individuais do jogador às20:33–20:34 são evidência concorrente, não teste nosso nem validação após reinício. Foi solicitada definição do escopo: equivalência remota mediante evidência adicional ou adaptação offline explicitamente autorizada; não inferir autorização pelo silêncio.

## Estado atual da fusão — v102, 14/10/2026

Build instalada `v102-fusion-atomic-recipes-selection-evidence`, package lab6 preservado por `adb install -r` e keystore existente. APK SHA-256 `D4B367A3E54B7A30093F6237AA54ECBD36B3A61DBCDEE25DB121498FEB87E5FC`; biblioteca `A7DED0DD84EC19309A7E58D3BD8E2537D0655F24E1FC56C0CDDC6492608DAD7D`. `firstInstallTime=2026-10-13 19:43:33`, `lastUpdateTime=2026-10-14 20:21:21`. Nenhuma cópia QA nova, wipe ou alteração visual de loja. Relatório completo, receitas, RVAs, passos e limitações: [FUSION_1_9_1_V102.md](analysis/native/FUSION_1_9_1_V102.md).

**Análise estática 1.9.1:** `EquipQualityCfg.cost` da qualidade atual é a receita explícita `[mesmo ID/mesma parte, qualidade dos materiais, quantidade]`. Qualidades1–3 exigem dois materiais iguais; qualidades superiores usam um material, com regras de ID/parte e qualidades diferentes conforme a faixa. O resultado conserva EquipId/UID/nível e aumenta Quality; limites/atributos/buffs seguem as configs originais. Há só um rank azul (Excellent/Excelente; Rare em textos de baús) e vermelho chega a +4. A receita dourada fornecida pelo jogador descreve os três passos acumulados roxo→+1→+2→dourado.

**Causas corrigidas:** o segundo `find_equipment_record` descartava a qualidade recém-alterada antes da gravação, embora os materiais fossem removidos. O bootstrap calculava caches antes de abrir função102; o cálculo original retornava cedo e deixava cópias compatíveis bloqueadas. A exigência fixa de três peças iguais também divergia das receitas superiores. Agora preserva a mutação, recalcula os caches pela API original depois dos unlocks, usa a receita viva, valida UIDs/save/live/wear/parte, prepara modelos antes do commit e faz readback/rollback. Inventário é atualizado antes do callback original de resultado. Guard e journal mantidos.

**Execução do bridge:** `build/kinja-v102-startup-logcat.txt` mostra hook individual e observador de `_selected` com prólogos verificados; os mesmos UIDs28–32 passam de can/relate0 para1 sem mudança de dados. `build/kinja-v99-json-regression.txt` registra teste ARM64 isolado com trecho real antigo/corrigido: reproduz qualidade descartada e confirma mutação correta. Esse teste não acessa o save do jogo.

**Visual:** `build/kinja-v102-pre-fusion.png` mostra os três UIDs QA confirmados por `Fusion UI selected=[28,29,30]`. `kinja-v102-fusion-result.png` mostra Colar de Cura Santa Excelente/azul, teto20→30, ATK31→43 e atributo de restauração de HP. `kinja-v102-bag-after-fusion.png` mostra a bolsa atualizada.

**Gameplay/save:** uma única fusão por ADB às20:23:48, somente cópias QA preexistentes da v84. Raiz28, EquipId3002, qualidade2→3, Nv.5/parte3/não equipado conservados; somente29/30 consumidos, sem moeda. Preflight e readbacks em `kinja-v102-fusion-once-logcat.txt`/`kinja-v102-fusion-result-logcat.txt`; plano pré-consumo em `kinja-v102-fusion-test-plan.json`. Após force-stop/start, processo22143 carrega UID28 q3; bolsa em `kinja-v102-after-restart-bag.png` tem azul Nv.5 e três verdes Nv.5 (antes seis). `kinja-v102-after-restart-selection-logcat.txt` reconfirma UID28. Ouro7.490.720/gemas938.307/EXP1.000.640 persistidos iguais; materiais não reapareceram nesse caso. Baselines JSON/logs são observações, não exportação integral do save privado; cobertura parcial de122/139 UIDs explicitada no relatório.

**Limites:** q2→q3 individual foi validada no aparelho e depois de reiniciar. Faixas superiores/receitas de um material, reentrada, erro/interrupção injetados e efeitos em combate continuam sem gameplay validado. Materiais equipados seguem recusados por segurança; comparar esse caso com o contrato original antes de ampliar. “Unir tudo” é outra rota ET e continua sem implementação local: a UI procura candidatos1–3, texto fala Epic e inferiores, seleção de raízes/grupos do servidor ainda desconhecida. Não consumir peças reais para testar a coletiva. Próximo item recomendado: confirmar esse contrato e implementar a transação própria, sem redirecioná-la para a individual.

**Revisão das notas antigas:** mensagens de commit v79/v80 não verificavam a qualidade gravada; não comprovam persistência diante do defeito confirmado. Root13 às18:14:20 precede instalação v80; root15 às18:17:57 está no arquivo `kinja-v81-before-install-logcat.txt`. Falha online v77 e recusa QA v83/v84 são casos distintos. Não repor materiais históricos por inferência; a v102 não fabrica reparações no save.

## Continuação — HangUp e energia v43/v44 (14/10/2026, 09:48–10:07 BRT)

**Build instalada:** v44 `build/kinja-run-offline-lab-v44-energy-measure.apk`, SHA-256 `20B4AC5B52A7FE93FC0C2D7592BE0A09064BA5F6DB0C5FD76B088DC7647DAF14`. Dispositivo Xiaomi 25069PTEBG/Android 16/API 36/ARM64; package `com.habby.kinjarun.offline.lab6`, jogo 1.9.1/versionCode 51. Atualização com `adb install -r`; `firstInstallTime=2026-10-13 19:43:33` permaneceu igual, sem limpar dados. Save observado: Capítulo 3/Lv.2, 1500 moedas, 0 gemas. APK original não foi alterado.

### HangUp: Receber e Volta Rápida

| Fluxo | Reprodução/evidência | Resultado e status |
|---|---|---|
| Abrir painel | Home 1280×2772; `adb shell input tap 630 2040`; `build/audit-v44-hangup.png`. | Painel abre com lista vazia, contadores 3/3 e botão verde Receber. Sem NRE no recorte v44. |
| Receber | Toque `(890,1835)`; capturas `build/audit-v44-receive.png`, `build/audit-v44-receive-recheck.png`; log do painel aberto `build/audit-v44-logcat.txt`; disassembly `build/disasm-hangup-receive.txt`. | **Causa confirmada:** `UITimeGainPanel.<OnClickReceive>d__45.MoveNext` verifica `_items` e encerra quando a lista está vazia; `RewardHangUp(0)` só é chamado com ao menos um item. O seed local `seed_hangup.json` tem `Items:[]`. O toque não falha em despachar a rotina: a rotina retorna pelo guard. Contudo, o botão verde não comunica que não há itens. Resgate/recompensas locais continuam sem implementação funcional. A recaptura manteve o painel vazio após o toque; o log do painel confirma a resposta local vazia. |
| Volta Rápida grátis | Fechar/abrir painel; tocar `(400,1820)`; no subpainel tocar opção grátis `(330,1890)`. `build/audit-quick-return-round.png`, `build/audit-quick-return-free-result.png`. | Abre a tela e a opção grátis volta ao HangUp. Log registra `RewardHangUp(type=1)` e resposta local `ET.G2C_RewardHangUp`, mas sem itens/recompensas; saldo ficou 1500/0. **Rota grátis despachada; concessão de recompensa não implementada.** Opções de gasto de 50 gemas/outras não foram tocadas. |

### Energia e entrada em fase

Na v43/v44 foi removido o nó estático `imgPower` do prefab `UIMainPanel/Content/Page/Main/Bottom/LevelHandle/Unclear/btnUnclearFight/imgPower`, que continha o rótulo `x5`; a implementação chama os métodos Unity/IL2CPP originais após `UIMainPanel.DoOpen`, mantendo o ícone `∞` da barra de recurso. `build/audit-v44-home.png` mostra a divergência visual corrigida. A medição em `UIMainPanel.OnClickFight` usa `PlayerDataManager.get_PlayerPower` e `ObscuredInt.GetDecrypted`, sem escrever no save.

Passos: na home tocar `(635,2300)` e conferir a captura `build/audit-v44-fight2.png` e log filtrado. Resultado observado: Power **0 antes → 0 depois (delta 0)**; a fase entrou normalmente em 1/10 com HP 90. Isso confirma entrada com saldo zero e ausência de dedução nessa tentativa. Não mede dedução partindo de saldo positivo: o save privado existente contém Power 0, o pacote não é debuggable (`run-as` negado) e não foi alterado/resetado para fabricar saldo. O teste terminou pelo botão Fim em 0/11, sem vitória nem recompensa; home permaneceu Capítulo 3/Lv.2, 1500/0. Portanto consumo numa entrada futura com Power positivo continua **não medido**.

| Área | Classificação após v44 |
|---|---|
| Energia infinita × `x5` | **Correção visual validada** em v43/v44 por captura e log do hook. Entrada em fase com Power 0 validada; delta medido 0. Débito com saldo positivo pendente por limitação de save. |
| HangUp/Receber | **Causa do não envio de RPC confirmada**: lista vazia ativa retorno antecipado. UX enganosa e recompensa de espera local pendentes. |
| Volta Rápida | **Abertura e rota grátis despachadas**; resposta offline é vazia, então nenhuma recompensa é concedida. Rotas com custo não testadas. |
| Estabilidade | Sem NRE de Shop/HangUp nem ANR no fluxo v44 capturado; não equivale a aprovação dos demais fluxos. |

**Próximo item:** tornar HangUp coerente quando não há itens e implementar regras locais de cálculo/recompensa de espera a partir dos dados 1.9.1, ou comunicar claramente indisponibilidade; em paralelo, medir gasto com saldo positivo somente se houver um meio não destrutivo de observar/obter essa condição. Vitória completa, recompensa, persistência pós-vitória e revive seguem pendentes.

## Continuação — HangUp local v41/v42 (14/10/2026, 09:33–09:46 BRT)

**Dispositivo/pacote:** Xiaomi 25069PTEBG, Android 16/API 36, ARM64; `com.habby.kinjarun.offline.lab6`, jogo 1.9.1/versionCode 51. APK final instalado nesta rodada: v42 `build/kinja-run-offline-lab-v42-hangup-reward-local.apk`, SHA-256 `578A89D24566EA7641EBE60D1249284CF9C014CB19047BF3B5D8EBDCF6DE415E`. Atualização com `adb install -r`; `firstInstallTime` permaneceu `2026-10-13 19:43:33`, sem limpar dados. Perfil observado antes e depois: Capítulo 3/Lv.2, 1500 moedas e 0 gemas.

### Causa, correção e resultado

A análise da função 1.9.1 `ET.HangUpHelper.GetHangUpData` (RVA `0x2964EB0`) confirmou que o MoveNext obtém `ET.Game.Scene`, procura `SessionComponent`, lê a sessão e envia `C2G_HangUpData`. A NRE observada ocorria antes da resposta de rede, quando a sessão offline não existe. A v40 tentou um prólogo incorreto; o guard reportou mismatch e recusou a alteração com segurança. Corrigido para o prólogo runtime exato `ff0301d1 f31300f9 fd7b03a9 fdc30091` na v41.

| Ação | Passos reproduzíveis/evidência | Resultado |
|---|---|---|
| Instalação e save | `adb install -r` no mesmo package; comparar `firstInstallTime` e a home. Capturas `build/audit-pre-v40.png`, `build/audit-v41-home.png`; logs `build/audit-v41-startup-logcat.txt`. | **Preservado:** firstInstallTime igual; home mostra 1500 moedas, 0 gemas e Capítulo 3/Lv.2. Nenhuma fase/recompensa foi acionada. |
| Inicialização HangUp | Limpar apenas o buffer transitório do logcat, iniciar app, aguardar e filtrar `HangUpHelper`, `NullReferenceException`, `FATAL EXCEPTION`, `ANR in`. | v41 instalou o hook; log confirma `JsonConvert.DeserializeObject ET.G2C_HangUpData`, `ETTask.Create(false)` e `SetResult`. **A NRE anterior não reapareceu** no startup observado. |
| Painel HangUp | ADB tap no ícone (Xiaomi 1280×2772: `(630,2040)`); captura `build/audit-v41-hangup.png`. | Painel abre e mostra lista “Nenhum item...”; sem crash. “Receber” ainda aparece verde, embora sem itens. Isso valida a abertura/apresentação, não o resgate. |
| Resposta do botão Receber | v42 adicionou hook verificado em `ET.HangUpHelper.RewardHangUp(short)` (RVA `0x2964F98`) com seed de resposta sem custo e sem recompensas. Hooks GetHangUpData e RewardHangUp instalados, registrado em `build/audit-v42-hook-check.txt`. Toques centrais ADB no botão `(890,1835)` e `(880,1845)`; captura `build/audit-v42-after-receive.png`. | **Inconclusivo:** o painel permaneceu igual e não houve log do hook RewardHangUp. O toque pode estar desabilitado/ignorado pela UI. Não afirmar que o resgate funciona nem que a RPC foi chamada. Saldo permaneceu 1500/0. |
| Estabilidade | Logs filtrados em `build/audit-v41-startup-logcat.txt` e `build/audit-v42-receive-logcat.txt`. | Sem NRE HangUp, `FATAL EXCEPTION` ou ANR no recorte; outras tentativas de AccountSDK/AppsFlyer/mail continuam observadas em background e são itens separados. |

**Classificação atual:** NRE de inicialização HangUp corrigida e confirmada por log/tela em v41 e novamente em v42. Dados vazios coerentes e painel aberto. O recebimento, “Volta Rápida”, atualização de contadores e persistência continuam **não testados/inconclusivos**; a chamada não disparada por ADB sugere estado de botão/UI que precisa ser rastreado antes de promover HangUp a funcional. A v40 não corrigiu nada e não é a build final.

**Nota histórica:** esta fila foi executada/reavaliada na seção v43/v44 acima. A causa do Receber foi localizada, a rota grátis de Volta Rápida foi exercitada e o `x5` foi ocultado. Permanecem pendentes a recompensa local coerente do HangUp e medição de energia com saldo positivo.

## Continuação da auditoria — v32 (13/10/2026, 23:05–23:16 BRT)

**Dispositivo/pacote:** Xiaomi 25069PTEBG, Android 16/API 36, ARM64; `com.habby.kinjarun.offline.lab6`, jogo 1.9.1, versionCode 51. APK instalado: `build/kinja-run-offline-lab-v32-shop-config-probe.apk` (SHA-256 `0F4EC15D02771EDD4747DDBEC18E04AFB082A99C748243A496F2FE268F582B8E`). Atualizado por `adb install -r`; sem desinstalar nem limpar dados.

O estado preservado continuou em Capítulo 3, nível 2, 1500 moedas e 0 gemas. O bootstrap registrou capítulos 1 e 2 concluídos e `stage=initialized`. O aparelho não foi iniciado numa fase nesta rodada.

### Verificações adicionais e classificação

| Área | Passos reproduzíveis e evidência | Resultado |
|---|---|---|
| Loja/baús | Após instalar v32, tocar o botão de loja/baú na home. `build/audit-v32-shop.png`; log Unity às 23:11–23:15. O probe no Unity chamou `TreasureBoxCfg.GetCfg(101/115/108)` e registrou `present` para os três. O toque repetiu `NullReferenceException` em `TreasureBoxItem.UpdateItem(TreasureBoxCfg)` via `UITreasureBoxPage.UpdateTreasurePage`/`UIMainPanel.OnClickShop`. | **Falha confirmada; hipótese de config ausente refutada.** Ainda não sabemos qual referência interna do item/prefab é nula. Não aplicar supressão genérica. |
| Correio | Observar log Unity após inicialização e aguardar resposta. Em v32, POST para `/api/v1/config` repetiu com `SocketError=11001`, `tryCount=4`; `delayTime` observado ~70–121 s. O painel não foi confirmado aberto nesta rodada. | **Dependência remota e espera confirmadas.** UI do correio permanece não testada. |
| HangUp | Log Unity após home mostrou `ET.HangUpHelper.GetHangUpData` com `System.NullReferenceException`; o fluxo ocorre sem tocar no botão também. | **Falha confirmada**, recompensa local não validada. |
| Persistência do save | Atualizar o APK com `adb install -r` e aguardar bootstrap; conferir log de capítulos e a home. | **Preservação observada**: Capítulo 3/Lv.2/1500/0 mantidos. Não é validação de recompensa após vitória/reinicialização. |
| Energia/partida | Home v32 manteve `∞` e o botão `x5`; não iniciamos fase nesta rodada. | **Inconsistência visual confirmada; consumo real não testado.** |
| Heróis/equipamento | Evidência v31 permanece: placeholder `sdgasgdsg`, requisito chinês no painel. Nesta rodada não houve correção nem novo fluxo de compra/equipamento. | **Falha de conteúdo confirmada em v31; v32 não revalidou essa tela.** |

### Diagnóstico do Shop (evidência e limite)

O disassembly para este APK mostra `UpdateTreasurePage` buscando os IDs 101, 115 e 108 e passando os resultados diretamente a `TreasureBoxItem.UpdateItem`. O probe temporário de v32 confirmou em runtime que todos retornam objeto. Portanto, a hipótese de linha de config ausente foi descartada. `UpdateItem` também busca referências de UI por `UIItemBase.GetGameObject(string)` e lê campos da config; o stack não informa qual delas é nula. O probe foi removido da fonte após esta medição; v32 instalada continua sendo apenas um artefato de diagnóstico. A causa ainda requer isolamento do primeiro acesso nulo no método e correlação com o prefab/config runtime.

### Estado prioritário após v32

1. **Confirmadas:** Shop quebra apesar de configs existentes; HangUp lança NullReference; correio repete chamada remota e espera longa; home mostra custo `x5` apesar do status infinito.
2. **Confirmadas em v31, sem nova revalidação na v32:** placeholder e chinês em equipamento/herói; tentativas dos SDKs de anúncio.
3. **Suspeitas:** UI do Shop tem child/widget faltante ou campo de config aninhado não preenchido. A evidência atual não distingue esses casos.
4. **Não testadas nesta rodada e ainda pendentes:** vitória completa/recompensa, revive, consumo real de energia, talentos/equipar, persistência de progresso após uma vitória e abertura funcional de correio.

Capturas: `build/audit-v32-home.png`, `build/audit-v32-home2.png`, `build/audit-v32-shop.png`. O recorte novo foi consultado diretamente via ADB/logcat e não foi salvo como log bruto por conter identificadores do dispositivo.

**Próximo passo:** isolar o primeiro acesso nulo em `TreasureBoxItem.UpdateItem` sem modificar layout de objetos IL2CPP diretamente; em paralelo, mapear chamadas de rede de mail e localização/seed de herói. Qualquer correção deve ganhar BuildId novo, instalar por atualização do mesmo package e repetir o fluxo afetado.

## Continuação — diagnóstico v33/v34 e correção v35 (13/10/2026, 23:28–23:36 BRT)

**Build instalada ao final:** v35 `build/kinja-run-offline-lab-v35-local-shop-chest-state.apk`, SHA-256 `E2A939630FD0AA0674205345E983ADEE82398C17FEDFE086A757377D4628F4A3`. Pacote `com.habby.kinjarun.offline.lab6`, jogo 1.9.1/versionCode 51, Xiaomi 25069PTEBG Android 16/API 36 ARM64. Instalada por `adb install -r`, sem desinstalar/limpar dados.

### Causa confirmada da exceção do Shop

Na v33, o trace de `UIItemBase.GetGameObject` observou widgets ausentes (`btnOpenBox` e `btnGuideFreeOpenBox`) em chamadas de inicialização. Porém, na v34, o trace da entrada de `TreasureBoxItem.UpdateItem` durante o toque real registrou `self=present cfg=present` e, depois, nenhuma chamada a `GetGameObject` antes da exceção. Isso descartou os widgets como causa deste crash específico.

O disassembly 1.9.1 resolve o ponto: `UpdateItem` chama `ShopManager.GetShopChestState(cfg.id)` (RVA `0x26D5ECC`) e lança NullReference se o resultado for nulo, antes de acessar widgets. `GetShopChestState` consulta `ShopManager._shopChestState.List`; sem uma linha com o ID solicitado, retorna nulo. A UI pede baús 101, 115 e 108. O bridge anterior não entregava `G2C_ShopChestState`, então faltava o estado de conta do Shop, embora as três configs estáticas existissem.

### Correção e validação v35

Foi adicionado `offline/assets/seed_shop_chests.json` com registros locais 101/115/108 e listas vazias não nulas. O bridge desserializa o modelo original `ET.G2C_ShopChestState` e chama `ShopManager.OnGetChestState` na thread Unity, junto dos outros callbacks locais. O seed novo foi persistido na primeira inicialização v35; seeds existentes e dados de progressão foram carregados sem migração ou sobrescrita.

Passos executados: atualizar v34 para v35 com `adb install -r`; iniciar o app; confirmar `Persisted initial local seed seed_shop_chests.json`, `Invoke OK: ShopManager.OnGetChestState` e `stage=initialized`; tocar Loja em `(115,2600)` na tela 1280×2772; observar o painel; voltar à home. Resultado: **Shop abre; nenhuma exceção `TreasureBoxItem.UpdateItem`/`UITreasureBoxPage.UpdateTreasurePage` após o toque**. Capturas `build/audit-v35-shop.png` e `build/audit-v35-home-after-shop.png`. A home manteve Capítulo 3, Lv.2, 1500 moedas, 0 gemas; nenhuma partida ou compra foi iniciada. Isso valida abertura/renderização do painel, não abertura de baú nem recompensas.

### Pendências expostas pela tela

- **Pagamento real: falha de produto confirmada.** A Loja exibe vários cartões de gemas com preços em dólares (ex.: `$0.99`) na captura v35, incompatível com a meta offline sem pagamentos. Nenhum cartão foi tocado. A próxima correção deve ocultar ou substituir esses cartões por informação explícita de indisponibilidade e bloquear a entrada no fluxo de compra.
- A tela exibe baús por 60/250 gemas. O saldo está em 0; nenhum baú foi aberto. As chamadas de compra e a possibilidade de alcançar serviços online continuam não testadas e precisam de tratamento offline antes de considerar a Loja plenamente funcional.
- **HangUp segue falhando:** ao abrir a home v35, log Unity às 23:36:07 mostrou `ET.HangUpHelper.GetHangUpData` NullReference por `ActivityManager.RequestHangUpInfo`/`Main.OpenMainPanel`. Não impediu a home ou Shop de abrir.
- O login Habby ainda tentou DNS e terminou depois de `tryCount=4`, com `SocketError=11001` por volta de 23:36:23. A v35 não neutraliza esse serviço.
- Energia ainda aparece como `∞` junto de `x5`; consumo real não foi exercitado.

Estado após v35: **Shop/baús — abertura corrigida e validada; operação dos baús não testada.** O próximo item prioritário é remover da Loja as ofertas de dinheiro real e neutralizar qualquer entrada de compra, mantendo o painel e as opções locais coerentes.

## Continuação — política de compras da Loja v36/v37 (13/10/2026, 23:41–23:44 BRT)

**APK instalada ao final:** v37 `build/kinja-run-offline-lab-v37-no-iap-shop.apk`, SHA-256 `F1CFB86CCE951C6DCDF667407641975EB5D49D53A97A7849795448703960A0E9`. Pacote `com.habby.kinjarun.offline.lab6`, 1.9.1/versionCode 51; Xiaomi 25069PTEBG Android 16/API 36/ARM64. Atualizada por `adb install -r`; `firstInstallTime` permaneceu 19:43:33 e `lastUpdateTime` passou a 23:43:09. Não houve limpeza de dados, desinstalação, entrada em fase ou toque em oferta.

### Evidência e alteração

Na captura pré-alteração `build/audit-v35-shop.png`, a seção Gemas mostrava produtos pagos `$0.99`. O dump do cliente 1.9.1 identifica `UITreasureBoxPage.UpdateBuyDiamond` em RVA `0x13DFB00` e `PurchaseManager.Purchase(int)` em RVA `0x2AA0120`. A v36 instalou o hook verificado da primeira função e registrou `Skipped real-money product catalog in offline Shop`, mas `build/audit-v36-shop.png` ainda mostrou os cartões com preço: essa substituição não remove o conteúdo do prefab. Não foi tocada nenhuma oferta.

A primeira expectativa estática de bytes para o hook de `PurchaseManager.Purchase` foi serializada na ordem errada; o guard recusou a alteração e logou o prólogo lido sem modificar a função. Corrigi a sequência usando os 16 bytes ARM64 efetivamente observados no runtime (`ff8304d1fc670da9f85f0ea9f6570fa9`), coerentes com a instrução do binário. Na v37, o log confirma `Installed PurchaseManager.Purchase memory hook` e `Installed UITreasureBoxPage.UpdateBuyDiamond memory hook`, ambos sem mismatch. O hook de compra retorna sem encaminhar a chamada ao original; nenhuma tentativa de compra foi acionada para evitar risco desnecessário. Isso valida instalação do bloqueio central, ainda não um clique ponta a ponta.

### Resultado, limitações e fila atual

- **Pagamento real — parcialmente mitigado, ainda falha visual confirmada:** a loja continua mostrando ofertas e preços; v37 ainda não as oculta. O caminho central `PurchaseManager.Purchase` está interceptado, mas não se tocou em um cartão para provar que todo botão chega a esse método. Não classificar pagamentos como aprovados.
- **Shop/baús:** v35 validou abertura do painel após fornecer os estados locais. A captura v37 mostra que os painéis bronze/diamante continuam visíveis e cobram gemas; abertura de baú, consumo de gemas e recompensas seguem sem teste.
- **HangUp — exceção recorrente confirmada, não regressão da Loja:** a stack v37 é `ET.HangUpHelper.<GetHangUpData>d__0.MoveNext` → `ActivityManager.<RequestHangUpInfo>d__268.MoveNext` → `Main.<OpenMainPanel>d__10.MoveNext`, a mesma rotina da falha prévia. O log aparece ao entrar na home/startup enquanto o painel da Loja renderiza depois; não atribuir essa NRE ao callback de baús. O seed de Shop v35 segue sem reproduzir a exceção original `TreasureBoxItem.UpdateItem`.
- **Ainda confirmado/anterior:** HangUp sem estado local lança NRE; o AccountSDK ainda tenta DNS e retorna erro após retries; home mostra `∞` junto de `x5`; consumo real de energia não testado.
- **Ainda não testado:** combate até vitória completa, persistência pós-vitória/reinício, recompensas de vitória, reviver em execução, compras/abertura de baú, clique seguro de IAP bloqueado, equipamento/heróis e fluxo de correio sem espera.

Passos reproduzíveis v37: `adb install -r build/kinja-run-offline-lab-v37-no-iap-shop.apk`; iniciar `com.habby.kinjarun.offline.lab6`; consultar `adb logcat -d -s KinjaOffline Unity`; tocar a aba Loja em `(115,2600)` numa tela 1280×2772; capturar `build/audit-v37-shop.png`. Evidências: `build/audit-v37-startup-logcat.txt`, `build/audit-v37-shop-logcat.txt`, `build/audit-v37-home.png`, `build/audit-v37-shop.png`. A tela v37 mantém 1500 moedas e 0 gemas; nenhum toque iniciou fase ou oferta. O próximo passo prioritário é remover visualmente as ofertas pagas e verificar o destino dos cliques sem abrir checkout; em seguida, separar e corrigir a nova exceção da Loja e reduzir retries do AccountSDK/correio.

## Continuação — ocultação do prefab de ofertas pagas v38/v39 (14/10/2026, 09:29–09:34 BRT)

**Build instalada ao final:** v39 `build/kinja-run-offline-lab-v39-hide-prefab-purchases.apk`, SHA-256 `A2D9E511A5CF94D5B5FD71B8D052EF66E3F0119A7B67C112AAFF05046DF577DE`. Mesmo pacote de laboratório, jogo 1.9.1/versionCode 51, Xiaomi 25069PTEBG/Android 16/API 36/ARM64. `adb install -r` atualizou `lastUpdateTime` para 09:33:53 sem mudar `firstInstallTime` (13/10/2026 19:43:33). Não foram usados `pm clear`, desinstalação, clique em oferta, compra, abertura de baú ou reivindicação de recompensa.

### Causa visual e correção

O teste v38 instalou um hook em `BuyDiamondItem.UpdateItem`, mas o método não foi executado na abertura da Loja e os cartões permaneceram. A inspeção do bundle original `assets/AssetBundles/Android/clientgameobjectsui48.ht` com o UnityPy local mostrou o prefab `UITreasureBoxPage`: o nó `UITreasureBoxPage/obj/ContentPanel/Top/BuyDiamond` contém `imgTitleBg`, os seis objetos `BuyDiamondItem1`–`BuyDiamondItem6` e `diamondShopEffect`. São cartões pré-renderizados, cuja visibilidade não depende de `BuyDiamondItem.UpdateItem` quando a lista remota está vazia.

A v39 substitui o hook incorreto por um trampoline para a implementação original de `UITreasureBoxPage.UpdateBuyDiamond` (RVA `0x13DFB00`). Após a chamada original, o hook usa APIs IL2CPP originais na thread Unity: `Component.get_transform`, `Transform.Find("obj/ContentPanel/Top/BuyDiamond")`, `Component.get_gameObject` e `GameObject.SetActive(false)`. O guard dos 16 bytes permanece ativo. O sibling com os painéis de baú é mantido; apenas o nó `BuyDiamond` (cabeçalho e cartões de pagamento real) é desativado. `PurchaseManager.Purchase(int)` segue bloqueado por hook central checked, embora um clique de produto não tenha sido exercitado.

### Validação v39

Passos: confirmar o pacote v38/1.9.1 no aparelho; atualizar com `adb install -r build/kinja-run-offline-lab-v39-hide-prefab-purchases.apk`; iniciar o app; consultar logcat; tocar Loja em `(115,2600)` em 1280×2772; capturar a tela. O log registra `Installed UITreasureBoxPage.UpdateBuyDiamond`, `Installed PurchaseManager.Purchase`, `UITreasureBoxPage.get_transform`, `Find paid Shop prefab section` e `Hidden prefab BuyDiamond section (six real-money cards); local chest section remains enabled`. `build/audit-v39-shop.png` mostra que os seis cartões pagos e o cabeçalho Gemas sumiram; os painéis Bronze/Diamante seguem visíveis e a seção Ouro, que usa gemas, aparece no lugar. A tela manteve 1500 moedas e 0 gemas; nenhuma fase/recompensa foi iniciada.

- **Ofertas por dinheiro real — correção visual validada na Loja v39.** O método central de pagamento está instalado mas sem teste de clique ponta a ponta.
- **Gasto de gemas em baús/ouro — não testado e ainda pendente.** São controles do jogo que usam moeda local; os callbacks podem depender do backend. Não tratar como funcional só porque os controles aparecem.
- **Anúncio de recompensa no cartão grátis de Ouro — não acionado.** O hook geral de rewarded ads substitui a reprodução por callback local imediata, mas a recompensa/saldo após esse fluxo não foi verificada para evitar alteração não necessária no save.
- **HangUp — falha conhecida mantida.** A exceção capturada v39 é `ET.HangUpHelper.<GetHangUpData>d__0.MoveNext` → `ActivityManager.<RequestHangUpInfo>d__268.MoveNext` → `Main.<OpenMainPanel>d__10.MoveNext`, e aparece no startup/home; a Loja renderizou depois. Não é NRE de baú. O comportamento local coerente para rewards de inatividade continua por implementar.
- **Fila ainda aberta:** tentativas do AccountSDK/correio e latência; estado da energia durante partida; vitória/recompensas/persistência; revive; equipamento/heróis; abertura de baús e gasto de gemas. Ver logs `build/audit-v39-startup-logcat.txt` e `build/audit-v39-shop-logcat.txt`, capturas `build/audit-v39-home.png` e `build/audit-v39-shop.png`.

**Data:** 13/10/2026 (coleta inicial 22:42–22:48 e teste ADB adicional por volta de 22:52, America/Sao_Paulo)  
**Dispositivo:** Xiaomi 25069PTEBG, Android 16 / API 36, ARM64  
**Pacote instalado:** `com.habby.kinjarun.offline.lab6`, versão do jogo `1.9.1`  
**Tela capturada:** seleção do Capítulo 3, home com nível 2, ícone de energia infinita e navegação inferior.

## Resumo

A build inicializa offline e alcança a home/capítulo selecionado. O log comprova que os callbacks locais carregam e que `FuncManager.SetOpeningFunc` recebe `[101,102,103,116]`. A home renderiza alguns menus, mas os erros do Unity mostram que vários painéis não estão funcionais. O principal foco pendente é preencher os modelos/configurações locais que esses painéis esperam, ou substituir os fluxos remotos por equivalentes offline.

## Falhas confirmadas por logs de execução

| Prioridade | Área | Evidência | Impacto observado / provável |
|---|---|---|---|
| Alta | Loja / baús | 22 `NullReferenceException` com topo `TreasureBoxItem.UpdateItem(TreasureBoxCfg)`, cadeia `UITreasureBoxPage.UpdateTreasurePage` → `UIMainPanel.OnClickShop`. | O clique da Loja chega ao painel, mas a atualização de itens lança exceção. A tela/itens de baú não estão confiáveis. O log não identifica qual referência é nula; investigar `TreasureBoxCfg`, lista de baús e dados do `TreasureBoxItem`. |
| Alta | Tela de herói | 4 exceções em `UIHeroPage.HeroProperty`, com cadeia `UIHeroPage.UpdatePage` / `ShowPage`. | A tela de herói tenta atualizar atributos e falha; gestão e visualização de heróis estão incompletas. |
| Alta | Ganhar personagem | 2 exceções em `UIHeroInfomation.OnClickGainHero`; também há falhas em `OnClickSelectHero` e `OnClickItemCallBack`. | Fluxo de obtenção/seleção de personagens não conclui corretamente; desbloqueio de heróis não está validado como funcional. |
| Alta | Evolução de herói | 4 exceções em `UIHeroInfomation.UpdateLevelInfo`. | A atualização/renderização do nível do herói falha; investigar modelo local, configuração de evolução e dados requeridos pelo painel. |
| Alta | Talentos | 4 exceções em `ET.TalentHelper.UpgradeTalent`. | O fluxo de melhoria de talento não consegue completar a operação local. O botão/painel visível não significa que a compra/upgrade foi aplicada. |
| Média | Recompensa de inatividade | 4 exceções em `ET.HangUpHelper.GetHangUpData`; aparece durante a abertura da home e também por `UIMainPanel.OnClickHangUp`. | A solicitação continua chamando lógica que depende do serviço online; o painel pode falhar ou não conceder recompensa. A implementação offline de `FuncType 116` ainda está incompleta. |

As contagens são ocorrências no log coletado, não quantidade de botões diferentes. Repetições provavelmente vieram de tentativas e atualizações periódicas de UI.

## Offline e dependências de rede

- **Login Habby:** o cliente ainda tenta `https://account.kinjarun.com/v1/identityProviders/device?action=login`. O log registra `Could not resolve host 'account.kinjarun.com'` (`SocketError 11001`). O bootstrap local termina com `stage=initialized` e abre a home apesar dessa falha. Resultado: **fallback local funciona; o cliente ainda tenta autenticar online**.
- **Correio:** o cliente envia POST para `https://mail.kinjarun.com/api/v1/config` e `/api/v1/mails/list`. A resposta de config registrada falha após quatro tentativas, `delayTime` de cerca de 43–45 s e `SocketError=11001`. É uma dependência remota residual e gera espera/logs.
- **Publicidade:** Google Ads e ironSource tentam carregar anúncios e registram falhas de inicialização/carregamento. O hook `AdManager.ShowRewardAdWithCallback` está instalado, mas esta auditoria não acionou a morte/revive para confirmar o efeito da callback na v31.
- Os erros de resolução demonstram que os hosts não resolveram neste aparelho/sessão. Não provam por si só que todas as funções online foram removidas, nem que a sessão inteira estava em modo avião.

## O que foi confirmado funcionando

- O pacote v31 está instalado e o processo Unity permanece aberto, sem `FATAL EXCEPTION` ou ANR no recorte coletado.
- O bridge instalou os hooks principais e passou por `AccountInit` com `stage=initialized`.
- Os callbacks locais de jogador, itens, equipamento, talentos, heróis, regiões e tutorial retornaram `Invoke OK`.
- `FuncManager.SetOpeningFunc` recebeu `[101,102,103,116]`, com `Level=2`, Capítulo 1 concluído e Capítulo 2 concluído no save.
- A UI da home chegou a renderizar Capítulo 3 e alguns ícones de navegação. O topo mostrava `∞`; o botão de batalha ainda mostrava `x5`.
- `CheckPlayerStatusValue` retornou 1 para IDs 8 (SkipAd), 9 (InfinitePower) e 10 (AdditionalResurrection).
- Os três métodos de painel online da guilda são ignorados pelo bridge. Isso é intencional; guilda/competição não está implementada offline.

## Testes automatizados via USB (22:52 em diante)

O usuário habilitou **Depuração USB (Config. de segurança)** nas opções do Xiaomi; os toques `adb shell input tap` passaram. Foi possível percorrer algumas telas Unity, que não aparecem no dump de acessibilidade por serem desenhadas no `SurfaceView`.

| Área / ação | Resultado observado | Classificação |
|---|---|---|
| Home | Abre em Capítulo 3, Lv.2, 1500 moedas, 0 gemas, energia `∞`; botão da fase continua mostrando `x5`. | Parcial: home funcional; apresentação de energia inconsistente. |
| Talentos (abrir aba) | Tela abre. A árvore mostra itens `Travado`, nós numerados 1/2 e alerta vermelho. | Abertura passou; compra/upgrade não acionado. Não equivale a validar o sistema de talentos. |
| Equipamento (abrir aba) | Tela abre com personagem e espaços vazios. Texto do personagem é placeholder `sdgasgdsg`; a faixa de herói mostra em chinês `通关12章解锁英雄` (“conclua o Capítulo 12 para desbloquear herói”). | Falha de conteúdo/localização; fluxo de trocar equipamento não testado. |
| Loja/baús (tocar ícone de baú) | Reproduzido ao vivo: `NullReferenceException` em `TreasureBoxItem.UpdateItem(TreasureBoxCfg)` → `UITreasureBoxPage.UpdateTreasurePage` → `UIMainPanel.OnClickShop`. A UI não sai da tela de equipamento ao tocar no ícone. | **Falha confirmada**, alta prioridade. |
| Configurações | Modal abre e mostra idioma Português, som/música, sensibilidade, feedback, privacidade, termos e PID. | Abertura passou; alterações e links externos não testados. |
| Correio | Requisições de config/list continuam falhando por DNS após quatro tentativas (`SocketError 11001`, demora ~31–53 s nos recortes). Tentativa de toque não confirmou abertura do painel de correio. | Falha de backend confirmada; interação de UI inconclusiva. |
| Começar Capítulo 3 | Botão inicia a fase 1/10; personagem aparece com 90 HP; gameplay roda e os inimigos avançam. | Abertura da fase passou. Não testamos o percurso completo. |
| Pausa / retomar | Botão superior direito abre painel escurecido com música, sensibilidade, **Fim** e **Continuar**. Toque em **Continuar** fecha o overlay e a fase continua rodando; nova captura mostra o personagem e a HUD de gameplay. | Passou no smoke test; não foi um teste prolongado de controles durante combate. |
| Fim da partida | Ao tocar **Fim**, a fase encerra imediatamente e abre resultado `Terminar 0/11`, `Você não coletou nenhum loot`, com botão “Toque para fechar”; não pede confirmação. Fechar volta à home. | Fluxo responsivo. UX pode ser brusca; abortar não conclui fase. |
| Moedas | A HUD da fase chegou a mostrar 25 moedas após alguns segundos; no resultado prematuro constava 0/11 e nenhum loot. Após fechar, home continuou em 1500 moedas. | Não demonstra falha de recompensa de fase completa: a partida foi encerrada antes de matar os 11 inimigos. Persistência da moeda de sessão/recompensa de vitória ainda precisa de uma partida concluída. |
| Reviver sem anúncio | Não testado neste recorte (não morremos). A inicialização ainda tenta carregar anúncios; IronSource registra `init() had failed`. | Pendente. |

Não houve `FATAL EXCEPTION` nem ANR nos recortes da sessão. A `NullReferenceException` do Shop é uma exceção Unity capturada e o app continua aberto; isso não significa que a função esteja operacional.

## Ainda não verificado na auditoria inicial v31 (registro histórico; veja atualizações posteriores)

- Consumo real de energia ao começar e repetir partidas; o ícone `∞` e o custo visual `x5` continuam divergindo.
- Dano, vida dos inimigos, recompensas e progressão do Capítulo 2 nesta v31; a partida iniciada no teste foi Capítulo 3 e encerrada em 0/11.
- Reviver sem anúncio, restauração de controle/tela após revive e morte completa.
- Vitória completa, coleta de ouro, concessão e persistência de recompensa, avanço de fase/capítulo e ciclo de save após reiniciar o app.
- Comprar talento, abrir/usar inventário e baús, equipar item, seleção/evolução/compra de herói e outras telas de campanha.
- Se os avisos do correio e SDK de anúncio atrasam bootstrap em inicialização limpa; o jogo abriu nesta sessão, mas houve tentativas remotas em background.

## Próxima etapa recomendada

1. Corrigir primeiro a configuração/lista de `TreasureBoxCfg` ou tornar a tela da loja segura com catálogo local; o clique já reproduz a exceção.
2. Revisar o seed do jogador e localização de heróis: placeholder de nome, requisito chinês do Capítulo 12 e espaços de equipamento vazios/sem explicação.
3. Fazer uma partida curta completa com automação de input controlado (ou assistência de teste) para confirmar moeda, primeira vitória, progressão e energia. O teste desta sessão só cobriu a entrada e o encerramento prematuro.
4. Acionar morte/revive e validar o retorno ao gameplay nesta build; anúncios não devem aparecer no modo offline.
5. Neutralizar as chamadas restantes de mail/login/ad SDK para retirar tentativas de rede e possíveis esperas.

Não mexer nos erros de painel com um patch genérico para suprimir `NullReferenceException`. Identificar primeiro qual campo/config/dado está ausente e decidir entre seed local, resposta offline, ocultar função não suportada ou UI substituta coerente.

## Evidência local coletada

- Capturas anteriores: `build/audit-before.png` e `build/audit-ui.xml`.
- Capturas desta sessão: `build/audit-home-current2.png`, `build/audit-talent.png`, `build/audit-equipment2.png`, `build/audit-settings2.png`, `build/audit-chest2.png`, `build/audit-stage-enter.png`, `build/audit-pause.png`, `build/audit-resume.png`, `build/audit-end-confirm.png` e `build/audit-after-stage.png`.
- Logs brutos foram filtrados na sessão e não salvos, para não manter identificadores do aparelho. A evidência textual foi transcrita acima; o stack da Loja é a falha reproduzida mais conclusiva.
- A permissão `INJECT_EVENTS` deixou de bloquear os toques após habilitar **Depuração USB (Config. de segurança)**; a barreira de automação anterior foi removida.

## Ordem de correção sugerida

1. Resolver o Shop/baús e o inventário local, pois a configuração/dados de recompensa também podem ser necessários para progressão.
2. Corrigir o fluxo de heróis (`HeroProperty`, `UpdateLevelInfo`, ganhar/selecionar`) com modelos/configurações locais e custos em moedas de jogo.
3. Corrigir upgrades de talentos e validar persistência/custo local.
4. Implementar uma recompensa de inatividade local simples ou ocultar esse botão e deixar o ID 116 fechado até funcionar.
5. Cortar/neutralizar as tentativas de login, correio e inicialização de SDKs de anúncios no wrapper offline; verificar que a inicialização não aguarda essas respostas.
6. Revisar a apresentação de energia para que o botão não mostre `x5` quando energia é infinita; confirmar consumo real em partida automatizada.

## Continuação — HangUp local e persistência v50/v51 (14/10/2026, 10:49–10:54 BRT)

**Dispositivo/pacote:** Xiaomi 25069PTEBG, Android 16/API 36, ARM64; `com.habby.kinjarun.offline.lab6`, jogo 1.9.1, versionCode 51. O `firstInstallTime` continuou `2026-10-13 19:43:33`; atualizações foram feitas com `adb install -r`, sem limpar dados ou desinstalar. APK v51 `build/kinja-run-offline-lab-v51-hangup-persist-currency.apk`, SHA-256 `2362657ADD56F358012AD0CC8A76CC7EB4396C2E877D02A79C7F77960A618B59`.

| Fluxo | Passos/evidência | Resultado/status |
|---|---|---|
| Inicialização e save | Atualizar v50 e depois v51 com `adb install -r`; iniciar pelo `monkey`; verificar home/log. Evidências: `build/audit-v51-claim-logcat.txt`, `build/audit-v51-startup-logcat.txt` (startup inicial registrado em `build/audit-v50-startup-logcat.txt`). | `stage=initialized`; sementes existentes `seed_player.json` e `seed_regions.json` carregadas. `firstInstallTime` não mudou. Sem `FATAL EXCEPTION`/ANR nos logs v51. Home permaneceu Capítulo 3/Lv.2, 0 gemas e sem limpar perfil. |
| Receber HangUp | Na tela 1280×2772, abrir ícone central `(630,2040)` e tocar Receber `(890,1835)`. v51 logou `GetOwnItemNum` antes/depois, setters originais, `SyncValueToShow`, persistência e resposta `Error=0`. Screenshot: `build/audit-v51-claim.png`. | **Passou em gameplay/save.** Saldo observado antes 1500 ouro/0 EXP; a coleta local do Capítulo 3 pagou 7/7 e a leitura posterior foi 1507 ouro/7 EXP. A lista não vazia despachou o botão. O bridge só avança o relógio HangUp após conferir os dois saldos. |
| Persistência após reiniciar | `am force-stop` e novo `monkey` (sem `pm clear`); aguardar home. Evidência: `build/audit-v51-restart.png`, `build/audit-v51-restart-logcat.txt`. | **Passou.** Após reiniciar, home mostra 1507 ouro e 7/2000 EXP; Capítulo 3/Lv.2. `GetHangUpData` informa apenas o novo tempo decorrido (~48 s, 2/2), sem reaplicar a coleta já recebida. |
| Volta Rápida | Abrir HangUp e botão `(390,1835)`. Captura do diálogo atual: `build/audit-v51-quickreturn.png`. | **Parcial/não concluído nesta build.** O diálogo só expõe o botão de 50 gemas; não foi tocado. A rota type 2 retorna indisponibilidade local com custo zero, mas a apresentação desse erro ao usuário e o fluxo grátis precisam de confirmação. Um teste de v46 alcançou a callback grátis sem anúncio, mas é anterior à correção de crédito/persistência e não comprova saldo. |
| Energia | O save continua com Power zero. A entrada de fase em build anterior leu 0→0 e foi permitida; v44 removeu `x5` estático. | **Pendente para saldo positivo.** Não alterar o save para fabricar energia. |

### Causa e correção HangUp

O bridge já calculava ouro/EXP por `TimeGainCfg` e o botão `Receber` só chama `RewardHangUp(0)` quando `_items` não está vazia (confirmado na state machine de `UITimeGainPanel.<OnClickReceive>`). O primeiro seed vazio explica os toques sem despacho em v42; com taxa acumulada, v50 apresentou os itens e recebeu o toque. Os testes v46–v49 provaram que `ProtocolManager.AddCoin/AddExp`, `ItemManager.AddLocalItem` e `ItemManager.OnItemUpdate` não alteravam os contadores especiais 101/104. A v50 passou na leitura de runtime com `PlayerDataManager.set_PlayerGold/set_PlayerExp` e `SyncValueToShow`, mas o saldo voltou ao seed após reiniciar. A v51 corrigiu a persistência: atualiza somente `Coin` e `Exp` dentro do `files/offline_save/seed_player.json` existente, via gravação temporária/rename, preservando os outros campos, e confirma os totais por `ItemManager.GetOwnItemNum` antes de aceitar. **A v50 é histórica e não persistente; usar v51 como evidência atual.**

Regra local v51: taxa lida da tabela original do capítulo selecionado (Capítulo 3 observou 180 ouro e 180 EXP por hora); coleta normal acumula no máximo seis horas. Volta Rápida é configurada para conceder seis horas, máximo 3/dia; a concessão type 1 ainda não foi relida/persistida nesta versão porque a interface atual não mostrou opção grátis. Tipos pagos por gemas/energia permanecem rejeitados sem custo pelo hook; não classificar a experiência de usuário desse erro como validada. O estado auxiliar fica em `offline_save/hangup_local_state.json` (`LastClaim`, `QuickReturnDay`, `QuickReturnCount`).

### Fila atual

1. HangUp normal — corrigido e validado quanto a cálculo, crédito em 101/104, UI e persistência após reinício em v51.
2. Volta Rápida — callback grátis só observada em build anterior; a tela atual mostra 50 gemas. Reconciliar apresentação da opção grátis/indisponibilidade e validar type 1 sem tocar em opção paga.
3. Energia infinita — apresentação `x5` removida; consumo com Power positivo permanece pendente porque o save preservado tem zero.
4. Continuam não testados: vitória completa/recompensa de fase, reviver com retorno jogável, gastos de gemas em baús/talentos/equipamento e persistência de progresso de vitória.

## Continuação — moedas de combate v51 (14/10/2026, 11:00–11:02 BRT)

Na v51 instalada, com home em Capítulo 3/Lv.2 e saldo 1507 ouro/7 EXP/0 gemas, abri a fase atual (bridge identificou região local 4, `enemyCount=121`). Sem tocar em controles, cinco inimigos comuns foram derrotados. `build/audit-v51-partial-fight-logcat.txt` registra cinco `Local KillEnemy` e cinco `Local kill payout ... gold=5`; `build/audit-v51-partial-fight.png` mostra 25 moedas na HUD de gameplay. Interrompi por `am force-stop` antes de `EndGame`, sem chamar o botão Fim; após reiniciar, `build/audit-v51-post-abort.png` mostra a home ainda em 1507 ouro/7 EXP, Capítulo 3/Lv.2 e 0 gemas. Não houve `FATAL EXCEPTION`/ANR no recorte. O processo parou antes do callback de fim, portanto nenhum progresso de fase foi gravado.

**Interpretação:** o pagamento de moeda da HUD é confirmado durante a sessão; abandono/encerramento do processo não o converte em saldo persistente, o que é esperado se a liquidação só ocorrer em vitória. O caminho de código confirma que `kill_enemy_hook` apenas define `G2C_KillEnemy.Gold`; `end_game_hook` usa `seed_end_game.json` (Gold=0) e `persist_local_clear` persiste somente recompensas configuradas em `ItemsInfo` (baú/marco), não esse ouro. Portanto, a moeda após **vitória completa** é suspeita, ainda não falha confirmada: falta observar se a rotina original acumula os payouts por kill em outro contador e como o resultado final os aplica. Não concluir que moedas de combate estão funcionando só pela HUD nem que estão perdidas só pelo teste abortado.

Próximo teste: rastrear a callback original `G2C_KillEnemy` → contador de loot → `G2C_EndGame`, e depois concluir uma fase automatizada somente se a assistência local permitir, conferindo saldo 101 e progresso antes/depois e após reiniciar. A tentativa parcial não limpou nem alterou o save; o perfil atual permanece com as 7 moedas/EXP obtidas por HangUp e volta à home.

## Continuação — run completa até derrota, moeda/XP e revive local v51 (14/10/2026, 11:04–11:10 BRT)

**Build/dispositivo:** mesmo APK v51 e pacote acima. Save preservado: home antes do teste em Capítulo 3/Lv.2, 1507 ouro/7 EXP/0 gemas; não houve `pm clear`, desinstalação ou edição do save. Iniciei a região local 4 (`enemyCount=107`) e deixei o gameplay avançar; escolhi habilidades pelas cartas. Evidências: `build/audit-v51-pre-victory.png`, `build/audit-v51-run-start.png`, `build/audit-v51-run-revive.png`, `build/audit-v51-full-run-logcat.txt`, `build/audit-v51-after-defeat.png`.

- **Gameplay/resultado parcial:** a HUD de combate exibiu ouro subindo até 45; log do bridge confirma kills comuns com `Local kill payout ... gold=5`. A personagem morreu na fase 2/10, usou o botão verde “50% HP Grátis”, retomou com 90 HP e depois aceitou uma segunda utilização (o log confirma duas chamadas a `GameSceneManager.OnResurgenceSucceed` e callbacks de rewarded-ad completadas localmente). Após selecionar outra habilidade, a run chegou à fase 3/10, mas terminou por derrota. A tela final indicou `Terminar 2/11` e “Você não colheu nenhum loot”. Screenshot de revive capturou também um menu Pausar imediatamente após o callback; tocar Continuar retomou a partida, que avançou. **Revive local: passou quanto ao callback e à restauração de HP; retomada sem intervenção adicional é parcial, pois a pausa exigiu Continuar.**
- **Liquidação da derrota:** `build/audit-v51-full-run-logcat.txt` registra `Local EndGame stage=3 ...`, `Local completion inference currentRegion=3 regionCount=10 passed=0` e `fight_reply ... passed=0 ... itemStacks=0`. Após fechar o resultado, home continuou em 1507 ouro/7 EXP, Capítulo 3/Lv.2 e 0 gemas (`build/audit-v51-after-defeat.png`). Force-stop/relaunch do processo (sem limpar dados) confirmou os mesmos valores em `build/audit-v51-after-defeat-restart.png`. Portanto, moeda/XP temporários da run não foram creditados por abandono/derrota; nenhum progresso de fase mudou. Isso é coerente com não liquidar uma run malsucedida e evita moeda gratuita por saída. **Não prova ainda a liquidação após vitória**, que exige uma run `passed=1`.
- O código confirma a fronteira de persistência: `kill_enemy_hook` define o `Gold` da resposta por inimigo e deixa a chamada original do cliente tratar o evento; `end_game_hook` chama `fight_won`, passa `passed` a `persist_local_clear` e só avança região/recompensas de marcos se houve vitória. A v51 não tem no hook um acumulador local de moeda/EXP de combate. A derrota observada não deve ser tratada como falha de moedas; a hipótese relevante segue sendo se o cliente original atualiza o contador 101 pelos callbacks de kill durante uma vitória. Nenhuma correção foi feita neste ciclo por falta de uma vitória reproduzida.
- **Rede residual encontrada ao voltar à home:** o log pós-derrota capturou novamente POST para `mail.kinjarun.com/api/v1/config` e `/mails/list`, ambos falhando após quatro tentativas em ~1,8–2,1 s neste momento (logs Unity indicam HTTP retry; não foi reproduzido o atraso anterior de 43–45 s). Persistem chamadas online de correio. Sem `FATAL EXCEPTION`/ANR no recorte `KinjaOffline`; não classificar a partida como vitória nem o sistema de correio como offline.

Resultado: resposta à dúvida do saldo — sim, a run não deve liquidar moedas/XP quando se abandona ou perde; nesta derrota o saldo ficou intacto apesar de 45 no HUD. A validação de vitória completa e recompensa continua **não concluída**. Próximo passo é reproduzir uma vitória com input de desvio/seleção de habilidades e comparar saldo 101/104 e progresso antes/depois e após reiniciar; não alterar artificialmente save nem classificar o payout de kill como aprovado antes disso.

## Continuação — liquidação de moedas de combate v51–v54 (14/10/2026, 11:41–12:07 BRT)

**Pacote/dispositivo:** Xiaomi 25069PTEBG, Android 16/API 36, ARM64; `com.habby.kinjarun.offline.lab6`, jogo 1.9.1, versionCode 51. Atualizações v52–v54 feitas com `adb install -r`; `firstInstallTime` permaneceu `2026-10-13 19:43:33`, sem `pm clear`, desinstalação ou edição direta do save. APK v54: `build/kinja-run-offline-lab-v54-fight-gold-result-sync.apk`, SHA-256 `CB1CF2C4AAFBE2C6A19FF1062621F3D955083C4A4BB11C891147FBDAB0176FAC`.

### Reprodução da causa e correção

Na run v51 vencida, as respostas de kill somaram 500 ouro, mas o saldo permaneceu igual porque `seed_end_game.json` fornece `Gold=0` e não havia liquidação local. A v52 adicionou um acumulador por run e pagou somente em `passed=1`. A vitória assistida de Capítulo 2 revelou uma corrida de ordem: v52 liquidou 495 no `EndGame`, e a última resposta de kill chegou 5 ms depois, elevando o total para 500. O saldo ficou 5 abaixo (1507→2002) e a tela mostrou 495.

A v53 passou a registrar quanto já foi creditado e, depois que `EndGame` confirma vitória, liquidar somente o delta de qualquer resposta `KillEnemy` tardia. Isso evitou duplicação e levou o saldo a 2502, mas a tela ainda mostrava 495. A v54 retém o objeto original `G2C_EndGame` por GC handle e, se chegar kill após a vitória, chama o setter IL2CPP original `set_Gold` para ajustar também o valor da tela. Uma run v54 seguinte teve todas as 500 moedas acumuladas antes do callback de `EndGame`; portanto validou a recompensa exibida/persistida no caminho normal, enquanto o ramo de atualização tardia da tela ainda depende de uma run que reproduza essa ordem específica.

### Teste de vitória v54

Antes: Capítulo 3/Lv.2, 2502 ouro, 7 EXP, 0 gemas. Selecionei Capítulo 2 já concluído, iniciei a Fase 1/10, escolhi habilidades pela tela e usei uma vez o revive verde gratuito (sem tocar na opção paga). A assistência já existente reduziu dano aplicado em hits não forçados; o revive restaurou 90 HP e abriu o painel de pausa, que foi fechado por Continuar. A run chegou à fase 10/10 e concluiu com `passed=1`, `currentRegion=11`, `regionCount=10`.

- O bridge somou os payouts por inimigo até 500. Nesta ordem de execução, o último kill aconteceu antes do `EndGame`; o log registra `Local run gold settled source=end_game total=500 delta=500`.
- A leitura original `ItemManager.GetOwnItemNum(101)` verificou 3002 após crédito de 500; EXP 104 permaneceu 7 (`exp=0` para a run). O JSON de save foi atualizado nos campos Coin/Exp, preservando os demais.
- A captura `build/audit-v54-run-end.png` exibe **500** moedas no resultado; `build/audit-v54-restart-home.png` exibe **3002 ouro, 7/2000 EXP e 0 gemas** após `am force-stop` e relançamento. A home segue em Capítulo 3/Lv.2.
- Log completo filtrado: `build/audit-v54-run-logcat.txt`; logs do reinício: `build/audit-v54-restart-logcat.txt`. Nenhum `FATAL EXCEPTION` ou ANR apareceu nos logs Unity/KinjaOffline capturados.
- A v53 já reproduziu a ordem atrasada: `EndGame` viu 495 e a resposta seguinte elevou para 500; o código creditou delta 5, sem duplicar o saldo. A tela de resultado v53 permaneceu em 495, motivando a alteração v54. A atualização tardia do objeto de resultado v54 está implementada e compilada, mas ainda **não observada no ramo tardio em execução**.

**Classificação atual:** payout de ouro durante run é temporário; derrota/abandono não paga (confirmado na v51). Vitória completa credita e persiste todos os payouts registrados (confirmado em v53/v54); saldo e tela concordaram na vitória v54 em que os kills chegaram antes de EndGame. Ouro é validado para replay de Capítulo 2 já concluído, `firstWin=0`; bônus de primeira vitória/marco e outras regiões ainda não foram avaliados para recompensas em ouro. EXP de combate continua sem fórmula local comprovada e não é creditado; 7 EXP do HangUp permaneceu inalterado.

A run automatizada v54 selecionou habilidades por toques ADB; quando o revive abriu pausa foi necessário tocar Continuar. Capturas de progresso: `build/audit-v54-run-start.png`, `build/audit-v54-revive1.png`, `build/audit-v54-resume.png`, `build/audit-v54-progress6.png`, `build/audit-v54-progress7.png`; a etapa final e a tela de resultado estão em `build/audit-v54-progress9.png` e `build/audit-v54-run-end.png`.

### Próximos itens da fila

1. Reproduzir, se possível, a ordem `EndGame` antes do último `KillEnemy` já na v54, para verificar alteração da recompensa mostrada no objeto retido. O crédito tardio de saldo já passou em runtime v53.
2. Validar progresso pós-vitória/primeira vitória em uma região ainda não concluída, com atenção para não gastar recursos nem sobrescrever o perfil.
3. EXP de combate segue pendente até localizar uma fórmula/configuração local confiável.
4. Consumo de energia com saldo positivo continua pendente; o save preservado permanece em Power zero. Não fabricar energia no perfil existente.

### Repetição v54 para o ramo de kill tardio (14/10/2026, 12:09–12:17 BRT)

Após a primeira vitória v54 e reinício, o saldo confirmado era 3002 ouro/7 EXP. Repeti a região local 3 (Capítulo 2) sem limpar dados, selecionei habilidades pela tela e completei novamente as fases. `build/audit-v54-run2-logcat.txt` registra `Local EndGame stage=11`, `currentRegion=11 regionCount=10 passed=1`, saldo antes/depois da liquidação 3002→3502, `total=500 delta=500` e resposta `fight_reply ... passed=1`. Neste segundo caso, todas as respostas de kill, inclusive a última soma até 500, foram registradas antes de `EndGame`; por isso `Local EndGame response gold updated after late kill` não aparece e o ramo setter com GC handle segue sem observação direta. A tela voltou à home, cujo saldo `3502` permaneceu após `am force-stop` + relançamento (`build/audit-v54-run2-restart-home.png`). A instalação continuou com o mesmo `firstInstallTime`. Sem `FATAL EXCEPTION` ou ANR no recorte filtrado.

Esta repetição confirma que o resultado e saldo são coerentes quando os callbacks de kill precedem EndGame, e reforça persistência após reiniciar. Para forçar a ordem contrária seria necessário alterar artificialmente a temporização do transporte; isso mudaria o fluxo testado. A v53 já capturou naturalmente EndGame antes do último kill e comprovou crédito tardio do delta 5, mas exibiu resultado 495. O setter de resultado v54 permanece como correção implementada ainda pendente de validação específica.

### Conferência de EXP de combate (14/10/2026)

Busca nas tabelas extraídas do cliente 1.9.1 encontrou `exp_reward` em `RegionAdventureCfg` (expedição) e `hour_exp` em `TimeGainCfg` (HangUp), mas não uma regra de XP por kill/fase de combate. Os seeds `seed_kill_enemy.json` e `seed_end_game.json` trazem `Exp: 0`; nas duas vitórias v54, leitura e persistência mantiveram 7 EXP, enquanto o ouro subiu 500 por run. Portanto, **não há evidência para creditar XP de combate**; isso não prova que o original online nunca concedesse XP, apenas que não existe fórmula local confirmada nesta cópia. Capturas/logs: `build/audit-v54-run2-restart-home.png`, `build/audit-v54-run2-logcat.txt`. O teste foi feito no pacote de laboratório sem limpar os dados.

## Moedas creditadas, mas sem fluxo de gasto local (relato e análise estática, 14/10/2026)

**Relato do usuário na build v54:** o saldo recebido em runs aparece, mas não consegue comprar na Loja nem melhorar talentos. Este bloco registra a hipótese inicial; o teste e a correção subsequentes estão na seção v54–v56 abaixo.

- **Loja de baús:** `TreasureBoxCfg` no cliente 1.9.1 mostra que o baú 101 custa 60 gemas e 0 moedas (`cost_coin=0`, `cost_diamond=60`). Os cards de compra com dinheiro real foram ocultados na v39 e `PurchaseManager.Purchase` bloqueia compras externas. Logo, essa tela não oferece atualmente uma compra com ouro; com 0 gemas, não há opção de baú adquirível. Não reativar pagamento real nem gastar gemas como teste.
- **Talentos por nó:** `TalentCfg` define custo em item 101 (moedas); o primeiro nó, ID 1001, custa 500, e o ID 1002 custa 1000. O projeto entrega `seed_talents.json` à inicialização, porém não tem hook/handler local para `TalentManager.ReqUpgradeTalent`/`C2G_TalentUpgrade` após o clique. É a principal hipótese de por que a ação não completa offline; falta reproduzir clique e obter log para confirmar a cadeia na build instalada.
- **Upgrade geral da árvore:** `TalentCostCfg` possui campos `exp` e `cost_gold`; a interpretação inicial de `exp` como requisito não foi comprovada e não deve ser usada para bloquear a progressão. A compra do nó 1001 funcionou com somente 7 EXP. O botão do livro e seus requisitos continuam sem validação completa.

**Status histórico da análise estática:** a causa foi confirmada e tratada na v56 conforme a seção seguinte.

## Gasto de moedas em talentos e limite atual da Loja (v54–v56, 14/10/2026)

**Reprodução v54, Xiaomi via ADB, package `com.habby.kinjarun.offline.lab6`:** home com 3502 ouro, 7 EXP e 0 gemas (`build/audit-v54-spend-before.png`). Na aba Talento, tocar no primeiro nó “Poder” mostrou custo de 500 moedas; tocar no botão verde não alterou o saldo. O log `build/audit-v54-talent-purchase-logcat.txt` registrou `NullReferenceException` em `ET.TalentHelper.<UpgradeTalent>d__0.MoveNext`, chamado por `TalentManager.ReqUpgradeTalent` e `UITalentPage.OnClickUpgrade`. A state machine procura `CurrentScenesComponent.Session.Call` sem uma sessão offline. O código original só aplicaria o upgrade após `G2C_TalentUpgrade`/`TalentManager.OnTalentDataResp`.

**Correção v56:** `offline/native/offline_bridge.cpp` intercepta `ET.TalentHelper.UpgradeTalent(int)` no RVA `0x28FD814` com prólogo exato da 1.9.1. Custo e item vêm de `TalentCfg.GetCfg`; saldo é lido por `ItemManager.GetOwnItemNum(101)`. A resposta local passa pelo callback original de talentos, e o débito usa setters originais do jogador com leitura de retorno. `Coin` e `TalentLevel` são gravados nos seeds existentes; um journal permite recuperar uma interrupção entre as gravações. A v55 recusou a primeira compra por um teste incorreto de `GetTalentLevelById` como posse do nó; não debitou. Essa guarda foi removida na v56.

**Teste v56:** nó 1001 debitou 500 (3502→3002) e ficou roxo/desbloqueado. Após `am force-stop` e relançar, saldo 3002 e nó comprado persistiram (`build/audit-v56-talent-restart-home.png`, `build/audit-v56-talent-restart-tree.png`). Tocar de novo no mesmo nó mostrou um visto, sem botão de compra nem nova dedução (`build/audit-v56-talent-owned-tap.png`). O nó seguinte “Vitalidade” mostrou HP máximo +20 por 1000 moedas (`build/audit-v56-talent-next-node.png`); comprado, debitou 3002→2002, log `Local talent purchase complete id=1002 ... TalentLevel=1->2` (`build/audit-v56-talent-second-filtered-logcat.txt`). A tela de equipamento tinha ATQ 10/HP 90 antes (`build/audit-equipment2.png`) e ATQ 15/HP 110 depois; após novo relançamento, continuou com 2002 ouro e ATQ 15/HP 110 (`build/audit-v56-talent-second-restart-equipment.png`). Isso confirma dedução, atributo na UI e persistência de duas compras; dano/HP em combate ainda não foram medidos. APK v56: `build/kinja-run-offline-lab-v56-talent-spend-levelcheck.apk`, SHA-256 `36FC1BEAEBCFEF3AA687A8B6CB78196D2DB2A7060055EBB51C9D6C220A865784`. Instalação por `adb install -r` manteve `firstInstallTime=2026-10-13 19:43:33`; `lastUpdateTime=2026-10-14 12:34:38`. Sem limpar dados.

**Loja:** a tela oferece baú de bronze por 60 gemas e de diamante por 250; a seção OURO concede/vende ouro em troca de anúncio/gemas, não compra equipamento com ouro. Na v56, com 0 gemas, tocar no baú de bronze mostrou “Gemas insuficientes” e o saldo permaneceu 2002 (`build/audit-v56-shop-bronze-zero-gems.png`). Abertura/loot de baú com gemas ou chave e uma eventual alternativa offline de compra com ouro ainda não foram implementadas/testadas. Não afirmar que a Loja está funcional só porque abre.

**Falha adicional confirmada:** ao exibir a tela de equipamento após a segunda compra, o log do processo registrou uma `NullReferenceException` em `UIHeroPage.HeroProperty -> UpdatePage -> ShowPage` via `TimerManager.Update`. A mesma exceção ocorreu uma vez no processo relançado ao abrir equipamento; a tela renderizou com os atributos persistidos. A causa precisa ser rastreada antes de alterar heróis, sem atribuí-la à compra. Placeholder `sdgasgdsg` e requisito em chinês permanecem visíveis.

**Pendências por prioridade:** (1) fluxo local de baús/equipamentos ou comunicação clara da indisponibilidade na Loja; (2) NRE de `UIHeroPage.HeroProperty` e textos de herói; (3) efeito dos talentos em combate, journal sob interrupção controlada e custo do botão de upgrade geral; (4) consumo de energia partindo de saldo positivo, indisponível no save atual sem alterar Power; (5) rede residual do correio e SDKs de anúncio. O teste v56 não exercitou compra com gemas, vitória nova ou revive.

## Gemas adicionais por recompensa de combate — v57 (14/10/2026)

### Pré-inspeção sem alterar o save

O ADB encontrou o Xiaomi `25069PTEBG` conectado e o package `com.habby.kinjarun.offline.lab6` instalado. A versão realmente instalada antes do trabalho era `versionCode=51`, `versionName=1.9.1`, `firstInstallTime=2026-10-13 19:43:33`, `lastUpdateTime=2026-10-14 12:34:38`; portanto não correspondia à v56 de referência. A tela inicial, `ItemManager.GetOwnItemNum` e os logs mostraram 2345 ouro (ID 101), 350 EXP (104) e 0 gemas (ID 102). Captura: `build/audit-pre-current.png`. Não foi usado `pm clear`, desinstalação, edição de save nem instalação sobre o APK original.

Seeds de pacote mantêm `Coin=1500`, `Diamond=0`, `Exp=0` como valores de primeira inicialização e não substituem o perfil atual. A metadata 1.9.1 identifica gemas como item 102 e a propriedade original `PlayerDataManager.PlayerDiamond`; `G2C_KillEnemy` e `G2C_EndGame` têm `Diamond`, e `UIFightResult` mantém `_curDiamond`. O APK empacotado continua com `versionCode=51` (BuildId do artefato é independente desse campo). A análise nativa REA foi tentada, mas não havia provider Ghidra/IDA/Hopper configurado; as identificações vieram do dump 1.9.1 e foram confirmadas em runtime pelos métodos originais.

### Implementação

No hook original `FightHelper.KillEnemy`, cada evento mantém seu payout de ouro já existente (comum 5, elite 25, boss 50) e soma também `floor(ouro_do_evento / 2)` gemas: comum 2, elite 12, boss 25. O arredondamento para baixo é feito separadamente por evento, sem carregar fração entre kills. `run_diamond_payout` e `run_diamond_credited` espelham o acumulador/delta já usado para ouro. A saída e a derrota não chamam a liquidação; vitória usa o mesmo `passed` determinado por `FightDataManager` e o mesmo ponto de `EndGame`.

A liquidação combina os deltas de ouro e gemas em uma atualização pelo caminho original: lê IDs 101/102/104, constrói `ObscuredInt` pela conversão IL2CPP original, chama `set_PlayerGold`, `set_PlayerDiamond`, `set_PlayerExp` e `SyncValueToShow` na thread Unity, confere leitura de retorno e altera apenas `Coin`, `Diamond` e `Exp` no JSON já existente. O campo de gemas anterior é lido e conferido antes da gravação, preservando saldo existente e os demais campos. A resposta `G2C_EndGame` recebe `Gold` e `Diamond`; se um kill chegar depois da vitória, o objeto retido por GC handle é atualizado pelos setters originais `set_Gold` e `set_Diamond`, e o crédito tardio é somente o delta ainda não liquidado. Nenhum offset gerenciado foi escrito.

### APK e teste no Xiaomi

- BuildId `v57-combat-gems`; APK `build/kinja-run-offline-lab-v57-combat-gems.apk`; SHA-256 `013F92391D619AD611AC6DE9F9738C75764C7B0F6B907ECAEBC823A24CDCCD59`.
- A instalação usou `adb install -r` no mesmo package/keystore. `firstInstallTime` continuou `2026-10-13 19:43:33`; `lastUpdateTime` passou a `2026-10-14 12:50:37`. Nenhum dado foi limpo.
- Antes das vitórias: 2345 ouro / 0 gemas / 350 EXP. Duas runs concluídas no Capítulo 2 chegaram a `EndGame stage=11`, `currentRegion=11`, `regionCount=10`, `passed=1`. Cada run creditou 500 ouro e 211 gemas. Após a primeira: 2845/211; após a segunda: 3345/422; EXP ficou 350. Os logs mostram leitura de retorno `gold101=3345 diamonds102=422 exp104=350` e gravação `Coin=3345 Diamond=422 Exp=350`.
- A captura `build/audit-v57-result-ui.png` mostra a resposta de resultado com 500 ouro e 211 gemas. A home pós-vitória e pós-`am force-stop`/relançamento em `build/audit-v57-restart-home.png` mostra 3345 ouro e 422 gemas. O log filtrado está em `build/audit-v57-run2-logcat.txt`; a primeira liquidação e resposta estão em `build/audit-v57-run-summary-logcat.txt`.
- Para validar abandono, iniciei outra run, observei um kill temporário (5 ouro/2 gemas no acumulador) e usei `am force-stop` antes de `EndGame`. Após relançar, o saldo continuou 2845 ouro/211 gemas, sem liquidação. Evidências de tela: `build/audit-v57-abort-running.png` e `build/audit-v57-abort-restart.png`. O cenário exercitado foi saída/encerramento antes de concluir; uma derrota por HP não foi repetida nesta build.
- Nenhum `FATAL EXCEPTION` ou ANR apareceu nos recortes filtrados do fluxo de combate. O save privado não foi extraído diretamente; persistência foi verificada pelo readback IL2CPP e pelo saldo de UI após reinício.

**Limites restantes:** as duas vitórias tiveram todas as respostas de kill antes de `EndGame`; portanto o ramo de callback tardio de gemas e a atualização tardia dos dois valores na tela continuam implementados, mas sem observação direta na v57. O teste v53 anterior observou somente delta tardio de ouro 495+5, não gemas. O próximo item é obter naturalmente uma sequência `EndGame` antes do último `KillEnemy` e conferir UI/saldo, sem alterar temporização do transporte. O pacote está instalado com 3345 ouro, 422 gemas e 350 EXP.

## Reprodução da Loja com gemas — baseline histórico v57/v58, 14/10/2026

**Nota:** esta seção registra somente o baseline anterior. O estado de “nenhum hook de compra” abaixo foi supersedido pela implementação v63/v65 documentada ao final deste arquivo.

### Estado e reprodução por ADB

Antes de qualquer alteração, confirmei no Xiaomi `25069PTEBG` o package `com.habby.kinjarun.offline.lab6`, `versionCode=51`, `versionName=1.9.1`, `firstInstallTime=2026-10-13 19:43:33` e `lastUpdateTime=2026-10-14 12:50:37`. O APK local da referência continua com SHA-256 `013F92391D619AD611AC6DE9F9738C75764C7B0F6B907ECAEBC823A24CDCCD59`. A tela da Loja mostra 422 gemas e 1845 ouro (o saldo de ouro da referência já havia mudado; não foi presumido nem restaurado). Captura: `build/audit-v58-shop-before.png`.

A Loja oferece baú de bronze por 60 gemas e baú de diamante por 250. A seção OURO exibe a oferta de 4000 por anúncio e trocas de 100 gemas por 20000 ouro e 500 gemas por 118888 ouro. Os cartões pagos em dinheiro real não aparecem. A tabela `TreasureBoxCfgCfgData` do cliente 1.9.1 confirma ID 101/60 gemas e ID 115/250 gemas; o bronze tem pools de recompensa normal `10000_1`, `1011_10`, `1012_30`, `1013_999`, e o diamante `10001_1`, `1021_30`, `1022_100`, `1023_999`, além do pool de equipamentos configurado por `TBContentCfg`.

Toquei os botões de compra por ADB sem provocar alteração aparente: os dois saldos e a tela permaneceram iguais. A repetição do botão bronze foi capturada em `build/audit-v58-shop-bronze-tap2.png`; o recorte de logs está em `build/audit-v58-shop-bronze-tap2-logcat.txt`. A captura e os logs anteriores estão em `build/audit-v58-shop-buytap.png`, `build/audit-v58-shop-diamond-tap-logcat.txt` e `build/audit-v58-shop-info-tap.png`. O toque no ícone de informação abriu as regras do baú, confirmando que a página recebeu input ADB. Não houve compra nem consumo de gemas.

### Trace e estado de implementação

Metadata e dump do APK 1.9.1 confirmam `ShopManager.BuyTreasureChest(int,bool,Transform)` em RVA `0x26D8AD0`, `OpenBoxResp(int,bool)` em `0x26D8BF4`, `OnBuyTreasureChestSucceed(G2C_BuyChestMsg)` em `0x26D7E58`, seguido por `AnalysisChest` e `DoOpenChest`. `OpenBoxResp` constrói `S2C_Buy_Treasure_Chest_Succeed_Data`; o callback moderno recebe `G2C_BuyChestMsg` com `Way`, `ChestId`, `ContentIds`, `Cost`, `ExtraReward` e `ServerPackItem`. A implementação instalada ainda não intercepta `BuyTreasureChest` nem cria/adapta essa resposta: por isso o toque não chega a uma concessão local observável.

Não foi debitado preço nem concedido conteúdo. O saldo de 422 gemas permitiria testar o bronze ou o diamante sem fabricar recursos, mas uma rota parcial que cobre sem conferir concessão e persistência seria insegura. O caso insuficiente também não foi testado nesta etapa. Não foi compilado nem instalado novo APK; não há BuildId ou hash desta etapa.

**Pendência bloqueadora:** implementar a compra local integrada ao callback original e demonstrar que cada recompensa de `TBContentCfg` chega ao inventário/equipamento e aos seeds existentes, com débito único e recuperação contra interrupção/reentrada. Até isso ocorrer, compras com gemas, compras de ouro por anúncio e conversões de gemas em ouro não devem ser descritas como funcionais offline. O teste de insuficiência deve ocorrer sem editar saves e sem gastar gemas em excesso.

## Loja local de baús — compras v63 e bloqueios v65 (14/10/2026)

### Estado inicial, reprodução e resultados

O aparelho é o Xiaomi `25069PTEBG` (Android 16/API 36, ARM64), package `com.habby.kinjarun.offline.lab6`, jogo 1.9.1. Antes dos gastos, conferi a v63 instalada (`versionCode=51`, `firstInstallTime=2026-10-13 19:43:33`); a Loja mostrava 1845 ouro/422 gemas. Não houve `pm clear`, desinstalação nem alteração manual do save. A referência v57 continuou intacta em `build/kinja-run-offline-lab-v57-combat-gems.apk` (SHA-256 `013F92391D619AD611AC6DE9F9738C75764C7B0F6B907ECAEBC823A24CDCCD59`).

Na v63, comprei uma vez cada baú usando botões reais por ADB: bronze ID 101 (60 gemas) e diamante ID 115 (250). A captura bronze `build/audit-v63-bronze-after.png` mostra a abertura e “Normal — Calças de Couro”; log `build/audit-v63-bronze-logcat.txt` registra `ContentIds=[1016]`, saldo 422→362 e sucesso de `ShopManager.OnBuyTreasureChestSucceed`, `ItemManager.OnItemInit` e `EquipManager.OnEquipDataInitResp`. A captura `build/audit-v63-diamond-after.png` mostra o item “Superior — Colar de Cura Santa”; log `build/audit-v63-diamond-logcat.txt` registra `ContentIds=[1182]`, saldo 362→112 e o callback original. A confirmação de recebimento e o retorno à Loja foram capturados em `build/audit-v63-shop-after-diamond.png` (112 gemas/1845 ouro).

Após reinício do processo, `build/audit-v63-restart.png` mostra 362 gemas/1845 ouro depois do primeiro baú; os callbacks de inicialização carregaram `seed_player.json`, `seed_items.json` e `seed_equipment.json`. Depois do segundo, reinício v65 carregou os mesmos seeds e a home mostrou 112 gemas/1845 ouro (`build/audit-v65-restart.png`). O log final de inicialização `build/audit-v65-final-restart-logcat.txt` confirma `ItemManager.OnItemInit` e `EquipManager.OnEquipDataInitResp`; captura mais recente: `build/audit-v65-final-restart-home.png`. Naveguei à tela de equipamento e a bolsa mostra os dois itens do teste — Colar de Cura Santa e Calças de Couro — em `build/audit-v65-equipment-after-purchases.png`. A tela também registrou NRE em `UIHeroPage.HeroProperty → UpdatePage`; os itens continuam visíveis, mas a falha separada do painel de herói não foi corrigida nesta etapa.

### Implementação e configuração preservada

O hook v63 de `ShopManager.BuyTreasureChest(int,bool,Transform)` usa os 16 bytes de prólogo conhecidos do APK 1.9.1, conferidos também por `patch_entry`; callbacks e getters/setters são chamados na thread Unity. A compra só aceita IDs 101 e 115, recusa a variante `free`, lê `cost_diamond` e saldo live via `ItemManager.GetOwnItemNum(102)`, verifica que o saldo salvo coincide, seleciona ponderadamente `TBContentCfg.base_weight`, monta `G2C_BuyChestMsg` (`Way=4`, `ChestId`, `Cost`, `ContentIds`) e entrega pelo `ShopManager.OnBuyTreasureChestSucceed` original. O callback de resultado faz a apresentação original. Itens simples são incorporados em `seed_items.json`; equipamentos recebem UID novo e campos `EquipId`, `Uid`, `Level`, `Quality`, `Part`, `IsWear` em `seed_equipment.json`. Os gerentes são atualizados pelos callbacks originais antes de concluir a compra. Falha conhecida de gravação/callback tenta restaurar os arquivos e estornar o preço.

As duas opções e pools da tabela client-side 1.9.1 foram preservadas: ID 101 custa 60 gemas, pool de equipamentos ID de conteúdo 1001–1036/`box_id=1011`; ID 115 custa 250 gemas, pool 1175–1240/`box_id=1021`. `item_nums` dirige o número de sorteios e `base_weight` o peso. Os textos de pool `normal_reward` observados na tabela são `10000_1,1011_10,1012_30,1013_999` e `10001_1,1021_30,1022_100,1023_999`; nos dois testes, o callback/UI concedeu um equipamento de `TBContentCfg`, sem recompensa adicional desses arrays. Não inventar recompensas extras sem confirmar a semântica desses campos no cliente. A frase “equipamento épico em 10 vezes” continua sendo a garantia mostrada pela UI; a configuração de quantidade de cada compra foi observada/testada como uma peça por baú.

Hook `chest_purchase_in_progress` recusa concorrência/reentrada durante o commit. Não há callback remoto tardio nessa rota local. Compras deliberadas em toques sequenciais são compras separadas; debounce de double-tap e recuperação transacional após encerramento do processo entre gravações ainda não foram implementados/testados. Essa é uma limitação: o rollback cobre falha detectada no mesmo processo, não garante atomicidade conjunta de três arquivos perante encerramento abrupto.

### Online, anúncios e saldo insuficiente

Na v65, `PurchaseManager.Purchase(int)` continua retornando sem iniciar compra externa. Hooks com prólogos conferidos bloqueiam `UITreasureBoxPage.OnClickAdCoin` (`0x13E0F80`), `OnClickBuyLotCoin` (`0x13E145C`) e `OnClickBuyHugeCoin` (`0x13E1730`). Em ADB, tocar anúncio de ouro e os dois cartões de conversão registrou `Blocked Shop...`; nenhum anúncio/callback foi aberto e 112 gemas/1845 ouro permaneceram iguais. Capturas: `build/audit-v65-shop-ad-block.png`, `build/audit-v65-shop-exchange-block.png`; logs respectivos `build/audit-v65-shop-ad-block-logcat.txt` e `build/audit-v65-shop-exchange-block-logcat.txt`.

Limite visual: cartões de anúncio e troca ainda aparecem porque `Transform.Find` não encontrou os nós inferiores candidatos do prefab; os handlers de ação, contudo, retornam antes de qualquer pedido ou recompensa. Os seis cartões de dinheiro real continuam visualmente ocultos pelo hook já existente. A tentativa de saldo insuficiente foi feita com 112 gemas frente ao preço de 250; o app preservou o saldo, mas a área da Loja ficou visualmente vazia e o hook/log de compra não foi observado. Portanto **a mensagem original de insuficiência e o caminho de clique insuficiente não estão validados**; não foi gasto mais saldo para tentar novamente. As capturas `build/audit-v63-insufficient-result.png` e `build/audit-v65-insufficient.png` mostram o estado sem débito, sem prova do aviso.

### Builds entregues

A v65 implementou/testou os bloqueios da Loja e foi instalada por `adb install -r`. A v66 adiciona debounce de 650 ms para toque repetido do mesmo baú e é a versão final instalada, também por `adb install -r` no mesmo package/keystore; `firstInstallTime` manteve `2026-10-13 19:43:33`. APK final `build/kinja-run-offline-lab-v66-local-gem-chests-debounce.apk`, SHA-256 `5038CB596BD3043E208F17DDB0EA7A86CF0923F988E6AC6BF98628C77E7B0902`. Captura final de home `build/audit-v66-final-home.png`; `build/audit-v66-final-logcat.txt` confirma instalação do hook e carregamento dos seeds/callbacks, saldo 112 gemas/1845 ouro. A janela de debounce foi compilada/instalada, mas não exercitada com compra dupla para evitar gastar outra peça de saldo. Não houve `FATAL EXCEPTION` durante abertura/reinício; a navegação posterior à tela de equipamento gerou o NRE do `UIHeroPage.HeroProperty` descrito acima. Não sobregravou `build/kinja-run-offline-lab-v57-combat-gems.apk` nem limpou os dados.

**Próximo item:** localizar/ocultar o grupo visual OURO no prefab, adicionar journal de recuperação de compra e identificar por que a página desaparece na tentativa de baú insuficiente; então validar o aviso/saldo sem consumir recursos adicionais. A proteção de debounce está instalada, mas falta exercitá-la. A rota principal dos dois baús, exibição e presença dos itens na bolsa, atualização dos managers e persistência após reinício passaram em runtime.

## Resgate diário da Loja, equipar item e drops de combate — v67/v68 (14/10/2026)

### Estado inicial e resultado do resgate grátis

Antes do teste, ADB confirmou Xiaomi `25069PTEBG`, package `com.habby.kinjarun.offline.lab6`, jogo 1.9.1, `firstInstallTime=2026-10-13 19:43:33`, build instalada v67 (`lastUpdateTime=2026-10-14 14:35:10`). A Loja mostrava 52 gemas e 1845 ouro; opções visíveis: bronze 60 gemas, diamante 250, resgate grátis de 4000 ouro e trocas de 20000/100 gemas e 118888/500 gemas. Os seis cartões de dinheiro real estavam ocultos. Captura antes: `build/kinja-v67-shop-before.png`.

Toque ADB único no cartão 4000 executou o resgate sem abrir anúncio nem gastar gemas. O saldo passou de 1845 para 5845 ouro; gemas continuaram em 52. O log confirma leitura/escrita pelos métodos originais, persistência `Coin=5845 Diamond=52 Exp=350`, journal recuperável e `Local Shop daily claim granted gold=4000 date=20261014`. Toques subsequentes no mesmo dia registraram `already used` e não duplicaram o prêmio. A captura `build/kinja-v67-shop-claimed.png` mostra saldo e `0 Reivindicações Diárias Restantes`. Após reinício do processo, `build/kinja-v67-restart.png` mostra 5845 ouro/52 gemas e 350 EXP. O limite reseta por data local do aparelho; mudança manual do relógio não foi testada.

### Equipamento

Na v67, abri Equipamento, selecionei `Colar de Cura Santa` (parte 3, UID 2) e toquei Vestir. O hook 1.9.1 `EquipHelper.WearEquip` atualizou `IsWear` no `seed_equipment.json`, desserializou `ET.G2C_EquipData` e chamou `EquipManager.OnEquipDataInitResp` original; log: `Local equipment wear persisted part=3 uid=2`, seguido de `ETTask<bool>` original concluída. Depois de reiniciar, o bootstrap voltou a carregar `seed_equipment.json` pelo callback original. A captura visual ainda faltava nesta rodada histórica; a captura pós-restart v69 abaixo confirma depois o colar vestido.

Uma exceção já conhecida continuou reproduzível ao abrir a página de equipamento: `NullReferenceException` em `UIHeroPage.HeroProperty -> UpdatePage -> ShowPage`; slots e popup ainda renderizam. Não foi mascarada nem atribuída ao novo handler. Capturas do fluxo: `build/kinja-v67-equipment2.png`, `build/kinja-v67-equip-item.png`, `build/kinja-v67-equipped.png`.

### Livros de treino e resultado de combate

O hook v67 já consulta na thread Unity `Monster.GetCfg(enemyId).reward_id` e chama `RewardManager.RandomRewardPack(rewardId,false)`, enfileirando `ItemId/ItemNum` para persistir apenas em `EndGame` vencedor. A tabela 1.9.1 confirma inimigos do Capítulo 3 usando reward ID 100030, cuja lista é `20001_48_1` até `20006_48_1` (seis livros, cada entrada com peso 48) e `300101_30_1`; o chefe usa 100031, com livros 20001–20006 de peso 100. As rolagens comuns preservam chance baixa; derrota/abandono não credita a fila.

Foi iniciada uma run Capítulo 3/Região 4 no aparelho. Os logs mostram `Monster.GetCfg` e `RewardManager.RandomRewardPack` em inimigos 10301–10305; a run chegou à fase 6 e terminou em derrota. A tela exibiu `Você não colheu nenhum loot`; o log final foi `passed=0 ... itemStacks=0`, sem crédito ao perfil. Nessa build, o logger só imprimia rolagens com ItemId/ItemNum positivos, portanto esse resultado não distinguia rolagem vazia por chance de erro na leitura do retorno. **Limitação histórica, resolvida para entrega positiva na v69:** v68 registrou as rolagens vazias e v69 validou livros no resultado, inventário e save após vitória, conforme seção abaixo.

A mesma run testou Reviver grátis duas vezes. Cada chamada registrou `FightHelper.Resurgence replaced with local response`, `GameSceneManager.OnResurgenceSucceed local revive` e callback `Rewarded ad replaced by immediate local completion`; não abriu anúncio nem gastou as 52 gemas. O desfecho da run continuou derrota. Capturas: `build/kinja-v67-run.png`, `build/kinja-v67-run2.png`, `build/kinja-v67-run3.png`, `build/kinja-v67-run4.png` e `build/kinja-v67-revive2.png`.

### Saldo insuficiente e Loja

Na v68 havia 52 gemas diante do preço de 60 do bronze. A tela da Loja (`build/kinja-v68-shop.png`) comprova ambos os valores. Toques automatizados nas coordenadas ADB testadas não produziram log `Local chest purchase insufficient gems`, não mostraram aviso verificável e a UI fechou/voltou sem alteração de saldo (`build/kinja-v68-insuff-final2.png`). Portanto não foi possível validar o caminho insuficiente nem seu texto; não foram gastas gemas. A proteção de saldo implementada chama o branch original para o aviso quando `BuyTreasureChest` é acionado, porém o despacho do clique permanece sem evidência nesta tentativa.

### Build v68 e integridade

Alteração v68: somente diagnóstico de rolagens vazias de recompensa, imprimindo `enemy`, `rewardId`, `item` e `count`; não muda chance nem concede conteúdo. BuildId `v68-drop-roll-diagnostics`, APK `build/kinja-run-offline-lab-v68-drop-roll-diagnostics.apk`, SHA-256 `F1223092D128938DD08135F288B24DC5F509016833D7DBF25F1356C1AF1F6DEE`. `adb install -r` retornou `Success`; `firstInstallTime` permaneceu `2026-10-13 19:43:33`, `lastUpdateTime=2026-10-14 14:47:36`. Startup recarregou os seeds e manteve 5845 ouro/52 gemas/350 EXP. O APK original e o APK v57 de referência não foram sobrescritos; save não foi limpo/editado. Build compilou com um warning preexistente de parâmetro `equipment_rewards` sem uso.

**Próximo item (estado histórico da v68; concluído em parte pela v69):** a v69 validou a vitória com livros persistidos e o equipamento vestido após restart. Permanecem pendentes o rastreamento do NRE `UIHeroPage.HeroProperty` e uma validação segura do aviso de saldo insuficiente; detalhes e limites atuais estão na seção v69 abaixo.


## Validação da proteção de livros em combate — v69 (14/10/2026)

A investigação v68 confirmou por runtime que o sorteador original estava funcionando: `RewardManager.RandomRewardPack` retornava `ItemId=0, ItemNum=1` para inimigos comuns. As tabelas 1.9.1 mostram o motivo: reward 100020 tem peso 8700 para sem item, 45 para cada livro 20001–20006 e 30 para item 300101 (total 9000; 0,5% para cada livro). O Capítulo 3 usa 8682/48/30 (total 9000; cerca de 0,53% por livro). Os bosses 100021/100031 usam somente seis livros de peso 100 cada. O sistema anterior preservava a tabela, mas essa raridade fazia livros quase nunca aparecerem em runs curtas.

### Mudança local e teste

A v69 mantém `RandomRewardPack` e todos os IDs/counts originais. Em reward IDs de combate de capítulo (`100020` a `100990`, terminados em zero), quando dez rolagens vazias ocorrerem na mesma run, enfileira um livro ID 20001–20006, quantidade 1, em rotação definida pelo ID da instância inimiga. Uma rolagem natural de livro zera o contador. A fila continua protegida por instância e só é aplicada por `EndGame` quando `passed=1`; derrota, abandono e callbacks duplicados não liquidam os livros. Não se mexe em offsets gerenciados; setters/callbacks e saves originais continuam sendo usados na Unity thread. É uma proteção de azar deliberada para progressão offline e está documentada como regra própria do bridge.

No Xiaomi, uma run de Capítulo 2/Região 3 chegou à fase 10/10 e venceu (`EndGame stage=11`, `passed=1`). O log registra sete gatilhos de proteção ao longo da run (IDs enfileirados 20003, 20001 e 20005), além das rolagens naturais. O callback de recompensa persistiu os livros: `ItemId=20003 Count=4`, `20001 Count=2`, `20002 Count=1`, `20005 Count=2`, `20004 Count=1`; também preservou o equipamento 300101. A tela de resultado `build/kinja-v69-result.png` mostra 500 ouro, 211 gemas e sete pilhas de itens. Após reinício, a home `build/kinja-v69-home-restart.png` mostra 6345 ouro/263 gemas/350 EXP; o popup na `build/kinja-v69-bag.png` mostra o Colar de Cura Santa equipado e “Livro de Forja de C” com 4 unidades. O log completo filtrado está em `build/kinja-v69-test-logcat.txt`.

A mesma captura pós-restart prova a persistência visual do equipamento que já havia sido vestido em v67. O NRE `UIHeroPage.HeroProperty -> UpdatePage -> ShowPage` continuou ocorrendo ao entrar em Equipamento, embora os slots, os livros da bolsa e o estado vestido renderizem. Essa exceção segue pendente.

### Build instalada e limites

BuildId `v69-combat-book-pity`, APK `build/kinja-run-offline-lab-v69-combat-book-pity.apk`, SHA-256 `AAD25CDB533C63DDF8E0E08E800B6481FB085A4910A1E35417516C207B40019F`. ADB instalou via `adb install -r` no mesmo package e keystore; `firstInstallTime` permaneceu `2026-10-13 19:43:33`, sem limpar dados. A recarga do save preservou os saldos e os itens. Build compilou com o warning preexistente de `equipment_rewards` sem uso.

O toque de saldo insuficiente ainda não foi validado: em v68 havia 52 gemas para preço de 60, mas os toques automatizados não produziram log de `BuyTreasureChest` nem aviso. Não foram gastas gemas. Para testar a insuficiência novamente será necessário que o saldo fique naturalmente abaixo do preço. O aviso original não deve ser chamado de validado até o handler ser atingido.

**Próximo item:** corrigir/rastrear o NRE da página de equipamento, reproduzir o aviso de saldo insuficiente quando houver saldo inferior sem alterar o save, e continuar validando outros fluxos offline. Livros de combate, vitória, UI de resultado, inventário e persistência passaram nesta v69.


## Investigação pré-correção — build v69 no Xiaomi (14/10/2026, 16:20–16:39 BRT)

**Baseline instalado, confirmado via ADB:** package `com.habby.kinjarun.offline.lab6`, jogo 1.9.1/versionCode 51, atualização `2026-10-14 14:59:55`; APK extraído do package tem SHA-256 `AAD25CDB533C63DDF8E0E08E800B6481FB085A4910A1E35417516C207B40019F`, igual ao artefato v69 `kinja-run-offline-lab-v69-combat-book-pity.apk`. Não houve limpeza, desinstalação, alteração manual nem concessão de save.

### Estado do save visível antes dos testes

Na home, o jogo mostrou Lv.2, Capítulo 3, 1845 ouro, 13 gemas e 350/2000 EXP. A tela de Talentos mostrou 2 cristais azuis (item 40001). No Anel do Oráculo selecionado, nível 1/20, ATK 15 e Livro de Forja de A `1/1`; o próximo botão Melhorar exibia 100 ouro. Capturas: `build/kinja-home-before-tests.png`, `build/kinja-talents-before-test.png`, `build/kinja-onekey-before-authorized.png`.

### Evidência de gameplay no aparelho

- **Melhorar:** um toque no botão exibido como disponível (100 ouro, 1/1 livro) deixou nível, livro e ouro inalterados. A tela continuou no mesmo nível. Log `build/kinja-equip-after-single-upgrade-logcat.txt`: exceção na thread Unity em `ET.EquipHelper.<UpgradeEquip>d__2.MoveNext`, propagada por `ET.EquipHelper.UpgradeEquip(int)` e `EquipManager.<UpgradeEquip>d__91.MoveNext`. Capturas antes/depois em `kinja-equip-before-upgrade.png` e `kinja-equip-after-single-upgrade.png`.
- **Reforço de um clique:** teste de um toque autorizado pelo usuário. `UIEquipTips` chama `EquipManager.UpgradeEquip(uid, true)`, que segue para o helper separado `ET.EquipHelper.OneKeyUpgradeEquip(int)`; o log `build/kinja-onekey-after-authorized-validtap-logcat.txt` contém `NullReferenceException` em `<OneKeyUpgradeEquip>d__3.MoveNext` na thread Unity. O objeto indicado continua Lv.1/20, o livro continua 1/1 e ouro 1845; não houve consumo parcial. Isso confirma que o botão age sobre o UID selecionado, não melhora todos os itens. Capturas anterior e posterior: `kinja-onekey-before-authorized.png`, `kinja-onekey-after-authorized-validtap.png`. Uma primeira coordenada não atingiu o botão e não teve efeito; a segunda tentativa foi no centro visível e gerou a exceção acima.
- **Correio:** o inicializador tenta abrir `https://mail.kinjarun.com/api/v1/config` e `/api/v1/mails/list`; ambas as rotas falharam com `SocketError=11001` após quatro tentativas. O log completo do relançamento sem limpeza está em `build/kinja-mail-startup-logcat-full.txt`; linhas filtradas em `kinja-mail-startup-logcat.txt`. Não há seed local de mail em `offline/assets/seed_*.json` nem entrega/callback de mail no bridge. Capturas: `kinja-hamburger-menu-before.png`, `kinja-mail-repro-20261014.png`. A captura do relançamento após as tentativas manteve 1845/13, Lv.2, Capítulo 3 e EXP 350, confirmando persistência após reinício do processo: `kinja-home-after-investigation-relaunch.png`.

### Análise estática separada

No dump 1.9.1, `UIEquipTips.OnClickTipsUpgrade/OnClickTipsOneKeyUpgrade` (RVAs `0x15B0ECC/0x15B0FA0`) são handlers distintos; ambos chegam a `EquipManager.UpgradeEquip(uid, bool oneKey)` (`0x2C39450`), que despacha `EquipHelper.UpgradeEquip` (`0x2C2D20C`) ou `OneKeyUpgradeEquip` (`0x2C2D300`). Os prólogos são específicos do binário 1.9.1 e ainda precisam de hooks locais com verificação de 16 bytes antes da instalação. A configuração de equipamento contém custo global por nível (`EquipUpgradeCfg`: nível 1 custa 100 ouro e 1 livro). A exceção observada, e não apenas a falta de recursos, é a causa imediata confirmada nos dois caminhos.

Os talentos Escolhido Divino (2001) e Mão Divina (2002) usam item 40001; custam 1 e 3, e exigem níveis 1 e 3, respectivamente. As tabelas configuram cristais em recompensas de progresso de capítulos (incluindo 2 no marco inicial do Capítulo 1 e 1 no do Capítulo 3) e em algumas atividades rankeadas. O bridge implementa a concessão local apenas do marco do Capítulo 1 (já resgatado no estado atual; 2 cristais exibidos). A disponibilidade offline de atividade rankeada não está provada; nenhuma moeda foi concedida durante esta investigação.

Na configuração `FunctionCfg`, Atividade ID104 requer nível 8 e Capítulo 7; HangUp ID116 requer passar o Capítulo 2; ID125 também é gated por nível 5. O bridge registra a lista local `[101,102,103,116]` para o save atual (level 2, capítulos 1 e 2 passados). A ausência de botões de evento gated no perfil atual não prova falha de hook nem justifica liberá-los prematuramente. Funções de compra, cartões, guilda/rank e eventos que usam backend/anúncio/pagamento não têm fluxo offline demonstrado.

**Ainda sem correção/build:** esta seção registra apenas a reprodução e análise da v69. Não representa validação de implementação. O próximo relatório acrescentará o BuildId v70 ainda não usado, hash, alterações, instalação por `adb install -r`, validação visual e de save e limitações.

## Resultado v72 — transação de equipamento, correio e layout (14/10/2026)

### Identidade, instalação e save

A v69 instalada foi confirmada pelo hash do APK extraído via ADB: `AAD25CDB533C63DDF8E0E08E800B6481FB085A4910A1E35417516C207B40019F`. Antes dos testes, a UI mostrava Lv.2, Capítulo 3, 1845 ouro, 13 gemas, 350/2000 EXP, 2 cristais azuis 40001; Anel do Oráculo UID 4, Lv.1/20, ATQ 15 e Livro de Forja de A 1/1. Captura: `build/kinja-v70-preinstall.png`.

A tentativa v70 `v70-local-equipment-transaction` compilou e foi instalada por `adb install -r`, mas a inicialização terminou em SIGSEGV na thread Unity durante a varredura recursiva da hierarquia para centralizar texto. Evidência preservada em `build/kinja-v70-crash-logcat.txt` e `build/kinja-v70-crash-detail.txt`. O save não foi limpo nem alterado manualmente. A varredura foi removida.

A v71 `v71-local-equipment-transaction` inicializou sem crash e passou o primeiro teste de melhoria. A v72 `v72-local-equipment-transaction` foi então instalada com `adb install -r`, no mesmo package `com.habby.kinjarun.offline.lab6` e keystore de laboratório. `versionCode=51`, `versionName=1.9.1`; `firstInstallTime=2026-10-13 19:43:33` continuou igual, `lastUpdateTime=2026-10-14 16:54:37`. O APK final está em `build/kinja-run-offline-lab-v72-local-equipment-transaction.apk`, SHA-256 `CC722CC4D664EF93245D778960E4CF0FECA1F935A1F48B42C41FBAC114E80E42`. A v57 e o APK original não foram sobrescritos.

A home da v71 já mostrava HangUp local acumulado em segundo plano: 2565 ouro/13 gemas/1070 EXP. Esse acréscimo de 720 ouro e 720 EXP ocorreu pelo tempo local entre inicializações; não é crédito de teste nem moeda fabricada. A melhoria autorizada debitou 100 ouro e um livro, resultando em 2465 ouro/13 gemas/1070 EXP. Após forçar apenas o encerramento do processo e iniciar a v72, esses saldos permaneceram iguais. Não foram tocados cristais azuis.

### Análise estática do APK exato e bridge

O APK 1.9.1 usado como alvo continua protegido pelo hash `7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e`. `UIEquipTips.OnClickTipsUpgrade` e `OnClickTipsOneKeyUpgrade` são handlers distintos e chamam `EquipManager.UpgradeEquip(uid, false/true)`; o segundo recebe o UID selecionado, portanto não é melhoria de todo o inventário. Helpers `ET.EquipHelper.UpgradeEquip(int)` e `OneKeyUpgradeEquip(int)` têm RVA/prólogo verificados para esse binário e falhavam em runtime quando tentavam continuar a sessão online ausente.

O bridge v71/v72 intercepta somente esses dois helpers após validar o prólogo de 16 bytes. No Unity thread, compara o EquipIns retornado por `EquipManager.GetEquipInfoByUid` com o registro existente no save; obtém limite, custo por nível (`EquipUpgradeCfg.GetCfg`) e material (`GetEquipCfgByIdAndLvAndQuality`) pelas APIs/configuração originais; verifica live/save para ouro e material; então grava snapshots e marcador de transação, atualiza os seeds existentes, debita ouro pelos setters originais, atualiza inventário/equipamento por `ItemManager.OnItemInit` e `EquipManager.OnEquipDataUpdateResp`, e confere o readback. Reentrada durante a transação retorna falha. O marcador restaura os três arquivos anteriores se o processo for interrompido antes do commit.

Para `oneKey`, a configuração que o bridge prepara é a melhoria sequencial do UID selecionado até o limite ou até faltar recurso; material misto não é debitado parcialmente. **O sucesso desse modo ainda não foi validado na v71/v72:** o teste inicial pré-correção reproduziu a NRE sem consumo; depois de gastar o único Livro A em uma melhoria unitária, não havia recurso para uma nova validação positiva de oneKey. Não se deve afirmar que a semântica até o limite está confirmada pelo gameplay; o próximo teste seguro exige livro obtido naturalmente. O toque de saldo insuficiente em oneKey também não produziu log de entrada confiável e segue não validado.

### Gameplay e persistência observados

Em v71, a tela confirmou o Anel do Oráculo UID 4 de Lv.1/ATQ15 para Lv.2/ATQ19 (+4), Livro A de 1/1 para 0/1 e ouro de 2565 para 2465. `build/kinja-v71-after-upgrade.png` mostra o resultado; `build/kinja-v71-after-upgrade-logcat.txt` registra leitura da configuração, os dois callbacks originais, readback e `Local equipment upgrade complete uid=4 oneKey=0 level=1->2 gold=100 books item=20004 count=1`. Depois do reinício, `build/kinja-v72-ring-after-restart.png` mostra Lv.2/ATQ19 e 2465 ouro/13 gemas; a UI mostra 0/1 Livro A. Essa melhoria unitária foi validada em gameplay e persistência.

### Correio, capítulo e funções laterais

O Correio não tem seed local nem callback de entrega no bridge. No arranque anterior foram registradas quatro tentativas falhas de `/api/v1/config` e `/api/v1/mails/list` no domínio `mail.kinjarun.com`, com `SocketError=11001`; veja `build/kinja-mail-startup-logcat-full.txt`. Em v71/v72, `UIMainPanel.DoOpen` percorre a hierarquia e desativa apenas `btnMail` e nós de aviso/red-point relacionados, usando `GameObject.SetActive`; a captura `build/kinja-v71-hamburger-menu.png` mostra Configurações sem Correio e o log v72 registra `Hidden backend-only mail UI node in UIMainPanel`. As requisições do inicializador de SDK continuam ocorrendo em background. Nenhuma mensagem/recompensa local configurada foi encontrada.

Para o capítulo, a rota estática identificada é `Content/Page/Main/Bottom/LevelHandle/Unclear/btnUnclearFight/txtRegionNameInBtn`. O bridge v72 usa `Transform.Find`, obtém `UILabel` por `GetComponent`, e chama `UIWidget.set_pivot(Center)`/`UILabel.set_alignment(Center)` depois do `DoOpen` original. Log em `build/kinja-v72-startup-logcat.txt` confirma os dois setters na thread Unity. `build/kinja-v72-home.png` é a evidência visual; o rótulo fica horizontalmente centralizado e ainda parece ligeiramente acima do centro geométrico do botão. Assim, o caminho/API foi exercitado, mas a centralização vertical completa não está confirmada visualmente.

As tabelas 1.9.1 continuam mostrando Atividade ID104 bloqueada até nível 8/Capítulo 7, HangUp ID116 após passar Capítulo 2, e ID125 a partir do nível 5. O menu local atual permite HangUp pelos dados existentes; menus de atividade gated não foram liberados. Funções que dependem de backend, anúncio ou pagamento continuam sem botões falsos. A lista de talentos configura Escolhido Divino/Mão Divina com item 40001 em custos 1/3 e níveis mínimos 1/3. Capítulos têm recompensas configuradas desse cristal, mas a concessão local atual só implementa o marco inicial do Capítulo 1 (2 cristais, já visíveis antes do teste); não foi criada fonte de moeda nem concedido cristal. Recompensas de progressão de capítulos posteriores e atividades rankeadas permanecem sem implementação/validação local.

### Classificação de validação

- **Análise estática:** fluxos dos dois handlers, custos/tabelas, gating de entradas e rotas de callbacks verificados no dump/config 1.9.1.
- **Bridge:** hooks instalados com prólogo validado; melhoria unitária usou callbacks originais e readback; correio e alinhamento registraram chamadas Unity. Não houve fault injection para testar recuperação de transação interrompida.
- **Visual na v72:** melhoria e correio conferidos por captura; alinhamento do capítulo ainda precisava ajuste vertical. **Status supersedido pela v73:** captura nova confirma centralização vertical/horizontal; ver “Continuação v73”.
- **Gameplay/save:** melhoria unitária passou com custo exato e sobreviveu a reinício. OneKey positivo, insuficiência de recurso, crash recovery, talentos azuis e novas recompensas locais de cristal não foram testados.

**Próximo item registrado na v72 (histórico):** ajustar alinhamento e validar OneKey. O ajuste visual foi concluído na v73; OneKey segue condicionado à confirmação da semântica original e à existência de recursos naturais, como detalhado na continuação v73.

### Retificação do saldo final após ação acidental de UI

Após a captura de persistência do Anel (2465 ouro), uma tentativa posterior de navegação por toque ADB acionou involuntariamente outra melhoria unitária: o log `build/kinja-v72-final-action-logcat.txt` registra `uid=2 oneKey=0 level=1->2 gold=100 books item=20003 count=1`. Não era parte do caso autorizado e não houve edição manual do save. Por transparência, o saldo final no aparelho caiu mais 100 ouro e um Livro de Forja de C: 2365 ouro, 13 gemas, 1070/2000 EXP. A tela `build/kinja-v72-final-after-extra-transaction-restart.png` confirma esses saldos após novo reinício do processo. O primeiro upgrade do Anel permanece persistido (UID4 Lv.2/ATQ19, livro A consumido); a segunda ação é registrada como efeito acidental já persistido e não será revertida editando dados. O estado do UID2 e a quantidade restante do livro C não foram reabertos visualmente; o log registra o callback/commit. Nenhum cristal azul foi gasto.

## Continuação v73 — alinhamento do capítulo e bloqueio de teste OneKey (14/10/2026)

### Estado preservado antes da atualização

ADB confirmou no package `com.habby.kinjarun.offline.lab6` a instalação da v72 (`versionCode=51`, `versionName=1.9.1`, `firstInstallTime=2026-10-13 19:43:33`, `lastUpdateTime=2026-10-14 16:54:37`). A captura `build/kinja-current-before-v73.png` mostra 2365 ouro, 13 gemas e 1070/2000 EXP. Ao abrir o detalhe do Anel do Oráculo, sem acionar melhoria, a captura `build/kinja-equip-detail-before-v73.png` mostra UID4 no nível 2/20 e Livro de Forja A `0/1`. Nenhum recurso foi gasto nesta continuação.

### Correção visual

A hierarquia identificada em 1.9.1 põe `txtRegionNameInBtn` diretamente sob `btnUnclearFight`. A v73 usa essa rota exata, obtém o pai pelo `Transform.get_parent` e o GameObject por `Component.get_gameObject`, e aplica a âncora do `UILabel` ao retângulo do botão por `UIRect.SetAnchor`; mantém `UIWidget.set_pivot(Center)` e `UILabel.set_alignment(Center)`. São chamadas Unity/IL2CPP na thread Unity, sem escrita de offsets gerenciados.

BuildId `v73-chapter-label-anchor`; APK `build/kinja-run-offline-lab-v73-chapter-label-anchor.apk`, SHA-256 `7F36012506293B13C964EC9474FC001CE6AFF5FD5D302DC5963F9C1AD54127E5`. Foi instalada com `adb install -r`, no mesmo package e certificado. `firstInstallTime` permaneceu inalterado. A captura `build/kinja-v73-home.png` mostra “Capítulo 3” centrado vertical e horizontalmente. Após force-stop/relaunch (sem limpar dados), `build/kinja-v73-after-restart.png` confirma novamente layout e saldos 2365 ouro/13 gemas/1070 EXP. O log correspondente `build/kinja-v73-after-restart-logcat.txt` contém sucesso explícito de `UIRect.SetAnchor`, `UIWidget.set_pivot` e `UILabel.set_alignment`, além de `anchor=1 pivot=1 alignment=1`. Portanto visual, chamadas bridge e persistência do save após reinício estão validados; o layout é reaplicado por `UIMainPanel.DoOpen`.

### OneKey: sem teste de gameplay nesta passagem

O handler 1.9.1 encaminha o UID selecionado a `EquipManager.UpgradeEquip(uid, true)`, que despacha para o helper separado `ET.EquipHelper.OneKeyUpgradeEquip(int)`. Isso confirma o alvo (um item), mas não quantos níveis a rotina tenta aplicar nem como para ao faltar livro/ouro. O state machine nativo está em `MoveNext` RVA `0xF3934C` no alvo validado; a descompilação REA não ficou disponível neste host (provedor Ghidra ausente). A rotina async ofuscada e a assinatura, isoladamente, não bastam para confirmar o algoritmo. A política local atual tenta níveis sequenciais até o limite ou a insuficiência, mas esse detalhe permanece hipótese de implementação, não comportamento 1.9.1 provado.

O Anel selecionado não tinha Livro A (`0/1`), portanto OneKey não foi tocado. Ouro/gemas observados antes da inspeção: 2365/13; nenhum gasto ocorreu. A quantidade dos demais livros não foi verificada nesta passagem. Retomar apenas depois de confirmar o algoritmo exato no cliente 1.9.1 e encontrar naturalmente os materiais requeridos. A implementação local de OneKey não está validada em gameplay.

### Classificação de validação v73

- **Análise estática:** alvo 1.9.1 continua SHA-256 `7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e`; hierarquia e APIs existem na metadata. OneKey aponta ao UID escolhido, mas número de níveis/parada seguem desconhecidos.
- **Bridge:** APK v73 compilado/assinado/instalado; log pós-restart confirma sucesso das três APIs de âncora/pivot/alinhamento na thread Unity.
- **Visual:** `build/kinja-v73-home.png` e `build/kinja-v73-after-restart.png` confirmam centralização vertical e horizontal antes e depois de reiniciar o processo.
- **Gameplay/save:** OneKey e gasto não foram testados. Após `adb install -r` e force-stop/relaunch, o save preservou 2365 ouro/13 gemas/1070 EXP, e `firstInstallTime` permaneceu igual.

**Próximo item:** confirmar no código nativo 1.9.1 quantos níveis `OneKeyUpgradeEquip` aplica e qual condição usa para parar; depois revisar a política local. Aguardar livros obtidos naturalmente; não testar o Anel enquanto Livro A estiver `0/1`. Também registrar retorno das chamadas de âncora e confirmar o layout após reinício.

## v74/v75 — perfil QA e talentos azuis (14/10/2026)

### Reprodução e causa

ADB no v73 reproduziu a falha no nó “Escolhido Divino” (ID 2001). Antes do toque, a tela mostrava 13 gemas, 1965 ouro e 2 cristais azuis. O toque não debitou nada. `build/kinja-talent-failure-repro.png` e `build/kinja-talent-failure-repro-logcat.txt` registram visualmente o botão x1 e a linha `Unsupported local talent cost id=2001 currency=40001 amount=1`.

Na configuração 1.9.1, nós `TalentCfg.type=0` são a árvore menor em ouro (item 101), enquanto `type=1` são nós especiais em cristal 40001. O handler anterior aceitava apenas item 101 e sempre incrementava `TalentLevel`; por isso recusava o ramo especial. `TalentManager` expõe `CurExtraUpgradeNum` e o retorno original `G2C_TalentUpgrade` contém separadamente `TalentLevel` e `ExtraTalentLevel`. A cadeia de IDs 2001–2026 é consecutiva e ligada por `parent`; 2001 custa 1 cristal, 2002 custa 3 e exige nível 3.

### Mudanças no bridge e preparação do perfil QA

`local_talent_upgrade_hook` agora seleciona `CurUpgradeNum`/`TalentLevel` para `type=0` e `CurExtraUpgradeNum`/`ExtraTalentLevel` para `type=1`. Lê preço, moeda, tipo e nível mínimo do `TalentCfg` original; exige correspondência live/save da moeda e do contador, respeita nível mínimo e só aceita o próximo ID azul da cadeia. O consumo do item 40001 é gravado em `seed_items.json`, entregue por `ItemManager.OnItemInit`; a resposta completa segue por `TalentManager.OnTalentDataResp`. A transação registra journal com moeda/campo/valores anteriores e posteriores, bloqueia reentrada e reconcilia a interrupção no bootstrap. As APIs IL2CPP originais e a Unity thread são preservadas; nenhum offset gerenciado foi escrito.

Como autorizado pelo usuário, a build de QA v74 também elevou moeda, EXP, nível e todos os 56 tipos `ItemCfg.bag_item=1` para pelo menos 1.000.000 por callbacks normais de inicialização. Isso altera o save de teste já existente de forma persistente. O nível de conta foi elevado para 100, limite que aparece na tabela dos talentos especiais. O incremento foi aplicado por edição de JSON local do bridge antes dos callbacks, sem apagar o save. A primeira versão reaplicava o mínimo em todo arranque e repunha os gastos; esse defeito foi corrigido na v75 com marcador privado `qa_max_profile_applied.json`. A v75 mantém o pacote de QA, mas seu build nativo normal não ativa esse modo, a menos que `build-native.ps1 -QaMaxProfile` seja usado.

### Builds, instalação e evidência

- v74 experimental: BuildId `v74-qa-max-resources-blue-talents`, APK `build/kinja-run-offline-lab-v74-qa-max-resources-blue-talents.apk`, SHA-256 `4F00E8FCF3A8147E987029C6185F6C79133D41F1F12FC610B44280518AE9F143`. Esta build repunha recursos em todo arranque; fica supersedida pela v75.
- v75 instalada: BuildId `v75-qa-profile-once-blue-talent-persist`, APK `build/kinja-run-offline-lab-v75-qa-profile-once-blue-talent-persist.apk`, SHA-256 `2171B6026F31DD6E391ABD9031B6E0376D57D2BD00A2B24BFE64B3CF9DD5802D`. APK original exato: SHA-256 `7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e`. Instalada com `adb install -r`, mesmo package e assinatura de laboratório; sem limpar dados. `firstInstallTime` permaneceu `2026-10-13 19:43:33`; pacote continua `versionCode=51`, `versionName=1.9.1`.
- A inicialização v74 registrou `QA max profile applied ... 56 bag item types >= 1000000`, e a home mostrou Lv.100 e 1M ouro/gemas. Capturas: `build/kinja-v74-qa-home.png`, `build/kinja-v74-talents.png`.
- Gameplay/bridge v74: compra de ID 2001 debitou 1.000.000→999.999 e avançou `ExtraTalentLevel 0→1`; compra seguinte ID 2002 debitou 999.999→999.996 e avançou `1→2`. Os logs `build/kinja-v74-talent-bought-logcat.txt` e `build/kinja-v75-restart-logcat.txt` registram getter de contador especial, callbacks originais de ItemManager/TalentManager e commit. `build/kinja-v74-talent-bought.png` mostra o nó azul dourado após a compra; `build/kinja-v75-restart-pre-second.png` e `build/kinja-v75-marker-restart.png` mostram o estado comprado após reiniciar.
- O restart v74 demonstrou a reposição indevida, que motivou a v75. Na v75, o primeiro bootstrap registrou a preparação QA e gravou o marcador; o segundo reinício registrou `QA max profile already prepared; preserving subsequent gameplay spends`. Assim a v75 não volta a repor recursos consumidos.

### Validação e limites

- **Análise estática:** custos/níveis/parent da árvore azul e campos/respostas de contador separados confirmados no dump e nas tabelas 1.9.1.
- **Bridge:** hook e validação de preço/saldo/contador/cadeia confirmados pelos logs; callbacks `ItemManager.OnItemInit` e `TalentManager.OnTalentDataResp` retornaram OK. O marcador QA de uso único foi observado após relançamento.
- **Visual:** saldo 1M e Lv.100 visíveis em `kinja-v74-qa-home.png`; o ramo do Escolhido Divino mudou de botão de compra para checkmark/estado dourado após compra e continuou após restart. A UI de Mão Divina não foi isolada em captura própria.
- **Gameplay/save:** ID 2001 e ID 2002 comprados em sequência com cobranças exatas e `ExtraTalentLevel` persistido após restart. Não medimos o efeito de habilidade em combate. A compra azul transacional não teve fault injection; saldo insuficiente e double tap não foram testados. O perfil de QA agora está no Lv.100 e possui pelo menos 1M em moedas, EXP e 56 tipos de itens; ele deixa de representar progressão natural. O futuro perfil de jogo deve ser novo/limpo, sem executar a flag QA.

**Próximo item:** validar em combate o efeito funcional do Escolhido Divino/Mão Divina, e então confirmar qualquer requisito de habilidade que só se manifeste durante a run. Ao sair do QA e iniciar o perfil real, empacotar sem `-QaMaxProfile` e criar um save inicial separado.

### Retificação QA v75 — toques acidentais e divergência de saldo (14/10/2026)

Durante navegação automatizada por coordenadas, o aparelho recebeu toques em controles diferentes dos pretendidos. O log `build/kinja-v75-unexpected-home-logcat.txt` registra sete talentos comuns (IDs 1006–1012; total 31.500 ouro), cinco compras do baú local 115 (250 gemas cada; conteúdos 1175, 1200, 1191, 1205 e 1178) e um Reforço de um clique no UID3: nível 2→10, 2.500 ouro e 17 livros 20002. O log confirma os handlers locais e cobranças; como foram ações involuntárias, isso **não** confirma a semântica do OneKey da versão 1.9.1 nem constitui teste planejado. Nenhuma edição manual ou reversão foi feita.

Há uma inconsistência que requer investigação: às 17:30:27 o bridge registrou readback e persistência de 966.000 ouro, 998.750 gemas e 1.000.000 EXP. Após force-stop e relançamento sem novos toques, a captura `build/kinja-v75-unexpected-after-restart.png` exibiu aproximadamente 235 mil ouro e 998 mil gemas. O log de bootstrap `build/kinja-v75-unexpected-after-restart-logcat.txt` não registra gasto ou reposição e confirma que o marcador QA já estava ativo. Como não foi possível ler diretamente os arquivos privados do save via `run-as`, o saldo de ouro persistido/mostrado permanece **inconclusivo**; não afirmar que o saldo visual foi preservado. O aparelho foi deixado na tela inicial e não recebeu mais entradas.

Evidência por categoria: a captura registra o resultado visual posterior; o primeiro log contém callbacks/readbacks dos handlers; o log pós-restart contém apenas bootstrap/marker; o gameplay acidental registra efeitos e consumo, mas não valida intenção nem equivalência com 1.9.1. A causa provável é toque ADB desalinhado durante exploração, enquanto a divergência 966 mil vs. aproximadamente 235 mil não tem causa estabelecida. Próximo passo: investigar essa leitura de saldo por captura/log somente de leitura ou build diagnóstica antes de continuar gastos ou interpretar o perfil QA.

## Troca local de gemas por ouro — v77 (14/10/2026)

### Causa e análise 1.9.1

O APK alvo foi rechecado: SHA-256 `7953EB369BA9B8C814C4736DAC4A7E3AA116A5583837F6F12D795EA70C20595E`. A tabela `analysis/bundles/tables/clientdata.ht/951442175585326260_VariableCfgData.json` confirma `diamond_buy_lot_coin.int_array_value=[20000,100]` e `diamond_buy_huge_coin=[118888,500]` (ouro, gemas). Os handlers exatos do cliente são `UITreasureBoxPage.OnClickBuyLotCoin` RVA `0x13E145C` e `OnClickBuyHugeCoin` RVA `0x13E1730`; os hooks preservam os guards de prólogo 1.9.1. Os registros estáticos de `UITreasureBoxPage.DoAwake` confirmam os dois listeners e as chaves de configuração.

A causa direta era o bridge anterior: os dois handlers apenas retornavam com `Blocked Shop gem-to-gold exchange`, sem chamada de compra nem mutação de moeda. O código de `UpdateBuyDiamond` também tentava ocultar candidatos da seção inferior OURO, apesar de as rotas de navegação não encontrarem esses nós de forma confiável. O bloqueio explicava o clique inerte; a tela existente podia continuar mostrando os cards.

### Implementação local

Os handlers locais v77 consultam `Variable.GetCfg` e `int_array_value` por `System.Array.GetValue(Int32)` via `runtime_invoke`, validam os pares contra os custos fixos esperados da tabela exata 1.9.1 e recusam valores diferentes. A compra exige que ouro/gemas lidos pelo `ItemManager.GetOwnItemNum` concordem com `Coin`/`Diamond` no save, verifica saldo e overflow, aplica os dois deltas via setters originais de `PlayerDataManager`, chama `SyncValueToShow`, confere readback e grava os campos no `seed_player.json` existente com substituição atômica do arquivo. Guard de reentrada protege os dois botões. A seção OURO permanece visível; pagamentos reais continuam ocultos, e o resgate diário local de ouro continua com seu handler já existente. Todas as chamadas ocorrem na Unity thread, sem escrita de offsets gerenciados.

A primeira tentativa v76 chegou ao novo hook mas parou antes da transação porque os exports nativos opcionais `il2cpp_array_length`/`il2cpp_array_addr_with_size` não existiam no runtime do aparelho. `build/kinja-v76-shop-lot-preconfirm-logcat.txt` registra esse erro; não houve débito. A v77 substituiu essa leitura pelas APIs gerenciadas originais de `System.Array`.

### Build, instalação e prova runtime

BuildId `v77-local-gem-gold-exchange-array-api`; APK `build/kinja-run-offline-lab-v77-local-gem-gold-exchange-array-api.apk`; SHA-256 `C8008999C37F6539716FC501BB6301F526203746A6F754378A47E0453B9B8614`. Biblioteca ARM64: SHA-256 `7DD4A152CB58A7BF8F5FF2633434703CAD93F00E5718391F7A56E4D60DD7EB01`. Compilada e assinada pelo keystore de laboratório. Atualizada no package `com.habby.kinjarun.offline.lab6` com `adb install -r`; `firstInstallTime=2026-10-13 19:43:33` permaneceu inalterado e `lastUpdateTime=2026-10-14 17:42:03`. APK original e builds de referência não foram sobrescritos.

Antes do toque, a captura `build/kinja-v77-shop-before-purchase.png` mostrava 198K ouro, 996K gemas, e os dois cards disponíveis. O log `build/kinja-v77-shop-lot-purchase-logcat.txt` registra o config original, ambos os `GetValue`, getters, setters e callback de sincronização; a transação commitou exatamente 198.200→218.200 ouro e 996.940→996.840 gemas, sem mudar 1.000.000 EXP. A captura posterior `build/kinja-v77-shop-lot-purchased.png` mostra 218K ouro e o card ainda disponível. Após force-stop/relaunch, a Loja voltou mostrando 218K/996K em `build/kinja-v77-after-restart-shop.png`; `build/kinja-v77-after-restart-logcat.txt` confirma callbacks de inicialização e `stage=initialized`. Na validação inicial, ainda não havia compra de 500 gemas; consulte o complemento runtime abaixo.

### Níveis de validação e limites

- **Análise estática:** versão/hash alvo e valores das duas ofertas conferidos na tabela 1.9.1; listeners/RVAs vêm do dump/metadata e do disassembly anotado local. REA não conseguiu decompilar o binário neste host Windows porque Ghidra não está configurado; a interpretação de alto nível nativa segue limitada ao dump, ao disassembly já anotado e às tabelas.
- **Bridge:** hooks foram instalados com prólogos verificados; config foi obtida com `Variable.GetCfg` e lida por `System.Array.GetValue`; readback e persistência registram ambos os deltas exatos.
- **Visual:** cards OURO visíveis antes/depois; saldos arredondados atualizados na hora e após reinício, capturas acima.
- **Gameplay/save:** uma compra do pacote barato passou e persistiu após reiniciar. Saldo insuficiente e toque duplicado não foram exercitados. O guard existe, mas não foi submetido a teste concorrente.

**Próximo item:** se necessário, testar saldo insuficiente sem editar saves e concorrência/double-tap; as duas opções de preço já têm compra e persistência pós-reinício observadas.

### Complemento runtime v77 — pacote maior

O segundo handler também foi exercitado uma vez no aparelho, em build v77. O log `build/kinja-v77-shop-huge-purchase-logcat.txt` confirma configuração original `diamond_buy_huge_coin` e commit de 218.200→337.088 ouro e 996.840→996.340 gemas; EXP continuou em 1.000.000. `build/kinja-v77-shop-huge-purchased.png` mostra o saldo arredondado após o clique. Após novo force-stop/relaunch, `build/kinja-v77-after-huge-restart-shop.png` mostra 337K/996K, e o log `build/kinja-v77-after-huge-restart-logcat.txt` registra `stage=initialized`. Portanto os dois pacotes (100 e 500 gemas) têm resultado visual e persistência após reinício observados. A compra de 500 já não é uma pendência runtime. Insuficiência e double-tap concorrente seguem sem teste.


## Nova falha reportada — combinação de equipamento (14/10/2026)

O jogador relata que a ação de unir equipamentos falha com erro offline. A build identificada por ADB é a v77, package `com.habby.kinjarun.offline.lab6`, `versionCode=51`, `versionName=1.9.1`; `firstInstallTime=2026-10-13 19:43:33` permaneceu preservado. A captura inicial da tela de equipamento é `build/kinja-merge-before.png`; o log bruto correspondente está em `build/kinja-merge-before-logcat.txt`.

**Análise estática 1.9.1:** a rota é `UICompoundEquipTips.OnClickCompound` → `EquipManager.CompoundEquip(uid, uid1, uid2)` → `ET.EquipHelper.CompoundEquip` e pedido ET `ET.C2G_CompoundEquip` (IDs de material `EquipUidCost1/2`). O modo coletivo usa `ET.EquipHelper.OneKeyCompoundEquip` e `ET.C2G_OneKeyCompoundEquip`. As respostas ET originais carregam o equipamento resultado e os materiais consumidos (`G2C_CompoundEquip.CostEquips`; no coletivo, `EquipUids` e `CostEquips`). Há também um dispatcher local em `ProtocolManager.HandleProtocol`, mas ele atende às classes legadas `C2S_Equip_Compound_Data` e `C2S_Equip_One_Key_Compound_Data`. A análise anterior confundiu essas classes com os pedidos `ET.C2G_*`; o dispatcher legado não atende à chamada feita pelo botão atual.

**Evidência de bridge e save:** durante a janela de observação houve atividade de gameplay no aparelho. O log `build/kinja-merge-equipment-return-logcat.txt` registra compras de talentos azuis e uma chamada `Local equipment upgrade complete uid=5 oneKey=1 level=1->20 gold=57600 books item=20001 count=85`, seguida de persistência/readback. A captura posterior `build/kinja-merge-equipment-return.png` mostra a arma no nível 20 e saldo arredondado diferente. Como havia entradas de usuário concorrentes e a navegação ADB não isolou a tela de combinação, este evento não é resultado de teste de fusão e os saldos da janela não são uma medição limpa antes/depois da fusão. Nenhum save foi editado manualmente, revertido ou substituído.

**Evidência de seleção no aparelho (v77, 14/10/2026):** sem confirmar a ação, foi aberto o seletor e escolhido um colar comum Nv.1 como raiz e outro como primeiro material (`build/kinja-merge-necklace-material2-v77.png`). A captura mostra a prévia de nível máximo 20→30, ATQ +15→+23 e uma nova atribuição, além de um segundo slot ainda vazio. A cópia adicional de colar Nv.1 estava marcada com cadeado/equipada e o toque não a tornou selecionável; a seleção foi encerrada sem confirmar nem consumir itens. `build/kinja-merge-exit-selection-v77.png` registra a tela após sair. O saldo visível permaneceu 992K gemas / 925K ouro durante a seleção; sem mutação observada. O log `build/kinja-merge-selection-logcat-v77.txt` contém `NullReferenceException` repetido em `ET.RecoverPowerHelper.RequestNextRecoverPowerTime` → `UIResourceBar.UpdatePowerCountdown`, sem stack trace de fusão; isso é evidência de erro do contador de energia offline, não de falha do handler de combinação.

**Reprodução controlada no aparelho (v77, 14/10/2026):** foram selecionados três colares comuns Nv.1 (raiz e dois materiais) e tocado “Criar” uma única vez. O log `build/kinja-merge-after-once-logcat-v77.txt` mostra `System.NullReferenceException` em `ET.EquipHelper+<CompoundEquip>d__4.MoveNext`, chamado por `ET.EquipHelper.CompoundEquip` → `EquipManager.CompoundEquip` → `UICompoundEquipTips.OnClickCompound`. A captura `build/kinja-merge-after-once-v77.png` mostra a tela ainda com os três itens selecionados, botão ativo e saldos arredondados 992K gemas / 925K ouro. Não houve callback visual de resultado nem consumo: a tentativa falhou antes da alteração do inventário.

**Causa encontrada:** no corpo nativo 1.9.1 de `EquipHelper.<CompoundEquip>d__4.MoveNext` (VA `0xF388F4`, identidade de `analysis/native/game-method-map.json`), o fluxo obtém `ET.Game.Scene` e lança `NullReferenceException` quando a propriedade é nula. Isso ocorre antes de `ET.Session.Call`. O bridge v77 não intercepta `CompoundEquip`, ao contrário de `WearEquip`/`UpgradeEquip`/`OneKeyUpgradeEquip`, então o helper de rede original é executado sem a cena ET online. O protocolo local antigo `ProtocolManager.HandleProtocol` não resolve esse caminho porque recebe outro tipo de pedido. Não há evidência de consumo parcial na tentativa capturada; save e inventário ficaram sem alteração visível.

**Próximo passo técnico:** implementar no bridge um caminho local para `ET.EquipHelper.CompoundEquip` usando os UIDs entregues pela UI, validar o contrato de fusão no código original (`EquipManager.OnGetCompoundEquip`, `G2C_CompoundEquip` e `EquipInfo`), persistir a alteração de forma transacional e só então invocar o callback original na thread Unity. O caminho coletivo é separado e não deve ser redirecionado até confirmar seu contrato próprio. Instalação, correção e persistência após reinício ainda não validadas; não há build nova. Capturas/logs em `build/kinja-merge-*.png` e `build/kinja-merge-*-logcat-v77.txt`.

## Continuação — fusão individual de equipamento (v77 → v79, 14/10/2026)

### Análise estática 1.9.1

A tela chama `UICompoundEquipTips.OnClickCompound` → `EquipManager.CompoundEquip(uid, uid1, uid2)` → `ET.EquipHelper.CompoundEquip(int,int,int)`. O helper online depende de `ET.Game.Scene` antes de `ET.Session.Call`; na instalação v77, a cena era nula e o state machine lançava `NullReferenceException` antes de enviar o pedido. O antigo `ProtocolManager.HandleProtocol` cobre tipos C2S legados distintos e não intercepta o pedido ET da UI. A resposta original `ET.G2C_CompoundEquip` contém `EquipUid` e `CostEquips: List<ET.EquipInfo>` (`EquipId`, `EquipLevel`, `EquipQuality`); `EquipManager.OnGetCompoundEquip` é o callback original da fusão. A rota coletiva `OneKeyCompoundEquip` continua separada.

### Bridge v79

O bridge intercepta `ET.EquipHelper.CompoundEquip` somente com os três parâmetros e prólogo ARM64 verificados contra `kinja-run-1-9-1.apk` SHA-256 `7953EB369BA9B8C814C4736DAC4A7E3AA116A5583837F6F12D795EA70C20595E` (VA `0x2c2d3f4`, bytes `FF 83 01 D1 F6 57 03 A9 F4 4F 04 A9 FD 7B 05 A9`). Não usa offsets de objetos gerenciados. O caminho local compara os UIDs e atributos dos três itens entre o save e `EquipManager.GetEquipInfoByUid`, exige o mesmo ID/qualidade e itens não equipados, e confirma a próxima qualidade por `EquipQualityCfg.GetCfg`. A transação prepara o aumento de qualidade da raiz e remove somente os dois UIDs selecionados; não debita ouro nem concede recursos. O callback recebe um `G2C_CompoundEquip` criado com `JsonConvert`, depois `OnEquipDataInitResp` sincroniza o inventário persistido. O guard impede reentrada; UIDs já consumidos são recusados em chamadas subsequentes. O journal reaproveita snapshots de equipamento/itens/jogador e restaura save e managers originais em falha; o bootstrap recupera uma transação interrompida.

A v79 também corrige a recuperação pós-callback: se desserialização, callback de resultado ou refresh falhar, restaura o snapshot e reaplica `ItemManager.OnItemInit` / `EquipManager.OnEquipDataInitResp` na thread Unity. Nenhum callback ou singleton é chamado por worker thread.

### Resultado de aparelho e save

A reprodução original v77 continua sendo a evidência visual/de gameplay da causa: após uma tentativa “Criar”, o helper online lançou NRE antes de qualquer consumo; ver `build/kinja-merge-after-once-v77.png` e `build/kinja-merge-after-once-logcat-v77.txt`.

A v78 foi instalada por `adb install -r`; `firstInstallTime` permaneceu `2026-10-13 19:43:33`. No seletor atual, os equipamentos aparecem bloqueados e a UI informou “Nenhum outro equipamento disponível” ao tentar escolher as peças. A captura `build/kinja-v78-root-select6.png` documenta esse estado. Por segurança, não confirmei uma fusão sem dois materiais elegíveis. Nenhuma peça ou moeda foi consumida pela validação v78/v79.

O save QA antes da atualização mostrava aproximadamente 992K gemas, 925K ouro e 1.000.000 EXP (`build/kinja-v78-before-install.png`); após a instalação/reabertura da v79 continuava mostrando os mesmos valores arredondados (`build/kinja-v79-after-restart.png`). `run-as` não permite ler os arquivos privados deste package não-debuggable, então os valores exatos e o inventário salvo não tiveram readback independente. Não houve edição manual nem limpeza do save. O log `build/kinja-v79-startup-filtered-logcat.txt` confirma instalação do hook após relançamento e ausência de `FATAL EXCEPTION` na janela filtrada; `build/kinja-v79-preinstall-logcat.txt` preserva a coleta anterior à atualização.

### Build e estado de validação

BuildId `v79-local-equipment-fusion-rollback-refresh`; APK `build/kinja-run-offline-lab-v79-local-equipment-fusion-rollback-refresh.apk`; SHA-256 `5AD160E8F0590B7E5E9766B1BCAB4D3D7E213AE26114B74E27261974F9668E02`; `libkinjaoffline.so` SHA-256 `6B48154C57CB0E865039322CBCAB4D3D7E213AE26114B74E27261974F9668E02`. Assinada pelo keystore de laboratório e instalada no package `com.habby.kinjarun.offline.lab6`, `versionCode=51`, `versionName=1.9.1`; `lastUpdateTime=2026-10-14 18:13:03`, `firstInstallTime=2026-10-13 19:43:33`. APK original, v57 e v69 de referência não foram sobrescritos.

- **Análise estática:** causa, assinatura, tipos de resposta e callback confirmados para o APK 1.9.1.
- **Bridge:** compilação/assinatura bem-sucedidas; o log do aparelho registra o hook instalado.
- **Visual:** menu de criação abre; a UI reportou ausência de equipamento compatível no estado atual.
- **Gameplay/save:** a fusão positiva, consumo das duas cópias, mudança de qualidade e persistência após reinício continuam sem validação. Próximo passo: disponibilizar naturalmente duas peças compatíveis que a própria UI marque como elegíveis e repetir uma única fusão; não contornar cadeados ou fabricar itens.

### Retificação v80 — contrato original de elegibilidade e estado mais recente (14/10/2026)

A revisão final v80 também chama as APIs originais `EquipManager.EquipCanCompound(rootUid)` e `EquipRelateCompound(costUid)` para raiz e cada material. Só prossegue se a UI/manager 1.9.1 também considerar todos elegíveis, além das validações do save/live e qualidade. Isso evita que a implementação local force peças que o seletor bloqueia. BuildId `v80-local-equipment-fusion-original-eligibility`; APK `build/kinja-run-offline-lab-v80-local-equipment-fusion-original-eligibility.apk`, SHA-256 `82B74E8048B53B3D959D264EFE2D33F7B2075FF37C9D2C867C2714BFD641810B`; biblioteca `FF5130D24B3B812C059686F8353379161CEC95B4214EE330A6D76F4F677A011F`. Instalada via `adb install -r` no mesmo package/certificado; `firstInstallTime=2026-10-13 19:43:33`, `lastUpdateTime=2026-10-14 18:15:57`. `build/kinja-v80-startup-filtered-logcat.txt` confirma o hook na nova inicialização. Capturas `build/kinja-v80-before-install.png` e `build/kinja-v80-after-restart.png` mostram ~632K ouro, ~992K gemas e 1.000.000 EXP nos dois lados da instalação.

O saldo mudou de cerca de 925K para 632K entre a captura v79 das 18:13 e a captura pré-v80. O log completo `build/kinja-v80-runtime-logcat.txt` registra sete chamadas separadas de Reforço de um clique no processo anterior à v80: cinco upgrades Nv.1→20 gastando 57.600 ouro e 85 livros cada (UIDs 11, 9, 20, 13, 16), depois dois Nv.1→10 gastando 2.600 ouro e 18 livros cada (UIDs 1 e 10). Os readbacks foram 867.828, 810.228, 752.628, 695.028, 637.428, 634.828 e 632.228 ouro; gemas ficaram 992.190 e EXP 1.000.000. São transações locais persistidas pela mecânica separada OneKey, observadas enquanto o app estava em uso paralelo; não são chamadas de fusão nem consequência do update. Não revertemos nem editamos esses gastos. O save privado não é legível por `run-as`, então o screenshot exibe saldos arredondados e os deltas exatos vêm do log do bridge.

A fusão segue sem teste positivo: a última captura do seletor continua sendo `build/kinja-v78-root-select6.png`, na qual a UI informa que não há peças compatíveis. Nenhuma confirmação de fusão foi enviada em v78–v80; sem consumo ou mudança de qualidade por esse caminho. O teste futuro precisa começar quando a própria UI habilitar uma raiz e dois materiais compatíveis, e então validar callback, remoções e persistência. A instalação v80, carregamento do hook e retomada do save foram confirmados; resultado visual/gameplay da fusão não.

Comando usado para a v80 (use um novo BuildId em uma nova execução neste checkout, pois este diretório de build já existe):

```powershell
.\offline\build.ps1 -BuildId v80-local-equipment-fusion-original-eligibility -PackageName com.habby.kinjarun.offline.lab6 -PackageApk
.\tools\android\toolchain\platform-tools\adb.exe install -r .\build\kinja-run-offline-lab-v80-local-equipment-fusion-original-eligibility.apk
```

## Fusão: resultados de gameplay e duplicatas QA (v81–v84, 14/10/2026)

### Resultado das duas fusões feitas pelo jogador

Após atualizar para v80, o jogador fez duas fusões. `build/kinja-v80-runtime-logcat.txt` registra: raiz UID13, qualidade 2→3, materiais 20/16, nível atual 20; e raiz UID15, qualidade 2→3, materiais 11/7, nível atual 1. Não houve custo de ouro. O bridge comparou os UIDs e dados vivos/salvos, chamou `EquipCanCompound`, `EquipRelateCompound`, `EquipQualityCfg.GetCfg`, o callback original `OnGetCompoundEquip` e atualizou com `OnEquipDataInitResp`. A fusão aumenta qualidade e limite máximo, não o nível atual — esse aumento é o fluxo Melhorar. O preview anterior da UI mostrava limite máximo 20→30 e atributo 15→23. `build/kinja-post-user-fusion.png` documenta a bolsa/saldos depois. Toques posteriores com UIDs já consumidos/incompatíveis foram recusados, sem outra alteração.

### Duplicatas QA autorizadas

O jogador autorizou cópias de equipamentos neste save de teste. O bridge QA clona registros existentes com UIDs novos, preserva os campos da peça, marca `IsWear=false`, persiste atomicamente e chama `EquipManager.OnEquipDataInitResp` na Unity thread. Não mexe em ouro, gemas, EXP ou livros. Marcadores idempotentes evitam novas cópias nos relançamentos, mesmo após consumo. A fusão normal continua exigindo a elegibilidade das APIs originais.

- v81 `v81-qa-equipment-fusion-repeats`, APK SHA-256 `2CA49B187C5B9243AD42D29126B625A96EA21FF7EB84884371D92A0CB44668A8`, `.so` `009407A5F96CA38F638D660615173D858B0E2098FDCFB6E1C19280823BF88DBD`. Sem alterações ao inventário: `EquipCanCompound` não encontrou raiz pronta. Log/captura: `build/kinja-v81-after-qa-seeding-logcat.txt`, `build/kinja-v81-after-qa-seeding.png`.
- v82 `v82-qa-equipment-fusion-material-eligibility`, APK `C5C979B56C78A296E13AEBA673B4F0712FB8DACB354ECB3DD2533B62CD943FA7`, `.so` `5599013045E0609E083C8815362B982D345B7669B116E1AFB80131800A0462B1`. Consultou também `EquipRelateCompound`; não alterou o inventário. Log `build/kinja-v82-after-qa-seeding-logcat.txt`.
- v83 `v83-qa-equipment-repeat-pool`, APK `47E47F76EBE478024FD45EA883B9DAB26E6461AA0C99CEBCB6D1F3C4789C6BA6`, `.so` `89C10FF5E9876715FD7AEA9F008F0B6EF62422D98860D52DF8C941B801750D84`. Adicionou cinco cópias de `EquipId=6001`, qualidade 1 (sete no total), usando item salvo UID1 como molde. Visual `build/kinja-v83-equipment-bag.png`; log `build/kinja-v83-after-qa-seeding-logcat.txt`. Depois do refresh, as APIs retornaram `canCompound=0` e `relateCompound=0`; a UI de fusão mostra cadeados em `build/kinja-v83-fusion-create.png` e nenhuma fusão foi confirmada.
- v84 `v84-qa-fusion-proven-quality-pool`, APK SHA-256 `C585F0AC89884A3ABBF06FD788EF630CFF3392DFF26174D3C2D6A9BB18181B70`, `.so` `C9D7C7440B54C29E887093D59143E42205A6C3F39CD4537050FD8CF5B7B87C58`. Adicionou cinco cópias de `EquipId=3002`, qualidade 2 (sete no total), a partir do registro UID2 já salvo. As APIs 1.9.1 retornaram false antes e depois do refresh. Log `build/kinja-v84-after-qa-seeding-logcat.txt`, bolsa `build/kinja-v84-equipment-bag.png` e, após reinício do processo, `build/kinja-v84-after-restart-equipment-bag.png`. `build/kinja-v84-after-restart-logcat.txt` registra `QA fusion repeats already prepared`; não houve reposição. Instalações usaram `adb install -r`, mesmo package/certificado, preservando `firstInstallTime=2026-10-13 19:43:33`. A tela manteve ~632K ouro / ~992K gemas e 1.000.000 EXP.

### Evidências e pendência

- **Análise estática:** configuração 1.9.1 separa qualidade e limite máximo (qualidade 1/2/3 → níveis máximos 10/20/30); `ET.EquipInfo` contém EquipId, EquipLevel e EquipQuality distintos.
- **Bridge:** fusões anteriores confirmam callback original, remoção dos dois materiais e qualidade +1. v83/v84 confirmam seed do inventário, refresh, marcador e ausência de mutação de moeda.
- **Visual:** os dois grupos de sete aparecem na bolsa após restart. Saldos exibidos seguem arredondados.
- **Gameplay/save:** não foi confirmada fusão dos grupos QA, pois `EquipCanCompound` e `EquipRelateCompound` seguem false e o seletor os marca com cadeado. Nenhuma peça ou recurso foi consumido. A regra exata que explica a inelegibilidade atual continua desconhecida; a análise REA das funções nativas não pôde ser executada neste host (Ghidra não configurado, Hopper incompatível com Windows, IDA sem conexão). Não relaxar as APIs para contornar isso.

Reprodução da v84 em um save QA (usar BuildId novo ao refazer; builds já existentes não devem ser sobrescritas):

```powershell
.\offline\native\build-native.ps1 -QaMaxProfile -QaFusionRepeats
.\offline\build.ps1 -BuildId v84-qa-fusion-proven-quality-pool -PackageName com.habby.kinjarun.offline.lab6 -NativeLibrary .\offline\build\arm64-v8a\libkinjaoffline.so -PackageApk
.\tools\android\toolchain\platform-tools\adb.exe install -r .\build\kinja-run-offline-lab-v84-qa-fusion-proven-quality-pool.apk
```

## Loja — investigação do baú de diamante 10x (14/10/2026)

### Análise estática e causa

O APK-alvo continua sendo 1.9.1, SHA-256 `7953EB369BA9B8C814C4736DAC4A7E3AA116A5583837F6F12D795EA70C20595E`. A tabela `TreasureBoxCfg` distingue `id=115` (baú de diamante unitário, 250 gemas, `item_nums=1`) de `id=108` (baú de diamante 10x, 2200 gemas, `item_nums=10`, pool `box_id=1021`). A disassembly do método 1.9.1 `UITreasureBoxPage.UpdateTreasurePage` em `0x13DF2CC` confirma chamadas a `TreasureBoxItem.UpdateItem` para IDs 101, 115 e 108. O bridge prévio recusava ID108; seu algoritmo já respeitava `item_nums` até 10 e pool 1021.

### Reprodução e evidência visual

Na v84, a tela da Loja mostrava dois cartões: bronze e diamante. O texto “Obtenha equipamento Épico em 10 vezes” descrevia pity da compra unitária, não dez aberturas numa compra. Um toque usado para reproduzir o problema acionou o ID115: o log `build/kinja-v84-shop-before-10x.log` registrou compra `chest=115`, `gems=990940->990690`, `ContentIds=[1187]`; a captura `build/kinja-v84-shop-after-10x-tap.png` mostra a animação e `build/kinja-v84-shop-return.png` a entrega de Botas de Couro. Esse gasto único de 250 gemas foi uma compra normal concluída pelo jogo. O save não foi editado nem limpo.

### Alteração e limitação de UI

O bridge passou a aceitar ID108, lendo preço e número de sorteios do `TreasureBoxCfg`, gerando dez itens do pool 1021, debitando com setters originais e persistindo pelo callback original. v85 (`v85-local-diamond-chest-ten-pack`) foi instalada, mas a captura `build/kinja-v85-shop.png` confirmou que isso sozinho não exibia a compra 10x. A v86 tentou reaproveitar o cartão 115 interceptando `TreasureBoxItem.UpdateItem` no RVA `0x134019C`, com prólogo conferido diretamente no ELF do APK 1.9.1. O log mostrou a troca de configuração, mas o Unity caiu com SIGBUS/PC `0xa` ao renderizar; coleta em `build/kinja-v86-crash.log`. A v86 foi removida do aparelho e seu hook de UI foi retirado do código. Não usar essa abordagem.

### Estado final do aparelho e validação

v87 (`v87-local-ten-draw-bridge-ready`) está instalada por `adb install -r` no mesmo package/certificado. APK SHA-256 `87F661856AF0036D8EDDFAAA519959E96703A742F87970E184085AB7C1429841`; `libkinjaoffline.so` SHA-256 `F981DD893D97DEC6007240D5AF3833DE6C7E44928179A57D67D49AF50007386E`. `firstInstallTime=2026-10-13 19:43:33` permaneceu; `lastUpdateTime=2026-10-14 18:43:34`. O processo permaneceu ativo e a Loja abriu; veja `build/kinja-v87-shop.png` e `build/kinja-v87-shop.log`. Os saldos na captura são arredondados (990K gemas, 632K ouro); o último readback exato do bridge após a compra unitária foi 990690 gemas, 632228 ouro e 1000000 EXP. A abertura dez vezes, recompensa agregada, débito de 2200 gemas e persistência pós-reinício continuam **não testados**. Não houve uma segunda compra.

- **Análise estática:** ID108, preço 2200 e dez sorteios confirmados pela configuração; `UpdateTreasurePage` original inclui a configuração.
- **Bridge:** aceitação de ID108 compilada em v87. A API de UI `UpdateItem` não foi alterada; o crash de v86 é evidência de tentativa rejeitada, não de funcionalidade.
- **Visual:** v87 abre a Loja sem crash, mas ainda mostra o cartão unitário atual; a opção separada 10x não foi exibida.
- **Gameplay/save:** compra unitária ID115/1 item reproduzida; compra de dez itens e persistência ainda pendentes. Próximo trabalho: disponibilizar a entrada ID108 usando a hierarquia/layout original com um hook estável; então validar uma única compra 10x após consentimento explícito para o débito configurado de 2200 gemas.

### v88–v94 — cartão do baú de diamante x10 na Loja (14/10/2026)

**Análise estática.** Reutilizado o objeto inativo original `UITreasureBoxPage/obj/ContentPanel/Top/ChestBox/TenDiamondBox` do prefab 1.9.1. A configuração ID108 é a compra de dez sorteios por 2200 gemas (pool 1021). O bridge já tinha a transação ID108; a pendência era mostrar o cartão original e reservar espaço no layout. Não alteramos o cartão unitário nem a tabela.

**Bridge.** v88 ativa o `GameObject` original usando `Component.get_transform`, `Transform.Find` e `GameObject.SetActive`. v89–v92 ajustaram `BuyCoin` e `imgDownFill` por getters/setters gerenciados de `Transform.localPosition` e recalcularam a área rolável. O hook `DoRefreshPage` em v92 foi instalado, mas não foi observado sendo chamado. v93 tentou `UITreasureBoxPage.UpdatePage` com bytes de prólogo na ordem errada e a verificação recusou corretamente o hook. v94 corrigiu a ordem little-endian; o log `build/kinja-v94-shop.log` confirma `Installed UITreasureBoxPage.UpdatePage memory hook`, ativa o cartão e registra a movimentação dos nós e atualização dos limites de rolagem. Mantidas as verificações de prólogo/hash e APIs Unity gerenciadas; sem offsets de objetos.

**Resultado visual.** Captura `build/kinja-v94-shop.png` (prévia reduzida `build/kinja-v94-shop-preview.jpg`): cartão laranja “Baú de Diamante x10” aparece abaixo dos cartões unitários, com texto, preço e botão originais; a seção Ouro começa abaixo dele sem sobreposição. A loja abriu sem crash. O texto/preço visível na captura está ampliado/arredondado pelos dados QA existentes.

**Gameplay/save.** Não tocamos no botão x10: o custo configurado é 2200 gemas e esta etapa era para disponibilizar o cartão. Portanto, dez resultados, débito, inventário e persistência pós-compra/reinício ainda não estão validados em gameplay. O log de inicialização carrega os seeds existentes (`seed_player.json`, `seed_items.json`, `seed_equipment.json`, entre outros) sem recriar o save. A tela mostra aproximadamente 990K gemas e 632K ouro; o último readback exato conhecido antes desta etapa é 990690 gemas, 632228 ouro e 1000000 EXP, não um readback pós-v94. Uma compra unitária anterior legítima ID115 custou 250 gemas e entregou o item 1187; nenhum gasto x10 ocorreu nesta investigação.

**Build instalada.** BuildId `v94-local-ten-draw-after-page-update`; APK `build/kinja-run-offline-lab-v94-local-ten-draw-after-page-update.apk`, SHA-256 `6E5A93CAF051AFF33F468D39D2F8BBDD9FDD8820B0C7344AF2B0ADEA7936F3C1`; ARM64 `.so` SHA-256 `00677F02CECB81CC46FE7EA9654B0B50E9735D5EFE4E6ED4E496688661BB79FC`. Instalada no aparelho Xiaomi em 14/10/2026 às 19:15 via `adb install -r`, package `com.habby.kinjarun.offline.lab6`, versão 1.9.1/code 51, certificado de laboratório. A consulta ADB confirma `firstInstallTime=2026-10-13 19:43:33` e `lastUpdateTime=2026-10-14 19:15:00`, sem reinstalação limpa. Reprodução: `build-native.ps1`, `build.ps1` com BuildId acima e `adb install -r` no APK v94. Próximo passo: quando autorizado, executar exatamente uma compra x10; registrar moedas e inventário antes/depois e reiniciar para confirmar persistência. Não pressionar para teste sem confirmar o gasto.

### Complemento de auditoria da UX v94 — 14/10/2026

A reprodução visual por ADB registrou `build/kinja-shop-ux-current.png` e duas rolagens em `build/kinja-shop-ux-scroll-bottom.png` / `build/kinja-shop-ux-scroll-bottom2.png`; o log completo capturado está em `build/kinja-shop-ux-current.log`. A rolagem torna a falha reproduzível: o conteúdo de `ContentPanel` passa por trás da barra fixa superior; no limite inferior, o cabeçalho/ícone do cartão x10 ficam parcialmente cobertos pela barra de saldo. Os cartões de Ouro continuam alcançáveis, então os limites de scroll foram recalculados, mas o viewport/clip não respeita a região abaixo do HUD.

Há também uma ambiguidade material no preço apresentado. **Retificação:** a captura mostra a chave configurada no botão x10 e a contagem `1000000/10`; a configuração ID108 usa `open_item_id=1002`, `cost_item=[10]` e `item_nums=10`. `coin_id=4001`/`coin_num=10` é outro campo da tabela e não identifica o ícone desenhado no botão. ID108 também contém `cost_diamond=2200`. O handler local anterior debitou gemas em qualquer caso. O complemento v97 abaixo registra o trace de `OnOpenByItem` e a tentativa de distinguir a rota da chave; ela não foi acionada em gameplay e não se deve considerar o débito da chave validado.

Durante a janela desta reprodução o log registrou uma transação ID108: `Local chest purchase committed chest=108 gems=990440->988240 contentIds=[1185,1177,1177,1206,1190,1185,1190,1181,1199,1177]`. O procedimento automatizado intencionalmente não tocou no botão comprar; apenas abriu/navegou e fez swipes da Loja, portanto a origem do acionamento (toque concorrente ou ativação inesperada durante navegação) não pôde ser determinada. Tratar como compra não planejada e não atribuí-la ao jogador nem declarar sua validação visual. O callback de compra e os dois callbacks de refresh de inventário aparecem no log; não foi capturado o recibo/resultado e não houve reinício pós-evento. Após o evento, a captura mostra saldo arredondado 988K; o valor exato pós-evento do log é 988240 gemas. O save não foi restaurado nem editado.

Até resolver semântica de pagamento e clipping, não fazer novos swipes que atravessem a área do botão x10 e não executar compra para validação. A build instalada permanece v94; sem alteração de APK nesta auditoria. Próximo passo é análise estática exata desses três métodos e depois um ajuste de viewport via APIs NGUI/Unity gerenciadas, validado visualmente sem tocar em compras.

### Complemento v97 — Loja, clipping e meio de pagamento (14/10/2026)

**Análise estática 1.9.1.** O `TreasureBoxCfg` ID108 configura dez sorteios (`item_nums=10`), `open_item_id=1002`, `cost_item=[10]` e preço alternativo `cost_diamond=2200`; `coin_id=4001`/`coin_num=10` é um campo separado e não é o item ilustrado no botão. `TreasureBoxItem.OnOpenByItem` (RVA `0x1341A9C`, assinatura `void(self, GameObject, MethodInfo)`) distingue os estados gerenciados Item=3 e Currency=4, mas encaminha ambos ao mesmo `ShopManager.BuyTreasureChest` (`0x26D8AD0`). A configuração e a UI dão base para tratar o estado 3 como chave 1002 × 10; essa conclusão ainda carece de confirmação por uma compra controlada.

**Bridge.** O v97 mantém a chamada original e as verificações dos 16 bytes do prólogo. Adiciona contexto thread-local, restrito à chamada síncrona na thread Unity, para que o hook central saiba se o widget estava em estado Item ou Currency. No caminho Item prepara débito de `open_item_id` por `item_nums`, confere saldo live/save, mantém gemas intactas, grava recompensas e usa callbacks de inventário originais. Um journal com snapshots permite restaurar os três seeds após falha ou interrupção; double-tap/reentrada continua protegido pelo guard existente. O caminho Currency continua cobrando `cost_diamond`. Nenhuma chamada `OnOpenByItem` foi observada neste teste, portanto o novo pagamento por chave está instalado, mas não validado em gameplay.

Para o clipping, o bridge encontra `ContentPanel`/`UIPanel` e usa `get_baseClipRegion`, `set_baseClipRegion`, `InvalidateClipping` e `UIScrollView.UpdateScrollbars(true)`. O readback confirma a região mudou de topo 624 para 464 e manteve a borda inferior em -660. Apesar disso, a captura v97 `build/kinja-v97-now.png` ainda mostra o título x10 parcialmente sob a HUD ao chegar à rolagem baixa. **Resultado visual: falha remanescente; não declarar a UX corrigida.** O setter e a invalidação executaram sem erro, mas a máscara resultante não corresponde ao recorte visual esperado. Próximo passo: mapear no prefab e no runtime os `UIPanel`/âncoras efetivamente associados aos widgets do cartão, consultar `finalClipRegion` e hierarquia de renderização e só então ajustar o viewport.

**Save e recursos.** Antes dos testes desta rodada, logs de readback registravam 632228 ouro, 986040 gemas e 1000000 EXP. Esse saldo já inclui duas aberturas ID108 não planejadas observadas na auditoria anterior (990440→988240→986040 gemas; conteúdo de dez IDs em cada evento); não houve restauração nem edição manual. A captura pré-atualização `build/kinja-v95-before.png` mostra arredondamentos 986K/632K. Na navegação de validação v95–v97 não houve novo log de compra, nem consumo observado; a tela v97 continua mostrando aproximadamente 986K/632K. Não foi feito teste de reinício pós-compra porque nenhuma compra foi feita.

**Build/dispositivo.** BuildId `v97-shop-clip-invalidate-readback`; APK `build/kinja-run-offline-lab-v97-shop-clip-invalidate-readback.apk`, SHA-256 `1EFF10D7099EC975DD3EA6C2520C0FACE393EB7F7DEA463B0F9B341FED811358`; `.so` ARM64 SHA-256 `CD5DFCEA8377643DB96E042012069BFCA95D0525D75E5012D77901209B7A2AE9`. Package `com.habby.kinjarun.offline.lab6`, jogo 1.9.1/code 51, assinatura de laboratório. Instalado por `adb install -r`; `firstInstallTime=2026-10-13 19:43:33` preservado e `lastUpdateTime=2026-10-14 19:38:09`. O log `build/kinja-v97-now-logcat.txt` confirma hooks, alteração/readback do clip e ausência de compra nesta captura. Builds intermediárias v95/v96 foram substituídas por v97 no mesmo package; APK original e v57 não foram sobrescritos.

Reprodução: compilar com `offline/native/build-native.ps1 -QaMaxProfile -QaFusionRepeats`, empacotar com `offline/build.ps1 -BuildId v97-shop-clip-invalidate-readback -PackageName com.habby.kinjarun.offline.lab6 -NativeLibrary .\\offline\\build\\arm64-v8a\\libkinjaoffline.so -PackageApk`, atualizar com `adb install -r` e consultar screenshot/logs. Não tocar no botão x10: o débito correto por chave e a resposta visual ainda precisam de uma validação autorizada e monitorada.
