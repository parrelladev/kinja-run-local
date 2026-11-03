# QA Heróis forte — v127

Em03/11/2026, usuário autorizou fortalecer somente o aplicativo de testes para concluir fases e observar recompensas. `build-native.ps1 -QaHeroCombat` produz biblioteca separada `libkinjaoffline-qa-heroes.so` com define KINJA_OFFLINE_QA_HERO_COMBAT. Builds sem a opção não ativam a assistência. Também exige filesDir com sufixo exato `/com.habby.kinjarun.offline.qa.heroes/files`; outro package nunca ativa.

Usa os hooks/trampolines de dano já existentes, com mesmos prólogos/hash/APIs originais: dano positivo de saída recebe mínimo1000000; dano positivo recebido sem forceKill vira0 (inclui armadilhas). ForceKill original preservado. Não escreve atributos/offsets gerenciados ou altera nível/estrela/saldo/tabelas/drop/cálculo de recompensa. Números do menu continuam originais; força é aplicada no combate. Qualquer herói selecionado no QA recebe assistência.

v127 packageQA atualizado por install-r com mesmo keystore: firstInstallTime `2026-11-03 14:43:50` preservado; lastUpdateTime `19:08:40`. APK SHA256 `24DCDBA57FFEB9324911340C1D3930C9F95778D699A9A35E861D101B4083214A`, biblioteca `B61184BE02B6D09F5A387868F7143DA108769A748EB1A985D894824C0378C1A4`. QA usa HeroFixture somente para instalação nova, sem reposição no update; save existente carregou3392ouro/7gemas/1442EXP/Chapter13 (atividade do jogador antes da instalação, não recursos concedidos pela assistência).

Normal continua v126/audit1: APK SHA256 `D3C84387656D17BF3CB5FE2E8E6DFCB98F67D04419B7B264474CB775DD02900B`, biblioteca padrão `B351E39C02FF370A1DD5AF326E846FB0499281DB5E93EFDF59DACE5ABA2362AA` reconferidos e não substituídos. Nenhuma instalação no normal/celular. Compilação ARM64/assinatura passaram, warning preexistente equipment_rewards. `git diff --check` passou.

## Teste do usuário e recompensa

Navegação automatizada ao Chapter6 foi interrompida pelo usuário. Ele executou uma partida no Chapter13/region14 às19:09:03–19:13:25, PID12757; não descrever como nossa vitória no Chapter6. Logs confirmam dano35/12→1000000 e proteção169→0 em armadilhas. Vitória stage11/score28943:6001×366,20006×3,20003×4,20005×1 persistidos;720ouro/299gemas, quatro callbacks AddLocalItem sem erro. Pools/quantidades não alterados. [Síntese dos logs e hashes](HERO_QA_COMBAT_V127.json).

Usuário relatou recompensa ausente na aba Melhorar (nível). Após reabrir o treino, captura instrumental mostra Cavaleiro Estelar Lv1/20,4112ouro e **366/50 manuais**, botão disponível. Isso confirma presença no painel nesse momento; não explica o estado anterior relatado, nem comprova atualização imediata de todas as telas. Hipótese de UI desatualizada não confirmada; sem alteração de concessão ou crédito duplicado. Pendência original de fonte natural agora tem evidência de drop/vitória/save/painel no Chapter13, ainda sem consumo de manual ganho ou teste específico do Chapter6. Reinício será registrado abaixo.

Reinício instrumental às19:16:04, PID14144: seed_items existente carregado, OnItemInit e initialized completados. Usuário voltou a interagir no aplicativo durante a coleta; não houve nova leitura instrumental do contador366 após esse restart nem consumo pela automação. Não promover o simples carregamento a verificação quantitativa.

Artefatos intermediários/QA locais removidos após síntese pela limpeza autorizada mantendo normalv126; v127 segue instalada no qa.heroes para o usuário continuar os testes. Acervo externo, Android saves, dependências e assinatura preservados.
