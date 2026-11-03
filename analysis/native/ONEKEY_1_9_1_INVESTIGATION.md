# Unir tudo — investigação do contrato 1.9.1

## Estado antes da autorização da adaptação

Não corrigido. Nenhum hook coletivo ou APK novo foi produzido nesta investigação. A v102 continua instalada. A exigência do usuário de só corrigir a coletiva com comportamento/contrato confirmados impede substituir seleção do servidor por uma política inventada.

**Atualização posterior:** o usuário autorizou expressamente a adaptação offline proposta no chat. A implementação e validação seguintes estão em [ONEKEY_OFFLINE_V107.md](ONEKEY_OFFLINE_V107.md). A frase acima descreve somente o estado anterior a essa autorização; a seleção remota ET continua desconhecida e não foi declarada equivalente.

## Artefato e aparelho

Original `kinja-run-1-9-1.apk` novamente conferido: SHA256 `7953EB369BA9B8C814C4736DAC4A7E3AA116A5583837F6F12D795EA70C20595E`. Disassembly usa a recuperação XOR + permutação `analysis/metadata/libil2cpp.stripes.so`, SHA256 `A2A4510A6650A6BFC9C23949E466392A098F9A1C992E21194B531C1E45BABDBE`; não a cópia de XOR simples.

ADB conectado, package `com.habby.kinjarun.offline.lab6`, versionCode51/versionName1.9.1; firstInstallTime `2026-10-13 19:43:33`, lastUpdateTime `2026-10-14 20:21:21`. Captura não destrutiva `build/kinja-onekey-current.png` mostra tela de fusão, botão Unir tudo, saldos arredondados 7,49M/938K e equipamentos reais. Log preservado em `build/kinja-onekey-current-logcat.txt`. Não houve toque consumidor, instalação, limpeza ou edição do save.

## Caminho moderno confirmado

- `UICompoundEquipTips.OnClickOneKey` (`0x12F689C`) consulta os grupos de qualidade 3,2,1 e `EquipCanCompound`; chama `EquipManager.OneKeyCompound` (`0x2C39970`) se houver candidato.
- `EquipManager.<OneKeyCompound>d__96.MoveNext` (`0xF3A67C`) aguarda `ET.EquipHelper.OneKeyCompoundEquip()`.
- Helper `0x2C2D50C`, entrada reconstruída `sub sp,sp,#0x40; str x19,[sp,#0x20]; stp x29,x30,[sp,#0x30]; add x29,sp,#0x30`. Nenhum patch foi instalado; bytes em runtime ainda não foram usados como validação de hook coletivo.
- `ET.EquipHelper.<OneKeyCompoundEquip>d__6.MoveNext` (`0xF38DA0`) usa Scene/SessionComponent e `Session.Call(C2G_OneKeyCompoundEquip)`, aguarda `G2C_OneKeyCompoundEquip`, testa Error e chama `OnGetOneKeyCompoundEquip`. O pedido não transporta uma lista de UIDs: seleção/transação da rota ET é responsabilidade remota.
- `G2C_OneKeyCompoundEquip` tem `EquipUids: List<int>` para resultados e `CostEquips: List<EquipInfo>` (Id/Level/Quality) para custos; não especifica os UIDs consumidos, ordem ou raiz por grupo.
- `EquipManager.OnGetOneKeyCompoundEquip` (`0x2C3372C`) dispara evento com EquipUids, registra telemetria dos custos e resolve as configs dos resultados por UID. Não executa a transação nem escolhe os grupos. Inventário precisa estar atualizado antes do callback.
- `OnClickOneKeyDetail` (`0x12F6B08`) abre `UIOneKeyDetailPanel`. `DoAwake` (`0x10BDB04`) registra handlers de fechar; não calcula grupos. A tela de detalhes não fornece um planejador coletivo oculto.
- Texto `c_onekey_compound_limit_hint` fala Épicos e inferiores; precheck nativo procura somente qualidades1–3. Essa divergência não autoriza incluir qualidade4 no bridge.

## Por que o handler legado não fecha o contrato ET

`ProtocolManager.HandleEquipOneKeyCompound` (`0x2A8C7F8`) é outro protocolo (`C2SProtocolBase`), com aplicação de resultados, eventos e devoluções próprios. Não é chamado pela state machine ET acima.

O legado percorre qualidades1→3 e cada lista de trás para frente. Usa `CheckCompound` (`0x2C36858`), cujo resultado provém de `GetSameOwnEquip` (`0x2C37F28`): esse método devolve **todos** os UIDs no cache de ID/qualidade, sem limite de três. No trecho `0x2A8CE70–0x2A8CF50`, o handler adiciona à lista de custos todos os itens retornados, exceto a raiz, sem parar em `cost[2]`.

`ProtocolManager.DoEquipCompound` (`0x2A8E2C8`) testa qualidade e ID/parte de cada material, mas não compara o tamanho da lista à quantidade configurada. Em sucesso, `0x2A8E4EC–0x2A8E520` constrói EquipIns com novo UID (`uidIncrease+1`), mesmo Id/nível, qualidade+1 e Part0. O handler também consulta `GetBreakEquipReturn` e aplica devoluções em seu protocolo. Portanto copiar esse caminho para a resposta ET poderia consumir mais cópias do que a receita e mudar identidade/devoluções sem confirmação.

Isso é uma divergência estática demonstrável, **não** prova de como o servidor ET 1.9.1 funciona. Não executar o legado sobre o save para descobrir.

## Evidências produzidas

Disassemblies anotados em `analysis/bundles/`:

- `ET.EquipHelper.OneKeyCompoundEquip.annotated.txt`
- `ET.EquipHelper._OneKeyCompoundEquip_d__6.MoveNext.annotated.txt`
- `EquipManager.OnGetOneKeyCompoundEquip.annotated.txt`
- `EquipManager.CheckCompound.annotated.txt`
- `EquipManager.GetSameOwnEquip.annotated.txt`
- `EquipManager.GetSameOwnEquipWithPart.annotated.txt`
- `EquipManager.GetOwnEquipsSortByQuality.annotated.txt`
- `UIOneKeyDetailPanel.DoAwake.annotated.txt`
- `ProtocolManager.HandleEquipOneKeyCompound.annotated.txt`
- `ProtocolManager.DoEquipCompound.annotated.txt`

Os logs atuais incluem ações posteriores do jogador: UID66 q4→5 consumindo94 às20:33:31; UID142 q3→4 consumindo166/188 às20:33:46; UID80 q4→5 consumindo183 às20:34:00. Não atribuir essas ações a teste automatizado, nem assumir que seu estado foi conferido após reinício. Baseline de20:25 antecede essas alterações e não é inventário atual completo.

## Separação de validação

- **Estática:** fluxo ET, assinatura/resposta/callback, receitas1–3 e divergência do legado confirmados no cliente exato.
- **Bridge:** a fonte v102 só intercepta CompoundEquip individual; nenhuma substituição de OneKeyCompoundEquip. Não há execução positiva de coletiva offline.
- **Visual:** botão e bolsa observados em captura atual, sem toque consumidor. Abrir tela não valida fusão.
- **Gameplay/save coletivo:** não validado. Há grupos reais no inventário e não há cenário isolado QA para Unir tudo; nenhuma peça foi fabricada ou consumida nesta investigação.

## Dado necessário para uma correção fiel

Falta uma evidência da transação **ET** original (implementação do servidor ou captura anterior/posterior acompanhada da resposta e atualização de inventário), especialmente com mais de três cópias, níveis diferentes, itens equipados e resultados que voltam a formar grupo. Ela deve estabelecer seleção da raiz, preservação/renovação de UID, cascata e devolução de upgrades. Outra possibilidade requer autorização explícita para definir uma adaptação offline documentada em vez de afirmar equivalência à seleção remota desconhecida. Não produzir build especulativa com consumo de itens reais.
