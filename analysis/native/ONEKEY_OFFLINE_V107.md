# Unir tudo — adaptação offline autorizada

## Contrato e autorização

O [relatório estático 1.9.1](ONEKEY_1_9_1_INVESTIGATION.md) confirma helper, resposta e callback ET, mas não a seleção remota dos UIDs. O usuário respondeu **“Autorizar essa adaptação offline”** à proposta: trios do mesmo EquipId/qualidade1–3, cascata até roxo, raiz de maior nível/UID preservado, equipados excluídos, resposta própria e transação atômica. A política local não é apresentada como implementação recuperada do servidor.

`offline/native/fusion_batch_plan.h` implementa essa política. Todos os itens equipados ficam fora, inclusive raízes. Para cada ID/qualidade, a raiz tem o maior nível; empate pelo menor UID. Materiais são os dois menores níveis; empate pelo maior UID. Sobras permanecem. Resultados passam à faixa seguinte1→2→3→4 na mesma transação; qualidade4 e superiores não são processadas. UID/EquipId/nível/parte da raiz permanecem. Não são fabricadas peças, moedas, devoluções ou atributos. Limites/buffs/atributos continuam vindo das configs originais. Não há cobrança ou refund de moeda/itens nesta política.

## Causa e implementação

A v102 substituía apenas `CompoundEquip`. A coletiva continuava passando por Scene/SessionComponent/Session.Call, dependências online indisponíveis. O hook novo é **independente** em `ET.EquipHelper.OneKeyCompoundEquip()` (`0x2C2D50C`, ETTask<bool>, método estático sem argumentos). Prólogo conferido: `FF0301D1 F31300F9 FD7B03A9 FDC30091`. Hash do APK e prólogos existentes preservados. Não chama a fusão individual nem o protocolo legado incompatível.

Na thread Unity, o bridge:

1. Reconcili­a journal pendente e recusa outra transação de equipamento/baú/wear. Usa guard compartilhado com a individual; o hook de wear também recusa reentrada durante fusão.
2. Lê todos os registros canônicos, rejeita UID duplicado/campos inválidos e compara contagem, UID/Id/Quality/Level/Part/wear com o manager vivo. Confere parte com GetEquipCfgByIdAndLvAndQuality.
3. Recalcula caches originais e mantém EquipCanCompound/EquipRelateCompound para os grupos inicialmente existentes. Confere GetCfg(1–4) e receitas atuais1–3 `[1,q,2]`; grupos em cascata usam essas mesmas receitas verificadas.
4. Prepara toda a mutação em uma string, sem gravar intermediários. Monta/deserializa `G2C_EquipData` e **G2C_OneKeyCompoundEquip** (EquipUids finais e CostEquips de cada material no rank/nível em que é consumido); confere contagens das listas gerenciadas.
5. Registra baseline completa e projeção em diagnóstico **privado** antes de consumir; salva snapshots dos três saves e journal. Snapshots/journal/resultado usam rename atômico, fsync do arquivo e diretório.
6. Grava equipamento uma vez, chama `OnEquipDataInitResp`, verifica todos os sobreviventes/removidos, contagem e igualdade dos saves de itens/jogador. Chama `OnGetOneKeyCompoundEquip`, verifica saves novamente e remove/sincroniza o journal. Somente então retorna sucesso ao ETTask original.
7. Em falha restaura todos os saves e managers pelos callbacks originais. Se a restauração em memória falhar, conserva snapshots/journal para recuperação; não apaga a evidência antecipadamente. A recuperação no bootstrap usa o journal compartilhado existente. Repetição sobre inventário já processado não refunde/consome outra vez: não restam trios1–3 elegíveis.

Nenhum offset de objeto gerenciado é escrito; acesso a campos/listas/configs usa APIs IL2CPP. A loja não mudou. Builds sem defines QA; nenhuma cópia nova foi adicionada.

## Diagnósticos e reprodução não destrutiva

O bootstrap executa `local_onekey_compound(roots,true)`: valida dados/configs/modelos e prepara projeção, mas não grava save/journal, não altera managers e não chama o callback de sucesso. O único arquivo escrito é o relatório privado de diagnóstico.

- `<filesDir>/kinja_onekey_dryrun.json`: última baseline/projeção do bootstrap.
- `<filesDir>/kinja_onekey_preflight.json`: baseline real antes de um lote consumidor.
- `<filesDir>/kinja_onekey_last_commit.json`: diagnóstico opcional após readback confirmado. Nunca é usado como origem de itens/save.

Esses arquivos são privados do app; não há exportação automática para armazenamento compartilhado. Logcat contém OneKey preflight/baseline/plan/prepared/models/diagnostic/committed. O app não é debuggable; uma captura parcial de logs não equivale à cópia integral do save. Logs controlados v106 capturaram todos195 registros e permitiram comparação de todos os campos listados.

Build e atualização: usar BuildId novo e `offline/build.ps1 -PackageName com.habby.kinjarun.offline.lab6 -PackageApk`, sem flags QA, depois `adb install -r`. Para observar, iniciar a Activity, esperar `OneKey models ready dryRun=1`, abrir Equipamento e Criar, capturar bolsa/tela, force-stop/start e comparar baselines dos PIDs. **Não tocar Unir tudo neste save real para reproduzir apenas diagnóstico.**

## Evidências e limites

**Estática:** RVAs/ABI/protocolos/receitas foram conferidos no artefato1.9.1; divergência do legado documentada. Seleção da política local foi autorizada, não inferida como contrato remoto.

**Testes isolados ARM64 no aparelho, sem jogo/save:** `build/kinja-v104-plan-regression.txt` passa cascata de27 cinzas→1 roxo, raiz de maior nível, repetição idempotente, dois grupos, sobras, empate determinístico, equipados excluídos, q4 intocada, ID diferente recusado e UID duplicado recusado. `build/kinja-v107-json-journal-regression.txt` usa helpers reais da fonte final para promover raiz/remover UIDs exatos, gravações sincronizadas, destino inexistente e recuperação/repetição do journal. Fixtures ficam apenas em `/data/local/tmp/kinja-onekey-fixtures-v107`; não acessam arquivos do lab6. Não houve interrupção/falha de callback injetada em gameplay.

**Bridge no save real, sem consumo:** v103/v104/v106 instalaram hook verificado e executaram preflight; v104/v106 desserializaram modelos originais e conferiram listas. Inventário195, 69 passos projetados, 138 materiais projetados,31 UIDs finais projetados e57 sobreviventes projetados. São projeções, não itens aplicados. Saldos salvos7.490.720 ouro/938.307 gemas/1.000.640 EXP.

**Visual/preservação após reinício v106:** `build/kinja-v106-onekey-before-restart.png`, `kinja-v106-bag-before-restart.png` e `kinja-v106-bag-after-restart.png`. PIDs18335→19992. `kinja-v106-validation.json` compara todos195 registros capturados antes/depois (UID/EquipId/qualidade/nível/parte/wear), sem diferença. Saldos e projeção iguais. Não é validação de persistência de um resultado coletivo, pois não ocorreu consumo.

**Gameplay/save coletivo:** ainda não validado com consumo/callback visual positivo/reinício de um lote aplicado. O lote inclui peças reais; a instrução de limitar a investigação a ações não destrutivas quando depender delas permanece. Nenhuma autorização de seeding/consumo real foi inferida da autorização da política. Próximo teste recomendado: lote coletivo em estado isolado legitimamente autorizado, baseline de todos os grupos antes, readback/resultado visual e reinício após um único toque. Não fabricar recursos.

## Histórico intermediário

- v103: primeiro lançamento abortou na inicialização original IL2CPP (PID1441), antes de hooks/metadados; segundo lançamento6015 concluiu bootstrap. Causa do abort não estabelecida; não atribuir resolução à coletiva. Log `kinja-v103-startup-retry-logcat.txt`.
- v104: modelos originais validados em dry-run, instalada temporariamente.
- v105: APK compilado e preservado, **não instalado**. Revisão automática rejeitou instalação/lançamento porque o diagnóstico copiaria saves completos para armazenamento externo; essa alternativa foi removida.
- v106: diagnóstico somente privado, instalada, dry-run e preservação integral dos campos de195 equipamentos após reinício confirmados.
- v107: adiciona fsync e conserva recuperação até a restauração dos managers. Artefatos/instalação final e observação estão registrados na seção seguinte.

APKs anteriores e original/v57 não foram sobrescritos. Backup da fonte v102: `build/offline_bridge.before-onekey-v103.cpp`.

## Build final instalada e reinício

| Artefato | SHA256 |
|---|---|
| `build/kinja-run-offline-lab-v107-onekey-atomic-durable.apk` | `5F244C374A26EAA44558E8F61C93C3A88830E1EC403EB7BBD420043E3A0437F0` |
| `build/offline-bridge-v107-onekey-atomic-durable/libkinjaoffline.so` | `3973702C547BE9C709EF62EA0AA1E82CFBFD7C159D1FBEC05C462ADE068CEB2A` |
| `offline/native/offline_bridge.cpp` | `6BF5FEEE2777CC29D5957D7BB631A3514AC79D5756FB09FABC2BCAFF59E08E20` |
| `offline/native/fusion_batch_plan.h` | `94EDAB4518EAE2112EA489C2EF5EC1AF43E36DAC6EC9D5AADD1398D553C32470` |

Package `com.habby.kinjarun.offline.lab6`, versionName1.9.1/versionCode51, firstInstallTime `2026-10-13 19:43:33`, lastUpdateTime `2026-10-14 20:56:02`. `adb install -r` retornou Success; `build/kinja-v107-installed-sha256.txt` confere SHA256 do base.apk instalado com o artefato. Assinatura com keystore de laboratório existente; Android aceitou atualização no mesmo package. `kinja-v107-build-log.txt` preserva compilação/empacotamento/verificação.

Instalação/PID20496 e reinício/PID22535: `kinja-v107-startup-logcat.txt`, `kinja-v107-before-restart-logcat.txt`, `kinja-v107-after-restart-logcat.txt`. Em ambas, hook verificado, todos195 itens passam validação save/live, configs/caches e modelos; projeção69 passos/138 custos/31 resultados finais/57 sobreviventes, mesmos saldos7490720/938307/1000640. O diagnóstico privado tem32900 bytes. Não apareceu `OneKey committed` nem invocação do callback de sucesso nesses testes.

`build/kinja-v107-validation.json` foi gerado por `kinja-v107-summarize-validation.py`:195 registros capturados antes e138 depois (combinação de OneKey baseline e Fusion baseline after-function-unlocks), todos os campos capturados iguais. Há57 UIDs sem linha no recorte externo por supressão de logd; não declarar essa captura um dump integral pós-reinício. A validação interna do preflight cobre195; a comparação externa completa dos195 foi obtida na v106, em `kinja-v106-validation.json`. Nenhuma dessas evidências valida persistência de lote consumidor.

Capturas finais: `kinja-v107-bag-before-restart.png`, `kinja-v107-onekey-before-restart.png` e `kinja-v107-bag-after-restart.png`. Bolsa e botão observados, sem consumo/resultado coletivo visual. Tela v104 capturada durante animação (`kinja-v104-fusion.png`) não serve como evidência estável da fusão; usar v106/v107.

**Conclusão:** causa online substituída por transação coletiva local autorizada e instalada, modelos/planejamento/JSON/journal validados nos níveis descritos. Ainda falta executar um lote consumidor seguro e autorizado para conferir callback visual, todos os resultados, atributos/limites e persistência após reinício. Não solicitar ao jogador uma reprodução manual que possa ser feita por ADB; quando houver estado autorizado, realizar o teste automaticamente com um único toque e baseline completa.
