# Backup manual — v131, 03/11/2026

Implementação para o APK **1.9.1**, SHA-256 `7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e`. Build normal `v131-manual-progress-backup`, package `com.habby.kinjarun.offline.audit1`, mesmo certificado de desenvolvimento. **Não instalada nem executada no Android nesta tarefa.** Nenhum save Android ou arquivo do acervo externo foi alterado. A v128 QA e as instalações existentes permanecem como estavam. A intermediária v130 foi compilada/assinada, sem instalação; a v131 corrige a resolução de caminhos-pai com aliases Android (`/data/user/0`), mantendo a recusa de arquivos-filho vinculados.

## Rastreio antes da alteração

Lidos AGENTS, guia, auditoria e fontes do bridge/wrapper; referência visual fornecida pelo usuário é o menu expansível da home, com o botão azul Configurações. A normal anterior era v129. O método original `UIMainPanel.OnClickSetting` RVA `0xFCAA88` carrega a literal `UISetting` e abre esse painel. Não é `UISettingPage`.

Inspeção estática delimitada com metadata/tabelas existentes e NDK llvm-objdump. Binário reconstruído localmente do APK exato pelo decoder já existente, SHA-256 `a2a4510a6650a6bfc9c23949e466392a098f9a1c992e21194b531c1e45babdbe`; acervo somente lido. REA confirmou essa identidade pelo `open_binary`; `analyze_function` ficou indisponível por ausência de provider compatível/configurado neste host. Não houve instalação/configuração de engines. Não atribuir os resultados do disassembler à decompilação REA.

O bootstrap lê oito modelos persistidos e os entrega aos callbacks ET originais. HangUp e o resgate diário têm ledgers separados. `last_fight.json` é diagnóstico escrito pelo bridge, sem leitura para restaurar uma partida. Preferências adicionais foram rastreadas no cliente original:

| Valor lógico | Armazenamento/API original | Evidência estática |
|---|---|---|
| `1fight_fail_count` | PlayerPrefs Int32 | RecordStageFail, chamadas GetLocalInt/SaveLocalInt `0x277D348/0x277D36C`; o helper prefixa PID local 1 |
| `1drop_die_hint` | PlayerPrefs Int32 | DoResurgence.MoveNext, GetLocalInt `0x26B6A44` |
| `boxguide_close_1` | PlayerPrefs Int32 | concatenação da chave com PID `0x279ABD4..0x279ABF8` |
| `fail_region_<id>` | ObscuredPrefs Int32 | RecordStageFail `0x277D2F0/0x277D300` |
| `_manga_state` | ObscuredPrefs Boolean | OnEnterFightSucceedResp `0x29C7844` |

`CurEndingGameHp` é estado transitório: SetInt `0x27963A4` escreve somente `_intValueDic`, não PlayerPrefs. `chapter_` forma um evento MMPManager.TrackEvent (`0x29CB67C/0x29CB690`), não uma chave de save. Ambos foram excluídos. A enumeração de fail_region usa os **700 IDs exatos** de GameRegion, incluindo estados ausentes, para poder também remover corretamente uma preferência presente somente no destino.

## Dados portáteis

| Arquivo/entrada | Dados |
|---|---|
| `seed_player.json` | Nome local, nível/EXP, ouro, gemas, energia, status, ícone e contadores persistidos da conta |
| `seed_regions.json` | Seleção/máximo, progresso, vitórias, pontuação, derrotas e estados de regiões |
| `seed_items.json` | Materiais, livros, manuais, fragmentos, chaves e demais pilhas de itens |
| `seed_equipment.json` | UID, identidade, parte, nível, qualidade e condição equipada |
| `seed_talents.json` | Talentos normais e extras |
| `seed_heroes.json` | Herói equipado, heróis possuídos, níveis, estrelas e talentos |
| `seed_guide.json` | IDs de tutorial já persistidos pelo bridge |
| `seed_shop_chests.json` | Estados/contadores/grupos dos baús |
| `hangup_local_state.json` | Última coleta, dia e quantidade de voltas rápidas; opcional se ainda não criado |
| `shop_daily_claim.json` | Data e estado do resgate diário; opcional se ainda não criado |
| `local_preferences.json` | Entrada virtual: presença e valor lógico das preferências acima, capturados por APIs originais |

Os oito seeds e a entrada virtual são obrigatórios. A ausência de um ledger opcional é preservada na importação: não se mistura o ledger do destino com o perfil importado. Respostas estáticas de combate/HangUp/revive continuam sendo os assets do APK. Não se exportam XML bruto de preferências, chaves cifradas vinculadas ao aparelho, SDKs, analytics, identificadores de dispositivo, assinatura/senha, logs, snapshots `.before`, journals, marcadores QA ou cache. Nome e estados da conta local são necessários ao perfil; nenhum login remoto é requerido.

Marcadores QA **já existentes no destino** são preservados privadamente na troca de diretório para evitar que uma build QA reponha recursos por perder sua marca de idempotência. Nunca vêm do arquivo portátil. Nenhum recurso QA é ativado na v131 normal.

## Interface e exportação

DoOpen original é chamado por trampoline; em seguida o hook captura os valores lógicos e os arquivos de forma síncrona na thread Unity. O snapshot representa o progresso salvo ao abrir Configurações, não uma run em andamento. Não há chamada a managers por worker Java. O hook DoClose remove a entrada.

O wrapper acrescenta um botão Android “Backup do progresso”, azul com borda e texto branco, no rodapé enquanto Configurações estiver aberto. O painel de backup usa fundo escuro, botões azuis e ações Exportar/Importar. Essa implementação reaproveita a entrada Configurações original, sem trocar callbacks de feedback/login. **Posição final, sobreposição, escalas e aderência visual precisam de observação no jogo.** O print original não comprova a tela implementada.

Exportar abre ACTION_CREATE_DOCUMENT/CATEGORY_OPENABLE, sugestão `Kinja-Run-yyyyMMdd-HHmmss.kinjabak`, preferência por armazenamento local. O arquivo só é declarado exportado depois de escrever, flush, sync, fechar, reabrir e comparar todos os bytes, além de validar novamente o formato. Destino que não suporta sync/leitura de conferência produz erro. Após falha, tenta excluir o documento incompleto escolhido; se o provedor impedir a exclusão, pode restar um arquivo incompleto, sem mensagem de sucesso. Cancelar o seletor não anuncia sucesso nem altera progresso. Não são requeridas permissões amplas de armazenamento nem rede.

## Formato 1

Contêiner binário sem compressão e sem caminhos arbitrários. Inteiros em big endian, strings de nomes/cabeçalho ASCII pela codificação DataOutputStream.writeUTF (tamanho UInt16 + bytes); JSONs em UTF-8 estrito.

1. Magic de oito bytes ASCII `KINJABKP`.
2. Int32 de versão: `1`.
3. String de compatibilidade: `kinja-1.9.1-7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e/offline-save-1`.
4. Int32 de quantidade, 9–11 entradas.
5. Para cada entrada: nome, Int32 de tamanho e bytes integrais do JSON.
6. SHA-256 de **todos os bytes anteriores**, incluindo nomes, tamanhos, versão e compatibilidade, em 32 bytes binários.

Ordem de exportação fixa em BackupStore.FILES. O hash cobre conjuntamente todos os arquivos e o manifesto; detecta corrupção, não é assinatura de autenticidade. Limite individual 128 KiB, compatível com load_seed; leitura total limitada a 12 × 128 KiB. Entradas desconhecidas, duplicadas, ausentes, caminhos ou bytes excedentes são recusados. Não existe importação de ZIP nem extração de paths.

JSON estrito, sem propriedades duplicadas/metadados `$...`, conteúdo após o objeto, números fracionários/overflow, UTF-8 inválido ou profundidade acima de 24. Validação de campos/arrays/tipos/limites, UIDs e IDs únicos, equipamento por parte, identidade/parte/qualidade/teto de nível, herói selecionado possuído e teto por estrela, estados de região, contadores e pares de listas. Arrays obrigatórios não podem ser null. Os IDs permitidos e limites derivam das tabelas do APK exato, em BackupCatalog/backup_regions.h; hashes das fontes em BackupCatalog.sources.json. Regeração: `python tools/metadata/build_backup_catalog.py <diretório-de-tabelas-1.9.1>` na raiz. Não editar para aceitar outra versão sem revisar o contrato.

## Importação e recuperação

ACTION_OPEN_DOCUMENT lê um único arquivo limitado. O contêiner **inteiro** passa por integridade, compatibilidade e esquema antes de apresentar a confirmação. A confirmação diz explicitamente que conta, capítulos, heróis, itens e moedas serão substituídos e que é necessário reabrir. Cancelar preserva o perfil.

Confirmar grava somente `manual-import.request` privado, com arquivo temporário, sync, readback, rename e sync de diretório; então fecha/remove a tarefa Android e encerra seu processo. Não substitui saves nem managers no jogo vivo. Ao reabrir, **antes de configure e Unity.onCreate**, o wrapper:

1. Revalida a requisição e o perfil atual inteiro. Qualquer journal de operação pendente impede a troca: `talent_purchase_pending.json`, `equip_upgrade_pending`, `chest_purchase_pending`, `hero_pending`, `shop_daily_claim_pending.json`.
2. Cria e valida `manual-import.recovery` do estado anterior; prepara `manual-import.next` completo com fsync/readback. Não altera cada seed do diretório vivo em sequência.
3. Persiste estado `prepared`; renomeia offline_save para `manual-import.previous`, faz sync do pai, renomeia next para offline_save e faz sync; marca `applying`. O rename Android usa Os.rename, sem fallback para cópia parcial.
4. Na thread Unity, antes dos callbacks do jogador, captura as preferências lógicas atuais e persiste-as no recovery e no diretório anterior. Só depois aplica todo o checkpoint importado por HasKey/GetInt/GetBool/SetInt/SetBool/DeleteKey/Save originais, conferindo cada valor e presença. Preferências ObscuredPrefs são recifradas pelo cliente no novo aparelho.
5. O bootstrap existente desserializa **todos** os modelos ET antes da primeira entrega e executa seus callbacks originais, progressão e migrações existentes. Só após retorno bem-sucedido o Java remove o checkpoint, grava `committed` duravelmente e anuncia sucesso. Uma recriação de Activity no mesmo processo não repete a troca: o boot é executado somente uma vez por processo.

Se o processo terminar em prepared/applying sem confirmação, a próxima abertura confere a recuperação, restaura o diretório anterior integralmente e reaplica seu checkpoint lógico antes dos callbacks. Não combina seeds de duas versões. Falha detectada ao carregar a importação mostra opção “Fechar e recuperar”. Erro de disco/recuperação incerta bloqueia configure/Unity e mostra aviso antes de encerrar, preservando arquivos para outra tentativa. Um committed durável pode concluir somente a limpeza no próximo boot.

Depois de sucesso, uma geração anterior permanece privada em previous/recovery até preparar a próxima importação. Uma candidata interrompida pode permanecer em failed até a próxima tentativa. Não se oferece “desfazer” na UI. Não excluir manualmente esses arquivos durante recuperação pendente.

## Validação e limites

Resultado final: **PASS, 1922 verificações, 80 cenários de falha nos 40 pontos de operação**. Java release8/API33, D8 minAPI23 com23classes reais (stubs Unity excluídos), ARM64/API23 e assinatura v1/v2/v3 passaram. APK SHA256 `EB2D0F51BA3F2A73E41CC1FFD507B3023BDB8650F48E2E9E534273A0440A2865`; biblioteca `457C58C5CB9D622F4C51D75C866F4145CD4A2208D8DEDABCA5B7D2CEF4456874`; DEX `2BE17FE18861B7934E45D9CC8A6AFC93F498B88260EC6803EDE953CDDCF9113C`. Conferidos os payloads do APK, CRC integral, seeds normais e ausência da string de assistência QA. Certificado SHA256 `7508ca8dd3ad4914f42b7d6dbd537c564a78925fb8f41b76a905324220a75b3f`, o de desenvolvimento existente. [Síntese estruturada](MANUAL_BACKUP_V131.json).

Os testes de produção do codec, exportação e transação rodam por `./tests/backup/run.ps1`, com fixtures **no host**, nunca em armazenamento Android. Exercitam round trip dos bytes e estados, exportação conferida, falha de escrita parcial/readback, corrupção/truncamento, versão e cliente incompatíveis com hash recalculado, JSON inválido/duplicado, arquivo obrigatório ausente, path desconhecido, IDs inválidos, limite de leitura, journals, ausência de ledgers opcionais, retenção de marcador QA só no destino e checkpoints de preferências. Fault injection percorre todas as operações de write/rename/sync de diretório com IOException e encerramento simulado; cada reabertura resulta no estado anterior ou no novo estado inteiro. O fsync real dos **arquivos** é executado no host; sync de diretório é injetável e não simula o cache/controlador de armazenamento Android.

Compilação Java/API33/D8 e contrato JNI/lifecycle, compilação ARM64/API23, empacotamento, alinhamento e assinatura devem ser lidos junto à síntese JSON deste relatório. Warning equipment_rewards preexistente não foi corrigido nesta tarefa.

**Não validados:** instalação/abertura desta build, aparência e toques do menu, performance dos 704 valores lógicos no aparelho, picker e permissões/provedores Android reais, mensagens visíveis, sync/rename perante corte de energia Android, importação consumada e recuperação por callbacks no jogo, transferência entre dois celulares/recifragem ObscuredPrefs, persistência após gameplay. Não usar testes de host como evidência dessas etapas.

O backup cobre o estado **persistido pelo modo offline implementado** e a whitelist lógica rastreada. Não restaura uma run em memória nem funções online. Limitação preexistente: PlayerGuideManager.SyncLocalData ainda usa PlayerVariableHelper.UpdateGuideId sem hook local; o backup preserva seed_guide e dicas locais, mas não pode recuperar avanço de tutorial que o próprio bridge não gravou. Tutorial e outros bugs não foram corrigidos aqui.

## Retenção

Após a síntese versionada dos resultados, usar `tools/workspace/clean-builds.ps1 -KeepBuildId v131-manual-progress-backup`. Somente APK normal assinado e relatório de assinatura devem permanecer em build. Fixtures, DEX, binário de análise e relatórios brutos são temporários; a síntese e o histórico de IDs/hashes permanecem versionados. Não limpar acervo externo, dependências, credenciais ou saves Android.

Limpeza concluída após WhatIf: **1.993.142.607 → 376.003.227 bytes**, liberados **1.617.139.380 bytes**. Restaram somente APK v131 assinado e relatório de assinatura; hash final reconferido. IDs/hashes de v129/v130/v131 preservados no histórico pequeno. Evidências brutas citadas nesta tarefa são históricas após a limpeza, não arquivos ainda disponíveis em build.
